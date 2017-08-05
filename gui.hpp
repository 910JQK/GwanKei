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
private:
  Game* game;
  bool started = false;
  Layout layout;
  Player player;
public:
  Hub();
  ~Hub();
  void update_board();
  void init(Player player, Layout layout);
  Q_INVOKABLE bool is_layout_able_to_swap(int index1, int index2);
  Q_INVOKABLE void submit_layout_swap(int index1, int index2);
  // Q_INVOKABLE void submit_move(int from, int to);
signals:
  void board_updated(Board* board);
};

#endif // GWANKEI_GUI_HPP
