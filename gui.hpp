#ifndef GWANKEI_GUI_HPP
#define GWANKEI_GUI_HPP
#include <QMainWindow>
#include <QWebView>
#include <QObject>
#include <QVariantMap>
#include "game.hpp"

using namespace GwanKei;

class Window;
class View;
class Hub;
class Board;

class QMenu;
class QAction;

class Window : public QMainWindow {
  Q_OBJECT
private:
  QMenu* debug_menu;
  QAction* test_action;
public:
  Window(QApplication* app, QWidget* parent = nullptr);
  View* view;
};

class View : public QWebView {
  Q_OBJECT
private:
  Hub* hub;
public:
  View(QWidget* parent);
  void test();
public slots:
  void javaScriptWindowObjectCleared();
};

enum BoardMode {
  Preparing, Playing, Watching
};


typedef QVariantMap RenderElement;

RenderElement RenderElementFromRouteNode(Cell cell);
RenderElement RenderElementFromUnknownPiece(Cell cell, Element element);
RenderElement RenderElementFromPiece(Cell cell, Element element, Piece piece);

class Board : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString mode READ get_mode);
  Q_PROPERTY(int length READ get_length);
  Q_PROPERTY(int perspective READ get_perspective)
private:
  BoardMode mode_value;
  Player perspective_value;
  QList<RenderElement> elements;
public:
  Board(Layout initial_layout = Layout(), Player perspective = Orange);
  Board(const Game& game, Player perspective, bool is_watching);
  QString get_mode() const;
  int get_length() const;
  int get_perspective() const;
  Q_INVOKABLE QVariantMap at(int index) const;
};

class Hub : public QObject {
  Q_OBJECT
public:
  Hub();
  ~Hub();
  Game* game;
  bool started = false;
  Layout layout;
  Player player;
signals:
  void update_board(Board* board);
  /*
public slots:  
  void submit_layout(const QList<int>& layout);
  void submit_move(int from, int to);
  */
};

#endif // GWANKEI_GUI_HPP
