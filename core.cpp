#include <queue>
#include "core.hpp"


#ifdef DEBUG
#include <iostream>
#endif


namespace GwanKei {

  /**
       Position Indicator (Cell Identity)

       Ordinary (G,Y,X,R):

       (G=Group, S=1, E=2, N=3, W=4)

       |-----|--------------------------------------------|
       | y\x |    1        2        3        2        1   |      
       |-----|--------------------------------------------|
       |  1  | [G110] = [G120] = [G130] = [G121] = [G111] |
       |     |   ||   \   |    /   |    \    |   /   ||   |
       |  2  | [G210] - (G220) - [G230] - (G221) - [G211] |
       |     |   ||   /   |    \   |    /    |   \   ||   |
       |  3  | [G310] - [G320] - [G330] - [G321] - [G311] |
       |     |   ||   \   |    /   |    \    |   /   ||   |
       |  4  | [G410] - (G420) - [G430] - (G421) - [G411] |
       |     |   ||   /   |    \   |    /    |   \   ||   |
       |  5  | [G510] = [G520] = [G530] = [G521] = [G511] |
       |     |   |        |        |         |        |   |
       |  6  | [G610] - {G620} - [G630] - {G621} - [G611] |
       |     |         L  E  F  T       |    R I G H T    |
       |-----|--------------------------------------------|

       Central (0,Y,X,0):

       (Group=None=0, LeftRight=Undefined=0)

       N->W [N] E->N
        [W] [0] [E]
       W->S [S] S->E

       (S,E,N,W = 1,2,3,4)

           ||       ||       ||
       = [0340] = [0330] = [0230] =
           ||       ||       ||
       = [0440] = [0000] = [0220] =
           ||       ||       ||
       = [0410] = [0110] = [0120] =
           ||       ||       ||

  */

  /**
     Piece Indicator
     
     41 | 地雷
     40 | 司令
     .. | ・・
     32 | 工兵
     00 | 炸彈

   */

  Orient prev_orient(Orient orient) {
    return static_cast<Orient>( ((orient-1)+(-1+4))%4+1 );
  }

  Orient next_orient(Orient orient) {
    return static_cast<Orient>( ((orient-1)+1)%4+1 );
  }

  Orient opposite_orient(Orient orient) {
    return static_cast<Orient>( ((orient-1)+2)%4+1 );
  }

  Orient prev_orient(int orient) {
    return static_cast<Orient>( ((orient-1)+(-1+4))%4+1 );
  }

  Orient next_orient(int orient) {
    return static_cast<Orient>( ((orient-1)+1)%4+1 );
  }

  Orient opposite_orient(int orient) {
    return static_cast<Orient>( ((orient-1)+2)%4+1 );
  }

  int get_dec_digit(int num, int pos) {
    for(int i=0; i<pos; i++) {
      num /= 10;
    }
    return num % 10;
  }

  bool is_valid_cell_id(int id) {
    int digit[4];
    for(int i=0; i<4; i++)
      digit[i] = get_dec_digit(id, i);
    if(id > 9999 || id < 0)
      return false;
    if(digit[0] != 0 && digit[0] != 1)
      return false;
    if(digit[3] == 0) {
      if(id == 0) {
	return true;
      } else if(digit[2] >= 1 && digit[2] <= 4) {
	if (digit[1] == ((digit[2]-1)+1)%4+1)
	  return true;
	else if(digit[1] == digit[2])
	  return true;
	else
	  return false;
      } else {
	return false;
      }
    } else if(digit[3] >= 1 && digit[3] <= 4) {
      if(digit[2] >= 1 && digit[2] <= 6 && digit[1] >= 1 && digit[1] <= 3)
	return true;
      else
	return false;
    } else {
      return false;
    }
  }

  void check_cell_id(int id) {
    if(!is_valid_cell_id(id)) {
      #ifdef DEBUG
      std::cerr << "!!! Invalid ID " << id << '\n';
      #endif
      throw InvalidCellException();
    }
  }

  bool is_valid_piece_id(int id) {
    if((id >= 32 && id <= 41) || id == 0)
      return true;
    else
      return false;    
  }

  void check_piece_id(int id) {
    if(!is_valid_piece_id(id))
      throw InvalidPieceException();
  }

  Cell::Cell() {
    *this = Cell(0);
  }

  Cell::Cell(int id) {
    check_cell_id(id);
    this->id = id;
  }

