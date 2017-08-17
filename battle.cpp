#include <QTimer>
#include "battle.hpp"


Battle::Battle(Player player) : QObject() {
  this->player = player;
}


Battle::~Battle() {
  delete desk;
}


void Battle::init_desk() {
  connect(desk, &Desk::status_changed,
	  this, [this](
	       Player msg_target, Game game, Player current, int wait_sec
	  ) {
	    if(msg_target == this->player) {
	      emit this->status_changed(game, current, wait_sec);
	    }
	  }
  );
  connect(desk, &Desk::fail, this, &Battle::fail);
  connect(desk, &Desk::end, this, &Battle::end);
}


Player Battle::get_player() const {
  return this->player;
}


QString Battle::get_player_name(Player player) const {
  return desk->get_player_name(player);
}


void Battle::ready(Layout layout) {
  desk->ready(player, layout);
}


void Battle::move(Cell from, Cell to) {
  desk->move(player, from, to);
}


Battle* Battle::Create(BattleType type) {
  Player player = static_cast<Player>(qrand() % 4);
  Battle* result;
  switch(type) {
  case BL_AI_2v2_NE:
    result = new BL_AI_2v2(player, NoExpose);
    break;
  case BL_AI_2v2_DE:
    result = new BL_AI_2v2(player, DoubleExpose);
    break;
  }
  return result;
}


BL_AI_2v2::BL_AI_2v2(Player player, MaskMode mask_mode) : Battle(player) {
  desk = new Desk(mask_mode, false);
  desk->set_player(Orange, "Orange");
  desk->set_player(Purple, "Purple");
  desk->set_player(Green, "Green");
  desk->set_player(Blue, "Blue");
  ai = new Brainless*[3];
  for(int i=0; i<3; i++) {
    Player player_of_ai = static_cast<Player>((player+1+i)%4);
    ai[i] = new Brainless(player_of_ai);
    desk->ready(player_of_ai, ai[i]->get_layout());
    connect(ai[i], &Brainless::move,
	    this, [this, player_of_ai](Cell from, Cell to) {
	      QTimer::singleShot(2500, this, [this, player_of_ai, from, to]() {
		this->desk->move(player_of_ai, from, to);
	      });
	    }
    );
  }
  connect(desk, &Desk::status_changed,
	  this, [this, player] (Player target, Game game,
				Player current, int wait) {
	    if(target != player) {
	      int index_of_ai = (target-player-1+4)%4;
	      this->ai[index_of_ai]->status_changed(game, current);
	    }
	  }
  );
  init_desk();
}
