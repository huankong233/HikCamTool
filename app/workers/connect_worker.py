from PySide6.QtCore import QThread, Signal

from app.services.isapi import Device, ISAPIError


class ConnectWorker(QThread):
    """后台检查 NVR 可用性，避免阻塞 UI。"""

    succeeded = Signal(dict)
    failed = Signal(str)

    def __init__(self, host, username, password, port, scheme, parent=None):
        super().__init__(parent)
        self.host = host
        self.username = username
        self.password = password
        self.port = port
        self.scheme = scheme
        self.device = None

    def run(self):
        try:
            self.device = Device(
                self.host,
                self.username,
                self.password,
                port=self.port,
                scheme=self.scheme,
                timeout=5,
            )
            info = self.device.get_device_info()
            self.succeeded.emit(info)
        except (ISAPIError, ValueError) as exc:
            self.failed.emit(str(exc))
