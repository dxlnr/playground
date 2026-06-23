#include <QApplication>

#include "window.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow w;
  w.resize(8000, 6000);
  w.show();
  return a.exec();
}
