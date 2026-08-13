import sys

from PySide6.QtCore import Qt
from PySide6.QtGui import QGuiApplication
from PySide6.QtWidgets import QApplication

from app.services.config import load_config, save_config
from app.ui.login_dialog import LoginDialog
from app.ui.main_window import MainWindow


def main():
    QGuiApplication.setHighDpiScaleFactorRoundingPolicy(
        Qt.HighDpiScaleFactorRoundingPolicy.PassThrough
    )
    app = QApplication(sys.argv)
    app.setApplicationName("hikcam")
    app.setOrganizationName("hikcam")

    config = load_config()
    dialog = LoginDialog(config)
    if dialog.exec() != LoginDialog.Accepted:
        return 0

    data = dialog.data()
    config.update(data)
    save_config(config)

    window = MainWindow(config, device=dialog.device, device_info=dialog.device_info)
    window.show()
    return app.exec()


if __name__ == "__main__":
    sys.exit(main())
