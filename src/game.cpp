#include <sstream>
#include <map>
#include "game.hpp"
#include "current_time.hpp"
#include "chesstypes.hpp"
#include "chessfuncs.hpp"
#include "Config_param.hpp"

namespace
{
C2_chess::CurrentTime current_time;
}

namespace C2_chess
{

using std::stringstream;

Game::Game() :
    _is_first_position(true),
    _move_log(),
    _chessboard(),
    _player1(playertype::human, col::white, _chessboard),
    _player2(playertype::computer, col::black, _chessboard),
    _moveno(1),
    _col_to_move(col::white),
    _score(0)
{
  _player[static_cast<int>(col::white)] = &_player1;
  _player[static_cast<int>(col::black)] = &_player2;
  _chessboard.setup_pieces();
  _chessboard.init(_col_to_move);
  _chessboard.calculate_moves(_col_to_move);
}

Game::Game(col color) :
    _is_first_position(true),
    _move_log(),
    _chessboard(),
    _player1(playertype::human, color, _chessboard),
    _player2(playertype::computer, color == col::white ? col::black : col:: white, _chessboard),
    _moveno(1),
    _col_to_move(col::white),
    _score(0)
{
  _player[static_cast<int>(color)] = &_player1;
  _player[static_cast<int>(color == col::white ? col::black : col::white)] = &_player2;
}

Game::Game(col color, playertype pt1, playertype pt2) :
    _is_first_position(true),
    _move_log(),
    _chessboard(),
    _player1(pt1, color, _chessboard),
    _player2(pt2, color == col::white ? col::black : col::white, _chessboard),
    _moveno(1),
    _col_to_move(col::white),
    _score(0)
{
  _player[static_cast<int>(color)] = &_player1;
  _player[static_cast<int>(color == col::white ? col::black : col::white)] = &_player2;
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

void Game::set_col_to_move(col color)
{
  _col_to_move = color;
}

void Game::set_col_to_start(col color)
{
  _move_log.set_col_to_start(color);
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
  if (_is_first_position)
  {
    _move_log.set_first_moveno(moveno);
    _is_first_position = false;
  }
}

ostream& Game::write_chessboard(ostream& os, outputtype ot, col from_perspective) const
{
  _chessboard.write(os, ot, from_perspective);
  return os;
}

ostream& Game::write_diagram(ostream& os) const
{
  if (_player[static_cast<int>(col::white)]->get_type() == playertype::human)
    _chessboard.write(os, outputtype::cmd_line_diagram, col::white) << endl;
  else if (_player[static_cast<int>(col::black)]->get_type() == playertype::human)
    _chessboard.write(os, outputtype::cmd_line_diagram, col::black) << endl;
  else
    // The computer is playing itself
    _chessboard.write(os, outputtype::cmd_line_diagram, col::white) << endl;
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

// This method is for the cmd-line interface only
void Game::start()
{
  cout << endl << "Game started" << endl;
  const int max_search_level = 7;
  const bool use_pruning = true;
  bool playing = true;
  while (playing)
  {
    write_diagram(cout);
//    write_chessboard(cout, debug, white);
    uint64_t nsec_start = current_time.nanoseconds();
    if (_player[static_cast<int>(_col_to_move)]->make_a_move(_moveno, _score, playing, max_search_level, use_pruning) != 0)
    {
      cout << "Stopped playing" << endl;
      playing = false;
    }
    uint64_t nsec_stop = current_time.nanoseconds();
    uint64_t timediff = nsec_stop - nsec_start;
    cout << "time spent = " << timediff << " ns" << endl;
    _col_to_move = (_col_to_move == col::white) ? col::black : col::white;

    float evaluation = _chessboard.evaluate_position(_col_to_move, outputtype::silent, 0);
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
      cout << _chessboard.evaluate_position(_col_to_move, outputtype::debug, 0) << endl << endl;
      Move last_move = _chessboard.get_last_move();
      if (last_move.get_take() || last_move.get_piece_type() == piecetype::Pawn)
        _half_move_counter = 0;
      else
        _half_move_counter++;
      _move_log.into_as_last(new Move(_chessboard.get_last_move()));
      cout << _move_log << endl;
    }
  }
}

Move Game::engine_go(Shared_ostream& logfile, std::atomic<bool>& logfile_is_open, map<string, Config_param>& config_params)
{
  int max_search_level;
  bool use_pruning = true;
  auto it = config_params.find("max_search_level");
  if (it != config_params.end())
    max_search_level = atol(it->second.get_value().c_str());
  else
    max_search_level = 7; // default
  it = config_params.find("use_pruning");
  if (it != config_params.end())
    use_pruning = it->second.get_value() == "true";
  else
    use_pruning = true;
  bool playing = true;
  uint64_t nsec_start = current_time.nanoseconds();
  if (_player[static_cast<int>(_col_to_move)]->make_a_move(_moveno, _score, playing, max_search_level, use_pruning) != 0)
  {
    if (logfile_is_open)
      logfile << "Error: Stopped playing" << "\n";
  }
  uint64_t nsec_stop = current_time.nanoseconds();
  uint64_t timediff = (nsec_stop - nsec_start);
  if (logfile_is_open)
    logfile << "time spent by C2 = " << (float)timediff << " milliseconds" << "\n";
  // We have made move and the board and possible_moves has been calculated for the other player,
  // (inside_make_a_move() so we must evaluate the position from his point of view.
  _col_to_move = _col_to_move == col::white? col::black : col::white;
  float evaluation = _chessboard.evaluate_position(_col_to_move, outputtype::silent, 0);
  if (evaluation == eval_max || evaluation == eval_min)
  {
    _chessboard.set_mate(true);
    _move_log.into_as_last(new Move(_chessboard.get_last_move()));
    if (logfile_is_open)
    {
      stringstream ss;
      ss << _move_log << endl;
      logfile << ss.str();
      write_diagram(logfile);
      logfile << ((evaluation == eval_max) ? "1 - 0, black was mated" : "0 - 1, white was mated") << "\n";
    }
  }
  else if (evaluation == 0.0 && _chessboard.no_of_moves() == 0)
  {
    _chessboard.set_stalemate(true);
    _move_log.into_as_last(new Move(_chessboard.get_last_move()));
    if (logfile_is_open)
    {
      stringstream ss;
      ss << _move_log << endl;
      logfile << ss.str();
      write_diagram(logfile);
      logfile << "1/2 - 1/2 draw by stalemate" << "\n";
    }
  }
  else
  {
    if (logfile_is_open)
      logfile << _chessboard.evaluate_position(_col_to_move, outputtype::silent, 0) << "\n" << "\n";
    Move last_move = _chessboard.get_last_move();
    if (last_move.get_take() || last_move.get_piece_type() == piecetype::Pawn)
      _half_move_counter = 0;
    else
      _half_move_counter++;
    _move_log.into_as_last(new Move(_chessboard.get_last_move()));
    if (logfile_is_open)
    {
      stringstream ss;
      ss << _move_log << endl;
      logfile << ss.str();
    }
  }
  return _chessboard.get_last_move();
}
}
