#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include "position_reader.hpp"
#include "game.hpp"
#include "piece.hpp"
#include "chessfuncs.hpp"

namespace C2_chess
{

Position_reader::Position_reader(Game& game) :
    _game(game)
{
}

FEN_reader::FEN_reader(Game& game) :
    Position_reader(game)
{
}

FEN_reader::~FEN_reader()
{
}

int read_position(const std::string& inputfile);

std::istream& FEN_reader::read_position(std::istream& is)
{
  return is;
}

const std::string FEN_reader::get_infotext(const std::string& line)
{
  std::istringstream pgn_line(line);
  std::string rubbish, info;
  std::getline(pgn_line, rubbish, '"');
  std::getline(pgn_line, info, '"');
  return info;
}

int FEN_reader::read_position(const std::string& inputfile)
{
  Shared_ostream& cmdline = *(Shared_ostream::get_cout_instance());

  _game.clear_chessboard();
  std::ifstream is(inputfile);
  std::string regex_string = "^[[]FEN.*";
  std::string rubbish;
  PGN_info pgn_info;
  bool FEN_found = false;
  for (std::string line; std::getline(is, line);)
  {
    if (regexp_match(line, "^[[]Event.*"))
    {
      pgn_info.set_event(get_infotext(line));
      continue;
    }
    if (regexp_match(line, "^[[]Site.*"))
    {
      pgn_info.set_site(get_infotext(line));
      continue;
    }
    if (regexp_match(line, "^[[]Date.*"))
    {
      pgn_info.set_date(get_infotext(line));
      continue;
    }
    if (regexp_match(line, "^[[]Round.*"))
    {
      pgn_info.set_round(get_infotext(line));
      continue;
    }
    if (regexp_match(line, "^[[]White.*"))
    {
      pgn_info.set_white(get_infotext(line));
      continue;
    }
    if (regexp_match(line, "^[[]Black.*"))
    {
      pgn_info.set_black(get_infotext(line));
      continue;
    }
    if (regexp_match(line, "^[[]Result.*"))
    {
      pgn_info.set_result(get_infotext(line));
      continue;
    }

    if (regexp_match(line, "^[[]FEN.*"))
    {
      Board dummy_board;
      FEN_found = true;
      cmdline << "\n" << "Position: " << line << "\n";
      std::istringstream fen_line(line);
      std::string fen_string;
      std::getline(fen_line, rubbish, '"');
      std::getline(fen_line, fen_string, '"');
      int status = parse_FEN_string(fen_string, dummy_board);
      if (status != 0)
      {
        cmdline << "Read error: [FEN-string could not be parsed" << "\n";
        return -1;
      }
      continue;
    }
  }
  if (!FEN_found)
    return -1;
  cmdline << pgn_info << "\n";
  return 0;
}

int FEN_reader::parse_FEN_string(const std::string& FEN_string, Board& b) const
{
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  std::cout << FEN_string << std::endl;
  int rank = 8;
  int file = a;
  char ch = '0';
  Board chessboard;
  for (unsigned i = 0; i < FEN_string.size() && ch != ' '; i++)
  {
    Piece* p = 0;
    ch = FEN_string[i];
    // cout << ch << endl;
    switch (ch)
    {
      case ' ':
        continue;
      case 'K':
        p = new Piece(piecetype::King, col::white);
        break;
      case 'k':
        p = new Piece(piecetype::King, col::black);
        break;
      case 'Q':
        p = new Piece(piecetype::Queen, col::white);
        break;
      case 'q':
        p = new Piece(piecetype::Queen, col::black);
        break;
      case 'B':
        p = new Piece(piecetype::Bishop, col::white);
        break;
      case 'b':
        p = new Piece(piecetype::Bishop, col::black);
        break;
      case 'N':
        p = new Piece(piecetype::Knight, col::white);
        break;
      case 'n':
        p = new Piece(piecetype::Knight, col::black);
        break;
      case 'R':
        p = new Piece(piecetype::Rook, col::white);
        break;
      case 'r':
        p = new Piece(piecetype::Rook, col::black);
        break;
      case 'P':
        p = new Piece(piecetype::Pawn, col::white);
        break;
      case 'p':
        p = new Piece(piecetype::Pawn, col::black);
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
        file += ch - '0';
        if (file > h + 1)
        {
          logfile << "Read error: corrupt input " << "\n";
          return -1;
        }
        break;
      case '/':
        file = a;
        rank--;
        if (rank < 1)
        {
          logfile << "Read error: corrupt input" << "\n";
          return -1;
        }
        break;
      default:
        logfile << "Read error: unexpected character" << ch << "\n";
        return -1;
    }
    if (p)
    {
      if (file > h)
        logfile << "Read error: corrupt input" << "\n";
      chessboard.put_piece(p, file, rank);
      file++;
    }
  } // end of for-loop

  // Read the trailing information
  std::istringstream is(FEN_string);
  std::string str;
  //skip until trailing info
  std::getline(is, str, ' ');
  std::string col_to_move;
  is >> col_to_move;
  // cout <<"col_to_move = " << col_to_move << endl;
  if (col_to_move != "w" and col_to_move != "b")
  {
    logfile << "Read Error: unexpected color" << "\n"; // TODO write to logfile if engine
    return -1;
  }
  Castling_state cs;
  is >> cs;
  // cout << "cs = " << cs << endl;
  chessboard.set_castling_state(cs);
  std::string eps; // en passant string;
  is >> eps;
  // cout << "eps = " << eps << endl;
  if (eps.size() > 2)
  {
    logfile << "Read Error: unexpected en passant character" << "\n";
    return -1;
  }
  if (eps.size() == 1 && eps != "-")
  {
    logfile << "Read Error: unexpected en passant character" << "\n";
    return -1;
  }
  if (eps.size() == 2 && (eps[0] < 'a' || eps[0] > 'h'))
  {
    logfile << "Read Error: unexpected en passant character" << "\n";
    return -1;
  }
  if (eps.size() == 2 && (eps[1] != '3' && eps[1] != '6'))
  {
    logfile << "Read Error: unexpected en passant character" << "\n";
    return -1;
  }
  if (eps.size() == 2)
    chessboard.set_enpassant_square(eps[0] - 'a', eps[1] - '1' + 1);
  int half_move_counter;
  is >> half_move_counter;
  // cout << "half_move_counter = " << half_move_counter << endl;
  int move_number;
  is >> move_number;
  // cout << "move_number = " << move_number << endl;
  b = chessboard;
  _game.figure_out_last_move(chessboard, col_from_string(col_to_move), half_move_counter, move_number);
  return 0;
}
} // End namespace 
