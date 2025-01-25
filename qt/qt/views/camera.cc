#include <QApplication>

#include "camera.h"


CameraWidget::CameraWidget(std::string stream_name, QWidget* parent) :
                          stream_name(stream_name), QOpenGLWidget(parent) {
  setAttribute(Qt::WA_OpaquePaintEvent);

  QObject::connect(QApplication::instance(), &QCoreApplication::aboutToQuit, this, &CameraWidget::stopvThread);
}

CameraWidget::~CameraWidget() {}

void CameraWidget::vThread() {
  std::unique_ptr<VisionIpcClient> vclient;

  while (!QThread::currentThread()->isInterruptionRequested()) {
    if (!vclient->connected) {
      auto streams = VisionIpcClient::getAvailableStreams(stream_name, false);
      if (streams.empty()) {
        printf("empty stream");
        QThread::msleep(100);
        continue;
      }
    }
  }
}

void CameraWidget::stopvThread() {}

void CameraWidget::showEvent(QShowEvent *event) {
  if (!vthread) 
  {
    vthread = new QThread();
    connect(vthread, &QThread::started, [=]() { vThread(); });
    connect(vthread, &QThread::finished, vthread, &QObject::deleteLater);
    vthread->start();
  }
}
