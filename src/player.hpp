#ifndef _PLAYER
#define _PLAYER

#include "chesstypes.hpp"

namespace C2_chess
{

class Board;

class Player {
  protected:
    Board &_chessboard;
    //Board _b[5];
    playertype _type;
    col _colour;
    col _other_col;

  public:
    Player(playertype p, col c, Board &chessboard);
    ~Player();
    //bool mate_in(const int& n, const Board& board, int k, int& make_move_no);
    int make_a_move(int &moveno, float &score, const int &search_level);
    col get_colour();
    void set_colour(col tc);
    playertype get_type();
    void set_type(playertype t);
    int find_best_move_index(int &move_no, float &score, const int &max_search_level, bool use_pruning, bool search_until_no_captures);
};
}
#endif
