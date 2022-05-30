#include <iostream>
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
#include "bitboard_with_utils.hpp"

namespace
{
C2_chess::CurrentTime now;
}

namespace C2_chess
{

Game::Game(Config_params& config_params) :
    _is_first_position(true),
    _move_log(),
    _chessboard(),
    _player_type {playertype::human, playertype::computer},
    _moveno(1),
    _score(0),
    _half_move_counter(0),
    _config_params(config_params),
    _playing(false)
{
  _chessboard.read_position(initial_position);
  _chessboard.find_legal_moves(gentype::all);
}

Game::Game(col color, Config_params& config_params) :
    _is_first_position(true),
    _move_log(),
    _chessboard(),
    _player_type {playertype::human, playertype::computer},
    _moveno(1),
    _score(0),
    _half_move_counter(0),
    _config_params(config_params),
    _playing(false)
{
  _player_type[index(color)] = playertype::human;
  _player_type[index(other_color(color))] = playertype::computer;
}

Game::Game(playertype pt1,
           playertype pt2,
           Config_params& config_params) :
    _is_first_position(true),
    _move_log(),
    _chessboard(),
    _player_type {pt1, pt2},
    _moveno(1),
    _score(0),
    _config_params(config_params),
    _playing(false)
{
}

Game::~Game()
{
}

void Game::init()
{
  _chessboard.find_legal_moves(gentype::all);
}

void Game::clear_move_log()
{
  _move_log.clear();
}

void Game::setup_pieces()
{
  _chessboard.read_position(initial_position);
}

col Game::get_col_to_move() const
{
  return _chessboard.get_col_to_move();
}

void Game::set_move_log_col_to_start(col color)
{
  _move_log.set_col_to_start(color);
}

//void Game::set_castling_state(const Castling_state &cs)
//{
//  _chessboard.set_castling_state(cs);
//}

//void Game::put_piece(Piece *const p, int file, int rank)
//{
//  _chessboard.put_piece(p, file, rank);
//}

//void Game::set_en_passant_square(int file, int rank)
//{
//  _chessboard.set_enpassant_square(file, rank);
//}

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

std::ostream& Game::write_chessboard(std::ostream& os, outputtype ot, col from_perspective) const
{
  Bitboard_with_utils(_chessboard).write(os, ot, from_perspective);
  return os;
}

std::ostream& Game::write_diagram(std::ostream& os) const
{
  Bitboard_with_utils bwu(_chessboard);
  if (_player_type[index(col::white)] == playertype::human)
    bwu.write(os, outputtype::cmd_line_diagram, col::white) << std::endl;
  else if (_player_type[index(col::black)] == playertype::human)
    bwu.write(os, outputtype::cmd_line_diagram, col::black) << std::endl;
  else
    // The computer is playing itself
    bwu.write(os, outputtype::cmd_line_diagram, col::white) << std::endl;
  return os;
}

Shared_ostream& Game::write_diagram(Shared_ostream& sos) const
{
  std::stringstream ss;
  write_diagram(ss);
  ss.flush();
  sos.write_UTF8_string(ss.str());
  return sos;
}

void Game::init_board_hash_tag()
{
  _chessboard.init_board_hash_tag();
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

  float evaluation = _chessboard.evaluate_position(_chessboard.get_col_to_move(), 0);
  if (evaluation == eval_max || evaluation == eval_min)
  {
    _chessboard.set_mate();
    cmdline << ((evaluation == eval_max) ? "1 - 0, black was mated" : "0 - 1, white was mated") << "\n" << "\n";
    logfile << ((evaluation == eval_max) ? "1 - 0, black was mated" : "0 - 1, white was mated") << "\n";
    _playing = false;
  }
  else if (evaluation == 0.0 && _chessboard.no_of_moves() == 0)
  {
    _chessboard.set_stalemate();
    cmdline << "1/2 - 1/2 draw by stalemate" << "\n";
    logfile << "1/2 - 1/2 draw by stalemate" << "\n";
    _playing = false;
  }
  else // Normal position
  {
    cmdline << "current postion evaluation: " << evaluation << "\n";
    logfile << "current postion evaluation: " << evaluation << "\n";
    // Update half-move coubter for the 50-moves-rule.
    BitMove last_move = _chessboard.last_move();
    if ((last_move.properties() & move_props_capture) || (last_move.piece_type() == piecetype::Pawn))
      _half_move_counter = 0;
    else
      _half_move_counter++;
    if (_half_move_counter > 50)
    {
      cmdline << "1/2 - 1/2 draw by the fifty-move rule" << "\n";
      logfile << "1/2 - 1/2 draw by the fifty-move rule" << "\n";
      _playing = false;
    }
  }
  // cmdline <<"Time_diff_sum = " << (int)_chessboard.get_time_diff_sum() << "\n";
  // logfile << "Time_diff_sum = " << (int)_chessboard.get_time_diff_sum() << "\n";
}

int Game::find_best_move_index(uint8_t& move_no, float& score, int max_search_level)
{
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  _chessboard.clear_node_counter();
  _chessboard.clear_hash_hits();
  _chessboard.clear_transposition_table();
  uint64_t nsec_start = now.nanoseconds();

  int8_t best_move_index = -1;
  float alpha = -100, beta = 100;
  if (_chessboard.get_col_to_move() == col::white)
  {
    score = _chessboard.max(0, move_no, alpha, beta, best_move_index, max_search_level);
    if (best_move_index == -1 && score == -100.0)
    {
      // logfile << "White was check mated." << "\n"; // TODO
      return 0;
    }
  }
  else // col::black
  {
    score = _chessboard.min(0, move_no, alpha, beta, best_move_index, max_search_level);
    if (best_move_index == -1 && score == 100.0)
    {
      // logfile << "Black was check mated." << "\n"; // TODO
      return 0;
    }
  }
  if (has_time_left())
  {
    logfile << "Evaluated on level:" << max_search_level << " " << _chessboard.get_node_counter() <<
      " nodes in " << (now.nanoseconds() - nsec_start)/1.0e6 << " milliseconds, hash_hits: " << _chessboard.get_hash_hits() << "\n";
  }
  return best_move_index;
}

BitMove Game::incremental_search(const std::string& max_search_time, int max_search_level)
{
  // Incremental search, to have a best-move available as quickly as possible.
  // When searching incrementally we consider max_search_time.

  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  if (!max_search_time.empty())
  {
    _chessboard.start_timer_thread(max_search_time);
    std::this_thread::sleep_for(std::chrono::microseconds(20));
  }
  else
    _chessboard.set_time_left(true);
  int8_t best_move_index = -1;
  for (int i = 2; i <= max_search_level; i++)
  {
    _chessboard.clear_transposition_table();
    _chessboard.clear_PV_table();
    //bool test = (use_pruning == false || search_until_no_captures == true);
    int move_index = find_best_move_index(_moveno, _score, i);
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
        logfile << "Time is out!\n";
        if (i == 2)
        {
          // The search has been interrupted on lowest level.
          // No best move has been found at all, so just choose
          // the first move (TODO: Choose randomly maybe.)
          logfile << i << "interrupted on level 2" << "\n";
          best_move_index = 0;
        }
      }
      break;
    }
    best_move_index = move_index;
  }
  _chessboard.make_move(best_move_index, _moveno);
  _move_log.push_back(_chessboard.last_move());

  // Stop possibly running timer by setting time_left to false.
  _chessboard.set_time_left(false);

  actions_after_a_move();
  return _chessboard.last_move();

}

