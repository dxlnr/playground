#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QThread>

const int FRAME_BUFFER_SIZE = 5;

class CameraWidget : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

public:
  explicit CameraWidget(std::string stream_name, QWidget* parent = nullptr);
  ~CameraWidget();

signals:

protected:
  std::string stream_name;
};
