#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include "game.hpp"
#include "current_time.hpp"
#include "chesstypes.hpp"
#include "chessfuncs.hpp"
#include "config_param.hpp"
#include "shared_ostream.hpp"
#include "bitboard_with_utils.hpp"
#include "uci.hpp"

namespace
{
C2_chess::Current_time steady_clock;
}

namespace C2_chess
{

Game::Game(Config_params& config_params) :
    _is_first_position(true), _move_log(), _chessboard(), _player_type {Playertype::Human, Playertype::Computer}, _score(0), _config_params(config_params), _playing(false)
{
  //std::cerr << "Game::Game(Config_params& config_params)" << std::endl;
  _chessboard.read_position(start_position_FEN, init_pieces);
}

Game::Game(Color side, Config_params& config_params) :
    _is_first_position(true), _move_log(), _chessboard(), _player_type {Playertype::Human, Playertype::Computer}, _score(0), _config_params(config_params), _playing(false)
{
  //std::cerr << "Game::Game(Color side, Config_params& config_params)" << std::endl;
  _player_type[index(side)] = Playertype::Human;
  _player_type[index(other_color(side))] = Playertype::Computer;
  _chessboard.read_position(start_position_FEN, init_pieces);
}

Game::Game(Playertype pt1, Playertype pt2, Config_params& config_params) :
    _is_first_position(true), _move_log(), _chessboard(), _player_type {pt1, pt2}, _score(0), _config_params(config_params), _playing(false)
{
  //std::cerr << "Game::Game(Playertype pt1, Playertype pt2, Config_params& config_params)" << std::endl;
  _chessboard.read_position(start_position_FEN, true);
}

Game::~Game()
{
}

void Game::init()
{
  _is_first_position = true;
  _move_log.clear_and_init(_chessboard.get_side_to_move(), _chessboard.get_move_number());
  _chessboard.init();
//  _chessboard.find_legal_moves(*_chessboard.get_movelist(0), Gentype::All);
}

void Game::clear_move_log(Color col_to_start, uint16_t move_number)
{
  _move_log.clear_and_init(col_to_start, move_number);
}

void Game::setup_pieces()
{
  _chessboard.read_position(start_position_FEN);
}

Color Game::get_side_to_move() const
{
  return _chessboard.get_side_to_move();
}

uint64_t Game::get_hash_tag() const
{
  return _chessboard.get_hash_tag();
}

float Game::get_material_diff() const
{
  return _chessboard.get_material_diff();
}

uint8_t Game::get_castling_rights() const
{
  return _chessboard.get_castling_rights();
}

uint8_t Game::get_half_move_counter() const
{
  return _chessboard.get_half_move_counter();
}

Playertype Game::get_playertype(const Color& side) const
{
  return _player_type[index(side)];
}

std::ostream& Game::write_chessboard(std::ostream& os, const Color from_perspective) const
{
  Bitboard_with_utils(_chessboard).write(os, from_perspective);
  return os;
}

std::ostream& Game::write_diagram(std::ostream& os) const
{
  Bitboard_with_utils bwu(_chessboard);
  if (_player_type[index(Color::White)] == Playertype::Human)
    bwu.write(os, Color::White) << std::endl;
  else if (_player_type[index(Color::Black)] == Playertype::Human)
    bwu.write(os, Color::Black) << std::endl;
  else
    // The computer is playing itself
    bwu.write(os, Color::White) << std::endl;
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

std::ostream& Game::write_movelist(std::ostream& os)
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

  list_t movelist;
  _chessboard.find_legal_moves(movelist, Gentype::All);
  if (movelist.size() == 0)
  {
    float evaluation = _chessboard.evaluate_empty_movelist(0);
    if (is_close(evaluation, eval_max) || is_close(evaluation, eval_min))
    {
      _chessboard.set_mate();
      cmdline << (is_close(evaluation, eval_max) ? "1 - 0, black was mated" : "0 - 1, white was mated") << "\n" << "\n";
      logfile << (is_close(evaluation, eval_max) ? "1 - 0, black was mated" : "0 - 1, white was mated") << "\n";
      _playing = false;
    }
    else if (is_close(evaluation, 0.0F))
    {
      _chessboard.set_stalemate();
      cmdline << "1/2 - 1/2 draw by stalemate" << "\n";
      logfile << "1/2 - 1/2 draw by stalemate" << "\n";
      _playing = false;
    }
  }
  else
  {
    if (_chessboard.is_threefold_repetition())
    {
      _chessboard.set_draw_by_repetition();
      cmdline << "1/2 - 1/2 draw by repetition" << "\n";
      logfile << "1/2 - 1/2 draw by repetition" << "\n";
      _playing = false;
    }
    else if (_chessboard.is_draw_by_50_moves())
    {
      _chessboard.set_draw_by_50_moves();
      cmdline << "1/2 - 1/2 draw by the fifty-move rule" << "\n";
      logfile << "1/2 - 1/2 draw by the fifty-move rule" << "\n";
      _playing = false;
    }
  }
}

Bitmove Game::find_best_move(float& score, const int search_depth, const bool nullmove_pruning)
{
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  _chessboard.clear_search_info();
  _chessboard.get_search_info().searching_side = _chessboard.get_side_to_move();
  _chessboard.get_search_info().max_search_depth = search_depth;
  _chessboard.clear_transposition_table();
  Bitmove best_move;
  steady_clock.tic();
  score = _chessboard.new_negamax_with_pruning(-infinite, infinite, best_move, search_depth, nullmove_pruning);
  _chessboard.get_search_info().time_taken = steady_clock.toc_ms();
  _chessboard.get_search_info().score = score;
  if (_chessboard.get_side_to_move() == Color::White)
  {
    if (best_move == NO_MOVE && is_close(score, eval_min))
    {
      logfile << "White was check mated." << "\n"; // TODO
    }
  }
  else // col::black
  {
    if (best_move == NO_MOVE && is_close(score, eval_max))
    {
      logfile << "Black was check mated." << "\n"; // TODO
    }
  }
  if (best_move == DRAW_BY_THREEFOLD_REPETITION)
  {
    logfile << "The game is a draw by threefold repetition." << "\n";
  }
  if (best_move == DRAW_BY_50_MOVES_RULE)
  {
    logfile << "The game is a draw by the fifty-moves rule." << "\n";
  }
  if (best_move == SEARCH_HAS_BEEN_INTERRUPTED)
  {
    logfile << "The search has been interrupted." << "\n";
  }
  return best_move;
}

void Game::send_uci_info(int search_time_ms, const std::vector<Bitmove>& pv_line)
{
  auto info = _chessboard.get_search_info();
  std::cout << "info score cp " << static_cast<int>(info.score * 100.0F) << " depth " << info.max_search_depth << " nodes " << info.node_counter << " time " << search_time_ms
            << " pv " << uci_pv_line(pv_line) << std::endl;
}

Bitmove Game::incremental_search(const double movetime_ms, const int max_depth, const bool nullmove_pruning)
{
  // Incremental search, to have a best-move available as quickly
  // as possible and it actually saves search-time due to move-ordering.
  // Best moves from the previous search depth will be searched first.
  // When searching incrementally we stop searching after movetime.
  // If movetime is zero we search infinitely until the GUI sends a stop command,
  // or the predefined search-depth limit has been reached and finished.

  // We will need some search plies for Quiescense-search too, so:
  const int max_search_depth = std::min((MAX_N_SEARCH_PLIES_DEFAULT / 2), max_depth);

  Bitmove best_move;
  Bitmove local_best_move; // Best move from a specific search-depth.

  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  std::vector<Bitmove> pv_line;

  // Keep track of how much time we have spent.
  auto time_taken_ms {0.0};
  steady_clock.tic();

  if (is_close(movetime_ms, 0.0))
  {
    // search infinitely, until the GUI sends a stop command,
    // or the search-board limit has been reached.
    _chessboard.set_time_left(true);
  }
  else
  {
    _chessboard.start_timer_thread(movetime_ms);
    //std::this_thread::sleep_for(std::chrono::microseconds(200));
  }

  _chessboard.clear_transposition_table();
  _chessboard.clear_search_vars();

  for (int search_depth = 1; search_depth <= max_search_depth; search_depth++)
  {
    _chessboard.set_iteration_depth(search_depth);
    local_best_move = find_best_move(_score, search_depth, nullmove_pruning);
    // Has the search on this ply been aborted by time limit?
    if (local_best_move == SEARCH_HAS_BEEN_INTERRUPTED)
    {
      logfile << "Time is out! or a stop-command has been received.\n";
      if (search_depth == 1)
      {
        // The search has been interrupted on lowest search_depth.
        // No best move has been found at all. What to do?
        // Just return the first move, maybe.
        logfile << "Search was interrupted on lowest search depth." << "\n";
        local_best_move = _chessboard.get_first_move();
      }
      break;
    }
    best_move = local_best_move;

    if (has_time_left())
    {
      _chessboard.write_search_info(logfile);
    }

    if (_score > eval_max / 2) // Forced mate
      break;

    time_taken_ms = static_cast<double>(steady_clock.toc_us()) / 1000.0;
    // TODO: Make it clear that movetime_ms == 0 means infinite search.
    if ((!is_close(movetime_ms, 0.0)) && time_taken_ms > movetime_ms / 2.0)
      break;
    _chessboard.get_pv_line(pv_line);
    send_uci_info(static_cast<int>(time_taken_ms), pv_line);
  }

  assert(best_move.is_valid());
  Takeback_state tb_state_dummy;
  _chessboard.new_make_move(best_move, tb_state_dummy);
  _move_log.push_back(_chessboard.get_latest_move());

  // Just write to logfile or command-line if position is mate or stalemate.
  // when command-line play, also stop the the game in that case.
  actions_after_a_move();

  // Stop possibly running timer-thread by setting time_left to false.
  _chessboard.set_time_left(false);
  return _chessboard.get_latest_move();
}

Bitmove Game::engine_go(const Config_params& config_params, const Go_params& go_params, const bool apply_max_search_depth)
{
  // Shared_ostream& logfile = *(Shared_ostream::get_instance());

  // Default values only.
  //bool use_incremental_search = true;
  //int max_search_depth = 7;
  //bool nullmove_pruning = true;

  // Read some configuration parameters
  auto max_search_depth = Get_config_value<int>("max_search_depth", config_params);
  auto use_incremental_search = Get_config_value<bool>("use_incremental_search", config_params);
  auto nullmove_pruning = Get_config_value<bool>("use_nullmove_pruning", config_params);
  _chessboard.init_material_evaluation(); // TODO: Should this be placed somewhere else, init_piece_state()?

  // Search for best move
  if (use_incremental_search)
  {
    if (go_params.infinite == true)
    {
      return incremental_search(0.0, MAX_N_SEARCH_PLIES_DEFAULT/2, nullmove_pruning); // movetime should be zero here
    }
    else if (go_params.movetime > 0.0)
    {
      if (apply_max_search_depth)
      {
        // This is for testing, to get the same best_move from different computers,
        // and when running in debug-mode or not. Different search_depths may be reached
        // and give different results otherwise.
        return incremental_search(go_params.movetime, max_search_depth, nullmove_pruning);
      }
      else
      {
        return incremental_search(go_params.movetime, MAX_N_SEARCH_PLIES_DEFAULT/2, nullmove_pruning);
      }
      //merge return incremental_search(go_params.movetime);
    }
    else if (!is_close(go_params.wtime, 0.0, 1e-10))
    {
      // Assuming that also btime, winc and binc has been set in the same command (according to the UCI-protocol),
      // try to figure out a reasonable max_search_time.
      int moves_left_approx = 40 - _chessboard.get_move_number();
      while (moves_left_approx < 10)
        moves_left_approx += 20;
      bool is_white_to_move = (_chessboard.get_side_to_move() == Color::White);
      double time = (is_white_to_move) ? go_params.wtime + moves_left_approx * go_params.winc : go_params.btime + moves_left_approx * go_params.binc;
      return incremental_search(time / moves_left_approx, MAX_N_SEARCH_PLIES_DEFAULT/2, nullmove_pruning);
    }
    else
    {
      return incremental_search(0.0, MAX_N_SEARCH_PLIES_DEFAULT/2, nullmove_pruning);
    }
  }
  else
  {
    // Not incremental search, start searching directly at max_search_depth and stop when finished.
    // Ignore movetime.
    _chessboard.set_time_left(true);
    _chessboard.set_iteration_depth(max_search_depth);
    Bitmove best_move = find_best_move(_score, max_search_depth, nullmove_pruning);
    if (best_move.is_valid())
    {
      Takeback_state tb_state_dummy;
      _chessboard.new_make_move(best_move, tb_state_dummy);
      _move_log.push_back(_chessboard.get_latest_move());
    }
  }
  actions_after_a_move();

  // Stop possibly running timer by setting time_left to false.
  _chessboard.set_time_left(false);

  return _chessboard.get_latest_move();
}

bool Game::has_time_left()
{
  return _chessboard.has_time_left();
}

void Game::set_time_left(bool value)
{
  _chessboard.set_time_left(value);
}

void Game::start_new_game()
{
  init();
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  logfile << "\nNew Game started\n";
  logfile << "----------------\n";
  logfile << "move number = " << _chessboard.get_move_number() << "\n";
  logfile << _config_params << "\n";
  //write_diagram(logfile) << "\n";
}

void Game::figure_out_last_move(const Bitboard& new_position)
{
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  Bitmove m;

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
      logfile << "Couldn't find index of " << m << "\n";
      (*dynamic_cast<Bitboard*>(&_chessboard)) = new_position;
      start_new_game();
      return;
    }
    Takeback_state tb_state_dummy;
    _chessboard.new_make_move(m, tb_state_dummy);
    _move_log.push_back(_chessboard.get_latest_move());
    actions_after_a_move();
  }
}

int Game::read_position(const Position_params& params)
{
  //Shared_ostream& logfile = *(Shared_ostream::get_instance());
  Takeback_state tb_state_dummy;
  read_position_FEN(params.FEN_string);
  if (params.moves)
  {
    for (const std::string& move : params.move_list)
    {
      make_move(move, tb_state_dummy);
    }
  }
  // write_diagram(logfile) << "\n";
  return 0;
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
  Bitboard new_position;
  if (new_position.read_position(FEN_string, init_pieces) != 0)
    return -1;
  new_position.init();
  figure_out_last_move(new_position);
  //  _chessboard.find_legal_moves(gentype::all);
  return 0;
}

void Game::make_move(const std::string& move, Takeback_state& tb_state)
{
  _chessboard.make_UCI_move(move, tb_state);
  _move_log.push_back(_chessboard.get_latest_move());
}

void Game::takeback_latest_move(Takeback_state& tb_state)
{
  _chessboard.takeback_latest_move(tb_state);
  _move_log.pop();
}

} // namespace C2_chess