  Cell::Cell(CellGroup group, int y, int x, LeftRight left_right /* =Left */) {
    if(group != Central && x == 3 && left_right == Right)
      left_right = Left;
    else if(group == Central)
      left_right = Left;
    this->id = group*1000+y*100+x*10+left_right;
    check_cell_id(this->id);
  }

  Cell::Cell(int group, int y, int x, int left_right /* = 0 */ ) {
    *this = Cell(
     static_cast<CellGroup>(group), y, x, static_cast<LeftRight>(left_right)
    );
  }

  int Cell::get_id() const {
    return this->id;
  }

  CellGroup Cell::get_group() const {
    return static_cast<CellGroup>(get_dec_digit(id, 3));
  }

  int Cell::get_y() const {
    return get_dec_digit(id, 2);
  }

  int Cell::get_x() const {
    return get_dec_digit(id, 1);
  }

  LeftRight Cell::get_lr() const {
    return static_cast<LeftRight>( get_dec_digit(id, 0) );
  }

  CellType Cell::get_type() const {
    int y = get_y();
    int x = get_x();
    if(get_group() == Central)
      return Station;
    else {
      if(y == 6 && x == 2)
	return Headquarter;
      else if((y == 2 || y == 4) && x == 2)
	return Camp;
      else
	return Station;
    }
  }

  std::string Cell::to_string() const {
    std::string result = "";
    CellGroup group = get_group();
    int y = get_y();
    int x = get_x();
    LeftRight lr = get_lr();
    result += ORIENT[group];
    if(group == Central) {
      if(x != y) {
	result += ORIENT[y];
	result += ORIENT[x];
      } else {
	result += ORIENT[y];
      }
    } else {
      result += ('0'+y);
      result += ('0'+x);
      if(x != 3)
	result += LR[lr];
    }
    return result;
  }

