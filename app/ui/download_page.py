from datetime import date, datetime, time as dtime

from PySide6.QtCore import Qt, QRect, Signal
from PySide6.QtGui import QGuiApplication
from PySide6.QtWidgets import (
    QAbstractItemView,
    QCheckBox,
    QComboBox,
    QDateEdit,
    QGroupBox,
    QHBoxLayout,
    QHeaderView,
    QLabel,
    QMessageBox,
    QPushButton,
    QTableWidget,
    QTableWidgetItem,
    QTimeEdit,
    QVBoxLayout,
    QWidget,
)

from app.services.isapi import ISAPIError
from app.workers.search_worker import SearchWorker


def fmt_storage(s):
    """'2026-08-08T00:23:40Z' -> '20260808002340'"""
    return s.replace("-", "").replace("T", "").replace(":", "").replace("Z", "")


def _stream_label(stream):
    return "main" if stream == 1 else "sub"


def _friendly_name(channel, stream, start, end):
    return (
        f"ch{int(channel):02d}_{_stream_label(stream)}_"
        f"{fmt_storage(start)}_{fmt_storage(end)}.mp4"
    )


class HeaderCheck(QCheckBox):
    """表头全选复选框：点击总是 全选<=>全不选，不受三态循环影响。"""

    def nextCheckState(self):  # noqa: N802
        # 禁用 Qt 默认点击切换，改由 mouseReleaseEvent 自行处理
        pass

    def mouseReleaseEvent(self, event):  # noqa: N802
        if event.button() == Qt.LeftButton and self.rect().contains(
                event.position().toPoint()):
            self.setChecked(self.checkState() != Qt.Checked)
            event.accept()
            return
        super().mouseReleaseEvent(event)

class CheckHeader(QHeaderView):
    """表头第 0 列放一个真正的 QCheckBox，原生样式/悬停/重绘由 Qt 负责。"""

    checked_changed = Signal(bool)

    def __init__(self, parent=None):
        super().__init__(Qt.Horizontal, parent)
        self._checkbox = HeaderCheck(self)
        self._checkbox.setFocusPolicy(Qt.NoFocus)
        self._checkbox.setToolTip("全选/全不选")
        self._checkbox.stateChanged.connect(self._on_state_changed)
        self.setSectionsClickable(False)
        self.sectionResized.connect(lambda *_: self._reposition())

    def _col0_rect(self):
        pos = self.sectionViewportPosition(0)
        width = self.sectionSize(0)
        return QRect(pos, 0, width, self.height())

    def _reposition(self):
        if self.count() > 0:
            self._checkbox.setGeometry(self._col0_rect())

    def _on_state_changed(self, state):
        self.checked_changed.emit(int(state) == Qt.CheckState.Checked.value)

    def set_checked(self, state):
        prev_sig = self._checkbox.blockSignals(True)
        if state == Qt.PartiallyChecked:
            self._checkbox.setTristate(True)
            self._checkbox.setCheckState(Qt.PartiallyChecked)
        else:
            self._checkbox.setTristate(False)
            self._checkbox.setChecked(state == Qt.Checked)
        self._checkbox.blockSignals(prev_sig)

    def showEvent(self, event):
        super().showEvent(event)
        self._reposition()

    def resizeEvent(self, event):
        super().resizeEvent(event)
        self._reposition()

    def setOffset(self, offset):
        super().setOffset(offset)
        self._reposition()

    def setOffsetToSectionPosition(self, index):
        super().setOffsetToSectionPosition(index)
        self._reposition()

    def setOffsetToLastSection(self):
        super().setOffsetToLastSection()
        self._reposition()


