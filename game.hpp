#ifndef GWANKEI_GAME_HPP
#define GWANKEI_GAME_HPP


#include "core.hpp"


namespace GwanKei {

  /**
     Layout Index

     [05] = [11] = [00] = [21] = [15]
      ||  \      /  |   \      /  ||
     [06] - (  ) - [01] - (  ) - [16]
      ||  /      \  |   /      \  ||
     [07] - [12] - (  ) - [22] - [17]
      ||  \      /  |   \      /  ||
     [08] - (  ) - [02] - (  ) - [18]
      ||  /      \  |   /      \  ||
     [09] = [13] = [03] = [23] = [19]
      |      |      |       |      |
     [10] - {14} - [04] - {24} - [20]

   */

  const int PIECES[12] = {00,31,32,33,34,35,36,37,38,39,40,41};
  
  const int NUMBER_OF_PIECES[42] = {
    2, // [00]*1
    0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, // [29]
    0, // [30]
    1, // [31]*1
    3, // [32]*3
    3, // [33]*3
    3, // [34]*3
    2, // [35]*2
    2, // [36]*2
    2, // [37]*2
    2, // [38]*2
    1, // [39]*1
    1, // [40]*1
    3, // [41]*3
    /* total = 25 */
  };

  const int Y_OF_INDEX[25] = {
    1,2,4,5,6,
    1,2,3,4,5,6,
    1,3,5,6,
    1,2,3,4,5,6,
    1,3,5,6
  };

  const int X_OF_INDEX[25] = {
    3,3,3,3,3,
    1,1,1,1,1,1,
    2,2,2,2,
    1,1,1,1,1,1,
    2,2,2,2
  };

  const LeftRight LR_OF_INDEX[25] = {
    Left, Left, Left, Left, Left,
    Left, Left, Left, Left, Left, Left,
    Left, Left, Left, Left,
    Right, Right, Right, Right, Right, Right,
    Right, Right, Right, Right
  };

  const int INDEX_OF_CELL[30] = {
    // 18*lr + 6*(x-1) + (y-1)
     5, 6, 7, 8, 9,10,
    11,-1,12,-1,13,14,
     0, 1,-1, 2, 3, 4,
    15,16,17,18,19,20,
    21,-1,22,-1,23,24
  };

  /* 佈局 */
  class Layout {
  private:
    Piece data[25] = {
      37,32,32,36,35,
      35,38,36,34,34,41,
      40,00,38,31,
      37,00,39,33,41,33,
      32,34,41,33
    };
    bool masked = false; // true: 未知佈局（顯示為無字棋子）
  public:
    Layout(bool masked = false);
    Layout(const Piece* data);
    bool is_masked() const;
    Piece get(int index) const;
    Piece get(int y, int x, LeftRight lr) const;
    bool is_able_to_swap(int index1, int index2) const;
    void swap(int index1, int index2); // 交換位置
    std::string to_string() const;
    Layout& operator = (const Layout& right);
    static Layout Masked() { return Layout(true); };
  };

  /* 玩家，與 1,2,3,4 of Orient / CellGroup 對應 */
  enum Player {
    Orange, Purple, Green, Blue
  };