  std::list<Bound> Cell::get_adjacents() const {
    CellGroup group = get_group();
    int y = get_y();
    int x = get_x();
    LeftRight lr = get_lr();
    std::list<Bound> result;
    if(group == Central) {
      if(y == 0 && x == 0) {
	/**
	            ||
	        = [0000] =
		    ||
	*/
	result.push_back(Bound(Cell(110), South));
	result.push_back(Bound(Cell(220), East));
	result.push_back(Bound(Cell(330), North));
	result.push_back(Bound(Cell(440), West));
      } else if (y == x) {
	/**
	                  [0000]
	                    ||
	     [0(y-1)y0] = [0yy0] = [0y(y+1)0]
	                    ||
	                  [y130]
			   
	*/
	result.push_back(Bound(Cell(0), opposite_orient(y)) );
	result.push_back(
		  Bound(Cell(Central, y, next_orient(y)), next_orient(y)) );
	result.push_back(
		  Bound(Cell(Central, prev_orient(y), y), prev_orient(y)) );
	result.push_back(Bound(Cell(y, 1, 3), y));
      } else {
	/**
	     [0yy0]
	       ||
	     [0yx0] = [0xx0]
	*/
	result.push_back(Bound(Cell(Central, y, y), opposite_orient(x)) );
	result.push_back(Bound(Cell(Central, x, x), opposite_orient(y)) );
	/**
	     [y111] = [0yx0]
	                ||
	              [x110]
	*/
	result.push_back(Bound(Cell(y, 1, 1, Right), y));
	result.push_back(Bound(Cell(x, 1, 1, Left), x));
      }
    } else {
      if(x == 1) {
	/* Horizontal */
	if(y == 1 || y == 5) {
	  /* Horizontal Railway (to y,2) */
	  /**
	       [11] = [12]
	        ..
	       [51] = [52]
	        ..
	  */
	  result.push_back(
		      Bound(
			    Cell(group, y, 2, lr),
			    lr? prev_orient(group): next_orient(group)
			    )
		      );
	} else {
	  /* Horizontal Road (to y,2) */
	  /**
	       [11]
	       [21] - (22)
	       [31] - [32]
	       [41] - (42)
	       [51]
	       [61] - {62}
	  */
	  result.push_back(Bound(Cell(group, y, 2, lr)) );
	}
	/* Vertical*/
	if(y >= 1 && y <= 4) {
	  /* Downward Railway (to y+1,1) */
	  /**
	       [11]
	        |/
	       [21]
	        |/
	       [31]
	        |/
	       [41]
	        |/
	       [51]
	  */
	  result.push_back(Bound(Cell(group, y+1, 1, lr), group));
	}
	if(y >= 2 && y <= 5) {
	  /* Upward Railway (to y-1,1) */
	  /**
	       [11]
	        |\
	       [21]
	        |\
	       [31]
	        |\
	       [41]
	        |\
	       [51]
	  */
	  result.push_back(Bound(Cell(group, y-1, 1, lr),
				 opposite_orient(group)) );
	}
	if(y == 1) {
	  /* Upward Railway (to Central) */
	  /**
	       [0(G-1)G0] ... [0G(G+1)0]
	           ||             ||
	       [G  11  L] ... [G  11  R]
	     */
	  if(lr == Left) {
	    result.push_back(
			Bound(
			      Cell(Central, prev_orient(group), group),
			      opposite_orient(group)
			      )
			);
	  } else {
	    result.push_back(
			Bound(
			      Cell(Central, group, next_orient(group)),
			      opposite_orient(group)
			      )
			);
	  }
	  /* Side Railway (to 11 of next and prev) */
	  /**
	       [(G-1)11R] ..................... [(G+1)11L]
	                \\                     //
	             [G  11  L] ......... [G  11  R]
	  */
	  if(lr == Right) {
	    result.push_back(
			     Bound(
				   Cell(next_orient(group), 1, 1, Left),
				   opposite_orient(group),
				   next_orient(group)
				   )
			     );
	  } else {
	    result.push_back(
			     Bound(
				   Cell(prev_orient(group), 1, 1, Right),
				   opposite_orient(group),
				   prev_orient(group)
				   )
			     );
	  }
	}
	if(y == 5) {
	  /* Road to 61 */
	  /**
	       [51]
	        |
	       [61]
	  */
	  result.push_back(Bound(Cell(group, 6, 1, lr)) );
	}
	/* Oblique */
	if(y == 1 || y == 3) {
	  /* Road to 22 */
	  /**
	       [11]
	           \
		    (22)
	           /
	       [31]
	  */
	  result.push_back(Bound(Cell(group, 2, 2, lr)) );
	}
	if(y == 3 || y == 5) {
	  /* Road to 42 */
	  /**
	       [31]
	           \
		    (42)
	           /
	       [51]
	  */
	  result.push_back(Bound(Cell(group, 4, 2, lr)) );
	}
      } else if (x == 2) {
	/* Horizontal */
	if(y == 1 || y == 5) {
	  /* Horizontal Railway */
	  /**
	       [11] = [12] = [13]
	        ..     ..     ..
	       [51] = [52] = [53]
	  */
	  result.push_back(
		      Bound(
			    Cell(group, y, 1, lr),
			    lr? next_orient(group): prev_orient(group)
			    )
		      );	    
	  result.push_back(
		      Bound(
			    Cell(group, y, 3, lr),
			    lr? prev_orient(group): next_orient(group)
			    )
		      );	    
	} else {
	  /* Horizontal Road to y,1 and y,3 */
	  /**
	        ..     ..     ..
	       [21] - (22) - [23]
	       [31] - [32] - (33)
	       [41] - (42) - [43]
	        ..     ..     ..
	       [61] - {62} - [63]
	  */
	  result.push_back(Bound(Cell(group, y, 1, lr)) );
	  result.push_back(Bound(Cell(group, y, 3, lr)) );
	}
	/* Vertical */
	if(y >= 2 && y <= 6) {
	  /* Upward Road */
	  /**
	       [12]
	        ^
	       (22)
	        ^
	       [32]
	        ^
	       (42)
	        ^
	       [52]
	        ^
	       {62}
	  */
	  result.push_back(Bound(Cell(group, y-1, 2, lr)) );
	}
	if(y >= 1 && y <= 5) {
	  /* Downward Road */
	  /**
	       [12]
	        \
	       (22)
	        \
	       [32]
	        \
	       (42)
	        \
	       [52]
	        \
	       {62}
	  */
	  result.push_back(Bound(Cell(group, y+1, 2, lr)) );
	}
	/* Oblique */
	if(y == 2 || y == 4) {
	  /* Roads from Camp */
	  /**
	       [11]  ..  [13]
	           \    /
	        ..  (22)  ..
	           /    \
	       [31]  ..  (33)
	  */
	  result.push_back(Bound(Cell(group, y+1, x+1, lr)) );
	  result.push_back(Bound(Cell(group, y+1, x-1, lr)) );
	  result.push_back(Bound(Cell(group, y-1, x+1, lr)) );
	  result.push_back(Bound(Cell(group, y-1, x-1, lr)) );
	}
      } else /* if (x == 3) */ {
	/* Horizontal */
	if(y == 1 || y == 5) {
	  /* Horizontal Railway to y,2 */
	  /**
	       [12L] = [13] = [12R]
	        ...     ..     ...
	       [52L] = [53] = [52R]
	        ...     ..     ...
	  */
	  result.push_back(Bound(Cell(group, y, 2, Left), prev_orient(group)) );
	  result.push_back(Bound(Cell(group, y, 2, Right), next_orient(group)) );
	} else {
	  /* Horizontal Road to y,2 */
	  /**
	        ...     ..     ...
	       (22L) - [23] - (22R)
	       [32L] - (33) - [32R]
	       (42L) - [43] - (42R)
	        ...     ..     ...
	       {62L} - [63] - {62R}
	  */
	  result.push_back(Bound(Cell(group, y, 2, Left)) );
	  result.push_back(Bound(Cell(group, y, 2, Right)) );
	}
	/* Vertical */
	if(y == 1) {
	  /* Railway to Central */
	  /**
	       [0GG0]
	         ||
	       [G130]
	  */
	  result.push_back(
		      Bound(
			    Cell(Central, group, group),
			    opposite_orient(group)
			    )
		      );
	}
	if(y >= 2 && y <= 6) {
	  /* Upward Road */
	  /**
	       [13]
	        ^
	       [23]
	        ^
	       (33)
	        ^
	       [43]
	        ^
	       [53]
	        ^
	       [63]
	  */
	  result.push_back(Bound(Cell(group, y-1, 3)) );
	}
	if(y >= 1 && y <= 5) {
	  /* Downward Road */
	  /**
	       [13]
	        \
	       [23]
	        \
	       (33)
	        \
	       [43]
	        \
	       [53]
	        \
	       {63}
	  */
	  result.push_back(Bound(Cell(group, y+1, 3)) );
	}
	/* Oblique */
	if(y == 1) {
	  /* 13 to 22 */
	  /**
	        ... [13] ...
		   /    \
	       (22L)    (22R)
	  */
	  result.push_back(Bound(Cell(group, y+1, x-1, Left)) );
	  result.push_back(Bound(Cell(group, y+1, x-1, Right)) );
	}
	if(y == 5) {
	  /* 53 to 42 */
	  /**
	       (42L)    (42R)
	           \    /
	        ... [53] ...
	  */
	  result.push_back(Bound(Cell(group, y-1, x-1, Left)) );
	  result.push_back(Bound(Cell(group, y-1, x-1, Right)) );
	}
	if(y == 3) {
	  /* Center Camp */
	  /**
	       (22L) .. (22R)
	           \    /
	        ... (33) ...
	           /    \
	       (42L) .. (42R)
	  */
	  result.push_back(Bound(Cell(group, y+1, x-1, Left)) );
	  result.push_back(Bound(Cell(group, y-1, x-1, Left)) );
	  result.push_back(Bound(Cell(group, y+1, x-1, Right)) );
	  result.push_back(Bound(Cell(group, y-1, x-1, Right)) );
	}
      } // x = ???
    } // Central or Others ?
    return result;
  }
  bool Cell::operator == (const Cell& right) const {
    return (this->id == right.id);
  }
  Cell& Cell::operator = (const Cell& right) {
    this->id = right.id;
    return *this;
  }

