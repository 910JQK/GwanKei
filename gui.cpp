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
  connect(test_action, &QAction::triggered, view, &View::test);
  debug_menu = menuBar()->addMenu(tr("&Debug"));
  debug_menu->addAction(test_action);
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
  Board* test_board = new Board(Layout(), Blue);
  hub->update_board(test_board);
  // release memory in front end
}


void View::javaScriptWindowObjectCleared() {
  page()->mainFrame()->addToJavaScriptWindowObject("Hub", hub);
}


RenderElement RenderElementFromRouteNode(Cell cell) {
  RenderElement result;
  result["cell"] = cell.get_id();
  result["piece"] = 43;
  result["player"] = 0;
  result["layout_index"] = 0;
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
	    convert_layout_index_to_cell(
		i, convert_player_to_orient(perspective)
	    ),
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
      }
    }
  }
  Feedback last_feedback = game.get_last_feedback();
  if(!last_feedback.is_nothing()) {
    std::list<Cell> route = last_feedback.get_route();
    for(auto I=route.begin(); I!=route.end(); I++)
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


Hub::Hub() : QObject() {

}


Hub::~Hub() {
  if(started)
    delete game;
}
