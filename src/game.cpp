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
C2_chess::Current_time steady_clock;
}

namespace C2_chess
{

Game::Game(Config_params& config_params) :
    _is_first_position(true),
    _move_log(),
    _chessboard(),
    _player_type{playertype::human, playertype::computer },
    _score(0),
    _config_params(config_params),
    _playing(false)
{
  _chessboard.read_position(initial_position);
  init();
}

Game::Game(col color, Config_params& config_params) :
    _is_first_position(true),
    _move_log(),
    _chessboard(),
    _player_type{playertype::human, playertype::computer },
    _score(0),
    _config_params(config_params),
    _playing(false)
{
  _player_type[index(color)] = playertype::human;
  _player_type[index(other_color(color))] = playertype::computer;
  _chessboard.read_position(initial_position, true);
  init();
}

Game::Game(playertype pt1,
           playertype pt2,
           Config_params& config_params) :
    _is_first_position(true),
    _move_log(),
    _chessboard(),
    _player_type{pt1, pt2 },
    _score(0),
    _config_params(config_params),
    _playing(false)
{
  _chessboard.read_position(initial_position, true);
  init();
}

Game::~Game()
{
}

void Game::init()
{
  _is_first_position = true;
  _move_log.clear_and_init(_chessboard.get_col_to_move(), _chessboard.get_move_number());
  _chessboard.init();
}

void Game::clear_move_log(col col_to_start, uint16_t move_number)
{
  _move_log.clear_and_init(col_to_start, move_number);
}

void Game::setup_pieces()
{
  _chessboard.read_position(initial_position);
}

col Game::get_col_to_move() const
{
  return _chessboard.get_col_to_move();
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

std::ostream& Game::write_movelog(std::ostream& os) const
{
  os << _move_log;
  return os;
}

std::ostream& Game::write_movelist(std::ostream& os) const
{
  _chessboard.write_movelist(os, true);
  return os;
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
  if (is_close(evaluation, eval_max) || is_close(evaluation, eval_min))
  {
    _chessboard.set_mate();
    cmdline << (is_close(evaluation, eval_max)? "1 - 0, black was mated":"0 - 1, white was mated") << "\n" << "\n";
    logfile << (is_close(evaluation, eval_max)? "1 - 0, black was mated":"0 - 1, white was mated") << "\n";
    _playing = false;
  }
  else if (is_close(evaluation, 0.0F) && _chessboard.no_of_moves() == 0)
  {
    _chessboard.set_stalemate();
    cmdline << "1/2 - 1/2 draw by stalemate" << "\n";
    logfile << "1/2 - 1/2 draw by stalemate" << "\n";
    _playing = false;
  }
  else if (is_close(evaluation, 0.0F) && _chessboard.is_threefold_repetition())
  {
    _chessboard.set_draw_by_repetition();
    cmdline << "1/2 - 1/2 draw by repetition" << "\n";
    logfile << "1/2 - 1/2 draw by repetition" << "\n";
    _playing = false;
  }
  else // Normal position
  {
    cmdline << "current position evaluation: " << evaluation << "\n";
    logfile << "current position evaluation: " << evaluation << "\n";
    if (_chessboard.is_draw_by_50_moves())
    {
      _chessboard.set_draw_by_50_moves();
      cmdline << "1/2 - 1/2 draw by the fifty-move rule" << "\n";
      logfile << "1/2 - 1/2 draw by the fifty-move rule" << "\n";
      _playing = false;
    }
  }
  // cmdline <<"Time_diff_sum = " << (int)_chessboard.get_time_diff_sum() << "\n";
  // logfile << "Time_diff_sum = " << (int)_chessboard.get_time_diff_sum() << "\n";
}

int Game::find_best_move_index(float& score, unsigned int max_search_ply)
{
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  _chessboard.clear_search_info();
  _chessboard.get_search_info().max_search_depth = max_search_ply - 1;
  _chessboard.clear_transposition_table();

  int8_t best_move_index = -1;

  // Set initial values of alpha and beta default from whites point of view.
  const float infinity = std::numeric_limits<float>::infinity();

  steady_clock.tic();
  score = _chessboard.negamax_with_pruning(0, -infinity, infinity, best_move_index, max_search_ply);
  _chessboard.get_search_info().time_taken = steady_clock.toc_ms();
  _chessboard.get_search_info().score = score;

  if (_chessboard.get_col_to_move() == col::white)
  {
    if (best_move_index == -1 && is_close(score, -100.0F))
    {
      logfile << "White was check mated." << "\n"; // TODO
      return 0;
    }
  }
  else // col::black
  {
    if (best_move_index == -1 && is_close(score, eval_max))
    {
      logfile << "Black was check mated." << "\n"; // TODO
      return 0;
    }
  }
  if (best_move_index == -2)
  {
    logfile << "The game is a draw." << "\n";
    return 0;
  }
  return best_move_index;
}

BitMove Game::incremental_search(const std::string& max_search_time, unsigned int max_search_ply)
{
  // Incremental search, to have a best-move available as quickly as possible
  // and it actually saves search time due to move-ordering.
  // When searching incrementally we consider max_search_time.

  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  std::vector<BitMove> pv_list;
  if (!max_search_time.empty())
  {
    _chessboard.start_timer_thread(max_search_time);
    std::this_thread::sleep_for(std::chrono::microseconds(20));
  }
  else
    _chessboard.set_time_left(true);
  int8_t best_move_index = -1;
  for (unsigned int i = 2; i <= max_search_ply; i++)
  {
    int move_index = find_best_move_index(_score, i);

    // Has the search on this ply been aborted by time limit?
    if (move_index == -1)
    {
      // This happens when max_search_ply has been set to 1
      // or when the search has been interrupted by the time limit.
      // (Or possibly when something has gone wrong.)
      // My min() or max() will just evaluate the current position
      // then and wont be able to choose best move.
      // So, searching with searh_ply = 1 is completely pointless.
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
          // the first move.
          logfile << static_cast<int>(i) << "interrupted on max_search_ply = 2" << "\n";
          best_move_index = 0;
        }
      }
      break;
    }
    best_move_index = move_index;

    if (has_time_left())
    {
      _chessboard.get_pv_line(pv_list);
      std::stringstream ss;
      write_vector(pv_list, ss, true);
      logfile.write_search_info(_chessboard.get_search_info(), ss.str());
    }
    logfile << "PV_list: " << pv_list << "\n";

    if (_score > eval_max / 2) // Forced mate
      break;
  }

