#include <cassert>
#include "desk.hpp"


Desk::Desk(MaskMode mask_mode, bool is_1_v_1) : QObject() {
  this->mask_mode = mask_mode;
  this->is_1_v_1 = is_1_v_1;
  timer.setSingleShot(true);
  timer.setInterval(WAITING_TIME*1000);
  connect(&timer, &QTimer::timeout, this, &Desk::timeout);
}


Desk::~Desk() {
  if(started) {
    delete game;
  }
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


void Desk::change_status(int single_player /* = -1 */) {
  assert(started);
  for(int i=0; i<4; i++) {
    if(single_player != -1 && single_player != i) {
      continue;
    }
    Player player = static_cast<Player>(i);
    emit status_changed(
	player,
	game->get_game_with_mask(player, mask_mode),
	current_player,
	timer.remainingTime()/1000
    );
  }
}


void Desk::next_turn() {
  timer.stop();
  do {
    // if not failed, check_live_piece, no -> failed[..] = true
    if(check_ending())
      return;
    current_player = static_cast<Player>((current_player+1)%4);
  } while(failed[current_player]);
  timer.start();
}


void Desk::try_to_start() {
  static auto start = [this]{
    this->started = true;
    this->current_player = Blue;
    this->next_turn();
    this->change_status();
  };
  if(is_1_v_1) {
    if(ready_state[0] && ready_state[2]) {
      game = new Game(layouts[0], layouts[2]);
      start();
    } else if(ready_state[1] && ready_state[3]) {
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
  if(failed[0] && failed[2]) {
    emit end(PurpleBlueWin);
    return true;
  } else if(failed[1] && failed[3]) {
    emit end(OrangeGreenWin);
    return true;
  } else {
    // tie, to be implemented
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
  if(started && player == current_player && game->is_movable(from, to)
     && game->element_of(from).get_player() == player) {
    Element to_element = game->element_of(to);
    bool is_to_flag = false;
    if(!to_element.is_empty() && game->piece_of(to_element).get_id() == 31) {
      is_to_flag = true;
    }    
    Feedback feedback = game->move(from, to);
    if(feedback.get_move_result() == Bigger && is_to_flag) {
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
