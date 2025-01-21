#include <QApplication>

#include "cam.h"


CameraWidget::CameraWidget(std::string stream_name, QWidget* parent) :
                          stream_name(stream_name), QOpenGLWidget(parent) {
  setAttribute(Qt::WA_OpaquePaintEvent);

  QObject::connect(QApplication::instance(), &QCoreApplication::aboutToQuit, this, &CameraWidget::stopvThread);
}

CameraWidget::~CameraWidget() {}

void CameraWidget::vThread() {}
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
