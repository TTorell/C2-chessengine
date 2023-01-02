#include <cstring>
#include "chesstypes.hpp"
#include "chessfuncs.hpp"
#include "bitboard_with_utils.hpp"
#include "game.hpp"
#include "current_time.hpp"
#include "uci.hpp"
#include "shared_ostream.hpp"
#include "magic_enum.hpp"

namespace // file-private namespace
{
C2_chess::Current_time current_time;

bool read_promotion_piecetype(C2_chess::Piecetype& pt, char ch)
{
  // method not important for efficiency
  switch (ch)
  {
    case 'Q':
      pt = C2_chess::Piecetype::Queen;
      break;
    case 'R':
      pt = C2_chess::Piecetype::Rook;
      break;
    case 'N':
      pt = C2_chess::Piecetype::Knight;
      break;
    case 'B':
      pt = C2_chess::Piecetype::Bishop;
      break;
    default:
      return false;
  }
  return true;
}

int write_menue_get_choice()
{
  C2_chess::Shared_ostream& cmdline = *(C2_chess::Shared_ostream::get_cout_instance());

  cmdline << "\n";
  cmdline << "----------------------------" << "\n";
  cmdline << "Welcome to C2 Chess Program." << "\n";
  cmdline << "" << "\n";
  cmdline << "Menue:" << "\n";
  cmdline << "1. Play a game against C2." << "\n";
  cmdline << "2. Load a position from a .pgn-file" << "\n";
  cmdline << "   and start playing from there." << "\n";
  cmdline << "3. Play both sides." << "\n";
  cmdline << "" << "\n";

  while (true)
  {
    cmdline << "Pick a choice [1]: ";

    std::string answer;
    std::cin >> answer;
    if (answer.size() == 0)
      return 1;
    switch (answer[0])
    {
      case '1':
        return 1;
      case '2':
        return 2;
      case '3':
        return 3;
      default:
        cmdline << "Sorry, try again." << "\n";
        continue;
    }
  }
}

int back_to_main_menu()
{
  C2_chess::Shared_ostream& cmdline = *(C2_chess::Shared_ostream::get_cout_instance());

  std::string input;
  cmdline << "\n" << "Back to main menu? [y/n]:";
  std::cin >> input;
  if (input == "y")
    return 0;
  else
  {
    cmdline << "Exiting C2" << "\n";
    return -1;
  }
}

C2_chess::Color white_or_black()
{
  C2_chess::Shared_ostream& cmdline = *(C2_chess::Shared_ostream::get_cout_instance());

  char st[100];
  bool try_again = true;
  while (try_again)
  {
    cmdline << "Which side would you like to play ? [w/b]: ";
    std::cin >> st;
    if (st[0] == 'w')
    {
      return C2_chess::Color::White;
      try_again = false;
    }
    else if (st[0] == 'b')
    {
      return C2_chess::Color::Black;
      try_again = false;

    }
    else
      cmdline << "Enter w or b" << "\n";
  }
  return C2_chess::Color::White;
}

} // End of fileprivate namespace

namespace C2_chess
{

// Method for the cmdline-interface
void play_on_cmd_line(Config_params& config_params)
{
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());

  while (true)
  {
    int choice = write_menue_get_choice();
    switch (choice)
    {
      case 1:
        {
        Color side = white_or_black();
        Game game(side, config_params);
        game.setup_pieces();
        game.init();
        game.start();
        if (back_to_main_menu() != 0)
          return;
        continue;
      }
      case 2:
        {
        std::string input;

        Color human_color = white_or_black();
        Game game(human_color, config_params);
        std::string filename = get_stdout_from_cmd("cmd java -classpath \".\" ChooseFile");
        int status = game.read_position(filename);
        if (status != 0)
        {
          cmdline << "Sorry, Couldn't read position from " << filename << "\n";
          continue;
        }
        game.init();
        game.start();
        if (back_to_main_menu() != 0)
          exit(0);
        continue;
      }
      case 3:
        {
        Game game(Playertype::Human,
                  Playertype::Human,
                  config_params);
        game.setup_pieces();
        game.init();
        game.start();
        if (back_to_main_menu() != 0)
          return;
        continue;
      }
      default:
        cmdline << "Sorry, 1, 2 or 3 were the options" << "\n" << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(3));
        continue;
    }
  }
}