  if (best_move_index >= 0)
  {
    _chessboard.make_move(static_cast<uint8_t>(best_move_index));
    _move_log.push_back(_chessboard.last_move());
  }
  actions_after_a_move();

// Stop possibly running timer by setting time_left to false.
  _chessboard.set_time_left(false);
  return _chessboard.last_move();
}

BitMove Game::engine_go(const Config_params& config_params, const std::string& max_search_time)
{
  Shared_ostream& logfile = *(Shared_ostream::get_instance());

// This value is just a default value;
  bool use_incremental_search = true;
  unsigned int max_search_ply = 7;

// Read some configuration parameters
  std::string s = config_params.get_config_param("max_search_level");
  if (!s.empty())
    max_search_ply = static_cast<unsigned int>(std::atoi(s.c_str()));
  s = config_params.get_config_param("use_incremental_search");
  if (!s.empty())
    use_incremental_search = (s == "true");

  _chessboard.init_material_evaluation(); // TODO: Should this be placed somewhere else, init_piece_state()?

// Search for best move
  if (use_incremental_search)
  {
    return incremental_search(max_search_time, max_search_ply);
  }
  else
  {
    // Not incremental search, start searching directly at max_search_ply and stop when finished.
    // Ignore max_search_time.
    _chessboard.set_time_left(true);
    int best_move_index = -1;
    int move_index = find_best_move_index(_score, max_search_ply);
    if (move_index == -1)
    {
      // This happens when max_search_ply has been set to 1.
      // (Or possibly when something else has gone wrong.)
      // My min() or max() will just evaluate the current position
      // then and wont be able to choose best move.
      // So, searching with max_search_ply = 1 is completely pointless.
      logfile << "Error: find_best_move() returned -1." << "\n";
      // We can't choose. Just set it to the first move.
      move_index = 0;
    }
    best_move_index = move_index;
//    uint64_t nsec_stop = current_time.nanoseconds();
//    logfile.log_time_diff(nsec_stop, nsec_start, max_search_level, _chessboard.get_possible_move(best_move_index), _score);
    _chessboard.make_move(best_move_index);
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

void Game::start_new_game()
{
  init();
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  logfile << "\nNew Game started\n";
  logfile << "----------------\n";
  logfile << "move number = " << _chessboard.get_move_number() << "\n";
  logfile.write_config_params(_config_params);
  write_diagram(logfile) << "\n";
}

void Game::figure_out_last_move(const Bitboard& new_position)
{
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  BitMove m;

  int return_value = _chessboard.figure_out_last_move(new_position, m);
  if (return_value != 0)
  {
    logfile << "Couldn't figure out last move, must be a new game." << "\n";
    switch (return_value)
    {
      case -1:
        logfile << "_half_move_counter values didn't match." << "\n";
        break;
      case -2:
        logfile << "Move numbers didn't match." << "\n";
        break;
      case -3:
        logfile << "Move-colors didn't match." << "\n";
        break;
      case -4:
        case -5:
        logfile << "En passant problems." << "\n";
        break;
      case -6:
        case -7:
        case -8:
        case -9:
        logfile << "Castling problems." << "\n";
        break;
      case -10:
        logfile << "Piece-diff is too big." << "\n";
        break;
      default:
        logfile << "Unexpected return value." << "\n";
    }
    (*dynamic_cast<Bitboard*>(&_chessboard)) = new_position;
    start_new_game();
    return;
  }
  else // OK move has been found out
  {
    logfile << "Opponents move was " << m << "\n";
    int moveindex = _chessboard.get_move_index(m);
    if (moveindex == -1)
    {
      logfile << "Coldn't find index of " << m << "\n";
      (*dynamic_cast<Bitboard*>(&_chessboard)) = new_position;
      start_new_game();
      return;
    }
    _chessboard.make_move(moveindex);
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

int Game::read_position_FEN(const std::string& FEN_string)
{
// TODO: read new position to temporary board, so
// we can call figure_out_last_move().
  Bitboard new_position;
  if (new_position.read_position(FEN_string, true) != 0) // true means init_piece_state().
    return -1;
  figure_out_last_move(new_position);
//  _chessboard.find_legal_moves(gentype::all);
  return 0;
}

void Game::make_move(const std::string& move)
{
  _chessboard.make_move(move);
  _move_log.push_back(_chessboard.last_move());
}

} // namespace C2_chess
