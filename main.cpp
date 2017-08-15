#include <QApplication>
#include <ctime>
#include "gui.hpp"


int main(int argc, char **argv) {  
  QApplication app(argc, argv);
  qsrand(time(nullptr));
  Window w(&app);
  w.show();
  return app.exec();
}
