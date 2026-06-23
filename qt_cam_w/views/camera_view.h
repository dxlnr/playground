#pragma once

#include <QString>
#include <QWidget>

class CameraWidget;

class CameraView : public QWidget {
public:
  explicit CameraView(const QString &stream_name, int stream_type, QWidget *parent = nullptr);
  ~CameraView() override;

private:
  CameraWidget *camera_widget = nullptr;
};
