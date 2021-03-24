#include <sstream>
#include <map>
#include <thread>
#include <chrono>
#include "game.hpp"
#include "current_time.hpp"
#include "chesstypes.hpp"
#include "chessfuncs.hpp"
#include "config_param.hpp"
#include "shared_ostream.hpp"

using namespace std;
using namespace std::chrono;

namespace
{
C2_chess::CurrentTime current_time;
}

namespace C2_chess
{

Game::Game(Config_params& config_params):
    _is_first_position(true), _move_log(), _chessboard(), _player1(playertype::human, col::white, _chessboard), _player2(playertype::computer, col::black, _chessboard), _moveno(1),
    _col_to_move(col::white), _score(0), _config_params(config_params), _playing(false)
{
  _player[static_cast<int>(col::white)] = &_player1;
  _player[static_cast<int>(col::black)] = &_player2;
  _chessboard.setup_pieces();
  _chessboard.init(_col_to_move);
  _chessboard.calculate_moves(_col_to_move);
}

Game::Game(col color, Config_params& config_params):
    _is_first_position(true), _move_log(), _chessboard(), _player1(playertype::human, color, _chessboard),
    _player2(playertype::computer, color == col::white ? col::black : col::white, _chessboard), _moveno(1),
    _col_to_move(col::white), _score(0), _config_params(config_params), _playing(false)
{
  _player[static_cast<int>(color)] = &_player1;
  _player[static_cast<int>(color == col::white ? col::black : col::white)] = &_player2;
}

Game::Game(col color,
           playertype pt1,
           playertype pt2,
           Config_params& config_params):
    _is_first_position(true), _move_log(), _chessboard(), _player1(pt1, color, _chessboard), _player2(pt2, color == col::white ? col::black : col::white, _chessboard), _moveno(1),
    _col_to_move(col::white), _score(0), _config_params(config_params), _playing(false)
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

void Game::clear_move_log()
{
  _move_log.clear();
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

void Game::set_move_log_col_to_start(col color)
{
  _move_log.set_col_to_start(color);
}

void Game::set_castling_state(const Castling_state &cs)
{
  _chessboard.set_castling_state(cs);
}

void Game::put_piece(Piece *const p, int file, int rank)
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

ostream& Game::write_chessboard(ostream &os, outputtype ot, col from_perspective) const
{
  _chessboard.write(os, ot, from_perspective);
  return os;
}

ostream& Game::write_diagram(ostream &os) const
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

Shared_ostream& Game::write_diagram(Shared_ostream& sos) const
{
  stringstream ss;
  write_diagram(ss);
  ss.flush();
  sos.write_UTF8_string(ss.str());
  return sos;
}

void Game::init_board_hash_tag()
{
  _chessboard.init_board_hash_tag(_col_to_move);
}

void Game::actions_after_a_move()
{
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());
  Shared_ostream& logfile = *(Shared_ostream::get_instance());

  // We have made a move.
  cmdline << _move_log << "\n";
  logfile << _move_log << "\n";
  write_diagram(cmdline);
  write_diagram(logfile);

  // Change color to evaluate from opponents view.
  _col_to_move = (_col_to_move == col::white) ? col::black : col::white;

  float evaluation = _chessboard.evaluate_position(_col_to_move, outputtype::debug, 0);
  if (evaluation == eval_max || evaluation == eval_min)
  {
    _chessboard.set_mate(true);
    cmdline << ((evaluation == eval_max) ? "1 - 0, black was mated" : "0 - 1, white was mated") << "\n" << "\n";
    logfile << ((evaluation == eval_max) ? "1 - 0, black was mated" : "0 - 1, white was mated") << "\n";
  }
  else if (evaluation == 0 && _chessboard.no_of_moves() == 0)
  {
    _chessboard.set_stalemate(true);
    cmdline << "1/2 - 1/2 draw by stalemate" << "\n";
    logfile << "1/2 - 1/2 draw by stalemate" << "\n";
  }
  else // Normal position
  {
    cmdline << "current postion evaluation: " << evaluation << "\n\n";
    logfile << "current postion evaluation: " << evaluation << "\n\n";
    // Update half-move coubter for the 50-moves-rule.
    Move last_move = _chessboard.get_last_move();
    if (last_move.get_take() || last_move.get_piece_type() == piecetype::Pawn)
      _half_move_counter = 0;
    else
      _half_move_counter++;
    if (_half_move_counter > 50)
    {
      cmdline << "1/2 - 1/2 draw by the fifty-move rule" << "\n";
      logfile << "1/2 - 1/2 draw by the fifty-move rule" << "\n";
    }
  }
  // cmdline <<"Time_diff_sum = " << (int)_chessboard.get_time_diff_sum() << "\n";
  // logfile << "Time_diff_sum = " << (int)_chessboard.get_time_diff_sum() << "\n";
}

// This method is for the cmd-line interface only
void Game::start()
{
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());