  /* 棋盤格子中填充的元素（棋子、空位） */
  /* 對於一般棋子，映射到 Layout 上，不直接記錄信息 */
  class Element {
  private:
    int id = 0;
    bool unknown = false;
    /**
       |------|-------------------------------|
       |  ID  |              意義             |
       |------|-------------------------------|
       |  00  |              空位             |
       |  01  |  #0 玩家 (Orange) 【保留】    |
       |  02  |  #0 layout index = 0 的棋子   |
       |  03  |  #0 layout index = 1 的棋子   |
       |  04  |  #0 layout index = 2 的棋子   |
       |  ..  |  #0 layout index = .......    |
       |  27  |  #1 玩家 (Purple) 【保留】    |
       |  28  |  #1 player, layout index = 0  |
       |  29  |  #1 player, layout index = 1  |
       |  30  |  #1 player, layout index = 2  |
       |  ..  |  #1 layout index = .......    |
       |  53  |  #2 玩家 (Green)  【保留】    |
       |  ..  |  #2 layout index = .......    |
       |  79  |  #3 玩家 (Blue)   【保留】    |
       |  ..  |  #3 layout index = .......    |
       |  104 |  #3 player, layout index = 24 |
       |------|-------------------------------|
     */
  public:
    Element();
    Element(Player player, int layout_index);
    int get_id() const;
    bool is_empty() const; // 空位？
    Player get_player() const;
    bool is_unknown() const; // 未知？
    void set_unknown();
    void set_known();
    int get_layout_index() const;
    Element& operator = (const Element& right);
    static Element Unknown(Player player, int layout_index) {
      Element result(player, layout_index);
      result.set_unknown();
      return result;
    };
    static Element Known(Player player, int layout_index) {
      return Element(player, layout_index);
    };
  };

  /* 玩家進行一次操作後得到的結果 */
  class Feedback {
  private:
    MoveResult move_result = Nothing; // 碰子結果，若只是移動未碰子則為 Nothing
    std::list<Cell> route; // 棋子行進路線
  public:
    Feedback();
    Feedback(MoveResult move_result, const std::list<Cell>& route);
    bool is_nothing() const;
    MoveResult get_move_result() const;
    std::list<Cell> get_route() const;
    Feedback& operator = (const Feedback& right);
  };

  /* 亮棋模式 */
  enum MaskMode {
    NoExpose, DoubleExpose, AllExpose
    /* 四暗，雙明，全明 */
  };

  /* 棋盤操作 Object, 保存每個格子的狀態，可以進行碰子等操作，不含回合信息 */
  class Game {
  private:
    Element board[4631]; // 格子狀態，以 Cell ID 為下標
    Layout layout[4]; // 佈局，以 player 為下標
    bool enabled[4] = {0}; // 哪些玩家參與了游戲
    bool show_flag[4] = {0}; // 亮棋？
    Feedback last_feedback; // 最近一次玩家操作結果
    void init_board();
  public:
    Game() {};
    /* 單挑局, 默認南北方向，若設 EW = true 則在東西方向 */
    Game(const Layout& layout_S, const Layout& layout_N, bool EW = false);
    /* 2v2, 輸入數組 layouts[0,1,2,3] */
    Game(const Layout* layouts);
    /* return board[cell.get_id()]; */
    Element element_of(Cell cell) const;
    /* 查詢棋子（大小），要求該 element 非空且已知 */
    Piece piece_of(Element element) const;
    /** 判斷是否可以從 from 移動到 to.
	若不假定棋子的身份，則要求 element_of(from) 非空已知 */
    bool is_movable(
      Cell from, Cell to, bool assume = false, Piece piece = Piece(0)
    ) const;
    /* 判斷 player 是否有活棋，用於無棋可走的判負條件 */
    bool has_living_piece(Player player) const;
    /* 移動棋子，可指定 force_result 為非 Nothing 的值以作假定 */
    Feedback move(Cell from, Cell to, MoveResult force_result = Nothing);
    /* return last_feedback; */
    Feedback get_last_feedback() const;
    /* 銷毁 player 的所有棋子 */
    void annihilate(Player player);
    /* 按照游戲模式（亮子模式）取得隐去未知信息的 Game Object */
    Game get_game_with_mask(
        Player perspective, MaskMode mask_mode = NoExpose
    ) const;
    /* 複製 */
    Game& operator = (const Game& right);
  };

  bool is_valid_game_piece_id(int id);
  bool is_valid_layout(const Piece* data);
  bool is_valid_layout_index(int index);
  Orient convert_player_to_orient(Player player);
  Player convert_orient_to_player(Orient orient);
  Cell convert_layout_index_to_cell(int index, Player player = Orange);
  int convert_cell_to_layout_index(const Cell& cell);

};


#endif // GWANKEI_GAME_HPP
