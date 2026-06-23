from __future__ import annotations

from typing import Optional

import numpy as np

from OpenGL.GL import (
    GL_COLOR_BUFFER_BIT,
    GL_DEPTH_BUFFER_BIT,
    GL_LINEAR,
    GL_MODELVIEW,
    GL_PROJECTION,
    GL_QUADS,
    GL_RED,
    GL_R16,
    GL_R8,
    GL_TEXTURE_2D,
    GL_TEXTURE_MAG_FILTER,
    GL_TEXTURE_MIN_FILTER,
    GL_UNPACK_ALIGNMENT,
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_SHORT,
    glBegin,
    glBindTexture,
    glClear,
    glClearColor,
    glDeleteTextures,
    glEnd,
    glGenTextures,
    glLoadIdentity,
    glMatrixMode,
    glOrtho,
    glPixelStorei,
    glTexCoord2f,
    glTexImage2D,
    glTexParameteri,
    glTexSubImage2D,
    glVertex2f,
    glViewport,
)
from PySide6.QtCore import QObject, Qt, QThread, Signal, Slot
from PySide6.QtOpenGLWidgets import QOpenGLWidget
from PySide6.QtWidgets import QApplication

from qdmq import VisionIpcClient, VisionType


class VipcWorker(QObject):
    frame_received = Signal(object, int)
    connected = Signal(int, int)
    available_streams = Signal(object)

    def __init__(self, stream_name: str | bytes, stream_type: VisionType) -> None:
        super().__init__()
        if isinstance(stream_name, bytes):
            self.stream_name = stream_name
        else:
            self.stream_name = stream_name.encode("utf-8")
        self._requested_stream = stream_type
        self._active_stream = stream_type

    @Slot(int)
    def set_stream_type(self, stream_type: VisionType) -> None:
        self._requested_stream = stream_type

    @Slot()
    def run(self) -> None:
        client: Optional[VisionIpcClient] = None
        while not QThread.currentThread().isInterruptionRequested():
            if client is None or self._requested_stream != self._active_stream:
                self._active_stream = self._requested_stream
                client = VisionIpcClient(self.stream_name, self._active_stream, False)

            if not client.is_connected():
                streams = VisionIpcClient.available_streams(self.stream_name, False)
                if not streams:
                    QThread.msleep(100)
                    continue
                self.available_streams.emit(streams)
                if not client.connect(False):
                    QThread.msleep(100)
                    continue
                self.connected.emit(client.width or 0, client.height or 0)

            buf = client.recv(1000)
            if buf is None:
                QThread.msleep(2)
                continue
            self.frame_received.emit(buf, client.frame_id)


class CameraWidget(QOpenGLWidget):
    def __init__(self, stream_name: str, stream_type: VisionType, parent=None) -> None:
        super().__init__(parent)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.stream_name = stream_name
        self.stream_type = stream_type
        self._worker: Optional[VipcWorker] = None
        self._thread: Optional[QThread] = None
        self._texture_id: Optional[int] = None
        self._texture_size = (0, 0)
        self._texture_format = GL_R8
        self._texture_type = GL_UNSIGNED_BYTE
        self._frame = None
        self._frame_id = None
        self._available_streams = set()

        app = QApplication.instance()
        if app is not None:
            app.aboutToQuit.connect(self.stop)

    def stop(self) -> None:
        if self._thread is None:
            return
        self._thread.requestInterruption()
        self._thread.quit()
        self._thread.wait(1000)
        self._thread = None
        self._worker = None

    def initializeGL(self) -> None:
        glClearColor(0.0, 0.0, 0.0, 1.0)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1)

    def resizeGL(self, width: int, height: int) -> None:
        glViewport(0, 0, width, height)

    def paintGL(self) -> None:
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        if self._frame is None:
            return

        width = int(getattr(self._frame, "width", 0))
        height = int(getattr(self._frame, "height", 0))
        if width <= 0 or height <= 0:
            return

        data = self._frame.data
        bytes_expected = width * height * 2
        texture_type = GL_UNSIGNED_SHORT
        texture_format = GL_R16
        upload_data = data
        if data.size == width * height:
            bytes_expected = width * height
            texture_type = GL_UNSIGNED_BYTE
            texture_format = GL_R8
        elif data.size >= bytes_expected:
            try:
                data_u16 = data.view(np.uint16)[: width * height]
                upload_data = (data_u16 >> 8).astype(np.uint8, copy=False)
                bytes_expected = width * height
                texture_type = GL_UNSIGNED_BYTE
                texture_format = GL_R8
            except ValueError:
                upload_data = data

        if data.size < bytes_expected:
            return
        if upload_data.size != bytes_expected:
            upload_data = upload_data[:bytes_expected]

        self._ensure_texture(width, height, texture_format, texture_type)

        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        glOrtho(0, width, height, 0, -1, 1)
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()

        glBindTexture(GL_TEXTURE_2D, self._texture_id)
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0,
            0,
            width,
            height,
            GL_RED,
            self._texture_type,
            upload_data,
        )

        glBegin(GL_QUADS)
        glTexCoord2f(0.0, 0.0)
        glVertex2f(0, 0)
        glTexCoord2f(1.0, 0.0)
        glVertex2f(width, 0)
        glTexCoord2f(1.0, 1.0)
        glVertex2f(width, height)
        glTexCoord2f(0.0, 1.0)
        glVertex2f(0, height)
        glEnd()
        glBindTexture(GL_TEXTURE_2D, 0)

    def showEvent(self, event) -> None:
        super().showEvent(event)
        if self._thread is None:
            self._start_worker()

    def closeEvent(self, event) -> None:
        self.stop()
        super().closeEvent(event)

    def _start_worker(self) -> None:
        self._thread = QThread(self)
        self._worker = VipcWorker(self.stream_name, self.stream_type)
        self._worker.moveToThread(self._thread)
        self._thread.started.connect(self._worker.run)
        self._thread.finished.connect(self._thread.deleteLater)
        self._worker.frame_received.connect(self._on_frame_received, Qt.QueuedConnection)
        self._worker.connected.connect(self._on_connected, Qt.QueuedConnection)
        self._worker.available_streams.connect(self._on_available_streams, Qt.QueuedConnection)
        self._thread.start()

    def _ensure_texture(self, width: int, height: int, texture_format, texture_type) -> None:
        if (
            self._texture_id is not None
            and self._texture_size == (width, height)
            and self._texture_format == texture_format
            and self._texture_type == texture_type
        ):
            return
        if self._texture_id is not None:
            glDeleteTextures([self._texture_id])
            self._texture_id = None

        texture_id = glGenTextures(1)
        self._texture_id = int(texture_id)
        self._texture_size = (width, height)
        self._texture_format = texture_format
        self._texture_type = texture_type
        glBindTexture(GL_TEXTURE_2D, self._texture_id)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            texture_format,
            width,
            height,
            0,
            GL_RED,
            texture_type,
            None,
        )
        glBindTexture(GL_TEXTURE_2D, 0)

    @Slot(object, int)
    def _on_frame_received(self, frame, frame_id: int) -> None:
        self._frame = frame
        self._frame_id = frame_id
        self.update()

    @Slot(int, int)
    def _on_connected(self, width: int, height: int) -> None:
        if width > 0 and height > 0:
            self._texture_size = (width, height)

    @Slot(object)
    def _on_available_streams(self, streams) -> None:
        self._available_streams = set(streams)

    def __del__(self) -> None:
        if self._texture_id is not None:
            glDeleteTextures([self._texture_id])
