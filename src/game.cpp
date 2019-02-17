#include <sstream>
#include "game.hpp"
#include "current_time.hpp"
#include "chesstypes.hpp"
#include "chessfuncs.hpp"

namespace
{
C2_chess::CurrentTime current_time;
}

namespace C2_chess
{

using std::stringstream;

Game::Game() :
    _move_log(), _chessboard(), _player1(human, white, _chessboard), _player2(computer, black, _chessboard), _moveno(1), _col_to_move(white), _score(0)
{
  _player[white] = &_player1;
  _player[black] = &_player2;
  _chessboard.setup_pieces();
  _chessboard.init(_col_to_move);
  _chessboard.calculate_moves(_col_to_move);
}

Game::Game(col c) :
    _move_log(), _chessboard(), _player1(human, c, _chessboard), _player2(computer, c == white ? black : white, _chessboard), _moveno(1), _col_to_move(white), _score(0)
{
  _player[c] = &_player1;
  _player[c == white ? black : white] = &_player2;
}

Game::Game(col c, player_type pt1, player_type pt2) :
    _move_log(), _chessboard(), _player1(pt1, c, _chessboard), _player2(pt2, c == white ? black : white, _chessboard), _moveno(1), _col_to_move(white), _score(0)
{
  _player[c] = &_player1;
  _player[c == white ? black : white] = &_player2;
}

Game::~Game()
{
}

void Game::init()
{
  _chessboard.init(_col_to_move);
  _chessboard.calculate_moves(_col_to_move);
}

void Game::clear_chessboard()
{
  _chessboard.clear();
}

void Game::setup_pieces()
{
  _chessboard.setup_pieces();
}

col Game::get_col_to_move() const
{
  return _col_to_move;
}

void Game::set_col_to_move(col c)
{
  _col_to_move = c;
}

void Game::set_col_to_start(col c)
{
  _move_log.set_col_to_start(c);
}

void Game::set_castling_state(const Castling_state &cs)
{
  _chessboard.set_castling_state(cs);
}

void Game::put_piece(Piece* const p, int file, int rank)
{
  _chessboard.put_piece(p, file, rank);
}

void Game::set_en_passant_square(int file, int rank)
{
  _chessboard.set_enpassant_square(file, rank);
}

void Game::set_half_move_counter(int half_move_counter)
{
  _half_move_counter = half_move_counter;
}

void Game::set_moveno(int moveno)
{
  _moveno = moveno;
}

ostream& Game::write_chessboard(ostream& os, output_type ot, col from_perspective) const
{
  _chessboard.write(os, ot, from_perspective);
  return os;
}

ostream& Game::write_diagram(ostream& os) const
{
  if (_player[white]->get_type() == human)
    _chessboard.write(os, cmd_line_diagram, white) << endl;
  else if (_player[black]->get_type() == human)
    _chessboard.write(os, cmd_line_diagram, black) << endl;
  else
    // The computer is playing itself
    _chessboard.write(os, cmd_line_diagram, white) << endl;
  return os;
}

Shared_ostream& Game::write_diagram(Shared_ostream& os) const
{
  stringstream ss;
  write_diagram(ss);
  ss.flush();
  os << ss.str();
  return os;
}

void Game::start()
{
  cout << endl << "Game started" << endl;
  const int max_search_level = 7;
  const bool use_pruning = true;
  bool playing = true;
  while (playing)
  {
    write_diagram(cout);
    //write_chessboard(cout, debug, white);
    uint64_t nsec_start = current_time.nanoseconds();
    if (_player[_col_to_move]->make_a_move(_moveno, _score, playing, max_search_level, use_pruning) != 0)
    {
      cout << "Stopped playing" << endl;
      playing = false;
    }
    uint64_t nsec_stop = current_time.nanoseconds();
    long timediff = nsec_stop - nsec_start;
    cout << "time spent = " << timediff << " ns" << endl;
    _col_to_move = (_col_to_move == white) ? black : white;

    float evaluation = _chessboard.evaluate_position(_col_to_move, silent, 0);
    if (evaluation == eval_max || evaluation == eval_min)
    {
      _chessboard.set_mate(true);
      _move_log.into_as_last(new Move(_chessboard.get_last_move()));
      cout << _move_log;
      write_diagram(cout);
      cout << ((evaluation == 100) ? "1 - 0, black was mated" : "0 - 1, white was mated") << endl << endl;
      return;
    }
    else if (evaluation == 0 && _chessboard.no_of_moves() == 0)
    {
      _chessboard.set_stalemate(true);
      _move_log.into_as_last(new Move(_chessboard.get_last_move()));
      cout << _move_log << endl;
      write_diagram(cout);
      cout << "1/2 - 1/2 draw by stalemate" << endl;
      return;
    }
    else
    {
      cout << _chessboard.evaluate_position(_col_to_move, debug, 0) << endl << endl;
      Move last_move = _chessboard.get_last_move();
      if (last_move.get_take() || last_move.get_piece_type() == Pawn)
        _half_move_counter = 0;
      else
        _half_move_counter++;
      _move_log.into_as_last(new Move(_chessboard.get_last_move()));
      cout << _move_log << endl;
    }
  }
}

Move Game::engine_go(Shared_ostream& logfile)
{
  logfile << "Engine go" << "\n";
  const int max_search_level = 7;
  const bool use_pruning = true;
  bool playing = true;

  write_diagram(logfile) << "\n";
  uint64_t nsec_start = current_time.nanoseconds();
  if (_player[_col_to_move]->make_a_move(_moveno, _score, playing, max_search_level, use_pruning) != 0)
  {
    logfile << "Error: Stopped playing" << "\n";
  }
  uint64_t nsec_stop = current_time.nanoseconds();
  long timediff = nsec_stop - nsec_start;
  logfile << "time spent = " << timediff << " ns" << "\n";
  float evaluation = _chessboard.evaluate_position(_col_to_move, silent, 0);
  if (evaluation == eval_max || evaluation == eval_min)
  {
    stringstream ss;
    _chessboard.set_mate(true);
    _move_log.into_as_last(new Move(_chessboard.get_last_move()));
    ss << _move_log << endl;
    logfile << ss.str();
    write_diagram(logfile);
    logfile << ((evaluation == 100) ? "1 - 0, black was mated" : "0 - 1, white was mated") << "\n";
  }
  else if (evaluation == 0 && _chessboard.no_of_moves() == 0)
  {
    stringstream ss;
    _chessboard.set_stalemate(true);
    _move_log.into_as_last(new Move(_chessboard.get_last_move()));
    ss << _move_log << endl;
    logfile << ss.str();
    write_diagram(logfile);
    logfile << "1/2 - 1/2 draw by stalemate" << "\n";
  }
  else
  {
    logfile << _chessboard.evaluate_position(_col_to_move, silent, 0) << "\n" << "\n";
    Move last_move = _chessboard.get_last_move();
    if (last_move.get_take() || last_move.get_piece_type() == Pawn)
      _half_move_counter = 0;
    else
      _half_move_counter++;
    _move_log.into_as_last(new Move(_chessboard.get_last_move()));
    stringstream ss;
    ss << _move_log << endl;
    logfile << ss.str();
  }
  return _chessboard.get_last_move();
}
}
