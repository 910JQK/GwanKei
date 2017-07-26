#ifndef GWANKEI_GAME_HPP
#define GWANKEI_GAME_HPP


namespace GwanKei {
  
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

  class Layout {
  private:
    Piece data[25] = {0};
    bool empty = true;
  public:
    Layout();
    Layout(Piece* data);
    bool is_empty() const;
    Piece get(int index) const;
    Piece get(int y, int x, LeftRight lr) const;
    void swap(int index1, int index2);
    Layout& operator = (const Layout& right);
  };

  enum Player {
    Red, Purple, Green, Blue
  };

  class GamePiece {
  private:
    int id = 0; // empty
  public:
    GamePiece();
    GamePiece(int id);
    GamePiece(Player player, Piece piece, int subscript);
    Player get_player();
    Piece get_piece();
    int get_subscript();
    bool is_empty() const;
    GamePiece& operator = (const GamePiece& right);
  };

  class MoveResultWithRoute {
  private:
    MoveResult move_result = Null;
    std::list<Cell> route;
  public:
    MoveResultWithRoute();
    MoveResultWithRoute(MoveResult move_result, const std::list<Cell>& route);
    bool is_nothing() const;
    MoveResultWithRoute& operator = (const MoveResultWithRoute& right);
  };

  class Game {
  private:
    GamePiece board[4631];
    Layout layouts[4];
    Player turn;
  public:
    Game(const Layout& layout_S, const Layout& layout_N);
    Game(const Layout& layout_E, const Layout& layout_W);
    Game(const Layout& layout_S, const Layout& layout_E,
	 const Layout& layout_N,const Layout& layout_W);
    Player whose_turn() const;
    bool is_movable(Player player, Cell from, Cell to) const;
    MoveResultWithRoute move(
			     Player player,
			     Cell from,
			     Cell to,
			     MoveResultWithRoute force_result
			         = MoveResultWithRoute()
			     );
    void skip();
  };

  bool is_valid_game_piece_id(int id);
  bool is_valid_layout(Piece* data);
  bool is_valid_layout_index(int index);
  Orient convert_player_to_orient(Player player);
  Player convert_orient_to_player(Orient orient);
  Cell convert_layout_index_to_cell(int index, CellGroup group = South);
  int convert_cell_to_layout_index(const Cell& cell);

};


#endif // GWANKEI_GAME_HPP
