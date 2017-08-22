#include <cassert>
#include "desk.hpp"


#ifdef DEBUG
#include <iostream>
#define DBG(text) std::cerr << "[Debug] [" << this << "] " << text << "\n";
#else
#define DBG(text)
#endif


Desk::Desk(MaskMode mask_mode, bool is_1v1) : QObject() {
  DBG("Desk Instance Created");
  timer = new QTimer(this);
  this->mask_mode = mask_mode;
  this->is_1v1 = is_1v1;
  timer->setSingleShot(true);
  timer->setInterval(WAITING_TIME*1000);
  connect(timer, &QTimer::timeout, this, &Desk::timeout);
}


Desk::~Desk() {
  DBG("Delete Desk Instance");
  timer->disconnect();
  if(started) {
    delete game;
  }
}


QString Desk::get_player_name(Player player) const {
  return players[player];
}


QStringList Desk::get_players() const {
  QStringList result;
  for(int i=0; i<4; i++)
    result.push_back(players[i]);
  return result;
}


QList<bool> Desk::get_ready_state() const {
  QList<bool> result;
  for(int i=0; i<4; i++)
    result.push_back(ready_state[i]);
  return result;
}


bool Desk::is_player_available(Player player) const {
  return (players[player] == "");
}


bool Desk::is_1v1_desk() const {
  return is_1v1;
}


void Desk::change_status(int single_player /* = -1 */) {  
  assert(started);
  DBG("Call Desk::change_status()");
  for(int i=0; i<4; i++) {
    if(single_player != -1 && single_player != i) {
      continue;
    }
    if(!ready_state[i]) {
      continue; // 1v1, some player is not enabled
    }
    Player player = static_cast<Player>(i);
    emit status_changed(
	player,
	game->get_game_with_mask(player, mask_mode),
	current_player,
	timer->remainingTime()/1000
    );
  }
}


void Desk::next_turn() {
  DBG("Call Desk::next_turn()");
  timer->stop();
  do {
    if(check_ending())
      return;
    current_player = static_cast<Player>((current_player+1)%4);
    DBG("Try player " << current_player);
  } while(failed[current_player] || !ready_state[current_player]);
  if(!game->has_living_piece(current_player)) {
    game->annihilate(current_player);
    failed[current_player] = true;
    emit fail(current_player, NoLivingPiece);
    next_turn();
    return;
  }
  timer->start();
}


void Desk::try_to_start() {
  if(started)
    return;
  DBG("Call Desk::try_to_start()");
  auto start = [this]{
    DBG("Call lambda start()");
    started = true;
    current_player = Blue; // next = Orange
    next_turn();
    change_status();
  };
  if(is_1v1) {
    if(  ready_state[0] && ready_state[2]
         && !ready_state[1] && !ready_state[3]
    ) {
      game = new Game(layouts[0], layouts[2]);
      start();
    } else if(  ready_state[1] && ready_state[3]
	        && !ready_state[0] && !ready_state[2]
    ) {
      game = new Game(layouts[1], layouts[3], true);
      start();
    }
  } else {
    if(ready_state[0] && ready_state[1] && ready_state[2] && ready_state[3]) {
      game = new Game(layouts);
      start();
    }
  }
}


bool Desk::check_ending() {
  #define END() \
    emit end(ending); \
    ended = true; \
    return true;

  Ending ending = {Tie, Tie, Tie, Tie};
  if(is_1v1) {
    for(int i=0; i<4; i++) {
      if(failed[i]) {
	ending[i] = GG;
	ending[(i+2)%4] = Win;
	END();
      }
    }
    // if ... -> tie (to be implemented)
  } else {
    if(failed[0] && failed[2]) {
      ending[0] = ending[2] = GG;
      ending[3] = ending[1] = Win;
      END();
    } else if(failed[1] && failed[3]) {
      ending[3] = ending[1] = GG;
      ending[0] = ending[2] = Win;
      END();
    } else /* if ... */ {
      // tie, to be implemented
    }
  }
  return false;
}


void Desk::timeout() {
  assert(started);
  next_turn();
  change_status();
}


void Desk::set_player(Player player, QString name) {
  players[player] = name;
  emit players_changed(get_players());
}


void Desk::remove_player(QString name) {
  for(int i=0; i<4; i++) {
    if(players[i] == name) {
      players[i] = "";
      ready_state[i] = false;
    }
  }
  emit players_changed(get_players());
}


void Desk::request_status_message(Player player) {
  if(started) {
    change_status(player);
  }  
}


void Desk::ready(Player player, Layout layout) {
  if(!started && players[player] != "") {
    layouts[player] = layout;
    ready_state[player] = true;
    emit ready_state_changed(get_ready_state());
  }
  try_to_start();
}


void Desk::move(Player player, Cell from, Cell to) {
  DBG(
      "Call Desk::move() [" << player << "]" << " " <<
      "(" << from.to_string() << ")" << " -> " <<
      "(" << to.to_string() << ")"
  );
  if(
     started
     && !ended
     && player == current_player
     && game->is_movable(from, to)
     && game->element_of(from).get_player() == player
  ) {
    Element to_element = game->element_of(to);
    bool is_to_flag = false;
    if(!to_element.is_empty() && game->piece_of(to_element).get_id() == 31) {
      is_to_flag = true;
    } 
    Feedback feedback = game->move(from, to);    
    if(
       (feedback.get_move_result() == Bigger
	|| feedback.get_move_result() == Equal
	)
       && is_to_flag) {
      Player to_player = to_element.get_player();
      game->annihilate(to_player);
      failed[to_player] = true;
      emit fail(to_player, FlagLost);
    }
    next_turn();
    change_status();
  }
}


void Desk::skip(Player player) {
  if(started && player == current_player) {
    next_turn();
  }
}
