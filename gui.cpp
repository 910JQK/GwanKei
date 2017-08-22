#include <QApplication>
#include <QDesktopWidget>
#include <QWebSettings>
#include <QWebFrame>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDir>
#include <QUrl>
#include <QTimer>
#include <QDebug>
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
  QMenu* start_menu = menuBar()->addMenu(tr("&Start"));
  QMenu* bl_menu = start_menu->addMenu(tr("&Brainless AI"));
  QMenu* li_menu = start_menu->addMenu(tr("&Low-IQ AI"));

  #define ADD_MENU_ITEM(menu, item, battle_type, text)	\
  QAction* item = new QAction(text); \
  connect(item, &QAction::triggered, \
	  view, [this]() { view->new_game(battle_type); }); \
  menu->addAction(item);

  ADD_MENU_ITEM(bl_menu, bl_2v2_ne, BL_AI_2v2_NE, tr("2v2 &Ordinary"));
  ADD_MENU_ITEM(bl_menu, bl_2v2_de, BL_AI_2v2_DE, tr("2v2 &Allied Visible"));
  ADD_MENU_ITEM(li_menu, li_1v1, LI_AI_1v1, tr("&1v1"));
  ADD_MENU_ITEM(li_menu, li_2v2_ne, LI_AI_2v2_NE, tr("2v2 &Ordinary"));
  ADD_MENU_ITEM(li_menu, li_2v2_de, LI_AI_2v2_DE, tr("2v2 &Allied Visible"));
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


void View::new_game(BattleType type) {
  if(battle_created) {
    disconnect(hub, &Hub::ready, 0, 0);
    disconnect(hub, &Hub::move, 0, 0);
    battle->disconnect();
    battle->deleteLater();
  }
  battle = Battle::Create(type);
  battle_created = true;
  init_battle();
}


void View::init_battle() {
  hub->init(battle->get_player(), Layout());
  connect(hub, &Hub::ready, battle, &Battle::ready);
  connect(hub, &Hub::move, battle, &Battle::move);
  connect(battle, &Battle::status_changed, hub, &Hub::status_changed);
  connect(battle, &Battle::fail, this, [this] (Player who, FailReason reason) {
    const QString reason_str[4] = {
      tr("flag was captured"),
      tr("has no living piece"),
      tr("surrendered"),
      tr("timeout count exceeded limit")
    };
    QMessageBox::information(
      this,
      tr("Message"),
      tr("%1 failed (%2)")
        .arg(battle->get_player_name(who))
        .arg(reason_str[reason])
    );
  });
  connect(battle, &Battle::end, this, [this](Ending ending) {
    End4Player my_ending = ending[battle->get_player()];
    static const QString ending_str[3] = {
      tr("Tie"), tr("You are victorious"), tr("You are defeated")
    };
    QMessageBox::information(
      this,
      tr("Message"),
      tr("Game Over (%1)").arg(ending_str[my_ending])
    );
    hub->game_over();
  });
}


void View::javaScriptWindowObjectCleared() {
  page()->mainFrame()->addToJavaScriptWindowObject("Hub", hub);
}


void Hub::execute_render() {
  if(!started) {
    Board* board = new Board(layout, player);
    render(board);
  } else {
    Board* board = new Board(game, player, (player != current_player) || ended);
    render(board);
  }
  // release memory in frontend
}


void Hub::init(Player player, Layout layout) {
  this->started = false;
  this->ended = false;
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


void Hub::submit_ready() {
  emit ready(layout);
}


void Hub::submit_move(int from, int to) {
  emit move(Cell(from), Cell(to));
}


void Hub::game_over() {
  ended = true;
}


void Hub::debug(QString str) const {
  qDebug() << str << "\n";
}


void Hub::status_changed(Game game, Player current_player, int wait_seconds) {
  if(!started) {
    started = true;
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
