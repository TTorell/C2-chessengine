#include <cstring>
#include "chesstypes.hpp"
#include "chessfuncs.hpp"
#include "bitboard_with_utils.hpp"
#include "game.hpp"
#include "current_time.hpp"

namespace // fileprivate namespace
{
C2_chess::CurrentTime current_time;

bool read_promotion_piecetype(C2_chess::piecetype& pt, char ch)
{
  // method not important for efficiency
  switch (ch)
  {
    case 'Q':
      pt = C2_chess::piecetype::Queen;
      break;
    case 'R':
      pt = C2_chess::piecetype::Rook;
      break;
    case 'N':
      pt = C2_chess::piecetype::Knight;
      break;
    case 'B':
      pt = C2_chess::piecetype::Bishop;
      break;
    default:
      return false;
  }
  return true;
}

std::ostream& write_piece_diagram_style(std::ostream& os, C2_chess::piecetype p_type, C2_chess::col color)
{
  switch (p_type)
  {
    case C2_chess::piecetype::King:
      os << ((color == C2_chess::col::white) ? ("\u2654") : ("\u265A"));
      break;
    case C2_chess::piecetype::Queen:
      os << ((color == C2_chess::col::white) ? ("\u2655") : ("\u265B"));
      break;
    case C2_chess::piecetype::Rook:
      os << ((color == C2_chess::col::white) ? ("\u2656") : ("\u265C"));
      break;
    case C2_chess::piecetype::Bishop:
      os << ((color == C2_chess::col::white) ? ("\u2657") : ("\u265D"));
      break;
    case C2_chess::piecetype::Knight:
      os << ((color == C2_chess::col::white) ? ("\u2658") : ("\u265E"));
      break;
    case C2_chess::piecetype::Pawn:
      os << ((color == C2_chess::col::white) ? ("\u2659") : ("\u265F"));
      break;
    default:
      std::cerr << ("Undefined piece type in Piece::write_diagram_style") << std::endl;
  }
  return os;
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

} // End fileprivate namespace

namespace C2_chess
{

// Method for the cmdline-interface
static int back_to_main_menu()
{
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());

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

// Method for the cmdline-interface
static col white_or_black()
{
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());

  char st[100];
  bool try_again = true;
  while (try_again)
  {
    cmdline << "Which color would you like to play ? [w/b]: ";
    std::cin >> st;
    if (st[0] == 'w')
    {
      return col::white;
      try_again = false;
    }
    else if (st[0] == 'b')
    {
      return col::black;
      try_again = false;

    }
    else
      cmdline << "Enter w or b" << "\n";
  }
  return col::white;
}

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
        col color = white_or_black();
        Game game(color, config_params);
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

        col human_color = white_or_black();
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
        Game game(playertype::human,
                  playertype::human,
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

int Game::make_a_move(uint8_t& move_no, float& score, const uint8_t max_search_level)
{
  if (_player_type[index(_chessboard.get_col_to_move())] == playertype::human)
  {
    return dynamic_cast<Bitboard*>(&_chessboard)->make_move(playertype::human, move_no);
  }
  else // _type == computer
  {
    int8_t best_move_index;
    float alpha = -100, beta = 100;
    _chessboard.clear_transposition_table();
    _chessboard.init_material_evaluation();
    if (_chessboard.get_col_to_move() == col::white)
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
    _chessboard.make_move(static_cast<uint8_t>(best_move_index), move_no);
    return 0;
  }
}

// This method is for the cmd-line interface only
void Game::start()
{
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());

  logfile << "\nNew Game started\n\n";
  logfile.write_config_params(_config_params);
  cmdline << "\n" << "New Game started" << "\n";
  // Init the hash tag for the initial board-position to
  // use in the Zobrist hash transposition-table.
  init_board_hash_tag();
  // Generate all moves.
  _chessboard.find_legal_moves(gentype::all);
  // Tell the engine that there are no time limits.
  // The time it takes is defined by the max_searh_level
  _chessboard.set_time_left(true);
  const int max_search_level = 7;
  write_diagram(cmdline);
  _playing = true;
  while (_playing)
  {
    uint64_t nsec_start = current_time.nanoseconds();
    if (_player_type[index(_chessboard.get_col_to_move())] == playertype::human)
    {
//      cmdline << "Hashtag: " << _chessboard.get_hash_tag() << "\n";
//      cmdline << "Material evaluation: " << _chessboard.get_material_evaluation() << "\n";
      if (make_a_move(_moveno, _score, max_search_level) != 0)
      {
        cmdline << "Stopped playing" << "\n";
        _playing = false;
      }
      // Update the movelog of the game.
      _move_log.push_back(_chessboard.last_move());
      actions_after_a_move();
    }
    else
    {
      // computer
      std::cout << "time_left before engine_go: " << has_time_left() << std::endl;
      engine_go(_config_params, "40000000");
    }
    uint64_t timediff = current_time.nanoseconds() - nsec_start;
    cmdline << "time spent thinking: " << timediff / 1.0e9 << " seconds" << "\n";
    logfile << "time spent thinking: " << timediff / 1.0e9 << " seconds" << "\n";
  }
}

bool Bitboard::is_in_movelist(const BitMove& m) const
{
  for (const BitMove& move:_movelist)
  {
    if (m == move)
      return true;
  }
  return false;
}

int Bitboard::make_move(playertype player, uint8_t& move_no)
{
  assert(player == playertype::human); // This method is only for humans playing on the cmd-line.
  if (player != playertype::human)
    return -1;
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());

