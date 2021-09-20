#ifndef _PLAYER
#define _PLAYER

#include "chesstypes.hpp"

namespace C2_chess
{

class Board;

class Player {
  protected:
    Board &_chessboard;
    playertype _type;
    col _colour;
    col _other_col;

  public:
    Player(playertype p, col c, Board &chessboard);
    ~Player();
    int make_a_move(int &moveno, float &score, const int &search_level);
    col color() const;
    void color(col tc);
    playertype type() const;
    void type(playertype t);
    int find_best_move_index(int &move_no, float &score, const int max_search_level, bool use_pruning, bool search_until_no_captures);
};
}
#endif
