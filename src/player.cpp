#include <random>
#include "chesstypes.hpp"
#include "player.hpp"
#include "board.hpp"

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

Player::Player(playertype p, col color, Board &bo) :
    _chessboard(bo), _type(p), _colour(color), _other_col(color == col::white ? col::black : col::white)
{
  //cout << "Player construcor1" << std::endl;
}

Player::~Player()
{
}

col Player::color() const
{
  return _colour;
}

void Player::color(col tc)
{
  _colour = tc;
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

int Player::make_a_move(int &move_no, float &score, const int &max_search_level)
{
  if (_type == playertype::human)
  {
    return _chessboard.make_move(playertype::human, move_no, _colour);
  }
  else // _type == computer
  {
    int best_move_index;
    float alpha = -100, beta = 100;
    _chessboard.clear_hash();
    _chessboard.init_material_evaluation();
    if (_colour == col::white)
    {
      score = _chessboard.max(0, move_no, alpha, beta, best_move_index, max_search_level);
      if (best_move_index == -1 && score == -100.0)
      {
        std::cout << "White was check mated." << std::endl; // TODO
        return 0;
      }
    }
    else
    {
      score = _chessboard.min(0, move_no, alpha, beta, best_move_index, max_search_level);
      if (best_move_index == -1 && score == 100.0)
      {
        std::cout << "Black was check mated." << std::endl; // TODO
        return 0;
      }
    }
    return _chessboard.make_move(best_move_index, move_no, _colour);
  }
}

int Player::find_best_move_index(int &move_no, float& score, const int max_search_level, bool use_pruning, bool search_until_no_captures)
{
  // TODO: The playertype should of course have been set correctly
  // for a computer vs. computer game.
  // It can be two computers playing so I skipped the following:
  // if (_type == playertype::human)
  //   return -1;

  _chessboard.clear_hash();
  _chessboard.init_material_evaluation();

  // _type == computer
  int best_move_index;
  float alpha = -100, beta = 100;
  if (_colour == col::white)
  {
    if (use_pruning && !search_until_no_captures)
      score = _chessboard.max(0, move_no, alpha, beta, best_move_index, max_search_level);
    else
      score = _chessboard.max_for_testing(0, move_no, alpha, beta, best_move_index, max_search_level, use_pruning, search_until_no_captures);
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
    else
      score = _chessboard.min_for_testing(0, move_no, alpha, beta, best_move_index, max_search_level, use_pruning, search_until_no_captures);
    if (best_move_index == -1 && score == 100.0)
    {
      // logfile << "Black was check mated." << "\n"; // TODO
      return 0;
    }
  }
  return best_move_index;
}
}

