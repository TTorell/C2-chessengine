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
  //cout << "Player construcor1" << endl;
}

Player::~Player()
{
}

col Player::get_colour()
{
  return _colour;
}

void Player::set_colour(col tc)
{
  _colour = tc;
  _other_col = tc == col::white ? col::black : col::white;
}

playertype Player::get_type()
{
  return _type;
}

void Player::set_type(playertype t)
{
  _type = t;
}

//bool Player::mate_in(const int& n, const Board& board, int k, int& make_move_no)
//{
//  cout << "Player::mate_in " << n << " k = " << k << "col_to_move = " << _colour << endl;
//  bool mate1 = false;
//  int i = 0;
//  //cerr<<"board noofmoves"<<board.no_of_moves()<<endl;
//  while ((i < board.no_of_moves()) && (!mate1))
//  {
//    int x = 3; // just a dummy in this case
//    _b[k] = board;
//    //if (k==0)
//    //cout << "i=" << i << " k=" << k << endl;
//    _b[k].make_move(i++, x, _colour, true);
//    if ((_b[k].no_of_moves()))
//    {
//      //      cerr<<"bk no of moves "<<b[k].no_of_moves()<<endl;
//      if (n > 1) //(k<(n-1))
//      {
//        bool mate2 = true;
//        int j = 0;
//        Board tempboard;
//        while ((j < _b[k].no_of_moves()) && (mate2))
//        {
//          tempboard = _b[k];
//          tempboard.make_move(j++, x, _other_col, true);
//          if (!mate_in(n - 1, tempboard, k + 1, make_move_no))
//            mate2 = false;
//        }
//        if (mate2)
//          mate1 = true;
//      }
//    }
//    else
//    {
//      if (_b[k].get_last_move().get_check())
//      {
//        mate1 = true;
//        // cerr<<"OK*********"<<endl;
//      }
//      //         else
//      //            cout << "stalemate" << endl;
//    }
//    //if (!mate1)
//    //   b[k].clear();
//  }
//  //cerr<<"mate_in END "<<n<<endl;
//  if (mate1 && (k == 0))
//  {
//    make_move_no = --i;
//    //cerr<<"the move was number"<< i << endl;
//  }
//  return mate1;
//}

int Player::make_a_move(int &move_no, float &score, const int &max_search_level, bool use_pruning)
{
  if (_type == playertype::human)
  {
    return _chessboard.make_move(playertype::human, move_no, _colour);
  }
  else // _type == computer
  {
    int best_move_index;
    float alpha = -100, beta = 100;
    if (_colour == col::white)
    {
      score = _chessboard.max(0, move_no, alpha, beta, best_move_index, max_search_level, use_pruning);
      if (best_move_index == -1 && score == -100.0)
      {
        cout << "White was check mated." << endl; // TODO
        return 0;
      }
    }
    else
    {
      score = _chessboard.min(0, move_no, alpha, beta, best_move_index, max_search_level, use_pruning);
      if (best_move_index == -1 && score == 100.0)
      {
        cout << "Black was check mated." << endl; // TODO
        return 0;
      }
    }
    return _chessboard.make_move(best_move_index, move_no, _colour);
  }
}

int Player::find_best_move_index(int &move_no, float &score, const int &max_search_level, bool use_pruning)
{
  // TODO: The playertype should of course have been set correctly
  // for a computer vs. computer game.
  // It can be two computers playing so I skipped the following:
  // if (_type == playertype::human)
  //   return -1;

  // _type == computer
  int best_move_index;
  float alpha = -100, beta = 100;
  if (_colour == col::white)
  {
    score = _chessboard.max(0, move_no, alpha, beta, best_move_index, max_search_level, use_pruning);
    if (best_move_index == -1 && score == -100.0)
    {
      cout << "White was check mated." << endl; // TODO
      return 0;
    }
  }
  else // _color == col::black
  {
    score = _chessboard.min(0, move_no, alpha, beta, best_move_index, max_search_level, use_pruning);
    if (best_move_index == -1 && score == 100.0)
    {
      cout << "Black was check mated." << endl; // TODO
      return 0;
    }
  }
  return best_move_index;
}
}

