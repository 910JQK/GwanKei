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
// 自已
#define IS_MYSELF(cell) (game.element_of(cell).get_player() == player)
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
// 取得 0 ~ 1 之間隨機數
#define RAND() (double(qrand() % 100) / 100)
// Convert std::list to std::vector
#define LIST_2_VEC(list, vector) \
  for(auto item=list.begin(); item!=list.end(); item++) { \
    vector.push_back(*item)				  \
  }

  std::vector<Cell> my_cells; // 地雷和大本營以外的棋子（所在的格子）
  std::vector<Cell> my_bombs;
  std::vector<Cell> my_engs;
  Cell my_flag;
  int count[42] = {0};
  int enemy_flags[5] = {0, 0, 0, 0, 0}; // 以 Orient 為下標，0 = invalid
  for(int i=0; i<4631; i++) {
    if(is_valid_cell_id(i)) {
      Cell cell(i);
      Element element = game.element_of(cell);
      if(!element.is_empty()) {
	if(element.get_player() == player) {
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
	} else if(IS_ENEMY(cell) && !element.is_unknown()) {
	  if(game.piece_of(element) == Piece(31)) {
	    enemy_flags[cell.get_group()] = cell.get_id();
	  }
	} // my piece or known piece of enemy
      } // not empty
    } // valid cell id
  } // for cell id
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
    int this_time_least = 0; // 用 0 表示還沒有信息
    if(!last_game.element_of(killed).is_unknown()) {
      int killed_piece = LAST_GET_PIECE(killed).get_id();
      if(killed_piece != 41) {
	this_time_least = killed_piece + 1;
      } else {
	this_time_least = 1; // 用 1 表示確信是工兵
      }
    } else if(least[killed_id] != 0) {
      this_time_least = least[killed_id] + 1;
    }
    if(this_time_least > least[killer_id]) {
      least[killer_id] = this_time_least;
    }
  }
  if(move_result == Nothing && last_feedback.is_route_turned()) {
    least[EID(target)] = 1;
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

    auto try2occupy = try2kill_use_big;
    
    auto try2kill_use_small = [this, &game, &my_cells, &my_engs](Cell target) {
      // 有棋可用？（從小棋開始找）
      for(auto I=my_cells.rbegin(); I!=my_cells.rend(); I++) {
	if(GET_PIECE(*I) != Piece(32) && game.is_movable(*I, target)) {
	  emit move(*I, target);
	  return true;
	}
      }
      // 實在不行出動工兵試圖打對
      int target_min = least[EID(target)];
      if(target_min == 1 || (target_min == 0 && aggressive < 0.3*RAND() )) {
	for(auto I=my_engs.begin(); I!=my_engs.end(); I++) {
	  if(game.is_movable(*I, target)) {
	    emit move(*I, target);
	    return true;
	  }
	}
      }
      return false;
    };

    /* 【護旗】 */
    std::list<Bound> flag_adj = my_flag.get_adjacents();
    for(auto I=flag_adj.begin(); I!=flag_adj.end(); I++) {
      Cell cell = I->get_target();
      Cell above = cell.get_above();
      if(NOT_EMPTY(cell)) {
	if(IS_ENEMY(cell)) {
	  // 軍旗遭到直接威脅
	  TRY(try2kill_use_big(cell));
	} else if(GET_PIECE(cell) == Piece(41)) {
	  // 疑似工兵尋破綻
	  if(above.get_type() != Camp && NOT_EMPTY(above) && IS_ENEMY(above)) {
	    if(least[EID(above)] == 0 || least[EID(above)] == 1) {
	      TRY(try2kill_use_small(above));
	    }
	  }
	} // enemy or mine	
      } else {
	// 旗左右空了，不擋就下去了
	if(above.get_type() != Camp && NOT_EMPTY(above) && IS_ENEMY(above)) {
	  TRY(try2kill_use_big(above));
	}
      }
    } // adjacents of flag

    /* 【炸大子】 */
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

    /* 【防御】 */
    static auto bottom_first = [](const Cell& l, const Cell& r) -> bool {
      return (l.get_y() > r.get_y());
    };
    std::vector<Cell> invaders;
    for(int i=0; i<25; i++) {
      Cell cell = convert_layout_index_to_cell(i, player);
      if(NOT_EMPTY(cell) && IS_ENEMY(cell)) {
	invaders.push_back(cell);
      }
    }
    std::sort(invaders.begin(), invaders.end(), bottom_first);
    for(auto I=invaders.begin(); I!=invaders.end(); I++) {
      Cell invader = *I;
      std::list<Bound> adj = invader.get_adjacents();
      std::vector<Cell> adj_camps;
      for(auto J=adj.begin(); J!=adj.end(); J++) {
	if(J->get_target().get_type() == Camp) {
	  adj_camps.push_back(J->get_target());
	}
      }
      std::sort(adj_camps.begin(), adj_camps.end(), bottom_first);
      if(adj_camps.size() > 0) {
	TRY(try2occupy(adj_camps[0]) );
      }
      int min = least[EID(invader)];
      int nk = num_of_kill[EID(invader)];
      std::list<Cell> list = game.reachables_of(invader);
      std::vector<Cell> reachables;
      for(auto J=list.begin(); J!=list.end(); J++) {
	if(IS_MYSELF(*J) && GET_PIECE(*J) != Piece(0)) {
	  reachables.push_back(*J);
	}
      }
      for(int j=0; j<reachables.size()*3; j++) {
	Cell neighbor = reachables[qrand() % reachables.size()];
        #define TRY_TO_RESIST() emit move(neighbor, invader); return;
	if(IS_MYSELF(neighbor)) {
	  int p = GET_PIECE(neighbor).get_id();
	  if(p > min) {
	    if((p - min) > 3*(1-aggressive)) {
	      TRY_TO_RESIST();
	    } else if(
	              (p >= 38 || (aggressive > 0.7 && p >= 37))
		      && num_of_kill[EID(neighbor)] <= 3*aggressive
	    ) {
	      TRY_TO_RESIST();
	    } else if(nk <= 2*aggressive && neighbor.get_type() != Camp) {
	      TRY_TO_RESIST();
	    }
	  } // p > min
	} // my piece
      } // rand a neighbor
    } // for invaders

    /* 【補營】 */
    static const CellGroup my_cell_group = convert_player_to_orient(player);
    static const Cell camps[5] = {
      Cell(my_cell_group, 2, 2, Left),
      Cell(my_cell_group, 2, 2, Right),
      Cell(my_cell_group, 4, 2, Left),
      Cell(my_cell_group, 4, 2, Right),
      Cell(my_cell_group, 3, 3, Left)
    };
    for(int i=0; i<5; i++) {
      Cell camp = camps[i];
      if(EMPTY(camp)) {
	std::list<Bound> adj = camp.get_adjacents();
	std::vector<Cell> adj_cells;
	for(auto J=adj.begin(); J!=adj.end(); J++) {
	  adj_cells.push_back(J->get_target());
	}
	for(int j=0; j<adj_cells.size()*3; j++) {
	  Cell neighbor = adj_cells[qrand() % adj_cells.size()];
          #define OCCUPY_CAMP() emit move(neighbor, camp); return;
	  if(NOT_EMPTY(neighbor) && IS_MYSELF(neighbor)) {
	    int p = GET_PIECE(neighbor).get_id();
	    if(my_cells.size() >= 15+4*(1-aggressive)) {
	      if(camp.get_y() != 3) {
		if(p == 0) {
		  if(aggressive < RAND()*RAND()*0.75) {
		    OCCUPY_CAMP();
		  }
		} else {
		  if(p <= 36+5*(1-aggressive)) {
		    OCCUPY_CAMP();
		  }
		}  // bomb or not bomb
	      } else {
		break;
	      } // non-central camp
	    } else {
	      if(camp.get_y() == 4) {
		OCCUPY_CAMP();
	      } else {
		break;
	      } // bottom camp or not
	    } // still have enough pieces or not
	  } // my piece
	} // for adjacents
      } // empty camp
    } // for camp at my cell group

    /* 【攻擊】 */
    if(aggressive > RAND()*RAND() && my_cells.size() > 9 + 4*(1-aggressive)) {
      std::vector<CellPair> attack_options;
      for(auto I=my_cells.begin(); I!=my_cells.end(); I++) {
	std::list<Cell> reachables = game.reachables_of(*I);
	for(auto J=reachables.begin(); J!=reachables.end(); J++) {
	  if(NOT_EMPTY(*J) && IS_ENEMY(*J)) {
	    int p = GET_PIECE(*I).get_id();
	    if(p != 0 && p > least[EID(*J)]) {
	      attack_options.push_back((CellPair){*I,*J});
	    } // not too big to attack
	  } // attackable
	} // for reachables
      } // for my cells
      std::sort(
	attack_options.begin(),
	attack_options.end(),
	[this, &game](const CellPair& l, const CellPair& r) -> bool {
          #define ATTACK_DELTA(p) (  \
	    GET_PIECE(p.first).get_id() - least[EID(p.second)] \
	  )
	  return (ATTACK_DELTA(l) < ATTACK_DELTA(r));
	}
      );
      if(attack_options.size() > 0) {
	int index = int(RAND()*attack_options.size()*0.6*(1-aggressive));
	CellPair opt = attack_options[index];
	emit move(opt.first, opt.second);
	return;
      }
    }    

    /* 【胡亂走】 */
    for(int i=0; i<my_cells.size()*5; i++) {
      Cell selected_cell = my_cells[qrand() % my_cells.size()];
      if(selected_cell.get_group() != Central && selected_cell.get_y() >= 5) {
	if(my_cells.size() >= 15 + 4*aggressive) {
	  continue;
	}
      }
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
	for(int j=0; j<reachables.size()*5; j++) {
	  Cell target = reachables[qrand() % reachables.size()];
	  if(
	     opposite_orient(target.get_group())
	     == convert_player_to_orient(player)
	  ) {
	    continue;
	  }
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
