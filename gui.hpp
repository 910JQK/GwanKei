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

class Window : public QMainWindow {
  Q_OBJECT
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
  Q_PROPERTY(QString mode_str READ get_mode);
  Q_PROPERTY(QList<RenderElement> elements_arr READ get_elements);
private:
  BoardMode mode;
  QList<RenderElement> elements;
public:
  Board(Layout initial_layout = Layout());
  Board(const Game& game, bool is_watching);
  QString get_mode() const;
  QList<RenderElement> get_elements() const;
};

class Hub : public QObject {
  Q_OBJECT
public:
  Hub();
signals:
  void update_board(const Board& board);
  /*
public slots:  
  void submit_layout(const QList<int>& layout);
  void submit_move(int from, int to);
  */
};

#endif // GWANKEI_GUI_HPP
