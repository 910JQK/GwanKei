#ifndef GWANKEI_GUI_HPP
#define GWANKEI_GUI_HPP
#include <QMainWindow>
#include <QWebEngineView>

class Window;
class View;

class Window : public QMainWindow {
  Q_OBJECT
public:
  Window(QApplication* app, QWidget* parent = nullptr);
  View* view;
};

class View : public QWebEngineView {
  Q_OBJECT
public:
  View(QWidget* parent);  
};

#endif // GWANKEI_GUI_HPP
