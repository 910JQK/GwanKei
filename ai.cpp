#include <algorithm>
#include <vector>
#include <cmath>
#include "ai.hpp"


using namespace GwanKei;


AI::AI() : QObject() {

}


Player AI::get_player() const {
  assert(initialized);
  return player;
}


void AI::set_player(Player player) {
  assert(!initialized);
  this->player = player;
  initialized = true;
}


bool AI::is_initialized() const {
  return initialized;
}


Brainless::Brainless() : AI() {

}


Brainless* Brainless::Create() {
  Brainless* result = new Brainless();
  return result;
}


Layout Brainless::get_layout() {
  Layout result;
  for(int i=0; i<500; i++) {
    int index1 = qrand() % 25;
    int index2 = qrand() % 25;
    if(result.is_able_to_swap(index1, index2)) {
      result.swap(index1, index2);
    }
  }
  if(qrand() % 2 == 0) {
    result.swap(14, 24); // 軍旗位置
  }
  return result;
}


void Brainless::status_changed(Game game, Player current_player) {
  assert(is_initialized());
  Player player = get_player();
  if(current_player == player) {
    std::list<Cell> my_cells;
    for(int i=0; i<4631; i++) {
      if(is_valid_cell_id(i)) {
	Cell cell(i);
	Element element = game.element_of(cell);
	if(!element.is_empty() && element.get_player() == player) {
	  my_cells.push_back(cell);
	}
      }
    }
    for(int i=0; i<4631; i++) {
      if(is_valid_cell_id(i)) {
	Cell cell(i);
	Element element = game.element_of(cell);
	if(
	   !element.is_empty()
	   && element.get_player() != player
	   && element.get_player() != static_cast<Player>((player+2)%4)
	   ) {
	  for(auto J=my_cells.begin(); J!=my_cells.end(); J++) {
	    if(game.is_movable(*J, cell)) {
	      emit move(*J, cell);
	      return;
	    }
	  }
	} else if(element.is_empty()) {
	  for(auto J=my_cells.begin(); J!=my_cells.end(); J++) {
	    if(game.is_movable(*J, cell)) {
	      emit move(*J, cell);
	      return;
	    }
	  }
	} else {
	  // do nothing
	}
      } // is valid cell id
    } // for cell id
  } // my turn
}


LowIQ::LowIQ(double aggressive) : AI() {
  assert(0 <= aggressive && aggressive <= 1);
  this->aggressive = aggressive;
}


LowIQ* LowIQ::Rand() {
  LowIQ* result = new LowIQ(double(qrand() % 100) / 100);
  return result;
}


bool LowIQ::is_proper_layout(const Layout& layout) const {
  static const int AGI_MAX = (200+200+38+38+150+150+37)*3+(37+36+36+35+35+10);
  static const int AGI_MIN = (10)*7*3 + (10+10+35+35+36+36);
  static const int WEIGHT[25] = {
    3, 0, 0, 0, 0,
    3, 3, 3, 1, 0, 0,
    1, 1, 0, 0,
    3, 3, 3, 1, 0, 0,
    1, 1, 0, 0
  };
  for(int i=0; i<25; i++) {
    int id = layout.get(i).get_id();
    if(id == 40 || id == 39) {
      if(aggressive > 0.35 && convert_layout_index_to_cell(i).get_y() > 4) {
	return false;
      }
    }
  }
  int agi = 0;
  for(int i=0; i<25; i++) {
    int id = layout.get(i).get_id();
    if(id == 0) {
      agi += 150*WEIGHT[i];
    } else if(id == 39 || id == 40) {
      agi += 200*WEIGHT[i];
    } else if(id <= 34 || id == 41) {
      agi += 10*WEIGHT[i];
    } else {
      agi += id*WEIGHT[i];
    }
  }
  int f_agi = pow(double(agi-AGI_MIN)/(AGI_MAX-AGI_MIN), 2);
  assert(0 <= f_agi && f_agi <= 1);
  if(fabs(f_agi - aggressive) < 0.15)
    return true;
  else
    return false;
}


