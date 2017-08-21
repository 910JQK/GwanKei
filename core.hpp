#ifndef GWANKEI_CORE_HPP
#define GWANKEI_CORE_HPP


#include <string>
#include <list>
#include <cassert>


namespace GwanKei {

  /* 方向，同時也是格子的分組 */
  enum Orient {
    Central, South, East, North, West
    /* 九宮格，南，東，北，西 */
  };

  typedef Orient CellGroup;

  const char ORIENT[5] = {'C', 'S', 'E', 'N', 'W'};

  /* 左右 */
  enum LeftRight {
    Left, Right
    /**
       註：
           座標有左右之分，
	   比如左側的 61 和右側的 61 不同，
	   正好處在相互對稱的位置上。
	   對於位於中軸線上的座標（如 53），
	   一律按左計。
     */
  };

  const char LR[2] = {'L', 'R'};

  /* 格子類型 */
  enum CellType {
    Station, Camp, Headquarter
    /* 兵站，行營，大本營 */
  };

  /* 道路類型 */
  enum BoundType {
    Sentinel, Ordinary, Railway
    /* 哨位，公路，鐵路 */
    /**
       註：
           類型爲「哨位」的道路是尋路時所用的起點，
	   用在隊列（佇列）中的第一個節點上，可與任意其他道路連接
     */
  };

  /* 碰子 / 移動 結果 */
  enum MoveResult {
    Nothing, Bigger, Smaller, Equal
    /* 無，吃，碰死，打對 */
  };

  typedef MoveResult AttackResult;

  /* 南東北西，逆時針轉 */
  Orient prev_orient(Orient orient);
  Orient next_orient(Orient orient);
  Orient opposite_orient(Orient orient);

  Orient prev_orient(int orient);
  Orient next_orient(int orient);
  Orient opposite_orient(int orient);

  int get_dec_digit(int num, int pos);

  bool is_valid_orient(int orient);
  bool is_valid_cell_id(int id);
  bool is_valid_piece_id(int id);

  class Cell;
  class Bound;

  /* 格子 */
  class Cell {
  private:
    int id; // 四位十進制數格子 ID, 詳見 implement
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
    Cell get_top() const;
    Cell get_bottom() const;
    Cell get_left() const;
    Cell get_right() const;
    std::string to_string() const;
    std::list<Bound> get_adjacents() const; // 取得鄰接的格子（道路）
    bool operator == (const Cell& right) const;
    bool operator != (const Cell& right) const { return !(*this == right); };
    Cell& operator = (const Cell& right);
  };

  /* 道路 */
  class Bound {
  private:
    Cell target;
    BoundType type;
    Orient railway_orient_origin = Central; // 起點鐵路方向
    Orient railway_orient_terminal = Central; // 終點鐵路方向
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
    /**
       is_linkable() 用於判斷該道路可否與上一條道路相連
       若棋子是工兵（可以轉向，able_to_turn == true），
       則只要求兩條路都是鐵路；
       若棋子不是工兵，不可以轉向，
       就必須要求兩段鐵路平滑連接，
       即 prev.railway_orient_terminal == this->railway_orient_origin
     */
    Bound& operator = (const Bound& right);
  };

  /* 尋路用的節點 */
  struct SearchNode {
    Bound bound; // 當前道路
    std::list<Cell> route; // 記錄下走過的所有格子
    SearchNode(Bound bound, std::list<Cell> route) {
      this->bound = bound;
      this->route = route;
    }
  };

  /* 尋路 */
  std::list<Cell> get_route(
			    Cell from, // 起點
			    Cell to, // 終點
			    bool* occupy_state, // 各個格子的佔用狀態
			    /* 註： ^此處鍵爲格子的四位數 ID */
			    bool able_to_turn = false // 工兵？
			    );

  /* 棋子 */
  class Piece {
  private:
    int id; // 兩位 ID, 40, 39, 38 ... 詳見 implement
  public:
    Piece();
    Piece(int id);
    int get_id() const;
    /* 碰子結果判定 */
    static AttackResult attack(const Piece& piece, const Piece& target);
    Piece& operator = (const Piece& right);
    bool operator == (const Piece& right) const;
    bool operator != (const Piece& right) const { return !(*this == right); };
  };

  bool is_valid_orient(int orient);
  bool is_valid_cell_id(int id);
  bool is_valid_piece_id(int id);

}


#endif // GWANKEI_CORE_HPP
