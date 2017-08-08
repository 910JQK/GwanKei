#include <QApplication>
#include <QDesktopWidget>
#include <QWebSettings>
#include <QWebFrame>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
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
  test_action = new QAction(tr("&Test"), this);
  ready_action = new QAction(tr("&Ready"), this);
  connect(test_action, &QAction::triggered, view, &View::test);
  connect(ready_action, &QAction::triggered, view->hub, &Hub::submit_ready);
  debug_menu = menuBar()->addMenu(tr("&Debug"));
  debug_menu->addAction(test_action);
  debug_menu->addAction(ready_action);
}


View::View(QWidget* parent) : QWebView(parent) {
  QWebSettings* defaultSettings = QWebSettings::globalSettings();
  defaultSettings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
  defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, true);
  hub = new Hub();
  connect(page()->mainFrame(), &QWebFrame::javaScriptWindowObjectCleared,
	  this, &View::javaScriptWindowObjectCleared );
  load(QUrl::fromLocalFile(
      QApplication::applicationDirPath() + "/board.html"
  ));
}


void View::test() {
  desk = new Desk(NoExpose, false);
  desk->set_player(Orange, "Orange");
  desk->set_player(Purple, "Purple");
  desk->set_player(Green, "Green");
  desk->set_player(Blue, "Blue");
  desk->ready(Purple, Layout());
  desk->ready(Green, Layout());
  desk->ready(Blue, Layout());
  hub->init(Orange, Layout());
  connect(hub, &Hub::submit_ready, this, [this]{
      emit this->desk->ready(Orange, this->hub->get_layout());
  });
  connect(hub, &Hub::submit_move, this, [this](int from, int to){
      emit this->desk->move(Orange, Cell(from), Cell(to));
  });
  connect(desk,
      &Desk::status_changed,
      this,
      [this](Player target, Game game, Player current, int wait_sec){
	   if(target == Orange) {
	       this->hub->status_changed(game, current, wait_sec);
	   }
      }
  );
}


void View::javaScriptWindowObjectCleared() {
  page()->mainFrame()->addToJavaScriptWindowObject("Hub", hub);
}


void Hub::execute_render() {
  if(!started) {
    Board* board = new Board(layout, player);
    render(board);
  } else {
    Board* board = new Board(game, player, player != current_player);
    render(board);
  }
  // release memory in frontend
}


void Hub::init(Player player, Layout layout) {
  this->player = player;
  this->layout = layout;
  execute_render();
}


Layout Hub::get_layout() const {
  return this->layout;
}


bool Hub::is_layout_able_to_swap(int index1, int index2) const {
  return layout.is_able_to_swap(index1, index2);
}


void Hub::layout_swap(int index1, int index2) {
  layout.swap(index1, index2);
  execute_render();
}


bool Hub::is_movable(int from, int to) const {
  if(!started)
    return false;
  return game.is_movable(Cell(from), Cell(to));
}


int Hub::get_current_player() const {
  return current_player;
}


void Hub::status_changed(Game game, Player current_player, int wait_seconds) {
  if(!started) {
    started = true;
    game_started();
  }
  this->game = game;
  this->current_player = current_player;
  execute_render();
  set_clock(wait_seconds);
}


RenderElement RenderElementFromEmptyCell(Cell cell) {
  RenderElement result;
  result["cell"] = cell.get_id();
  result["piece"] = -1;
  result["player"] = -1;
  result["layout_index"] = -1;
  return result;
}


RenderElement RenderElementFromRouteNode(Cell cell) {
  RenderElement result;
  result["cell"] = cell.get_id();
  result["piece"] = 43;
  result["player"] = -1;
  result["layout_index"] = -1;
  return result;
}


RenderElement RenderElementFromUnknownPiece(Cell cell, Element element) {
  RenderElement result;
  result["cell"] = cell.get_id();
  result["piece"] = 42;
  result["player"] = element.get_player();
  result["layout_index"] = element.get_layout_index();
  return result;
}


RenderElement RenderElementFromPiece(Cell cell, Element element, Piece piece) {
  RenderElement result;
  result["cell"] = cell.get_id();
  result["piece"] = piece.get_id();
  result["player"] = element.get_player();
  result["layout_index"] = element.get_layout_index();
  return result;
}


Board::Board(Layout initial_layout, Player perspective) : QObject() {
  mode_value = Preparing;
  perspective_value = perspective;
  for(int i=0; i<25; i++) {
    elements.push_back(
	RenderElementFromPiece(
	    convert_layout_index_to_cell(i, perspective),
	    Element::Known(perspective, i),
	    initial_layout.get(i)
	)
    );
  }
}


Board::Board(const Game& game, Player perspective, bool is_watching) : QObject() {
  if(is_watching)
    mode_value = Watching;
  else
    mode_value = Playing;
  perspective_value = perspective;
  for(int i=0; i<4631; i++) {
    if(is_valid_cell_id(i)) {
      Element e = game.element_of(i);
      if(!e.is_empty()) {
	if(e.is_unknown())
	  elements.push_back(RenderElementFromUnknownPiece(Cell(i), e));
	else
	  elements.push_back(RenderElementFromPiece(
			         Cell(i), e, game.piece_of(e))
			     );
      } else {
	elements.push_back(RenderElementFromEmptyCell(Cell(i)) );
      }
    }
  }
  Feedback last_feedback = game.get_last_feedback();
  std::list<Cell> route = last_feedback.get_route();
  for(auto I=route.begin(); I!=route.end(); I++) {
    elements.push_back(RenderElementFromRouteNode(*I));
  }
}


QString Board::get_mode() const {
  if(mode_value == Preparing)
    return "preparing";
  else if(mode_value == Playing)
    return "playing";
  else
    return "watching";
}


int Board::get_length() const {
  return elements.length();
}


int Board::get_perspective() const {
  return perspective_value;
}


QVariantMap Board::at(int index) const {
  assert(0 <= index && index < elements.length());
  return elements[index];
}
