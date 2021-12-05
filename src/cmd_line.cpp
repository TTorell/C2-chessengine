#include <cstring>
#include "chesstypes.hpp"
#include "chessfuncs.hpp"
#include "piece.hpp"
#include "square.hpp"
#include "bitboard_with_utils.hpp"
#include "game.hpp"
#include "current_time.hpp"
//#include "position_reader.hpp"

namespace// fileprivate namespace
{
C2_chess::CurrentTime current_time;

bool read_piece_type(C2_chess::piecetype& pt, char ch)
{
  // method not important for efficiency
  switch (ch)
  {
    case 'P':
      pt = C2_chess::piecetype::Pawn;
      break;
    case 'K':
      pt = C2_chess::piecetype::King;
      break;
    case 'N':
      pt = C2_chess::piecetype::Knight;
      break;
    case 'B':
      pt = C2_chess::piecetype::Bishop;
      break;
    case 'Q':
      pt = C2_chess::piecetype::Queen;
      break;
    case 'R':
      pt = C2_chess::piecetype::Rook;
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

} // End fileprivate namespace

namespace C2_chess
{
// Method for the cmdline-interface
static int write_menue_get_choice()
{
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());

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
        Game game(col::white,
                  playertype::human,
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
        cmdline << "Sorry, 1, 2 or 3 was the options" << "\n" << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(3));
        continue;
    }
  }
}

int Player::make_a_move(uint8_t& move_no, float& score, const uint8_t& max_search_level)
{
  if (_type == playertype::human)
  {
    return _chessboard.make_move(playertype::human, move_no);
  }
  else // _type == computer
  {
    int8_t best_move_index;
    float alpha = -100, beta = 100;
    _chessboard.clear_hash();
    _chessboard.init_material_evaluation();
    if (_color == col::white)
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
  // Tell the engine that there are no time limits.
  // The time it takes is defined by the max_searh_level
  _chessboard.set_time_left(true);
  const int max_search_level = 7;
  write_diagram(cmdline);
  _playing = true;
  while (_playing)
  {
    uint64_t nsec_start = current_time.nanoseconds();
    if (get_playertype(_col_to_move) == playertype::human)
    {
//      cmdline << "Hashtag: " << _chessboard.get_hash_tag() << "\n";
//      cmdline << "Material evaluation: " << _chessboard.get_material_evaluation() << "\n";
      if (_player[index(_col_to_move)]->make_a_move(_moveno, _score, max_search_level) != 0)
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

int Bitboard::make_move(playertype player, uint8_t& move_no)
{
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());

  if (player != playertype::human)
  {
    Shared_ostream& logfile = *(Shared_ostream::get_instance());
    logfile << "Error: Using wrong make_move() method for computer." << "\n";
    return -1;
  }
  Position from;
  Position to;
  uint8_t move_props = 0;
  char st[100];
  bool first = true;
  while (true)
  {
    if (!first)
      cmdline << "The Move you entered is impossible!" << "\n\n";
    first = false;
    cmdline << "Your move: ";
    std::cin >> st;
    bool from_file_read = false;
    bool from_rank_read = false;
    bool to_file_read = false;
    bool to_rank_read = false;
    bool ep_checked = false;
    bool en_passant = false;
    bool check = false;
    bool take = false;
    piecetype pt = piecetype::Pawn;
    bool promotion = false;
    piecetype promotion_pt = piecetype::Queen;
    int i;
    for (i = 0; i < (int) strlen(st); i++)
    {
      if (!from_file_read)
      {
        if (from.set_file(st[i]))
          from_file_read = true;
        else if (!read_piece_type(pt, st[i]))
          break;
        continue;
      };
      if (!from_rank_read)
      {
        if (from.set_rank(st[i]))
          from_rank_read = true;
        else
          break;
        continue;
      }
      if (!to_file_read)
      {
        if (to.set_file(st[i]))
          to_file_read = true;
        else if (st[i] != '-' && st[i] != 'x')
          break;
        else if (st[i] == 'x')
          take = true;
        continue;
      }
      if (!to_rank_read)
      {
        if (to.set_rank(st[i]))
          to_rank_read = true;
        else
          break;
        continue;
      }
      if (!ep_checked)
      {
        if (st[i] == 'e')
        {
          en_passant = true;
          move_props |= move_props_en_passant;
        }
        else if (read_piece_type(promotion_pt, st[i]))
        {

        }
        else if (st[i] == '+')
        {
          check = true;
          move_props |= move_props_check;
        }
        else
          break;
        ep_checked = true;
        continue;
      }
      if (en_passant)
        if (st[i] == '.' || st[i] == 'p')
          continue;
      if (!check)
      {
        if (st[i] == '+')
        {
          check = true;
          move_props |= move_props_check;
        }
      }
    } // end of for loop
    if (i < (int) strlen(st))
      continue;
    uint64_t from_square = file[from.get_file()] & rank[from.get_rank()];
    if ((from_square & _own->pieces) == zero)
      continue;

    // Check take
    uint64_t to_square = file[to.get_file()] & rank[to.get_rank()];

    if ((to_square & _other->pieces) == zero)
    {
      if (take == true)
        continue;
    }
    else
    {
      take = true;
      move_props |= move_props_capture;
    }
    // Check piece_type
    if (pt != piecetype::Pawn)
      if (get_piece_type(from_square) != pt)
        continue;
    pt = get_piece_type(from_square);
    // Check promotion
    if (promotion) //It is supposed to be a promotion
    {
      if (_col_to_move == col::white)
      {
        if (from.get_rank() != 7)
          continue;
        else if (get_piece_type(from_square) != piecetype::Pawn)
          continue;
      }
      else //col_to_move==col::black
      {
        if (from.get_rank() != 2)
          continue;
        else if (get_piece_type(from_square) != piecetype::Pawn)
          continue;
      }
    }
    if (pt == piecetype::Pawn)
    {
      if (_ep_square)
        if (_ep_square & to_square)
        {
          en_passant = true;
          move_props |= move_props_en_passant;
        }
    }
    if (pt == piecetype::King && _col_to_move == col::white)
    {
      if ((from_square & e1_square) && (to_square & (g1_square | c1_square)))
        move_props |= move_props_castling;
    }
    else if (pt == piecetype::King && _col_to_move == col::black)
    {
      if ((from_square & e8_square) && (to_square & (g8_square | c8_square)))
        move_props |= move_props_castling;
    }
    BitMove m(pt, move_props, from_square, to_square, promotion_pt);
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
