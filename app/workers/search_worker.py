from PySide6.QtCore import QThread, Signal

from app.services.isapi import ISAPIError


class SearchWorker(QThread):
    """后台搜索录像分段，避免阻塞 UI。"""

    succeeded = Signal(int, int, object)  # channel, stream, segments
    failed = Signal(str)

    def __init__(self, device, channel, stream, start, end, parent=None):
        super().__init__(parent)
        self.device = device
        self.channel = channel
        self.stream = stream
        self._start = start
        self._end = end

    def run(self):
        try:
            segs = self.device.search_records(
                self.channel, self._start, self._end, stream=self.stream
            )
            self.succeeded.emit(self.channel, self.stream, segs)
        except ISAPIError as exc:
            self.failed.emit(str(exc))
