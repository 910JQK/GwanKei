#include <QTimer>
#include "battle.hpp"


Battle::Battle(Player player, AI** ai, bool is_1v1, MaskMode mask_mode)
                                                                : QObject() {
  #define PLAYER(t) static_cast<Player>(t)
  static const char* player_names[4] = {"Orange", "Purple", "Green", "Blue"};
  this->player = player;
  desk = new Desk(mask_mode, is_1v1);
  for(int i=0; i<4; i++) {
    desk->set_player(PLAYER(i), player_names[i]);
  }
  int n_of_ai = is_1v1? 1: 3;
  this->ai = ai;
  for(int i=0; i<n_of_ai; i++) {
    Player player_of_ai = is_1v1? PLAYER((player+2)%4)
                                : PLAYER((player+1+i)%4);
    ai[i]->set_player(player_of_ai);
    connect(ai[i], &AI::move,
	    this, [this, player_of_ai](Cell from, Cell to) {
	      QTimer::singleShot(2500, this, [this, player_of_ai, from, to]() {
		this->desk->move(player_of_ai, from, to);
	      });
	    }
    );
    desk->ready(player_of_ai, ai[i]->get_layout());
  }
  connect(desk, &Desk::status_changed,
	  this, [this, player, is_1v1] (Player target, Game game,
					Player current, int wait) {
	    if(target != player) {
	      int index_of_ai = is_1v1? 0: (target-player-1+4)%4;
	      this->ai[index_of_ai]->status_changed(game, current);
	    }
	  }
  );
  init_desk();
}


Battle::~Battle() {
  int n_of_ai = desk->is_1v1_desk()? 1: 3;
  for(int i=0; i<n_of_ai; i++) {
    ai[i]->disconnect();
    ai[i]->deleteLater();
  }
  delete[] ai;
  desk->disconnect();
  desk->deleteLater();
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
  Player player = PLAYER(qrand() % 4);
  Battle* result;
  AI** ai;

  #define CREATE_1_AI(AI_Type) ai = new AI*[1]; ai[0] = AI_Type();
  #define CREATE_3_AI(AI_Type) \
    ai = new AI*[3]; for(int i=0; i<3; i++) ai[i] = AI_Type();
  
  switch(type) {
  case BL_AI_2v2_NE:
    CREATE_3_AI(Brainless::Create);
    result = new Battle(player, ai, false, NoExpose);
    break;
  case BL_AI_2v2_DE:
    CREATE_3_AI(Brainless::Create);
    result = new Battle(player, ai, false, DoubleExpose);
    break;
  case LI_AI_1v1:
    CREATE_1_AI(LowIQ::Rand);
    result = new Battle(player, ai, true, NoExpose);
    break;
  case LI_AI_2v2_NE:
    CREATE_3_AI(LowIQ::Rand);
    result = new Battle(player, ai, false, NoExpose);
    break;
  case LI_AI_2v2_DE:
    CREATE_3_AI(LowIQ::Rand);
    result = new Battle(player, ai, false, DoubleExpose);
    break;
  }
  return result;
}