Layout LowIQ::get_layout() {
  Layout result;
  if(qrand() % 2 == 0) {
    result.swap(14, 24); // 軍旗位置
  }
  for(int i=0; i<1000; i++) {
    int index1 = qrand() % 25;
    int index2 = qrand() % 25;
    if(result.is_able_to_swap(index1, index2)) {
      int id1 = result.get(index1).get_id();
      int id2 = result.get(index2).get_id();
      // 假旗只放排長或地雷
      if((index1 == 14 || index1 == 24) && (id2 != 33 && id2 != 41)) {
	continue;
      } else if((index2 == 14 || index2 == 24) && (id1 != 33 && id1 != 41)) {
	continue;
      }
      // 地雷不堵住太大的棋
      if(!(33 <= id2 && id2 <= 36)) {
	if(index1 == 4 || index1 == 10 || index1 == 20) {
	  if(result.get(index1 - 1) == Piece(41)) {
	    continue;
	  } // 上面是雷
	} // 雷下位
      } // 大子
      if(!(33 <= id1 && id1 <= 36)) {
	if(index2 == 4 || index2 == 10 || index2 == 20) {
	  if(result.get(index2 - 1) == Piece(41)) {
	    continue;
	  }
	}
      }
      // 交換
      result.swap(index1, index2);
    }
    if(i > 200 && is_proper_layout(result))
      return result;
  }
  return result;
}


void LowIQ::status_changed(Game game, Player current_player) {

  assert(is_initialized());
  Player player = get_player();

// 取得棋子大小
#define GET_PIECE(cell) game.piece_of(game.element_of(cell))
// 取得 Element ID
#define EID(cell) game.element_of(cell).get_id()
// 空
#define EMPTY(cell) game.element_of(cell).is_empty()  
// 非空
#define NOT_EMPTY(cell) !EMPTY(cell)
// 己方
#define IS_ALLIED(cell) (!game.is_1v1() \
		       && !((game.element_of(cell).get_player() - player) % 2))
