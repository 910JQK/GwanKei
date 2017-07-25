#include <list>


namespace GwanKei {

  int get_dec_digit(int num, int pos) {
    for(int i=0; i<pos; i++) {
      num /= 10;
    }
    return num % 10;
  }

  enum Orient {
    None, South, East, North, West
  };

  Orient prev_orient(Orient orient) {
    return ((orient-1)+(-1+4))%4+1;
  }

  Orient next_orient(Orient orient) {
    return ((orient-1)+1)%4+1;
  }

  Orient opposite_orient(Orient orient) {
    return ((orient-1)+2)%4+1;
  }

  enum LeftRight {
    Left, Right
  };

  enum CellGroup {
    Central, South, East, North, West
  };

  enum CellType {
    Ordinary, Camp, Headquarter
  };

  enum BoundType {
    Ordinary, Railway
  };

  struct Bound;
  struct Cell;
  
  struct Cell {
    /**
       Position Indicator (Cell Identity)

       Ordinary (G,Y,X,R):

       (G=Group, S=1, E=2, N=3, W=4)

       ----|--------------------------------------------|
       y\x |    1        2        3        2        1   |      
       ----|--------------------------------------------|
       1   | [G110] = [G120] = [G130] = [G121] = [G111] |
           |   ||   \   |    /   |    \    |   /   ||   |
       2   | [G210] - (G220) - [G230] - (G221) - [G211] |
           |   ||   /   |    \   |    /    |   \   ||   |
       3   | [G310] - [G320] - [G330] - [G321] - [G311] |
           |   ||   \   |    /   |    \    |   /   ||   |
       4   | [G410] - (G420) - [G430] - (G421) - [G411] |
           |   ||   /   |    \   |    /    |   \   ||   |
       5   | [G510] = [G520] = [G530] = [G521] = [G511] |
           |   |        |        |         |        |   |
       6   | [G610] - {G620} - [G630] - {G621} - [G611] |
           |         L  E  F  T       |    R I G H T    |
       ----|--------------------------------------------|

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
    int id;
    Cell(int id) {
      this->id = id;
    }
    Cell(CellGroup group, int y, int x, LeftRight left_right = Left) {
      if(group != Central && x == 3 && left_right = Right)
	left_right = 0;
      else if(group == Central)
	left_right = 0;
      this->id = group*1000+y*100+x*10+left_right;
    }
    CellGroup get_group() const {
      return get_dec_digit(id, 3);
    }
    int get_y() const {
      return get_dec_digit(id, 2);
    }
    int get_x() const {
      return get_dec_digit(id, 1);
    }
    LeftRight get_lr() const {
      return get_dec_digit(id, 0);
    }
    CellType get_type() const {
      int y = get_y();
      int x = get_x();
      if(get_group() == Central)
	return Ordinary;
      else {
	if(y == 6 && x == 2)
	  return Headquarter;
	else if((y == 2 || y == 4) && x == 2)
	  return Camp;
	else
	  return Ordinary;
      }
    }
    std::list<Bound> get_adjacents() const {
      CellGroup group = get_group();
      int y = get_y();
      int x = get_x();
      LeftRight lr = get_lr();
      std::list result;
      if(group == Central) {
	if(y == 0 && x == 0) {
	  /**
	            ||
	        = [0000] =
		    ||
	   */
	  result.push(Bound(Cell(0110), South));
	  result.push(Bound(Cell(0220), East));
	  result.push(Bound(Cell(0330), North));
	  result.push(Bound(Cell(0440), West));
	} else if (y == x) {
	  /**
	                  [0000]
	                    ||
	     [0(y-1)y0] = [0yy0] = [0y(y+1)0]
	                    ||
	                  [y130]
			   
	   */
	  result.push(Bound(Cell(0000), opposite_orient(y)) );
	  result.push(Bound(Cell(0, y, next_orient(y)), next_orient(y)) );
	  result.push(Bound(Cell(0, prev_orient(y), y), prev_orient(y)) );
	  result.push(Bound(Cell(y, 1, 3, 0), y));
	} else {
	  /**
	     [0yy0]
	       ||
	     [0yx0] = [0xx0]
	   */
	  result.push(Bound(Cell(0, y, y), opposite_orient(x)) );
	  result.push(Bound(Cell(0, x, x), opposite_orient(y)) );
	  /**
	     [y111] = [0yx0]
	                ||
	              [x110]
	   */
	  result.push(Bound(Cell(y, 1, 1, Right), y));
	  result.push(Bound(Cell(x, 1, 1, Left), x));
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
	    result.push(
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
	    result.push(Bound(Cell(group, y, 2, lr)) );
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
	    result.push(Bound(Cell(group, y+1, 1, lr), group));
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
	    result.push(Bound(Cell(group, y-1, 1, lr), group));
	  }
	  if(y == 1) {
	    /* Upward Railway (to Central) */
	    /**
	       [0(G-1)G0] ... [0G(G+1)0]
	           ||             ||
	       [G  11  L] ... [G  11  R]
	     */
	    if(get_lr() == Left) {
	      result.push(
			  Bound(
				Cell(0, prev_orient(group), group),
				opposite_orient(group)
				)
			  );
	    } else {
	      result.push(
			  Bound(
				Cell(0, group, next_orient(group)),
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
	    result.push(
			Bound(
			      Cell(next_orient(group), 1, 1, Left),
			      opposite_orient(group),
			      next_orient(group)
			      )
			);
	    result.push(
			Bound(
			      Cell(prev_orient(group), 1, 1, Right),
			      opposite_orient(group),
			      prev_orient(group)
			      )
			);
	  }
	  if(y == 5) {
	    /* Road to 61 */
	    /**
	       [51]
	        |
	       [61]
	     */
	    result.push(Bound(Cell(group, 6, 1, lr)) );
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
	    result.push(Bound(Cell(group, 2, 2, lr)) );
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
	    result.push(Bound(Cell(group, 4, 2, lr)) );
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
	    result.push(
			Bound(
			      Cell(group, y, 1, lr),
			      lr? next_orient(group): prev_orient(group)
			      )
			);	    
	    result.push(
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
	    result.push(Bound(Cell(group, y, 1, lr)) );
	    result.push(Bound(Cell(group, y, 3, lr)) );
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
	    result.push(Bound(Cell(group, y-1, 2, lr)) );
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
	    result.push(Bound(Cell(group, y+1, 2, lr)) );
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
	    result.push(Bound(Cell(group, y+1, x+1, lr)) );
	    result.push(Bound(Cell(group, y+1, x-1, lr)) );
	    result.push(Bound(Cell(group, y-1, x+1, lr)) );
	    result.push(Bound(Cell(group, y-1, x-1, lr)) );
	  }
	} else {
	  /* (x == 3) */
	  /* Horizontal */
	  if(y == 1 || y == 5) {
	    /* Horizontal Railway to y,2 */
	    /**
	       [12L] = [13] = [12R]
	        ...     ..     ...
	       [52L] = [53] = [52R]
	        ...     ..     ...
	     */
	    result.push(Bound(Cell(group, y, 2, Left), prev_orient(group)) );
	    result.push(Bound(Cell(group, y, 2, Right), next_orient(group)) );
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
	    result.push(Bound(Cell(group, y, 2, Left)) );
	    result.push(Bound(Cell(group, y, 2, Right)) );
	  }
	  /* Vertical */
	  if(y == 1) {
	    /* Railway to Central */
	    /**
	       [0GG0]
	         ||
	       [G130]
	     */
	    result.push(
			Bound(
			      Cell(Central, group, group, 0),
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
	    result.push(Bound(Cell(group, y-1, 3)) );
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
	    result.push(Bound(Cell(group, y+1, 3)) );
	  }
	  /* Oblique */
	  if(y == 1) {
	    /* 13 to 22 */
	    /**
	        ... [13] ...
		   /    \
	       (22L)    (22R)
	     */
	    result.push(Bound(Cell(group, y+1, x-1, Left)) );
	    result.push(Bound(Cell(group, y+1, x-1, Right)) );
	  }
	  if(y == 1) {
	    /* 53 to 42 */
	    /**
	       (42L)    (42R)
	           \    /
	        ... [53] ...
	     */
	    result.push(Bound(Cell(group, y-1, x-1, Left)) );
	    result.push(Bound(Cell(group, y-1, x-1, Right)) );
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
	    result.push(Bound(Cell(group, y+1, x-1, Left)) );
	    result.push(Bound(Cell(group, y-1, x-1, Left)) );
	    result.push(Bound(Cell(group, y+1, x-1, Right)) );
	    result.push(Bound(Cell(group, y-1, x-1, Right)) );
	  }
	} // x = ???
      } // Central or Others ?
    }
    bool operator == (const Cell& right) const {
      return (this->id == right.id);
    }
  };

  struct Bound {
    Cell target;
    BoundType type;
    Orient railway_orient_origin;
    Orient railway_orient_terminal;
    Bound(Cell target) {
      this->target = target;
      this->type = Ordinary;
    }
    Bound(Cell target, Orient railway_orient) {
      this->target = target;
      this->type = Railway;
      this->railway_orient_origin
	= this->railway_orient_terminal
	= railway_orient;
    }
    Bound(
	  Cell target,
	  Orient railway_orient_origin,
	  Orient railway_orient_terminal
	  ) {
      this->target = target;
      this->type = Railway;
      this->railway_orient_origin = railway_orient_origin;
      this->railway_orient_terminal = railway_orient_terminal;
    };
  };

}
