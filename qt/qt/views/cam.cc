#include "cam.h"

CameraWidget::CameraWidget(std::string stream_name, QWidget* parent) :
                          stream_name(stream_name), QOpenGLWidget(parent) {
  setAttribute(Qt::WA_OpaquePaintEvent);
}

CameraWidget::~CameraWidget() {}
