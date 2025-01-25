#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QThread>

#include "msgq/cc/vision/vipc_client.h"

const int FRAME_BUFFER_SIZE = 5;

class CameraWidget : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

public:
  explicit CameraWidget(std::string stream_name, QWidget* parent = nullptr);
  ~CameraWidget();

  void stopvThread();
signals:

protected:
  void showEvent(QShowEvent *event) override;
  void vThread();

  std::string stream_name;

  QThread *vthread = nullptr;
};
