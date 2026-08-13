from PySide6.QtWidgets import QLabel, QMainWindow, QTabWidget, QVBoxLayout, QWidget

from app.services.download_bridge import DownloadBridge
from app.ui.download_page import DownloadPage


class MainWindow(QMainWindow):
    def __init__(self, config=None, device=None, device_info=None, parent=None):
        super().__init__(parent)
        self.config = config or {}
        self.device = device
        self.device_info = device_info or {}
        self.download_bridge = DownloadBridge(
            host=self.config.get("bridge_host", "127.0.0.1"),
            port=int(self.config.get("bridge_port") or 0),
        )
        self.download_bridge.start()
        self.setWindowTitle("hikcam - 海康录像下载")
        self.resize(1280, 800)

        self.tabs = QTabWidget(self)
        self.tabs.addTab(self._placeholder("实时预览"), "实时预览")
        self.download_page = DownloadPage(
            self.config, self.device, download_bridge=self.download_bridge
        )
        self.tabs.addTab(self.download_page, "录像下载")
        self.tabs.addTab(self._placeholder("画面巡检"), "画面巡检")

        self.setCentralWidget(self.tabs)

        self._build_status_bar()

    def closeEvent(self, event):
        self.download_bridge.stop()
        super().closeEvent(event)

    @staticmethod
    def _placeholder(text):
        page = QWidget()
        layout = QVBoxLayout(page)
        layout.addWidget(QLabel(text))
        return page

    def _build_status_bar(self):
        status = self.statusBar()
        name = self.device_info.get("deviceName") or "未连接"
        model = self.device_info.get("model") or ""

        self.device_name_label = QLabel(f"设备名称：{name}    ")
        status.addWidget(self.device_name_label)

        try:
            bridge_addr = self.download_bridge.base_url
        except RuntimeError:
            bridge_addr = "未启动"
        self.bridge_label = QLabel(f"下载服务：{bridge_addr}  ")
        status.addPermanentWidget(self.bridge_label)

        self.model_label = QLabel(f"设备型号：{model}  ")
        status.addPermanentWidget(self.model_label)
