from PySide6.QtCore import Qt
from PySide6.QtWidgets import (
    QComboBox,
    QDialog,
    QFormLayout,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMessageBox,
    QPushButton,
    QVBoxLayout,
)

from app.workers.connect_worker import ConnectWorker


class LoginDialog(QDialog):
    """启动时弹出，请求服务器连接信息。已有配置则预填。"""

    def __init__(self, config, parent=None):
        super().__init__(parent)
        self.config = config
        self.device = None
        self.device_info = {}
        self._worker = None
        self._accept_on_ok = False
        self.setWindowTitle("连接海康 NVR")
        self.setMinimumWidth(400)

        layout = QVBoxLayout(self)

        hint = QLabel("请输入 NVR 连接信息")
        layout.addWidget(hint)

        form = QFormLayout()
        self.host_edit = QLineEdit(config.get("host", ""))
        self.host_edit.setPlaceholderText("如 192.168.1.64")
        self.username_edit = QLineEdit(config.get("username", ""))
        self.password_edit = QLineEdit(config.get("password", ""))
        self.password_edit.setEchoMode(QLineEdit.Normal)

        self.port_edit = QLineEdit(str(config.get("port", 80)))
        self.scheme_combo = QComboBox()
        self.scheme_combo.addItems(["http", "https"])
        scheme = config.get("scheme", "http")
        self.scheme_combo.setCurrentText(scheme if scheme in ("http", "https") else "http")

        self.bridge_host_edit = QLineEdit(config.get("bridge_host", "127.0.0.1"))
        self.bridge_host_edit.setPlaceholderText("127.0.0.1 或 0.0.0.0")
        self.bridge_port_edit = QLineEdit(str(config.get("bridge_port", 0)))
        self.bridge_port_edit.setPlaceholderText("0 = 随机端口")

        form.addRow("服务器 IP:", self.host_edit)
        form.addRow("用户名:", self.username_edit)
        form.addRow("密码:", self.password_edit)

        separator = QLabel("下面是高级设置 — 多数情况保持默认即可")
        separator.setStyleSheet("color: #888; margin-top: 6px;")
        form.addRow(separator)

        form.addRow("HTTP 端口:", self.port_edit)
        form.addRow("协议:", self.scheme_combo)
        form.addRow("下载服务地址:", self.bridge_host_edit)
        form.addRow("下载服务端口:", self.bridge_port_edit)
        layout.addLayout(form)

        self.status_label = QLabel("")
        self.status_label.setWordWrap(True)
        layout.addWidget(self.status_label)

        self.open_button = QPushButton("打开录像机")
        self.open_button.setDefault(True)
        self.close_button = QPushButton("关闭应用")
        btn_row = QHBoxLayout()
        btn_row.addWidget(self.open_button)
        btn_row.addWidget(self.close_button)
        btn_row.addStretch(1)
        layout.addLayout(btn_row)

        self.open_button.clicked.connect(self._on_open)
        self.close_button.clicked.connect(self._on_close)

        if config.get("host"):
            self.host_edit.selectAll()
            self.host_edit.setFocus()

    def data(self):
        bridge_port_text = self.bridge_port_edit.text().strip()
        return {
            "host": self.host_edit.text().strip(),
            "port": int(self.port_edit.text().strip() or "80"),
            "username": self.username_edit.text().strip(),
            "password": self.password_edit.text(),
            "scheme": self.scheme_combo.currentText(),
            "bridge_host": self.bridge_host_edit.text().strip() or "127.0.0.1",
            "bridge_port": int(bridge_port_text or "0"),
        }

    def _on_open(self):
        data = self.data()
        if not data["host"] or not data["username"]:
            QMessageBox.warning(self, "提示", "请先填写服务器 IP 和用户名")
            return
        self._accept_on_ok = True
        self._start_check()

    def _start_check(self):
        data = self.data()
        self.open_button.setEnabled(False)
        self.close_button.setEnabled(False)
        self.status_label.setText("正在后台检查录像机可用性...")
        self._worker = ConnectWorker(
            data["host"], data["username"], data["password"],
            data["port"], data["scheme"],
        )
        self._worker.succeeded.connect(self._on_check_ok)
        self._worker.failed.connect(self._on_check_fail)
        self._worker.start()

    def _on_check_ok(self, info):
        self.device = self._worker.device
        self.device_info = info
        self._worker = None
        self._set_idle()
        name = info.get("deviceName") or ""
        model = info.get("model") or ""
        self.status_label.setText(f"录像机可用：{name}  {model}")
        if self._accept_on_ok:
            self.accept()
        else:
            QMessageBox.information(self, "检查结果", f"录像机可用：{name}  {model}")

    def _on_check_fail(self, err):
        self._worker = None
        self._set_idle()
        self.status_label.setText("")
        QMessageBox.critical(self, "录像机不可用", err)

    def _set_idle(self):
        self.open_button.setEnabled(True)
        self.close_button.setEnabled(True)

    def _on_close(self):
        if self._worker is not None:
            try:
                self._worker.succeeded.disconnect()
                self._worker.failed.disconnect()
            except (RuntimeError, TypeError):
                pass
        self.reject()
