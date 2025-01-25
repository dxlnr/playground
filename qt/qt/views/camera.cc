#include <QApplication>

#include "camera.h"


CameraWidget::CameraWidget(std::string stream_name, VisionType type, QWidget* parent) :
                           stream_name(stream_name), active_vision_type(type), requested_vision_type(type), QOpenGLWidget(parent) 
{
  setAttribute(Qt::WA_OpaquePaintEvent);

  QObject::connect(QApplication::instance(), &QCoreApplication::aboutToQuit, this, &CameraWidget::stopvThread);
  QObject::connect(this, &CameraWidget::vipcThreadConnected, this, &CameraWidget::vipcConnected, Qt::BlockingQueuedConnection);
  QObject::connect(this, &CameraWidget::vipcAvailableStreamsUpdated, this, &CameraWidget::availableStreamsUpdated, Qt::QueuedConnection);
  QObject::connect(this, &CameraWidget::vipcThreadFrameReceived, this, &CameraWidget::vipcFrameReceived, Qt::QueuedConnection);
}

CameraWidget::~CameraWidget() { glDeleteTextures(1, &texture_id); }

void CameraWidget::vThread() 
{
  std::unique_ptr<VisionIpcClient> vclient;
  VisionType cur_stream = requested_vision_type;
  VisionIpcBufExtra meta_main = {0};

  while (!QThread::currentThread()->isInterruptionRequested()) {
    if (!vclient) {
      qDebug().nospace() << "connecting to stream " << requested_vision_type << ", was connected to " << cur_stream;
      cur_stream = requested_vision_type;
      vclient.reset(new VisionIpcClient(stream_name, cur_stream, false));
    }
    active_vision_type = cur_stream;

    if (!vclient->connected) {
      auto streams = VisionIpcClient::getAvailableStreams(stream_name, false);

      if (streams.empty()) {
        QThread::msleep(100);
        continue;
      }
      emit vipcAvailableStreamsUpdated(streams);

      if (!vclient->connect(false)) {
        QThread::msleep(100);
        continue;
      }
      emit vipcThreadConnected(vclient.get());
    }
    if (VisionBuf *buf = vclient->recv(&meta_main, 1000)) {
      printf("buf recv : %d", meta_main.frame_id);
      {
        std::lock_guard lk(flk);
        frames.push_back(std::make_pair(meta_main.frame_id, buf));
        while (frames.size() > FRAME_BUFFER_SIZE) { frames.pop_front(); }
      }
      emit vipcThreadFrameReceived();
    } else { 
      if (!isVisible()) { vclient->connected = false; }
    }
  }
}

void CameraWidget::stopvThread() {}

void CameraWidget::resizeGL(int w, int h) { glViewport(0, 0, w, h); }

void CameraWidget::paintGL() 
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  std::lock_guard lk(flk);
  if (frames.empty()) return;

  int frame_idx = frames.size() - 1;
  if (frames[frame_idx].first == prev_frame_id) {
    qDebug() << "Drawing same frame twice" << frames[frame_idx].first;
  } else if (frames[frame_idx].first != prev_frame_id + 1) {
    qDebug() << "Skipped frame" << frames[frame_idx].first;
  }
  prev_frame_id = frames[frame_idx].first;
  VisionBuf *frame = frames[frame_idx].second;
  assert(frame != nullptr);

  printf("frmeee 10  : %d\n", frame->data[10]);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, width(), height(), 0, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Update the texture with the new frame
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->width, frame->height, GL_RED, GL_UNSIGNED_BYTE, frame->data);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Draw the updated texture
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
  glTexCoord2f(1.0f, 0.0f); glVertex2f(frame->width, 0);
  glTexCoord2f(1.0f, 1.0f); glVertex2f(frame->width, frame->height);
  glTexCoord2f(0.0f, 1.0f); glVertex2f(0, frame->height);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
}

void CameraWidget::initializeGL() 
{
  initializeOpenGLFunctions();
  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 480, 480, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void CameraWidget::showEvent(QShowEvent *event) 
{
  if (!vthread) 
  {
    vthread = new QThread();
    connect(vthread, &QThread::started, [=]() { vThread(); });
    connect(vthread, &QThread::finished, vthread, &QObject::deleteLater);
    vthread->start();
  }
}
void CameraWidget::availableStreamsUpdated(std::set<VisionType> streams) 
{
  available_streams = streams;
}
void CameraWidget::vipcConnected(VisionIpcClient *vclient) 
{
  stream_width = vclient->buffers[0].width;
  stream_height = vclient->buffers[0].height;
}
void CameraWidget::vipcFrameReceived() { update(); }