int Game::make_a_move(float& score, const uint8_t max_search_ply)
{
  if (_player_type[index(_chessboard.get_side_to_move())] == Playertype::Human)
  {
    return dynamic_cast<Bitboard*>(&_chessboard)->make_move(Playertype::Human);
  }
  else // _type == computer
  {
    Bitmove best_move;
    _chessboard.clear_transposition_table();
    _chessboard.clear_search_vars();
    _chessboard.init_material_evaluation();
    _chessboard.set_search_ply(0);
    score = _chessboard.negamax_with_pruning(-infinite, infinite, best_move, max_search_ply);
    if (best_move == NO_MOVE && is_close(score, -100.0F))
    {
      std::cout << "White was check mated." << std::endl; // TODO
      return 0;
    }
    if (best_move == NO_MOVE && is_close(score, 100.0F))
    {
      std::cout << "Black was check mated." << std::endl; // TODO
      return 0;
    }
    Takeback_state tb_state_dummy;
    _chessboard.new_make_move(best_move, tb_state_dummy);
    return 0;
  }
}

// This method is for the cmd-line interface only
void Game::start()
{
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());

  logfile << "\nNew Game started\n\n";
  logfile << _config_params;
  cmdline << "\n" << "New Game started" << "\n";
// Init the hash tag for the initial board-position to
// use in the Zobrist hash transposition-table.
  init_board_hash_tag();
// Generate all moves.
//  _chessboard.find_legal_moves(*_chessboard.get_movelist(0), Gentype::All);
// Tell the engine that there are no time limits.
// The time it takes is defined by the max_searh_level
  _chessboard.set_time_left(true);
  const int max_search_ply = 7;
  write_diagram(cmdline);
  _playing = true;
  while (_playing)
  {
    uint64_t nsec_start = current_time.nanoseconds();
    if (_player_type[index(_chessboard.get_side_to_move())] == Playertype::Human)
    {
      if (make_a_move(_score, max_search_ply) != 0)
      {
        cmdline << "Stopped playing" << "\n";
        _playing = false;
      }
      // Update the movelog of the game.
      _move_log.push_back(_chessboard.get_latest_move());
      actions_after_a_move();
    }
    else
    {
      // computer
      std::cout << "time_left before engine_go: " << has_time_left() << std::endl;
      Go_params go_params;
      go_params.movetime = 40000000.0; // Just something big (in milliseconds).
      engine_go(_config_params, go_params);
    }
    uint64_t timediff = current_time.nanoseconds() - nsec_start;
    cmdline << "time spent thinking: " << timediff / 1.0e9 << " seconds" << "\n";
    logfile << "time spent thinking: " << timediff / 1.0e9 << " seconds" << "\n";
  }
}
bool Bitboard::is_in_movelist(list_ref movelist, const Bitmove& m) const
{
  for (const Bitmove& move : movelist)
  {
    if (m == move)
      return true;
  }
  return false;
}

