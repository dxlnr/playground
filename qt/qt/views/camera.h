#pragma once

#include <deque>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QThread>

#include "msgq/cc/vision/vipc_client.h"

const int FRAME_BUFFER_SIZE = 10;

class CameraWidget : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

public:
  explicit CameraWidget(std::string stream_name, VisionType stream_type, QWidget* parent = nullptr);
  ~CameraWidget();

  void stopvThread();
signals:    
  void vipcAvailableStreamsUpdated(std::set<VisionType>);
  void vipcThreadConnected(VisionIpcClient *);
  void vipcThreadFrameReceived();

protected:
  void paintGL() override;
  void resizeGL(int w, int h) override;
  void initializeGL() override;
  void showEvent(QShowEvent *event) override;

  void vThread();

  int stream_width  = 0;
  int stream_height = 0;

  std::string stream_name;
  std::set<VisionType> available_streams;
  std::atomic<VisionType> active_vision_type;
  std::atomic<VisionType> requested_vision_type;

  QThread *vthread = nullptr; 

  std::recursive_mutex flk;
  std::deque<std::pair<uint32_t, VisionBuf*>> frames;
  uint32_t draw_frame_id = 0;
  uint32_t prev_frame_id = 0;

protected slots:
  void vipcFrameReceived();
  void vipcConnected(VisionIpcClient *vclient);
  void availableStreamsUpdated(std::set<VisionType> streams);

private:
  GLuint texture_id;
};
