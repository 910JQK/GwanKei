#include <queue>
#include <cmath>
#include "ai.hpp"


using namespace GwanKei;


AI::AI(Player player) : QObject() {
  this->player = player;
}


Brainless::Brainless(Player player) : AI(player) {

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


LowIQ::LowIQ(Player player, double aggressive) : AI(player) {
  assert(0 <= aggressive && aggressive <= 1);
  this->aggressive = aggressive;
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
  if(current_player != player) {
    return;
  }
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

  // to be implemented
  
}