class DownloadPage(QWidget):
    """录像搜索：选通道/时间 -> 搜索 -> 勾选结果 -> 复制下载链接。"""

    def __init__(
            self, config=None, device=None, download_bridge=None, parent=None):
        super().__init__(parent)
        self.config = config or {}
        self.device = device
        self.download_bridge = download_bridge
        self.segments = []
        self.search_worker = None

        self._build_ui()
        self._load_channels()

    # ---------- UI ----------
    def _build_ui(self):
        root = QVBoxLayout(self)

        search_box = QGroupBox("搜索录像")
        srow = QHBoxLayout(search_box)
        srow.addWidget(QLabel("通道:"))
        self.channel_combo = QComboBox()
        srow.addWidget(self.channel_combo)
        srow.addWidget(QLabel("码流:"))
        self.stream_combo = QComboBox()
        self.stream_combo.addItem("主码流", 1)
        self.stream_combo.addItem("子码流", 2)
        srow.addWidget(self.stream_combo)

        today = date.today()
        self.start_date = QDateEdit(today, self)
        self.start_date.setCalendarPopup(True)
        self.end_date = QDateEdit(today, self)
        self.end_date.setCalendarPopup(True)
        self.start_time = QTimeEdit(dtime(0, 0, 0), self)
        self.end_time = QTimeEdit(dtime(23, 59, 59), self)
        self.start_time.setDisplayFormat("HH:mm:ss")
        self.end_time.setDisplayFormat("HH:mm:ss")

        srow.addWidget(QLabel("开始日期:"))
        srow.addWidget(self.start_date)
        srow.addWidget(self.start_time)
        srow.addWidget(QLabel("结束日期:"))
        srow.addWidget(self.end_date)
        srow.addWidget(self.end_time)

        self.search_button = QPushButton("搜索")
        self.search_button.clicked.connect(self._on_search)
        srow.addWidget(self.search_button)
        srow.addStretch(1)
        root.addWidget(search_box)

        res_box = QGroupBox("搜索结果")
        rcol = QVBoxLayout(res_box)
        self.result_table = QTableWidget(0, 6)
        self.result_table.setHorizontalHeaderLabels(
            [" ", "文件名", "开始时间", "结束时间", "大小", "编码"])
        header = CheckHeader()
        header.setStyleSheet("""QHeaderView::section:first { padding: 0; }""")
        self.result_table.setHorizontalHeader(header)
        header.setSectionResizeMode(1, QHeaderView.Stretch)
        for c in (2, 3, 4, 5):
            header.setSectionResizeMode(c, QHeaderView.ResizeToContents)
        header.setSectionResizeMode(0, QHeaderView.Fixed)
        header.resizeSection(0, 32)
        header.checked_changed.connect(self._on_select_all)
        self.result_table.verticalHeader().setVisible(False)
        self.result_table.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.result_table.itemChanged.connect(self._on_item_changed)
        self._syncing = False
        rcol.addWidget(self.result_table)
        arow = QHBoxLayout()
        arow.addStretch(1)
        self.copy_link_button = QPushButton("复制下载链接")
        self.copy_link_button.clicked.connect(self._on_copy_links)
        arow.addWidget(self.copy_link_button)
        rcol.addLayout(arow)
        root.addWidget(res_box, 2)

        self._set_busy(False)

    def _set_busy(self, busy):
        self.search_button.setEnabled(not busy)

    # ---------- 通道 ----------
    def _load_channels(self):
        if self.device is None:
            self.channel_combo.addItem("未连接", None)
            return
        try:
            channels = self.device.list_channels()
        except ISAPIError:
            channels = []
        if not channels:
            self.channel_combo.addItem("无通道", None)
            return
        for c in channels:
            self.channel_combo.addItem(f"{c['id']} - {c['name'] or '(未命名)'}", c["id"])

    # ---------- 搜索 ----------
    def _on_search(self):
        if self.device is None:
            QMessageBox.warning(self, "提示", "未连接到录像机")
            return
        channel = self.channel_combo.currentData()
        if channel is None:
            QMessageBox.warning(self, "提示", "请选择通道")
            return
        if self.search_worker is not None:
            return
        stream = self.stream_combo.currentData()
        start = datetime.combine(self.start_date.date().toPython(),
                                 self.start_time.time().toPython())
        end = datetime.combine(self.end_date.date().toPython(),
                               self.end_time.time().toPython())
        if start >= end:
            QMessageBox.warning(self, "提示", "开始时间必须早于结束时间")
            return
        start_s = start.strftime("%Y-%m-%dT%H:%M:%S") + "Z"
        end_s = end.strftime("%Y-%m-%dT%H:%M:%S") + "Z"
        self.search_button.setText("搜索中...")
        self._set_busy(True)
        self.search_worker = SearchWorker(self.device, channel, stream, start_s, end_s)
        self.search_worker.succeeded.connect(self._on_search_done)
        self.search_worker.failed.connect(self._on_search_failed)
        self.search_worker.start()

    def _on_search_done(self, channel, stream, segs):
        self.search_worker = None
        self._set_busy(False)
        self.search_button.setText("搜索")
        self.segments = segs
        self.result_table.setRowCount(0)
        for seg in segs:
            row = self.result_table.rowCount()
            self.result_table.insertRow(row)
            check = QTableWidgetItem()
            check.setFlags(Qt.ItemIsUserCheckable | Qt.ItemIsEnabled)
            check.setCheckState(Qt.Unchecked)
            self.result_table.setItem(row, 0, check)
            self.result_table.setItem(row, 1, QTableWidgetItem(
                _friendly_name(channel, stream, seg["start"], seg["end"])))
            self.result_table.setItem(row, 2, QTableWidgetItem(seg["start"][:19]))
            self.result_table.setItem(row, 3, QTableWidgetItem(seg["end"][:19]))
            self.result_table.setItem(row, 4, QTableWidgetItem(
                f"{seg['size'] / 1024 / 1024:.0f} MB"))
            self.result_table.setItem(row, 5, QTableWidgetItem(seg.get("codecType") or ""))
        self._syncing = True
        self.result_table.horizontalHeader().set_checked(False)
        self._syncing = False
        QMessageBox.information(self, "搜索结果", f"找到 {len(segs)} 个录像分段")

    def _on_search_failed(self, err):
        self.search_worker = None
        self._set_busy(False)
        self.search_button.setText("搜索")
        QMessageBox.critical(self, "搜索失败", err)

    def _checked_rows(self):
        return [
            r for r in range(self.result_table.rowCount())
            if self.result_table.item(r, 0).checkState() == Qt.Checked
        ]

    def _on_select_all(self, checked):
        self._syncing = True
        try:
            for r in range(self.result_table.rowCount()):
                self.result_table.item(r, 0).setCheckState(
                    Qt.Checked if checked else Qt.Unchecked)
            self.result_table.horizontalHeader().set_checked(
                Qt.Checked if checked else Qt.Unchecked)
        finally:
            self._syncing = False

    def _on_item_changed(self, item):
        if item.column() != 0 or self._syncing:
            return
        rows = self.result_table.rowCount()
        if rows == 0:
            return
        first = self.result_table.item(0, 0)
        if first is None:
            return
        state = first.checkState()
        all_same = all(
            self.result_table.item(r, 0) is not None
            and self.result_table.item(r, 0).checkState() == state
            for r in range(rows)
        )
        self._syncing = True
        self.result_table.horizontalHeader().set_checked(
            state if all_same else Qt.PartiallyChecked)
        self._syncing = False

    # ---------- 下载链接 ----------
    def _on_copy_links(self):
        rows = self._checked_rows()
        if not rows:
            QMessageBox.information(self, "提示", "请先勾选要复制链接的录像")
            return
        if self.device is None or self.download_bridge is None:
            QMessageBox.warning(self, "提示", "下载链接服务未启动")
            return

        channel = self.channel_combo.currentData()
        stream = self.stream_combo.currentData()
        links = []
        for row in rows:
            seg = self.segments[row]
            filename = _friendly_name(
                channel, stream, seg["start"], seg["end"]
            )
            links.append(self.download_bridge.create_url({
                "host": self.device.host,
                "port": self.device.port,
                "scheme": self.device.scheme,
                "username": self.device.username,
                "password": self.device.password,
                "playbackURI": seg["playbackURI"],
            }, filename))
        QGuiApplication.clipboard().setText("\n".join(links))
        QMessageBox.information(self, "提示", f"已复制 {len(links)} 个下载链接")
