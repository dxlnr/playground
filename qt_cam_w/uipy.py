from PySide6.QtWidgets import QApplication

from qdmq import VisionType
from views.camera_view import CameraView

a = QApplication([])
view = CameraView("lived", VisionType.VISION_SENSOR_MONO16)
view.resize(800, 600)
view.show()
a.exec()
