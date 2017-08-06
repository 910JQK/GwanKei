#ifndef GWANKEI_GUI_HPP
#define GWANKEI_GUI_HPP
#include <QMainWindow>
#include <QWebView>
#include <QObject>
#include <QList>
#include <QString>
#include <QVariantMap>
#include "desk.hpp"

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
  Preparing, Waiting, Playing, Watching
};

typedef QVariantMap RenderElement;

RenderElement RenderElementFromEmptyCell(Cell cell);

RenderElement RenderElementFromRouteNode(Cell cell);

RenderElement RenderElementFromUnknownPiece(Cell cell, Element element);

RenderElement RenderElementFromPiece(Cell cell, Element element, Piece piece);

class Hub : public QObject {
  Q_OBJECT
private:
  bool started = false;
  Game game;
  Layout layout;
  Player player;
  Player current_player;
public:
  void execute_render();
  void init(Player player, Layout layout);
  Q_INVOKABLE bool is_layout_able_to_swap(int index1, int index2) const;
  Q_INVOKABLE void layout_swap(int index1, int index2);
  Q_INVOKABLE bool is_movable(int from, int to) const;
signals:
  /* -- Signals Emitted from Frontend -- */
  void submit_ready();
  void submit_move(int from, int to);
  void submit_update_request();
  /* -- Signals for Rendering -- */
  void render(Board* board);
  void set_clock(int wait_seconds);
  void game_started();
public slots:
  void status_changed(Game game, Player current_player, int wait_seconds);
};

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

#endif // GWANKEI_GUI_HPP