  Bound::Bound() {
    *this = Bound(Cell(), true);
  }

  Bound::Bound(Cell target, bool is_sentinel /* = false */) {
    this->target = target;
    if(is_sentinel)
      this->type = Sentinel;
    else
      this->type = Ordinary;
  }

  Bound::Bound(Cell target, Orient railway_orient) {
    this->target = target;
    this->type = Railway;
    this->railway_orient_origin
      = this->railway_orient_terminal
      = railway_orient;
  }

  Bound::Bound(Cell target, int railway_orient) {
    *this = Bound(target, static_cast<Orient>(railway_orient));
  }

  Bound::Bound(
	       Cell target,
	       Orient railway_orient_origin,
	       Orient railway_orient_terminal
	       ) {
    this->target = target;
    this->type = Railway;
    this->railway_orient_origin = railway_orient_origin;
    this->railway_orient_terminal = railway_orient_terminal;
  }

  Bound::Bound(
	       Cell target,
	       int railway_orient_origin,
	       int railway_orient_terminal
	       ) {
    *this = Bound(
	  target,
	  static_cast<Orient>(railway_orient_origin),
	  static_cast<Orient>(railway_orient_terminal)
	  );
  }

  Cell Bound::get_target() const {
    return this->target;
  }

  BoundType Bound::get_type() const {
      return this->type;
  }

