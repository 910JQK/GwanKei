#include <cassert>
#include "game.hpp"


namespace GwanKei {

  bool is_valid_game_piece_id(int id) {
    return (0 <= id && id <= 1+(25+1)*4);
  }

  bool is_valid_layout_index(int index) {
    return (0 <= index && index < 25);
  }

  bool is_valid_layout(Piece *data) {
    int count[42] = {0};
    for(int i=0; i<25; i++) {
      count[data[i].get_id()]++;
    }
    for(int i=0; i<12; i++) {
      if(count[PIECES[i]] != NUMBER_OF_PIECES[PIECES[i]])
	return false;
    }
    return true;
  }


  Orient convert_player_to_orient(Player player) {
    return static_cast<Orient>(player+1);
  }

  Player convert_orient_to_player(Orient orient) {
    assert(orient != Central);
    return static_cast<Player>(orient-1);
  }

  Cell convert_layout_index_to_cell(int index, CellGroup group /* = South */) {
    assert(is_valid_layout_index(index));
    return Cell(
	        group, Y_OF_INDEX[index], X_OF_INDEX[index], LR_OF_INDEX[index]
	   );
  }

  int convert_cell_to_layout_index(const Cell& cell) {
    assert(cell.get_group() != Central);
    int t = 18*cell.get_lr()+6*(cell.get_x()-1) + cell.get_y()-1;
    // Camp = -1
    return INDEX_OF_CELL[t];
  }

  Layout::Layout(bool masked /* = false */) {
    assert(is_valid_layout(this->data));
    this->masked = masked;
  }

  Layout::Layout(Piece *data) {
    assert(is_valid_layout(data));
    for(int i=0; i<25; i++)
      this->data[i] = data[i];
  }

  bool Layout::is_masked() const {
    return this->masked;
  }

  Piece Layout::get(int index) const {
    assert(!is_masked());
    return data[index];
  }

  Piece Layout::get(int y, int x, LeftRight lr) const {
    assert(!is_masked());
    return data[convert_cell_to_layout_index(Cell(South,y,x,lr))];
  }

  void Layout::swap(int index1, int index2) {
    assert(!is_masked());
    Piece t = data[index1];
    data[index1] = data[index2];
    data[index2] = t;
  }

  std::string Layout::to_string() const {
    std::string result = "";
    if(is_masked()) {
      result = "[Masked]";
    } else {
      for(int y=1; y<=6; y++) {
	for(int t=1; t<=5; t++) {
	  int x;
	  LeftRight lr;
	  if(t <= 3) {
	    x = t;
	    lr = Left;
	  } else if(t == 4) {
	    x = 2;
	    lr = Right;
	  } else {
	    x = 1;
	    lr = Right;
	  }
	  int index = convert_cell_to_layout_index(Cell(South,y,x,lr));
	  if(index == -1)
	    result += "[  ]";
	  else if(data[index].get_id() == 0)
	    result += "[00]";
	  else
	    result += "[" + std::to_string(data[index].get_id()) + "]";
	  result += " ";
	} // for t in 1..5
	result += "\n";
      } // for y in 1..6
    } // masked ?
    return result;
  }

  Layout& Layout::operator = (const Layout& right) {
    for(int i=0; i<25; i++)
      data[i] = right.data[i];
    this->masked = right.masked;
    return *this;
  }

  Element::Element() {
    // empty
    // this->id = 0;
  }

  Element::Element(int id) {
    this->id = id;
  }

  Element::Element(Player player) {
    // unknown
    this->id = 1+player*26+1;
  }

  Element::Element(Player player, int layout_index) {
    assert(is_valid_layout_index(layout_index));
    this->id = 1+player*26+(layout_index+1)+1;
  }

  int Element::get_id() const {
    return id;
  }

  bool Element::is_empty() const {
    return (id == 0);
  }

  Player Element::get_player() const {
    assert(!is_empty());
    return static_cast<Player>((id-1) / 26);
  }

  bool Element::is_unknown() const {
    return (id != 0) && ((id-1)%26 == 0);
  }

