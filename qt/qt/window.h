#pragma once

#include <QWidget>
#include <QStackedLayout>

class MainWindow : public QWidget 
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);

private:
  QStackedLayout *wlayout;
};