BitMove Game::engine_go(const Config_params& config_params, const std::string& max_search_time)
{
  Shared_ostream& logfile = *(Shared_ostream::get_instance());

  // This values is just a default value;
  bool use_incremental_search = true;
  int max_search_level = 7;

  // Read some configuration parameters
  std::string s = config_params.get_config_param("max_search_level");
  if (!s.empty())
    max_search_level = std::atoi(s.c_str());
  s = config_params.get_config_param("use_incremental_search");
  if (!s.empty())
    use_incremental_search = (s == "true");

  _chessboard.init_material_evaluation(); // TODO: Should this be placed somewhere else, init_piece_state()?

  // Search for best move
  if (use_incremental_search)
  {
    return incremental_search(max_search_time, max_search_level);
  }
  else
  {
    // Not incremental search, start searching directly at max_search
    // level and stop when finished. Ignore max_search_time.
    _chessboard.set_time_left(true);
    int best_move_index = -1;
    int move_index = find_best_move_index(_moveno, _score, max_search_level);
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
//    uint64_t nsec_stop = current_time.nanoseconds();
//    logfile.log_time_diff(nsec_stop, nsec_start, max_search_level, _chessboard.get_possible_move(best_move_index), _score);
    _chessboard.make_move(best_move_index, _moveno);
    _move_log.push_back(_chessboard.last_move());
  }

  // Stop possibly running timer by setting time_left to false.
  _chessboard.set_time_left(false);

  actions_after_a_move();
  return _chessboard.last_move();
}