// 敵方
#define IS_ENEMY(cell) !IS_ALLIED(cell)
// last_game 是上一步的狀態
#define IS_LAST_GAME_VALID() (last_game.get_steps() == (game.get_steps() - 1))
// 取得上一步時的棋子大小
#define LAST_GET_PIECE(cell) last_game.piece_of(last_game.element_of(cell))
// try to do sth.
#define TRY(sth) if(sth) return;

  std::vector<Cell> my_cells; // 地雷和大本營以外的棋子（所在的格子）
  std::vector<Cell> my_bombs;
  std::vector<Cell> my_engs;
  Cell my_flag;
  int count[42] = {0};
  for(int i=0; i<4631; i++) {
    if(is_valid_cell_id(i)) {
      Cell cell(i);
      Element element = game.element_of(cell);
      if(!element.is_empty() && element.get_player() == player) {
	Piece piece = game.piece_of(element);
	if(piece != Piece(41) && cell.get_type() != Headquarter) {
	  my_cells.push_back(cell);
	}
	if(piece == Piece(31)) {
	  my_flag = cell;
	} else if(piece == Piece(32)) {
	  my_engs.push_back(cell);
	} else if(piece == Piece(0)) {
	  my_bombs.push_back(cell);
	}
	count[piece.get_id()]++;
      }
    }
  }
  std::sort(
    my_cells.begin(), my_cells.end(),
    [&game](const Cell& l, const Cell& r) -> bool {
      int pl = GET_PIECE(l).get_id();
      int pr = GET_PIECE(r).get_id();
      if(pl == 0) pl = 42;
      if(pr == 0) pr = 42;
      return (pl > pr);
    }
  );
  Feedback last_feedback = game.get_last_feedback();
  MoveResult move_result = last_feedback.get_move_result();
  Cell moved = last_feedback.get_moved_cell();
  Cell target = last_feedback.get_target_cell();
  if(IS_LAST_GAME_VALID() &&
     (move_result == Smaller || move_result == Bigger) ) {
    // 要看碰子前的資料只能靠上次的記錄，因為 killed 已經沒了
    Cell killer;
    Cell killed;
    if(move_result == Bigger) {
      killer = moved;
      killed = target;
    } else {
      killer = target;
      killed = moved;
    }
    int killer_id = last_game.element_of(killer).get_id();
    int killed_id = last_game.element_of(killed).get_id();
    num_of_kill[killer_id]++;
    int this_time_least = 0;
    if(!last_game.element_of(killed).is_unknown()) {
      this_time_least = LAST_GET_PIECE(killed).get_id() + 1;
    } else if(least[killed_id] != 0) {
      this_time_least = least[killed_id] + 1;
    }
    if(this_time_least > least[killer_id]) {
      least[killer_id] = this_time_least;
    }
  }

  if(current_player == player) {

    auto try2kill_use_big = [this, &game, &my_cells](Cell target) -> bool {
      for(auto I=my_cells.begin(); I!=my_cells.end(); I++) {
	if(game.is_movable(*I, target)) {
	  emit move(*I, target);
	  return true;
	}
      }
      return false;
    };
    
    auto try2kill_use_small = [this, &game, &my_cells, &my_engs](Cell target) {
      for(auto I=my_cells.rbegin(); I!=my_cells.rend(); I++) {
	if(GET_PIECE(*I) != Piece(32) && game.is_movable(*I, target)) {
	  emit move(*I, target);
	  return true;
	}
      }
      if(least[EID(target)] == 0) {
	for(auto I=my_engs.begin(); I!=my_engs.end(); I++) {
	  if(game.is_movable(*I, target)) {
	    emit move(*I, target);
	    return true;
	  }
	}
      }
      return false;
    };

    std::list<Bound> flag_adj = my_flag.get_adjacents();
    for(auto I=flag_adj.begin(); I!=flag_adj.end(); I++) {
      Cell cell = I->get_target();
      if(NOT_EMPTY(cell)) {
	if(IS_ENEMY(cell)) {
	  TRY(try2kill_use_big(cell));
	} else if(GET_PIECE(cell) == Piece(41)) {
	  Cell up(
	    cell.get_group(), (cell.get_y() - 1), cell.get_x(), cell.get_lr()
	  );
	  if(up.get_type() != Camp && NOT_EMPTY(up) && IS_ENEMY(up)) {
	    TRY(try2kill_use_small(cell));
	  }
	} // enemy or mine	
      } // a piece
    } // adjacents of flag
    for(auto I=my_bombs.begin(); I!=my_bombs.end(); I++) {
      std::list<Cell> reachables = game.reachables_of(*I);
      for(auto J=reachables.begin(); J!=reachables.end(); J++) {
	if(NOT_EMPTY(*J) && IS_ENEMY(*J)) {
          #define BANG() emit move(*I, *J); return;
	  int min = least[EID(*J)];
	  if(min == 40 || min == 39) {
	    BANG();
	  } else if(min == 38) {
	    if(my_bombs.size() == 2
	       && aggressive > pow(0.85, num_of_kill[EID(*J)]) ) {
	      BANG();
	    } else if(aggressive > pow(0.98, num_of_kill[EID(*J)]) ) {
	      BANG();
	    }
	  }
	} // enemy
      } // all reachables of bomb      
    } // all bombs

    for(unsigned int i=0; i<my_cells.size()*5; i++) {
      Cell selected_cell = my_cells[qrand() % my_cells.size()];
      std::list<Cell> list = game.reachables_of(selected_cell);
      std::vector<Cell> reachables;
      for(auto J=list.begin(); J!=list.end(); J++) {
	if( !(NOT_EMPTY(*J) && IS_ALLIED(*J)) ) {
	  reachables.push_back(*J);
	}
      }
      if(reachables.size() == 0) {
	continue;
      } else {
	for(unsigned int j=0; j<reachables.size()*5; j++) {
	  Cell target = reachables[qrand() % reachables.size()];
          #define GO_TO_TARGET() emit move(selected_cell, target); return
	  if(EMPTY(target)) {
	    GO_TO_TARGET();
	  } else if(NOT_EMPTY(target) && IS_ENEMY(target)) {
	    if(GET_PIECE(selected_cell).get_id() >= least[EID(target)]) {
	      GO_TO_TARGET();
	    } // big enough
	  } // empty or enemy
	} // random a reachable
	continue;
      } // has reachables?
    } // random a cell
    
  } // this player

  last_game = game;
}
