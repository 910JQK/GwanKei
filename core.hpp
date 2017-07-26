#include <exception>
#include <string>
#include <list>


namespace GwanKei {

  const char ORIENT[5] = {'C', 'S', 'E', 'N', 'W'};

  const char LR[2] = {'L', 'R'};

  enum Orient {
    Central, South, East, North, West
  };

  typedef Orient CellGroup;

  enum LeftRight {
    Left, Right
  };

  enum CellType {
    Station, Camp, Headquarter
  };

  enum BoundType {
    Sentinel, Ordinary, Railway
  };

  enum AttackResult {
    Bigger, Smaller, Equal
  };

  Orient prev_orient(Orient orient);
  Orient next_orient(Orient orient);
  Orient opposite_orient(Orient orient);

  Orient prev_orient(int orient);
  Orient next_orient(int orient);
  Orient opposite_orient(int orient);

  int get_dec_digit(int num, int pos);
  bool is_valid_cell_id(int id);
  void check_cell_id(int id);
  bool is_valid_piece_id(int id);
  void check_piece_id(int id);

  class InvalidCellException : public std::exception {
    virtual const char* what() const throw()
    {
      return "An illegeal Cell was specified. Please check your program.";
    }
  };
  
  class InvalidPieceException : public std::exception {
    virtual const char* what() const throw()
    {
      return "An illegeal Piece was specified. Please check your program.";
    }
  };
  
  class InvalidOperationException : public std::exception {
    virtual const char* what() const throw()
    {
      return "An illegeal operation was detected. Please check your program.";
    }
  };

  class Cell;
  class Bound;
  
  class Cell {
  private:
    int id;
  public:
    Cell();
    Cell(int id);
    Cell(CellGroup group, int y, int x, LeftRight left_right = Left);
    Cell(int group, int y, int x, int left_right = 0);
    int get_id() const;
    CellGroup get_group() const;
    int get_y() const;
    int get_x() const;
    LeftRight get_lr() const;
    CellType get_type() const;
    std::string to_string() const;
    std::list<Bound> get_adjacents() const;
    bool operator == (const Cell& right) const;
    bool operator != (const Cell& right) const { return !(*this == right); };
    Cell& operator = (const Cell& right);
  };

  class Bound {
  private:
    Cell target;
    BoundType type;
    Orient railway_orient_origin;
    Orient railway_orient_terminal;
  public:
    Bound();
    Bound(Cell target, bool is_sentinel = false);
    Bound(Cell target, Orient railway_orient);
    Bound(Cell target, int railway_orient);
    Bound(
	  Cell target,
	  Orient railway_orient_origin,
	  Orient railway_orient_terminal
	  );
    Bound(
	  Cell target,
	  int railway_orient_origin,
	  int railway_orient_terminal
	  );
    Cell get_target() const;
    BoundType get_type() const;
    Orient get_railway_orient_origin() const;
    Orient get_railway_orient_terminal() const;
    bool is_linkable(const Bound& prev, bool able_to_turn = false) const;
    Bound& operator = (const Bound& right);
  };

  AttackResult attack(int piece, int target);

  struct SearchNode {
    Bound bound;
    std::list<Cell> route;
    int counter;
    SearchNode(Bound bound, std::list<Cell> route, int counter = 0) {
      this->bound = bound;
      this->route = route;
      this->counter = counter;
    }
  };
  
  std::list<Cell> get_route(
			    Cell from,
			    Cell to,
			    bool* occupy_state,
			    bool able_to_turn = false
			    );
}
