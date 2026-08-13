import base64
import hashlib
import hmac
import json
import secrets
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import quote, unquote, urlsplit
from xml.sax.saxutils import escape

from app.services.isapi import Device, ISAPIError


def _b64_encode(data):
    return base64.urlsafe_b64encode(data).rstrip(b"=").decode("ascii")


def _b64_decode(value):
    padding = "=" * (-len(value) % 4)
    return base64.urlsafe_b64decode(value + padding)


class DownloadBridge:
    """本地 HTTP 网关：
    - /download/<token>：把 ISAPI playbackURI 翻译成签名下载链接
    """

    def __init__(self, host="127.0.0.1", port=0):
        self.host = host
        self.port = port
        self._secret = secrets.token_bytes(32)
        self._server = None
        self._thread = None

    @property
    def base_url(self):
        if self._server is None:
            raise RuntimeError("download bridge is not running")
        host, port = self._server.server_address
        return f"http://{host}:{port}"

    def start(self):
        if self._server is not None:
            return
        bridge = self

        class Handler(BaseHTTPRequestHandler):
            def do_GET(self):  # noqa: N802
                bridge._handle(self)

            def log_message(self, format, *args):
                return

        self._server = ThreadingHTTPServer((self.host, self.port), Handler)
        self._server.daemon_threads = True
        self._thread = threading.Thread(
            target=self._server.serve_forever,
            name="hikcam-download-bridge",
            daemon=True,
        )
        self._thread.start()

    def stop(self):
        if self._server is None:
            return
        self._server.shutdown()
        self._server.server_close()
        self._server = None
        if self._thread is not None:
            self._thread.join(timeout=2)
            self._thread = None

    def create_url(self, params, filename):
        payload = json.dumps(
            params, ensure_ascii=False, separators=(",", ":")
        ).encode("utf-8")
        token = _b64_encode(payload)
        signature = _b64_encode(
            hmac.new(self._secret, token.encode("ascii"), hashlib.sha256).digest()
        )
        return (
            f"{self.base_url}/download/{token}?sig={signature}"
            f"&filename={quote(filename, safe='')}"
        )

    def decode_url(self, url):
        parsed = urlsplit(url)
        prefix = "/download/"
        if not parsed.path.startswith(prefix):
            raise ValueError("invalid download path")
        token = parsed.path[len(prefix):]
        return self._decode_token(token, parsed.query)

    def _decode_token(self, token, query):
        query_values = {}
        for item in query.split("&"):
            if "=" in item:
                key, value = item.split("=", 1)
                query_values[key] = unquote(value)
        signature = query_values.get("sig", "")
        expected = _b64_encode(
            hmac.new(self._secret, token.encode("ascii"), hashlib.sha256).digest()
        )
        if not hmac.compare_digest(signature, expected):
            raise ValueError("invalid signature")
        params = json.loads(_b64_decode(token).decode("utf-8"))
        required = {
            "host", "port", "scheme", "username", "password", "playbackURI"
        }
        if not required.issubset(params):
            raise ValueError("missing download parameters")
        return params, query_values.get("filename") or "recording.mp4"

    def _handle(self, handler):
        parsed = urlsplit(handler.path)
        if parsed.path.startswith("/download/"):
            self._handle_download(handler, parsed)
            return
        handler.send_error(404)

    def _handle_download(self, handler, parsed):
        prefix = "/download/"
        token = parsed.path[len(prefix):]
        try:
            params, filename = self._decode_token(token, parsed.query)
            device = Device(
                params["host"],
                params["username"],
                params["password"],
                port=params["port"],
                scheme=params["scheme"],
                timeout=30,
            )
            body = (
                "<?xml version='1.0'?><downloadRequest>"
                f"<playbackURI>{escape(params['playbackURI'])}</playbackURI>"
                "</downloadRequest>"
            )
            response = device.post(
                "/ISAPI/ContentMgmt/download", body, stream=True, timeout=120
            )
        except (ValueError, KeyError, TypeError, json.JSONDecodeError) as exc:
            handler.send_error(400, str(exc))
            return
        except ISAPIError as exc:
            handler.send_error(502, str(exc))
            return

        handler.send_response(200)
        content_type = response.headers.get("Content-Type") or "application/octet-stream"
        handler.send_header("Content-Type", content_type)
        content_length = response.headers.get("Content-Length")
        if content_length:
            handler.send_header("Content-Length", content_length)
        safe_filename = filename.replace('"', "")
        ascii_filename = safe_filename.encode("ascii", "ignore").decode("ascii")
        if not ascii_filename:
            ascii_filename = "recording.mp4"
        handler.send_header(
            "Content-Disposition",
            f'attachment; filename="{ascii_filename}"; '
            f"filename*=UTF-8''{quote(safe_filename, safe='')}",
        )
        handler.end_headers()
        try:
            for chunk in response.iter_content(512 * 1024):
                if chunk:
                    handler.wfile.write(chunk)
        except (BrokenPipeError, ConnectionResetError):
            pass
        finally:
            response.close()
