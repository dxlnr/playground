#include "camera_view.h"

#include <QVBoxLayout>

#include "camera.h"

CameraView::CameraView(const QString &stream_name, int stream_type, QWidget *parent)
    : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  camera_widget = new CameraWidget(stream_name.toStdString(),
                                   static_cast<VisionType>(stream_type),
                                   this);
  layout->addWidget(camera_widget);
}

CameraView::~CameraView() = default;
