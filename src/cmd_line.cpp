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

  logfile << "\nGame started\n\n";
  logfile.write_config_params(_config_params);
  cmdline << "\n" << "Game started" << "\n";
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
    if (!_possible_moves.in_list(m.get(), &move_index))
    {
      continue;
    }
    //  Move is OK,make it
    return make_move(move_index, move_no, col_to_move);
  } // while not read
}
} // namespace C2_CHESS
