#include <random>
#include "chesstypes.hpp"
#include "player.hpp"

//    std::random_device rd; // obtain a random number from hardware
//    std::mt19937 eng(rd()); // seed the generator
//    std::uniform_int_distribution<> distr(25, 63); // define the range
//    for(int n=0; n<40; ++n)
//       std::cout << distr(eng) << ' '; // generate numbers

namespace
{
// random generator
std::random_device rd; // obtain a random number from hardware
std::mt19937 eng(rd()); // seed the generator engine
}

namespace C2_chess
{

Player::Player(playertype p, col color, Bitboard& board) :
    _chessboard(board),
    _type(p),
    _color(color),
    _other_col(color == col::white ? col::black : col::white)
{
  //cout << "Player construcor1" << std::endl;
}

Player::~Player()
{
}

col Player::color() const
{
  return _color;
}

void Player::color(col tc)
{
  _color = tc;
  _other_col = tc == col::white ? col::black : col::white;
}

playertype Player::type() const
{
  return _type;
}

void Player::type(playertype t)
{
  _type = t;
}


int Player::find_best_move_index(uint8_t& move_no, float& score, const int max_search_level, bool use_pruning, bool search_until_no_captures)
{
// TODO: The playertype should of course have been set correctly
// for a computer vs. computer game.
// It can be two computers playing so I skipped the following:
// if (_type == playertype::human)
//   return -1;

  _chessboard.clear_transposition_table();

// _type == computer
  int8_t best_move_index = -1;
  float alpha = -100, beta = 100;
  if (_color == col::white)
  {
    if (use_pruning && !search_until_no_captures)
      score = _chessboard.max(0, move_no, alpha, beta, best_move_index, max_search_level);
    if (best_move_index == -1 && score == -100.0)
    {
      // logfile << "White was check mated." << "\n"; // TODO
      return 0;
    }
  }
  else // _color == col::black
  {
    if (use_pruning && !search_until_no_captures)
      score = _chessboard.min(0, move_no, alpha, beta, best_move_index, max_search_level);
    if (best_move_index == -1 && score == 100.0)
    {
      // logfile << "Black was check mated." << "\n"; // TODO
      return 0;
    }
  }
  return best_move_index;
}
} // namespace C2_chess

