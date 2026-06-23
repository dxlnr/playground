from PySide6.QtWidgets import QVBoxLayout, QWidget

from .camera import CameraWidget


class CameraView(QWidget):
    def __init__(self, stream_name: str, stream_type: int, parent=None) -> None:
        super().__init__(parent)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)
        self.camera_widget = CameraWidget(stream_name, stream_type, self)
        layout.addWidget(self.camera_widget)