void Game::start_timer_thread(const std::string& max_search_time)
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
  return _player_type[index(color)];
}

void Game::start_new_game(col col_to_move, int half_move_counter, int moveno)
{
  clear_move_log();
  _half_move_counter = half_move_counter;
  set_moveno(moveno);
  set_move_log_col_to_start(col_to_move);
  init();
  init_board_hash_tag();
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  logfile << "\nNew Game started\n";
  logfile << "----------------\n";
  logfile << "moveno = " << moveno << "\n";
  logfile.write_config_params(_config_params);
  write_diagram(logfile) << "\n";
}

void Game::figure_out_last_move(const Bitboard& new_position, col col_to_move, int half_move_counter, int moveno)
{
  // new_position.write(cout, outputtype::cmd_line_diagram, col_to_move);
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  BitMove m;
  if (_chessboard.figure_out_last_move(new_position, m))
  {
    logfile << "Could not figure out last move, must be a new game." << "\n";
    (*dynamic_cast<Bitboard*>(&_chessboard)) = new_position;
    start_new_game(col_to_move, half_move_counter, moveno);
  }
  else // OK move has been found out
  {
    logfile << "Opponents move was " << m << "\n";
    int moveindex = _chessboard.get_move_index(m);
    if (moveindex == -1)
    {
      logfile << "Coldn't find index of " << m << "\n";
      start_new_game(col_to_move, half_move_counter, moveno);
      return;
    }
    _chessboard.make_move(moveindex, _moveno);
    _move_log.push_back(_chessboard.last_move());
    actions_after_a_move();
  }
}

int Game::read_position(const std::string& filename)
{
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());

  std::ifstream is(filename);
  std::string rubbish;
  bool FEN_found = false;
  for (std::string line; std::getline(is, line);)
  {
    if (regexp_match(line, "^[[]Event.*"))
    {
      //pgn_info.set_event(get_infotext(line));
      continue;
    }
    if (regexp_match(line, "^[[]Site.*"))
    {
      //pgn_info.set_site(get_infotext(line));
      continue;
    }
    if (regexp_match(line, "^[[]Date.*"))
    {
      //pgn_info.set_date(get_infotext(line));
      continue;
    }
    if (regexp_match(line, "^[[]Round.*"))
    {
      //pgn_info.set_round(get_infotext(line));
      continue;
    }
    if (regexp_match(line, "^[[]White.*"))
    {
      //pgn_info.set_white(get_infotext(line));
      continue;
    }
    if (regexp_match(line, "^[[]Black.*"))
    {
      //pgn_info.set_black(get_infotext(line));
      continue;
    }
    if (regexp_match(line, "^[[]Result.*"))
    {
      //pgn_info.set_result(get_infotext(line));
      continue;
    }

    if (regexp_match(line, "^[[]FEN.*"))
    {
      FEN_found = true;
      cmdline << "\n" << "Position: " << line << "\n";
      std::istringstream fen_line(line);
      std::string fen_string;
      std::getline(fen_line, rubbish, '"');
      std::getline(fen_line, fen_string, '"');
      int status = _chessboard.read_position(fen_string);
      if (status != 0)
      {
        cmdline << "Read error: FEN-string could not be parsed" << "\n";
        return -1;
      }
      continue;
    }
  }
  if (!FEN_found)
    return -1;
  return 0;
}

} // namespace C2_chess