  Orient Bound::get_railway_orient_origin() const {
    if(this->type != Railway)
      throw InvalidOperationException();
    return this->railway_orient_origin;
  }

  Orient Bound::get_railway_orient_terminal() const {
    if(this->type != Railway)
      throw InvalidOperationException();
    return this->railway_orient_terminal;
  }

  bool Bound::is_linkable(const Bound& prev, bool able_to_turn
			  /* = false */) const {
    const Bound& next = *this;
    if(prev.type == Railway && next.type == Railway) {
      if(able_to_turn)
	return true;
      else if(prev.railway_orient_terminal == next.railway_orient_origin)
	return true;
      else
	return false;
    } else if(prev.type == Sentinel) {
      return true;
    } else /* if (prev.type == Ordinary) */ {
      return false;
    }
  }

  Bound& Bound::operator = (const Bound& right) {
    Bound& left = *this;
    left.target = right.get_target();
    left.type = right.get_type();
    if(right.type == Railway) {
      left.railway_orient_origin = right.get_railway_orient_origin();
      left.railway_orient_terminal = right.get_railway_orient_terminal();
    }
    return left;
  }

  AttackResult attack(int piece, int target) {
    check_piece_id(piece);
    check_piece_id(target);
    if(piece == 0 || target == 0)
      return Equal;
    else if(piece == 32 && target == 41)
      return Bigger;
    else if(piece > target)
      return Bigger;
    else if(piece == target)
      return Equal;
    else /* if(piece < target) */
      return Smaller;
  }

  std::list<Cell> get_route(
			    Cell from,
			    Cell to,
			    bool *occupy_state,
			    bool able_to_turn /* = false */
			    ) {
    std::queue<SearchNode> queue;
    bool visited[4999] = {0};

    #ifdef DEBUG
    std::cerr << "Search Route for "
	      << from.to_string() << " -> " << to.to_string()
	      << " with a piece "<< (able_to_turn? "able": "unable")
	      << " to turn" << '\n';
    #endif

    std::list<Cell> initial_route;
    initial_route.push_back(from);
    Bound initial_bound = Bound(from, true);
    queue.push(SearchNode(initial_bound, initial_route));

    while(!queue.empty()) {
      SearchNode node = queue.front();
      queue.pop();
      #ifdef DEBUG
      std::cerr << "Shift " << node.bound.get_target().to_string() << '\n';
      #endif
      
      std::list<Bound> new_bounds = node.bound.get_target().get_adjacents();
      for(auto I=new_bounds.begin(); I!=new_bounds.end(); I++) {
	Bound& new_bound = *I;
	if(!(
	     occupy_state[new_bound.get_target().get_id()]
	     && new_bound.get_target() != to
	     )
	   && new_bound.get_target() != from
	   && !visited[new_bound.get_target().get_id()]
	   && new_bound.is_linkable(node.bound, able_to_turn)
	   ) {
	  std::list<Cell> route = node.route;
	  route.push_back(new_bound.get_target());
	  if(new_bound.get_target() == to) {
	    #ifdef DEBUG
	    bool first = true;	    
	    for(auto I=route.begin(); I!=route.end(); I++) {
	      if(!first)
		std::cerr << " -> ";
	      std::cerr << I->to_string();
	      first = false;
	    }
	    std::cerr << '\n';
	    #endif
	    return route;
	  }
	  queue.push(SearchNode(new_bound, route, node.counter+1));
	  visited[new_bound.get_target().get_id()] = true;
	  #ifdef DEBUG
	  std::cerr << "Push " << new_bound.get_target().to_string() << '\n';
	  #endif
	} // if
      } // for
    } // while not empty
    #ifdef DEBUG
    std::cerr << "No route found" << '\n';
    #endif
    return std::list<Cell>();
  }
}
