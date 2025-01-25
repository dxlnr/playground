#include "window.h"

#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>

#include "views/camera.h"


MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(0);

  layout->addWidget(new CameraWidget("camerad"));

  setLayout(layout);
}