int Bitboard::make_move(Playertype player)
{
  assert(player == Playertype::Human); // This method is only for humans playing on the cmd-line.
  if (player != Playertype::Human)
    return -1;
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());

  uint8_t from_file_idx;
  uint8_t from_rank_idx;
  uint8_t to_file_idx;
  uint8_t to_rank_idx;
  Piecetype p_type;
  Piecetype promotion_piecetype = Piecetype::Queen; // Default
  uint8_t move_props = move_props_none;
  std::string move_string;

  bool first = true;
  while (true)
  {
    if (!first)
      cmdline << "The Move you entered is impossible!" << "\n\n";
    first = false;
    cmdline << "(Just enter from-square and to-square," << "\n" <<
            "like: "
            << "e2e4, or g7g8q if it's a propmotion.)" << "\n";
    cmdline << "Your move: ";
    std::cin >> move_string;
    if (move_string.size() < 4 || move_string.size() > 5)
      continue;
    if (move_string[0] < 'a' || move_string[0] > 'h')
      continue;
    if (move_string[1] < '1' || move_string[1] > '8')
      continue;
    if (move_string[2] < 'a' || move_string[2] > 'h')
      continue;
    if (move_string[3] < '1' || move_string[3] > '8')
      continue;
    from_file_idx = move_string[0] - 'a';
    from_rank_idx = move_string[1] - '0';
    to_file_idx = move_string[2] - 'a';
    to_rank_idx = move_string[3] - '0';
    if (move_string.size() == 5)
    {
      if (!read_promotion_piecetype(promotion_piecetype, move_string[4]))
        continue;
      move_props |= move_props_promotion;
    }
    if (move_props & move_props_promotion)
      if ((_side_to_move == Color::White) ? from_rank_idx != 7 : from_rank_idx != 2)
        continue;
    uint64_t from_square = file[from_file_idx] & rank[from_rank_idx];
    uint64_t to_square = file[to_file_idx] & rank[to_rank_idx];
    if ((from_square & _own->pieces) == zero)
      continue;
    p_type = get_piece_type(from_square);
    switch (p_type)
    {
    case Piecetype::Undefined:
      continue;
    case Piecetype::Pawn:
      if (to_square & _ep_square)
        move_props |= move_props_en_passant | move_props_capture;
      break;
    case Piecetype::King:
      if (_side_to_move == Color::White)
      {
        if ((from_square & e1_square) && (to_square & (g1_square | c1_square)))
          move_props |= move_props_castling;
      }
      else
      {
        if ((from_square & e8_square) && (to_square & (g8_square | c8_square)))
          move_props |= move_props_castling;
      }
      break;
    default:
      ;
    }
    if (to_square & _other->pieces)
      move_props |= move_props_capture;
    Bitmove m(p_type, get_piece_type(to_square), move_props, from_square, to_square, promotion_piecetype);
    list_t movelist;
    find_legal_moves(movelist, Gentype::All);
    if (!is_in_movelist(movelist, m))
      continue;
    //  Move is OK,make it
    Takeback_state tb_state_dummy;
    new_make_move(m, tb_state_dummy);
    return 0;
  } // while not read
}


std::ostream& Bitboard_with_utils::write_cmdline_style(std::ostream& os, const Color from_perspective) const
{
  if (from_perspective == Color::White)
  {
    os << "###################" << std::endl;
    for (int i = 8; i >= 1; i--)
    {
      os << "#";
      for (int j = a; j <= h; j++)
      {
        os << "|";
        uint64_t square = file[j] & rank[i];
        if (square & _own->pieces)
          write_piece_diagram_style(os, get_piece_type(square), _side_to_move);
        else if (square & _other->pieces)
          write_piece_diagram_style(os, get_piece_type(square), other_color(_side_to_move));
        else
          os << "-";
      }
      os << "|#" << " " << i << std::endl;
    }
    os << "###################" << std::endl;
    os << "  a b c d e f g h " << std::endl;
  }
  else // From blacks point of view
  {
    os << "###################" << std::endl;
    for (int i = 1; i <= 8; i++)
    {
      os << "#";
      for (int j = h; j >= a; j--)
      {
        os << "|";
        uint64_t square = file[j] & rank[i];
        if (square & _own->pieces)
          write_piece_diagram_style(os, get_piece_type(square), _side_to_move);
        else if (square & _other->pieces)
          write_piece_diagram_style(os, get_piece_type(square), other_color(_side_to_move));
        else
          os << "-";
      }
      os << "|#" << " " << i << std::endl;
    }
    os << "###################" << std::endl;
    os << "  h g f e d c b a " << std::endl;
  }
  return os;
}

std::ostream& Bitboard::write_piece_diagram_style(std::ostream& os, C2_chess::Piecetype p_type, C2_chess::Color side) const
{
  switch (p_type)
  {
  case Piecetype::King:
    os << ((side == Color::White) ? ("\u2654") : ("\u265A"));
    break;
  case Piecetype::Queen:
    os << ((side == Color::White) ? ("\u2655") : ("\u265B"));
    break;
  case Piecetype::Rook:
    os << ((side == Color::White) ? ("\u2656") : ("\u265C"));
    break;
  case Piecetype::Bishop:
    os << ((side == Color::White) ? ("\u2657") : ("\u265D"));
    break;
  case Piecetype::Knight:
    os << ((side == Color::White) ? ("\u2658") : ("\u265E"));
    break;
  case Piecetype::Pawn:
    os << ((side == Color::White) ? ("\u2659") : ("\u265F"));
    break;
  default:
    std::cerr << "Undefined piece type: " << magic_enum::enum_name(p_type) << std::endl;
    Shared_ostream& logfile = *(Shared_ostream::get_instance());
    logfile << "Undefined piece type: " << magic_enum::enum_name(p_type) << "\n";
    assert(false);
  }
  return os;
}

} // namespace C2_CHESS
