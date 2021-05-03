#include "chesstypes.hpp"
#include "chessfuncs.hpp"
#include "piece.hpp"
#include "square.hpp"
#include "board.hpp"
#include "game.hpp"
#include "current_time.hpp"
#include "position_reader.hpp"

using namespace std;
using namespace std::chrono;

namespace
{
C2_chess::CurrentTime current_time;
}

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

    string answer;
    cin >> answer;
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

  string input;
  cmdline << "\n" << "Back to main menu? [y/n]:";
  cin >> input;
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
    cin >> st;
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
        string input;

        col human_color = white_or_black();
        Game game(human_color, config_params);
        FEN_reader fr(game);
        Position_reader& pr = fr;
        string filename = GetStdoutFromCommand("cmd java -classpath \".\" ChooseFile");
        int status = pr.read_position(filename);
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
        this_thread::sleep_for(seconds(3));
        continue;
    }
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
      _move_log.into_as_last(new Move(_chessboard.get_last_move()));
      actions_after_a_move();
    }
    else
    {
      // computer
      engine_go(_config_params, "40000");
    }
    uint64_t timediff = current_time.nanoseconds() - nsec_start;
    cmdline << "time spent thinking: " << timediff/1.0e9 << " seconds" << "\n";
    logfile << "time spent thinking: " << timediff/1.0e9 << " seconds" << "\n";
  }
}

// This method is only for the cmd-line interface
int Board::make_move(playertype player, int &move_no, col col_to_move)
{
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());

  unique_ptr<Move> m;
  if (player != playertype::human)
  {
    Shared_ostream& logfile = *(Shared_ostream::get_instance());
    logfile << "Error: Using wrong make_move() method for computer." << "\n";
    return -1;
  }
  //this->write("testfile.doc");
  //   cerr<<"*** Reading the latest move ***\n";
  Position from;
  Position to;
  char st[100];
  bool first = true;
  while (true)
  {
    if (!first)
      cmdline << "The Move you entered is impossible!" << "\n\n";
    first = false;
    cmdline << "Your move: ";
    cin >> st;
    bool from_file_read = false;
    bool from_rank_read = false;
    bool to_file_read = false;using namespace std;
    bool to_rank_read = false;
    bool ep_checked = false;
    bool en_passant = false;
    bool check = false;
    bool take = false;
    piecetype pt = piecetype::Pawn;
    bool promotion = false;
    piecetype promotion_pt = piecetype::Pawn;
    piecetype target_pt = piecetype::Pawn;
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
          en_passant = true;
        else if (read_piece_type(promotion_pt, st[i]))
        {

        }
        else if (st[i] == '+')
          check = true;
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
          check = true;
      }
    } // end of for loop
    if (i < (int) strlen(st))
      continue;
    Square *from_square = _file[from.get_file()][from.get_rank()];
    Piece *p = from_square->get_piece();
    if (!p)
      continue;

    // Check take
    Square *to_square = _file[to.get_file()][to.get_rank()];
    Piece *p2 = to_square->get_piece();
    if (!p2)
    {
      if (take == true)
        continue;
    }
    else if (p2->get_color() == p->get_color())
      continue;
    else
    {
      take = true;
      target_pt = p2->get_type();
    }
    // Check piece_type
    if (pt != piecetype::Pawn)
      if (p->get_type() != pt)
        continue;
    pt = p->get_type();
    // Check promotion
    if (promotion) //It is supposed to be a promotion
    {
      if (col_to_move == col::white)
      {
        if (from.get_rank() != 7)
          continue;
        else if (_file[from.get_file()][from.get_rank()]->get_piece()->get_type() != piecetype::Pawn)
          continue;
      }
      else //col_to_move==col::black
      {
        if (from.get_rank() != 2)
          continue;
        else if (_file[from.get_file()][from.get_rank()]->get_piece()->get_type() != piecetype::Pawn)
          continue;
      }
    }
    if (pt == piecetype::Pawn)
    {
      if (_en_passant_square)
        if (_en_passant_square->get_position() == to)
          en_passant = true;
    }
    m.reset(new Move(from, to, pt, take, target_pt, en_passant, promotion, promotion_pt, check));
    // find index of move
    int move_index;
    if (!_possible_moves.in_list(m.get(), move_index))
    {
      continue;
    }
    //  Move is OK,make it
    return make_move(move_index, move_no, col_to_move);
  } // while not read
}

ostream& Board::write_cmdline_style(ostream &os, outputtype wt, col from_perspective) const
{
  switch (wt)
  {
    case outputtype::debug:
      os << "The latest move was: ";
      os << _last_move << endl;
      os << "Castling state is: " << _castling_state << endl;
      os << "En passant square is: " << _en_passant_square << endl;
      for (int fileindex = a; fileindex <= h; fileindex++)
        for (int rankindex = 1; rankindex <= 8; rankindex++)
          _file[fileindex][rankindex]->write_describing(os);
      os << endl << "*** Possible moves ***" << endl;
      for (int i = 0; i < _possible_moves.size(); i++)
        os << *_possible_moves[i] << endl;
      os << endl;
      this->write(os, outputtype::cmd_line_diagram, col::white) << endl;
      break;
    case outputtype::cmd_line_diagram:
      if (from_perspective == col::white)
      {
        os << "###################" << endl;
        for (int i = 8; i >= 1; i--)
        {
          os << "#";
          for (int j = a; j <= h; j++)
          {
            os << "|";
            Piece *p = _file[j][i]->get_piece();
            if (p)
              p->write_diagram_style(os);
            else
              os << "-";
          }
          os << "|#" << " " << i << endl;
        }
        os << "###################" << endl;
        os << "  a b c d e f g h " << endl;
      }
      else // From blacks point of view
      {
        os << "###################" << endl;
        for (int i = 1; i <= 8; i++)
        {
          os << "#";
          for (int j = h; j >= a; j--)
          {
            os << "|";
            Piece *p = _file[j][i]->get_piece();
            if (p)
              p->write_diagram_style(os);
            else
              os << "-";
          }
          os << "|#" << " " << i << endl;
        }
        os << "###################" << endl;
        os << "  h g f e d c b a " << endl;
      }
      break;
    default:
      os << "Sorry: Output type not implemented yet." << endl;
  }
  return os;
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


} // namespace C2_CHESS
