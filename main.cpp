#include "ShapelyWindow.hpp"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  ShapelyWindow w;
  w.show();

  return a.exec();
}
