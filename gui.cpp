#include <QApplication>
#include <QDesktopWidget>
#include <QWebSettings>
#include <QWebFrame>
#include <QWebInspector>
#include <QDialog>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDir>
#include <QUrl>
#include <QFile>
#include <QTimer>
#include <QSoundEffect>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include "gui.hpp"


#define APP_DIR() QApplication::applicationDirPath()


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
  QMenu* sound_menu = menuBar()->addMenu(tr("&Sound"));
  QMenu* debug_menu = menuBar()->addMenu(tr("&Debug"));

  load_sound();
  QActionGroup* sound_options = new QActionGroup(this);
  sound_options->setExclusive(true);
  QAction* no_sound = new QAction(tr("&None"), sound_options);
  connect(no_sound, &QAction::triggered, this, [this](){ sound_theme = ""; });
  no_sound->setCheckable(true);
  sound_menu->addAction(no_sound);
  for(auto I=sound_themes_title.begin(); I!=sound_themes_title.end(); I++) {
    QString name = I.key();
    QString title = I.value();
    QAction* toggle_action = new QAction(title, sound_options);
    connect(toggle_action, &QAction::triggered,
	    this, [this, name]() { sound_theme = name; });
    toggle_action->setCheckable(true);
    sound_menu->addAction(toggle_action);
  }
  no_sound->setChecked(true);
  connect(view->hub, &Hub::play_sound, this, &Window::try_to_play_sound);

  QAction* inspector_action = new QAction(tr("Open &Inspector"), this);
  connect(inspector_action, &QAction::triggered, view, &View::open_inspector);
  debug_menu->addAction(inspector_action);

  #define ADD_MENU_ITEM(menu, item, battle_type, text)	\
    QAction* item = new QAction(text, this);		\
    connect(item, &QAction::triggered,			    \
	    view, [this]() { view->new_game(battle_type); });	\
    menu->addAction(item);

  ADD_MENU_ITEM(bl_menu, bl_2v2_ne, BL_AI_2v2_NE, tr("2v2 &Ordinary"));
  ADD_MENU_ITEM(bl_menu, bl_2v2_de, BL_AI_2v2_DE, tr("2v2 &Allied Visible"));
  ADD_MENU_ITEM(li_menu, li_1v1, LI_AI_1v1, tr("&1v1"));
  ADD_MENU_ITEM(li_menu, li_2v2_ne, LI_AI_2v2_NE, tr("2v2 &Ordinary"));
  ADD_MENU_ITEM(li_menu, li_2v2_de, LI_AI_2v2_DE, tr("2v2 &Allied Visible"));
}


void Window::load_sound() {
  QDir sound_dir(APP_DIR() + "/Sound");
  QStringList list = sound_dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  for(auto I=list.begin(); I!=list.end(); I++) {
    QFile index_file(sound_dir.absoluteFilePath(*I + "/index.json"));
    QString str;
    index_file.open(QIODevice::ReadOnly | QIODevice::Text);
    str = index_file.readAll();
    index_file.close();
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());
    if(!doc.isNull() && doc.isObject()) {
      QJsonObject obj = doc.object();
      QString title = obj["title"].toString();
      QJsonObject files = obj["files"].toObject();
      if(!title.isEmpty() && !files.isEmpty()) {
	QMap<QString, QSoundEffect*> sounds;
	for(auto J=files.begin(); J!=files.end(); J++) {
	  QString sound_name = J.key();
	  QString file_name = J.value().toString();
	  if(!file_name.isEmpty()) {
	    QString path = (
	      APP_DIR() + "/Sound/" + *I + "/" + file_name
	    );
	    if(QFile::exists(path)) {
	      sounds[sound_name] = new QSoundEffect();
	      sounds[sound_name]->setSource(QUrl::fromLocalFile(path));
	    }
	  }
	} // for files
	sounds_of_theme[*I] = sounds;
	sound_themes_title[*I] = title;
      } // title and files is not empty
    } // valid JSON document
  } // for I of list
}


void Window::try_to_play_sound(QString name) {
  if(!sound_theme.isEmpty()) {
    QMap<QString, QSoundEffect*> sounds = sounds_of_theme[sound_theme];
    if(sounds.find(name) != sounds.end()) {
      sounds_of_theme[sound_theme][name]->play();
    }
  }
}


View::View(QWidget* parent) : QWebView(parent) {
  QWebSettings* defaultSettings = QWebSettings::globalSettings();
  defaultSettings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
  defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, true);
  setContextMenuPolicy(Qt::NoContextMenu);
  hub = new Hub(this);
  connect(page()->mainFrame(), &QWebFrame::javaScriptWindowObjectCleared,
	  this, &View::javaScriptWindowObjectCleared );
  load(QUrl::fromLocalFile(APP_DIR() + "/board.html"));
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
    emit hub->play_sound("fail");
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
    emit hub->play_sound("end");
  });
}


void View::open_inspector() {
  if(!inspector_created) {
    inspector = new QWebInspector(this);
    inspector->setPage(page());
    inspector_dialog = new QDialog();
    inspector_dialog->setLayout(new QVBoxLayout());
    inspector_dialog->layout()->addWidget(inspector);
    inspector_dialog->setModal(false);
    inspector_dialog->resize(800, 360);
    inspector_dialog->layout()->setContentsMargins(0, 0, 0, 0);
    inspector_dialog->setWindowTitle(tr("Web Debug Toolkit"));
  }
  inspector_dialog->show();
  inspector_dialog->raise();
  inspector_dialog->activateWindow();
}


void View::javaScriptWindowObjectCleared() {
  page()->mainFrame()->addToJavaScriptWindowObject("Hub", hub);
}


Hub::Hub(QObject *parent) : QObject(parent) {

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
  Feedback feedback = game.get_last_feedback();
  MoveResult move_result = feedback.get_move_result();
  if(game.get_steps() != last_steps) {
    if(feedback.is_flag_shown()) {
      emit play_sound("show_flag");
    } else if(move_result == Bigger) {
      emit play_sound("bigger");
    } else if(move_result == Smaller) {
      emit play_sound("smaller");    
    } else if(move_result == Equal) {
      emit play_sound("equal");
    } else if(move_result == Nothing) {
      emit play_sound("move");
    }
  }
  if(game.get_steps() == 0) {
    emit play_sound("start");
  }
  last_steps = game.get_steps();
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
	  elements.push_back(
	    RenderElementFromPiece(Cell(i), e, game.piece_of(e))
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