  int Element::get_layout_index() const {
    assert(!is_empty());
    assert(!is_unknown());
    return ((id-1)%26 - 1);
  }

  Element& Element::operator = (const Element& right) {
    this->id = right.id;
    return *this;
  }

  Feedback::Feedback() {
    // null
    // this->move_result = Null
  }

  Feedback::Feedback(MoveResult move_result, const std::list<Cell>& route) {
    this->move_result = move_result;
    this->route = route;
  }

  bool is_nothing() const {
    return (move_result == Nothing);
  }

  Feedback& Feedback::operator = (const Feedback& right) {
    this->move_result = right.move_result;
    this->route = right.route;
  }

  void Game::init_board() {
    for(int i=0; i<4; i++) {
      for(int j=0; j<4631; j++) {
	if(is_valid_cell_id(j)) {
	  if(enabled[i]) {
	    int layout_index = convert_cell_to_layout_index(Cell(j));
	    if(layout_index != -1)
	      board[j] = Element(static_cast<Player>(i), layout_index);
	    else
	      board[j] = Element();
	  } else {
	    board[j] = Element();
	  }
	} // if j is a valid cell id
      } // for j in 0..4630
    } // for i in 0..3
  }

  Game::Game(const Layout& layout_S, const Layout& layout_N, bool EW /*=1*/) {
    if(!EW) {
      layout[convert_orient_to_player(South)] = layout_S;
      layout[convert_orient_to_player(North)] = layout_N;
      enabled[convert_orient_to_player(South)] = true;
      enabled[convert_orient_to_player(North)] = true;
    } else {
      layout[convert_orient_to_player(East)] = layout_S;
      layout[convert_orient_to_player(West)] = layout_N;
      enabled[convert_orient_to_player(East)] = true;
      enabled[convert_orient_to_player(West)] = true;
    }
    init_board();
  }

  Game::Game(const Layout* layouts) {
    for(int i=0; i<4; i++) {
      this->layout[i] = layouts[i];
      this->enabled[i] = true;
    }
    init_board();
  }

  Piece Game::get_piece(const Element& element) const {
    assert(!element.is_empty() && !element.is_unknown());
    return layout[element.get_player()].get(element.get_layout_id());
  }

  bool Game::is_movable(Cell from, Cell to) const {
    Element from_element = board[from.get_id()];
    Element to_element = board[from.get_id()];
    assert(!from_element.is_empty() && !from_element.is_unknown());
    if(from.get_type() == Headquarter) {
      return false;
    } else {
      bool occupy_state[4631] = {0};
      for(int i=0; i<4631; i++) {
	if(is_valid_cell_id(i)) {
	  occupy_state[i] = !board[i].is_empty();
	} // if i is a valid cell id
      } // for i in 0..4630
      std::list<Cell> route = get_route(
          from, to, occupy_state, get_piece(from_element) == Piece(32)
      );
      if(route.empty())
	return false;
      else
	return true;
    }
  }

  Feedback Game::move(Cell from, Cell to, MoveResult force_result /*=Null*/) {
    Element from_element = board[from.get_id()];
    Element to_element = board[from.get_id()];
    assert(!from_element.is_empty());
    assert(from.get_type != Headquarter);
    bool occupy_state[4631] = {0};
    for(int i=0; i<4631; i++) {
      if(is_valid_cell_id(i)) {
	occupy_state[i] = !board[i].is_empty();
      }
    }
    MoveResult result;
    std::list<Cell> route;
    if(!from_element.is_unknown() && !to_element.is_unknown()) {
      assert(force_result == Null);
      if(!to_element.is_empty()) {
	result = Piece::attack(get_piece(from_element), get_piece(to_element));
      } else {
	result = Null;
      }
      route = get_route(
	  from, to, occupy_state, get_piece(from_element) == Piece(32)
      );
      assert(!route.empty());
      board[to.get_id()] = from_element;
      board[from.get_id()] = Element();
    } else {
      assert(force_result != Null);
      result = force_result;
      route = get_route(from, to, occupy_state, true);
      assert(!route.empty());
      board[to.get_id()] = from_element;
      board[from.get_id()] = Element();
    }
    return Feedback(result, route);
  }
}
