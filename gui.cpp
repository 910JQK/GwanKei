#include <QApplication>
#include <QDesktopWidget>
#include <QWebSettings>
#include <QDir>
#include <QUrl>
#include "gui.hpp"


Window::Window(QApplication* app, QWidget* parent) : QMainWindow(parent) {
  view = new View(this);
  setCentralWidget(view);
  setWindowTitle(tr("GwanKei"));
  resize(800, 600);
  QDesktopWidget *desktop = QApplication::desktop();
  move(
   (desktop->width() - this->width())/2,
   (desktop->height() - this->height())/2
  );
}


View::View(QWidget* parent) : QWebView(parent) {
  QWebSettings* defaultSettings = QWebSettings::globalSettings();
  defaultSettings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
  defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, true); 
  load(QUrl::fromLocalFile(
      QApplication::applicationDirPath() + "/board.html"
  ));
}
