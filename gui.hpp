#ifndef GWANKEI_GUI_HPP
#define GWANKEI_GUI_HPP
#include <QMainWindow>
#include <QWebView>
#include <QObject>
#include <QList>
#include <QString>
#include <QMap>
#include <QVariantMap>
#include <QJsonObject>
#include "battle.hpp"

using namespace GwanKei;

class Window;
class View;
class Hub;
class Board;

class QMenu;
class QAction;
class QDialog;
class QWebInspector;

class Window : public QMainWindow {
  Q_OBJECT
private:
  QMap<QString, QJsonObject> sound_files;
  QMap<QString, QString> sound_themes_title;
  QString sound_theme = "";
  void load_sound();
public:  
  View* view;
  Window(QApplication* app, QWidget* parent = nullptr);
public slots:
  void try_to_play_sound(QString name);
};

class View : public QWebView {
  Q_OBJECT
private:
  Battle* battle;
  QDialog* inspector_dialog;
  QWebInspector* inspector;
  bool inspector_created = false;
  bool battle_created = false;
  void init_battle();
public:
  Hub* hub;
  View(QWidget* parent);
  void new_game(BattleType type);
public slots:
  void open_inspector();				
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
  bool ended = false;
  Game game;
  Layout layout;
  Player player;
  Player current_player;
  int last_steps = 0;
public:
  Hub(QObject* parent);
  void execute_render();
  void init(Player player, Layout layout);
  Layout get_layout() const;
  Q_INVOKABLE bool is_layout_able_to_swap(int index1, int index2) const;
  Q_INVOKABLE void layout_swap(int index1, int index2);
  Q_INVOKABLE bool is_movable(int from, int to) const;
  Q_INVOKABLE int get_current_player() const;
  Q_INVOKABLE void submit_ready();
  Q_INVOKABLE void submit_move(int from, int to);
  Q_INVOKABLE void debug(QString str) const;
  // Q_INVOKABLE void submit_update_request();
signals:
  /* -- Signals Emitted from Frontend -- */
  void ready(Layout layout);
  void move(Cell from, Cell to);
  /* -- Signals for Rendering -- */
  void render(Board* board);
  void set_clock(int wait_seconds);
  void play_sound(QString name);
public slots:
  void game_over();
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
