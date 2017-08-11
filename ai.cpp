#include "ai.hpp"


using namespace GwanKei;


AI::AI(Player player) : QObject() {
  this->player = player;
}


Brainless::Brainless(Player player) : AI(player) {

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
