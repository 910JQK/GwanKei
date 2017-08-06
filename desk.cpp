#include <cassert>
#include "desk.hpp"


Desk::Desk(MaskMode mask_mode) : QObject() {
  this->mask_mode = mask_mode;
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
	timer.remainingTime()
    );
  }
}


void Desk::next_turn() {
  timer.stop();
  current_player = static_cast<Player>((current_player+1)%4);
  timer.start();
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
  /*
    [ start game ]
  */
}


void Desk::move(Player player, Cell from, Cell to) {
  if(started && player == current_player && game->is_movable(from, to)) {
    game->move(from, to);
    next_turn();
    change_status();
  }
}


void Desk::skip(Player player) {
  if(started && player == current_player) {
    next_turn();
  }
}
