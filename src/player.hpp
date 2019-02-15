#ifndef _PLAYER
#define _PLAYER

#include "chesstypes.hpp"

namespace C2_chess
{

class Board;

class Player
{
  protected:
    Board& _chessboard;
    //Board _b[5];
    player_type _type;
    col _colour;
    col _other_col;

  public:
    Player(player_type p, col c, Board& chessboard);
    ~Player();
    //bool mate_in(const int& n, const Board& board, int k, int& make_move_no);
    int make_a_move(int& moveno,
                    float& score,
                    bool& playing,
                    const int& search_level,
                    bool use_pruning);
    col get_colour();
    void set_colour(col tc);
    player_type get_type();
    void set_type(player_type t);
};
}
#endif
