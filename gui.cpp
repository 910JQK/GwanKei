#include <QApplication>
#include <QDesktopWidget>
#include <QWebSettings>
#include <QWebFrame>
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
  hub = new Hub();
  connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
	  this, SLOT(javaScriptWindowObjectCleared()) );
  load(QUrl::fromLocalFile(
      QApplication::applicationDirPath() + "/board.html"
  ));
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


Board::Board(Layout initial_layout /* = Layout() */) : QObject() {
  mode = Preparing;
  for(int i=0; i<25; i++) {
    elements.push_back(
	RenderElementFromPiece(
	    convert_layout_index_to_cell(i),
	    Element::Known(Orange, i),
	    initial_layout.get(i)
	)
    );
  }
}


Board::Board(const Game& game, bool is_watching) : QObject() {
  if(is_watching)
    mode = Watching;
  else
    mode = Playing;
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
  if(mode == Preparing)
    return "preparing";
  else if(mode == Playing)
    return "playing";
  else
    return "watching";
}


QList<RenderElement> Board::get_elements() const {
  return elements;
}


Hub::Hub() : QObject() {

}
