import os
import uuid
import urllib.parse
from xml.etree import ElementTree as ET
from xml.sax.saxutils import escape

import requests
import xmltodict
from requests.auth import HTTPDigestAuth
from tqdm import tqdm


class ISAPIError(Exception):
    pass


def _dict_of(xml_text):
    return xmltodict.parse(xml_text)


def _descendant(data, name):
    return _find(data, name)


def _find(node, name):
    if isinstance(node, dict):
        for key, value in node.items():
            if key.split("}")[-1] == name:
                return _text(value)
            found = _find(value, name)
            if found is not None:
                return found
    elif isinstance(node, list):
        for item in node:
            found = _find(item, name)
            if found is not None:
                return found
    return None


def _text(value):
    if isinstance(value, dict):
        if "#text" in value:
            return value["#text"]
        for v in value.values():
            if isinstance(v, (str, int, float)):
                return str(v)
        return ""
    return str(value).strip() if value is not None else ""


def _iter_tag(node, name):
    if isinstance(node, dict):
        for key, value in node.items():
            if key.split("}")[-1] == name:
                if isinstance(value, list):
                    for item in value:
                        yield item
                else:
                    yield value
            else:
                yield from _iter_tag(value, name)
    elif isinstance(node, list):
        for item in node:
            yield from _iter_tag(item, name)


def _collect_channels(data):
    out = []

    def walk(node):
        if isinstance(node, dict):
            for key, value in node.items():
                tag = key.split("}")[-1]
                if tag in ("InputProxyChannel", "VideoInputChannel"):
                    items = value if isinstance(value, list) else [value]
                    for item in items:
                        ch_id = _find(item, "id")
                        if ch_id:
                            out.append(
                                {"id": int(ch_id), "name": _find(item, "name") or ""}
                            )
                else:
                    walk(value)
        elif isinstance(node, list):
            for item in node:
                walk(item)

    walk(data)
    return out


class Device:
    def __init__(self, host, username, password, port=80, scheme="http", timeout=30):
        self.host = host
        self.port = int(port)
        self.scheme = scheme
        self.username = username
        self.password = password
        self.timeout = timeout
        self.base = f"{scheme}://{host}:{port}"
        self.session = requests.Session()
        self.session.auth = HTTPDigestAuth(username, password)
        if scheme == "https":
            self.session.verify = False
            requests.packages.urllib3.disable_warnings()

    def request(self, method, path, body=None, stream=False, timeout=None):
        timeout = self.timeout if timeout is None else timeout
        headers = {}
        data = None
        if body is not None:
            headers["Content-Type"] = "application/xml"
            data = body.encode("utf-8")
        url = path if path.startswith("http") else self.base + path
        try:
            resp = self.session.request(
                method, url, data=data, headers=headers, stream=stream, timeout=timeout
            )
        except requests.RequestException as exc:
            raise ISAPIError(f"request failed: {exc}") from exc
        if resp.status_code >= 400:
            raise ISAPIError(f"HTTP {resp.status_code} for {method} {path}")
        return resp

    def get(self, path, stream=False):
        return self.request("GET", path, stream=stream)

    def post(self, path, body, stream=False, timeout=30):
        return self.request("POST", path, body=body, stream=stream, timeout=timeout)

    def get_device_info(self):
        resp = self.get("/ISAPI/System/deviceInfo")
        data = _dict_of(resp.text)
        return {
            k: _descendant(data, k)
            for k in ("deviceName", "model", "serialNumber", "firmwareVersion")
        }

    def get_time(self):
        try:
            resp = self.get("/ISAPI/System/time/localTime")
            return resp.text.strip()
        except ISAPIError:
            return None

    def list_channels(self):
        candidates = (
            "/ISAPI/ContentMgmt/InputProxy/channels",
            "/ISAPI/System/Video/inputs/channels",
            "/ISAPI/System/Video/inputs",
        )
        for path in candidates:
            try:
                resp = self.get(path)
            except ISAPIError:
                continue
            try:
                data = _dict_of(resp.text)
            except Exception:
                continue
            channels = _collect_channels(data)
            if channels:
                return channels
        return []

    def search_records(self, channel, start, end, stream=1, max_results=40):
        track = 100 * channel + int(stream)
        matches = []
        position = 0
        while True:
            body = self._search_body(track, start, end, max_results, position)
            try:
                resp = self.post("/ISAPI/ContentMgmt/search", body)
                batch, status = self._parse_search(resp.text)
            except (ISAPIError, ET.ParseError) as exc:
                raise ISAPIError(f"search failed: {exc}") from exc
            matches.extend(batch)
            if status.upper() != "MORE":
                break
            position += max_results
        return matches

    def _search_body(self, track, start, end, max_results, position):
        return (
            "<?xml version='1.0' encoding='utf-8'?>\n"
            "<CMSearchDescription>\n"
            f"  <searchID>{uuid.uuid4()}</searchID>\n"
            f"  <trackList><trackID>{track}</trackID></trackList>\n"
            "  <timeSpanList><timeSpan>\n"
            f"    <startTime>{start}</startTime>\n"
            f"    <endTime>{end}</endTime>\n"
            "  </timeSpan></timeSpanList>\n"
            f"  <maxResults>{max_results}</maxResults>"
            f"<searchResultPostion>{position}</searchResultPostion>\n"
            "  <metadataList><metadataDescriptor>//recordType.meta.std-cgi.com"
            "</metadataDescriptor></metadataList>\n"
            "</CMSearchDescription>\n"
        )

    def _parse_search(self, xml):
        data = _dict_of(xml)
        status = _descendant(data, "responseStatusStrg") or ""
        matches = []
        for node in _iter_tag(data, "searchMatchItem"):
            uri = _descendant(node, "playbackURI")
            if not uri:
                continue
            seg = {
                "sourceID": _descendant(node, "sourceID"),
                "trackID": _descendant(node, "trackID"),
                "start": _descendant(node, "startTime"),
                "end": _descendant(node, "endTime"),
                "codecType": _descendant(node, "codecType"),
                "playbackURI": uri,
                "name": "",
                "size": 0,
            }
            params = urllib.parse.parse_qs(urllib.parse.urlsplit(uri).query)
            if "name" in params:
                seg["name"] = params["name"][0]
            if "size" in params:
                try:
                    seg["size"] = int(params["size"][0])
                except ValueError:
                    pass
            matches.append(seg)
        return matches, status

    def download_segment(self, uri, dest, progress=True, progress_cb=None):
        body = (
            "<?xml version='1.0'?><downloadRequest>"
            f"<playbackURI>{escape(uri)}</playbackURI></downloadRequest>"
        )
        resp = self.post("/ISAPI/ContentMgmt/download", body, stream=True, timeout=120)
        total = int(resp.headers.get("Content-Length") or 0)
        written = 0
        bar = None
        if progress and progress_cb is None:
            bar = tqdm(
                total=total or None,
                unit="B",
                unit_scale=True,
                unit_divisor=1024,
                desc=os.path.basename(str(dest)),
            )
        with open(dest, "wb") as fh:
            for chunk in resp.iter_content(512 * 1024):
                if chunk:
                    fh.write(chunk)
                    written += len(chunk)
                    if progress_cb:
                        progress_cb(written, total)
                    elif bar:
                        bar.update(len(chunk))
        if bar:
            bar.close()
        return written
