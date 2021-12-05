#ifndef _PLAYER
#define _PLAYER

#include "chesstypes.hpp"
#include "bitboard.hpp"

namespace C2_chess
{

class Board;

class Player {
  protected:
    Bitboard& _chessboard;
    playertype _type;
    col _color;
    col _other_col;

  public:
    Player(playertype p, col c, Bitboard& chessboard);
    ~Player();
    int make_a_move(uint8_t& move_no, float& score, const uint8_t& max_search_level);
    col color() const;
    void color(col tc);
    playertype type() const;
    void type(playertype t);
    int find_best_move_index(uint8_t &move_no, float &score, const int max_search_level, bool use_pruning, bool search_until_no_captures);
};
}
#endif
