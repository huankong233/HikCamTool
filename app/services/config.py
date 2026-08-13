import json
from pathlib import Path

DEFAULT_FILE = Path("config.json")
HOME_FILE = Path.home() / ".hikcam" / "config.json"


def load_config(path=None):
    p = Path(path) if path else (DEFAULT_FILE if DEFAULT_FILE.exists() else HOME_FILE)
    if not p.exists():
        return {}
    return json.loads(p.read_text(encoding="utf-8-sig"))


def save_config(data, path=None):
    p = Path(path) if path else DEFAULT_FILE
    p.write_text(json.dumps(data, indent=2, ensure_ascii=False), encoding="utf-8")
    return p