  uint8_t from_file_idx;
  uint8_t from_rank_idx;
  uint8_t to_file_idx;
  uint8_t to_rank_idx;
  piecetype p_type;
  piecetype promotion_piecetype = piecetype::Queen; // Default
  uint8_t move_props = move_props_none;
  std::string move_string;

  bool first = true;
  while (true)
  {
    if (!first)
      cmdline << "The Move you entered is impossible!" << "\n\n";
    first = false;
    cmdline << "(Just enter from-square and to-square," << "\n" <<
        "like: " << "e2e4, or g7g8Q if it's a propmotion.)" << "\n";
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
      if ((_col_to_move == col::white) ? from_rank_idx != 7 : from_rank_idx != 2)
        continue;
    uint64_t from_square = file[from_file_idx] & rank[from_rank_idx];
    uint64_t to_square = file[to_file_idx] & rank[to_rank_idx];
    if ((from_square & _own->pieces) == zero)
      continue;
    p_type = get_piece_type(from_square);
    switch (p_type)
    {
      case piecetype::Undefined:
        continue;
      case piecetype::Pawn:
        if (to_square & _ep_square)
          move_props |= move_props_en_passant;
        break;
      case piecetype::King:
        if (_col_to_move == col::white)
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
      move_props = move_props_capture;
    BitMove m(p_type, move_props, from_square, to_square, promotion_piecetype);
    if (!is_in_movelist(m))
      continue;
    //  Move is OK,make it
    make_move(m, move_no);
    return 0;
  } // while not read
}

std::ostream& Bitboard_with_utils::write_cmdline_style(std::ostream& os, outputtype ot, col from_perspective) const
{
  switch (ot)
  {
    case outputtype::debug:
      //      os << "The latest move was: ";
//      os << _last_move << std::endl;
//      os << "Castling state is: " << _castling_state << std::endl;
//      os << "En passant square is: " << _en_passant_square << std::endl;
//      for (int fileindex = a; fileindex <= h; fileindex++)
//        for (int rankindex = 1; rankindex <= 8; rankindex++)
//          _file[fileindex][rankindex]->write_describing(os);
//      os << std::endl << "*** Possible moves ***" << std::endl;
//      for (int i = 0; i < _possible_moves.size(); i++)
//        os << *_possible_moves[i] << std::endl;
//      os << std::endl;
//      this->write(os, outputtype::cmd_line_diagram, col::white) << std::endl;
      break;
    case outputtype::cmd_line_diagram:
      if (from_perspective == col::white)
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
              write_piece_diagram_style(os, get_piece_type(square), _col_to_move);
            else if (square & _other->pieces)
              write_piece_diagram_style(os, get_piece_type(square), other_color(_col_to_move));
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
              write_piece_diagram_style(os, get_piece_type(square), _col_to_move);
            else if (square & _other->pieces)
              write_piece_diagram_style(os, get_piece_type(square), other_color(_col_to_move));
            else
              os << "-";
          }
          os << "|#" << " " << i << std::endl;
        }
        os << "###################" << std::endl;
        os << "  h g f e d c b a " << std::endl;
      }
      break;
    default:
      os << "Sorry: Output type not implemented yet." << std::endl;
  }
  return os;
}

//bool Player::mate_in(const int& n, const Board& board, int k, int& make_move_no)
//{
//  cout << "Player::mate_in " << n << " k = " << k << "col_to_move = " << _colour << std::endl;
//  bool mate1 = false;
//  int i = 0;
//  //cerr<<"board noofmoves"<<board.no_of_moves()<<std::endl;
//  while ((i < board.no_of_moves()) && (!mate1))
//  {
//    int x = 3; // just a dummy in this case
//    _b[k] = board;
//    //if (k==0)
//    //cout << "i=" << i << " k=" << k << std::endl;
//    _b[k].make_move(i++, x, _colour, true);
//    if ((_b[k].no_of_moves()))
//    {
//      //      cerr<<"bk no of moves "<<b[k].no_of_moves()<<std::endl;
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
//        // cerr<<"OK*********"<<std::endl;
//      }
//      //         else
//      //            cout << "stalemate" << std::endl;
//    }
//    //if (!mate1)
//    //   b[k].clear();
//  }
//  //cerr<<"mate_in END "<<n<<std::endl;
//  if (mate1 && (k == 0))
//  {
//    make_move_no = --i;
//    //cerr<<"the move was number"<< i << std::endl;
//  }
//  return mate1;
//}

}// namespace C2_CHESS