  logfile << "\nGame started\n";
  logfile.write_config_params(_config_params);
  cmdline << "\n" << "Game started" << "\n";
  // Init the hash tag for the initial board-position to
  // use in the Zobrist hash table.
  // (also called transposition table)
  init_board_hash_tag();
  // Tell the engine that there are no time limits.
  // The time it takes is defined by the max_searh_level
  _chessboard.set_time_left(true);
  const int max_search_level = 7;
  _playing = true;
  while (_playing)
  {
    uint64_t nsec_start = current_time.nanoseconds();
    if (get_playertype(_col_to_move) == playertype::human)
    {
      write_diagram(cmdline);
      cmdline << "Hashtag: " << _chessboard.get_hash_tag() << "\n";
      cmdline << "Material evaluation: " << _chessboard.get_material_evaluation() << "\n";
      if (_player[static_cast<int>(_col_to_move)]->make_a_move(_moveno, _score, max_search_level) != 0)
      {
        cmdline << "Stopped playing" << "\n";
        _playing = false;
      }
      // Update the movelog of the game.
      _move_log.into_as_last(new Move(_chessboard.get_last_move()));
      actions_after_a_move();
    }
    else
    {
      // computer
      engine_go(_config_params, "20000");
    }
    uint64_t timediff = current_time.nanoseconds() - nsec_start;
    cmdline << "time spent thinking: " << timediff/1.0e9 << " seconds" << "\n";
    logfile << "time spent thinking: " << timediff/1.0e9 << " seconds" << "\n";
  }
}

Move Game::engine_go(const Config_params& config_params, const string& max_search_time)
{
  Shared_ostream& logfile = *(Shared_ostream::get_instance());

  // These values are just default values
  int max_search_level = 7;
  bool use_incremental_search = true;
  bool use_pruning = true;
  bool search_until_no_captures = false;

  _chessboard.set_time_diff_sum(0);

  // Read some configuration parameters
  string s = config_params.get_config_param("max_search_level");
  if (!s.empty())
    max_search_level = atoi(s.c_str());
  s = config_params.get_config_param("use_incremental_search");
  if (!s.empty())
    use_incremental_search = (s == "true");
  s = config_params.get_config_param("use_pruning");
  if (!s.empty())
    use_pruning = (s == "true");
  s = config_params.get_config_param("search_until_no_captures");
  if (!s.empty())
    search_until_no_captures = (s == "true");

  // Search for best move
  if (use_incremental_search)
  {
    // Incremental search, to have a best-move available as quickly
    // as possible. When searching incrementally we consider
    // max_search_time.
    if (!max_search_time.empty())
    {
      _chessboard.start_timer_thread(max_search_time);
      this_thread::sleep_for(microseconds(100));
    }
    else
      _chessboard.set_time_left(true);
    int best_move_index = -1;
    for (int i = 2; i <= max_search_level; i++)
    {
      uint64_t nsec_start = current_time.nanoseconds();
      _chessboard.clear_hash();

      //bool test = (use_pruning == false || search_until_no_captures == true);
      int move_index = _player[static_cast<int>(_col_to_move)]->find_best_move_index(_moveno, _score, i, use_pruning, search_until_no_captures);
      // Has the search on this level been aborted by time limit?
      if (move_index == -1)
      {
        // This happens when max_search_level has been set to 1
        // or when the search has been interrupted by the time limit.
        // (Or possibly when something else has gone wrong.)
        // My min() or max() will just evaluate the current position
        // then and wont be able to choose best move.
        // So, searching with level 1 is completely pointless.
        if (has_time_left())
        {
          logfile << "Error: find_best_move() returned -1." << "\n";
          // We can't choose. Just set it to the first move.
          // (TODO: Choose randomly maybe.)
          if (best_move_index == -1)
            best_move_index = 0;
        }
        else
        {
          // Time is out.
          if (i == 2)
          {
            // The search has been interrupted on lowest level.
            // No best move has been found at all, so just choose
            // the first move (TODO: Choose randomly maybe.)
            logfile << "interrupted on level 2" << "\n";
            best_move_index = 0;
          }
        }
        break;
      }
      best_move_index = move_index;

      uint64_t nsec_stop = current_time.nanoseconds();
      logfile.log_time_diff(nsec_stop, nsec_start, i, _chessboard.get_possible_move(best_move_index), _score);
    }
    _chessboard.make_move(best_move_index, _moveno, _col_to_move);
    // The _move_log only makes sense if playing from the command line,
    // Or when the engine plays against itself ("play out position").
    // In the normal case the engine only knows its own moves.
    _move_log.into_as_last(new Move(_chessboard.get_last_move()));
  }
  else
  {
    // Not incremental search, start searching directly at max_search
    // level and stop when finished. Ignore max_search_time.
    _chessboard.set_time_left(true);
    int best_move_index = -1;
    uint64_t nsec_start = current_time.nanoseconds();
    int move_index = _player[static_cast<int>(_col_to_move)]->find_best_move_index(_moveno, _score, max_search_level, use_pruning, search_until_no_captures);
    if (move_index == -1)
    {
      // This happens when max_search_level has been set to 1.
      // (Or possibly when something else has gone wrong.)
      // My min() or max() will just evaluate the current position
      // then and wont be able to choose best move.
      // So, searching with level 1 is completely pointless.
      logfile << "Error: find_best_move() returned -1." << "\n";
      // We can't choose. Just set it to the first move.
      move_index = 0;
    }
    best_move_index = move_index;
    uint64_t nsec_stop = current_time.nanoseconds();
    logfile.log_time_diff(nsec_stop, nsec_start, max_search_level, _chessboard.get_possible_move(best_move_index), _score);
    _chessboard.make_move(best_move_index, _moveno, _col_to_move);
    _move_log.into_as_last(new Move(_chessboard.get_last_move()));
  }
  actions_after_a_move();
  return _chessboard.get_last_move();
}

void Game::start_timer_thread(const string& max_search_time)
{
  _chessboard.start_timer_thread(max_search_time);
}

bool Game::has_time_left()
{
  return _chessboard.has_time_left();
}

void Game::set_time_left(bool value)
{
  _chessboard.set_time_left(value);
}

playertype Game::get_playertype(const col& color) const
{
  return _player[static_cast<int>(color)]->get_type();
}

} // namespace C2_chess
