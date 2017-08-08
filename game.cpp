#include <cstring>
#include <cassert>
#include "game.hpp"


namespace GwanKei {

  bool is_valid_game_piece_id(int id) {
    return (0 <= id && id <= 1+(25+1)*4);
  }

  bool is_valid_layout_index(int index) {
    return (0 <= index && index < 25);
  }

  bool is_valid_layout(const Piece *data) {
    int count[42] = {0};
    for(int i=0; i<25; i++) {
      Cell cell = convert_layout_index_to_cell(i);
      if( (cell.get_y() == 1 && data[i].get_id() == 0)	 
	 || (cell.get_y() < 5 && data[i].get_id() == 41) ) {
	return false;
      }
      // 計數
      count[data[i].get_id()]++;
    }
    
    for(int i=0; i<12; i++) {
      if(count[PIECES[i]] != NUMBER_OF_PIECES[PIECES[i]])
	return false;
    }
    if(data[convert_cell_to_layout_index(Cell(1,6,2,0))] != Piece(31)
       && data[convert_cell_to_layout_index(Cell(1,6,2,1))] != Piece(31))
      return false;
    return true;
  }


  Orient convert_player_to_orient(Player player) {
    return static_cast<Orient>(player+1);
  }

  Player convert_orient_to_player(Orient orient) {
    assert(orient != Central);
    return static_cast<Player>(orient-1);
  }

  Cell convert_layout_index_to_cell(int index, Player player /* = Orange */) {
    assert(is_valid_layout_index(index));
    return Cell(
	        convert_player_to_orient(player),
		Y_OF_INDEX[index],
		X_OF_INDEX[index],
		LR_OF_INDEX[index]
	   );
  }

  int convert_cell_to_layout_index(const Cell& cell) {
    // Central = -2
    if(cell.get_group() == Central)
      return -2;
    // Ordinary
    int t = 18*cell.get_lr()+6*(cell.get_x()-1) + cell.get_y()-1;
    // Camp = -1
    return INDEX_OF_CELL[t];
  }

  Layout::Layout(bool masked /* = false */) {
    assert(is_valid_layout(this->data));
    this->masked = masked;
  }

  Layout::Layout(const Piece *data) {
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

  bool Layout::is_able_to_swap(int index1, int index2) const {
    assert(!is_masked());
    Piece copy[25];
    for(int i=0; i<25; i++)
      copy[i] = data[i];
    copy[index1] = data[index2];
    copy[index2] = data[index1];
    return is_valid_layout(copy);
  }

  void Layout::swap(int index1, int index2) {
    assert(!is_masked());
    assert(is_able_to_swap(index1, index2));
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
    std::copy(right.data, right.data+25, data);
    this->masked = right.masked;
    return *this;
  }

  Element::Element() {
    // empty
    // this->id = 0;
  }

  Element::Element(Player player, int layout_index) {
    assert(is_valid_layout_index(layout_index));
    this->id = 1+player*26+(layout_index+1);
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
    return unknown;
  }

  void Element::set_unknown() {
    unknown = true;
  }

  int Element::get_layout_index() const {
    assert(!is_empty());
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

  bool Feedback::is_nothing() const {
    return (move_result == Nothing);
  }

  std::list<Cell> Feedback::get_route() const {
    return route;
  }

  Feedback& Feedback::operator = (const Feedback& right) {
    this->move_result = right.move_result;
    this->route = right.route;
    return *this;
  }

  void Game::init_board() {
    for(int i=0; i<4; i++) {
      Player player = static_cast<Player>(i);
      for(int j=0; j<25; j++) {
	if(enabled[i]) {
	  Cell cell = convert_layout_index_to_cell(j, player);
	  board[cell.get_id()] = Element(player, j);
	}
      } // for j in 0..24
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

  Element Game::element_of(Cell cell) const {
    return board[cell.get_id()];
  }

  Piece Game::piece_of(Element element) const {
    assert(!element.is_empty() && !element.is_unknown());
    return layout[element.get_player()].get(element.get_layout_index());
  }

  bool Game::is_movable(Cell from, Cell to) const {
    Element from_element = board[from.get_id()];
    Element to_element = board[to.get_id()];
    if(from_element.is_empty() || from_element.is_unknown()) {
      return false;      
    } else if(from.get_type() == Headquarter) {
      return false;
    } else if(!to_element.is_empty() && to.get_type() == Camp) {
      return false;
    } else if(piece_of(from_element) == Piece(41)) {
      return false;
    } else {
      bool occupy_state[4631] = {0};
      for(int i=0; i<4631; i++) {
	if(is_valid_cell_id(i)) {
	  occupy_state[i] = !board[i].is_empty();
	} // if i is a valid cell id
      } // for i in 0..4630
      std::list<Cell> route = get_route(
          from, to, occupy_state, piece_of(from_element) == Piece(32)
      );
      if(route.empty())
	return false;
      else
	return true;
    }
  }

  Feedback Game::move(Cell from, Cell to, MoveResult force_result) {
    Element from_element = board[from.get_id()];
    Element to_element = board[to.get_id()];
    assert(!from_element.is_empty());
    assert(from.get_type() != Headquarter);
    bool occupy_state[4631] = {0};
    for(int i=0; i<4631; i++) {
      if(is_valid_cell_id(i)) {
	occupy_state[i] = !board[i].is_empty();
      }
    }
    MoveResult result;
    std::list<Cell> route;
    if(!from_element.is_unknown() && !to_element.is_unknown()) {
      assert(force_result == Nothing);
      if(!to_element.is_empty()) {
	result = Piece::attack(piece_of(from_element), piece_of(to_element));
      } else {
	result = Nothing;
      }
      route = get_route(
	  from, to, occupy_state, piece_of(from_element) == Piece(32)
      );
      assert(!route.empty());
      if(result == Nothing || result == Bigger) {
	board[to.get_id()] = from_element;
      } else if(result == Equal) {
	board[to.get_id()] = Element();
      }
      board[from.get_id()] = Element();
    } else {
      assert(force_result != Nothing);
      result = force_result;
      route = get_route(from, to, occupy_state, true);
      assert(!route.empty());
      if(result == Nothing || result == Bigger) {
	board[to.get_id()] = from_element;
      } else if(result == Equal) {
	board[to.get_id()] = Element();
      }
      board[from.get_id()] = Element();
    }
    last_feedback = Feedback(result, route);
    return last_feedback;
  }

  Feedback Game::get_last_feedback() const {
    return last_feedback;
  }

  Game Game::get_game_with_mask(Player perspective, MaskMode mask_mode) const {
    Game result = *this;
    auto set_unknown = [&result, perspective](int delta) {
      Player player = static_cast<Player>((perspective+delta)%4);
      result.layout[player] = Layout::Masked();
      for(int j=0; j<4631; j++) {
	if(is_valid_cell_id(j))
	  if(!result.board[j].is_empty()
	     && result.board[j].get_player() == player)
	    result.board[j].set_unknown();
      }
    };
    if(mask_mode == NoExpose) {
      for(int i=1; i<=3; i++) {
	set_unknown(i);
      }
    } else if(mask_mode == DoubleExpose) {
      set_unknown(1);
      set_unknown(3);
    }
    return result;
  }

  Game& Game::operator = (const Game& right) {
    memcpy(board, right.board, 4631*sizeof(Element));
    memcpy(layout, right.layout, 4*sizeof(Layout));
    memcpy(enabled, right.enabled, 4*sizeof(bool));
    last_feedback = right.last_feedback;
    return *this;
  }
}
