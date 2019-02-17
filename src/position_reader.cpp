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

  using std::istringstream;
  using std::regex;

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

int read_position(const string& inputfile);
istream& FEN_reader::read_position(istream& is)
{
  return is;
}

const string FEN_reader::get_infotext(const string& line)
{
  istringstream pgn_line(line);
  string rubbish, info;
  getline(pgn_line, rubbish, '"');
  getline(pgn_line, info, '"');
  return info;
}

int FEN_reader::read_position(const string& inputfile)
{
  _game.clear_chessboard();
  ifstream is(inputfile);
  string regex_string = "^[[]FEN.*";
  string rubbish;
  PGN_info pgn_info;
  bool FEN_found = false;
  for (string line; getline(is, line);)
  {
    //cout << line << endl;
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
      FEN_found = true;
      cout << endl << "Position: " << line << endl;
      istringstream fen_line(line);
      string fen_string;
      getline(fen_line, rubbish, '"');
      getline(fen_line, fen_string, '"');
      //cout << fen_string << endl;
      int status = parse_FEN_string(fen_string);
      if (status != 0)
      {
        cerr << "Read error: [FEN-string could not be parsed" << endl;
        return -1;
      }
      continue;
    }
  }
  if (!FEN_found)
    return -1;
  cout << pgn_info << endl;
  return 0;
}

int FEN_reader::parse_FEN_string(const string& FEN_string) const
{
  int rank = 8;
  int file = a;
  char c = '0';
  for (unsigned i = 0; i < FEN_string.size() && c != ' '; i++)
  {
    Piece* p = 0;
    c = FEN_string[i];
    //cout << "c = " << c << endl;
    switch (c)
    {
      case ' ':
        continue;
      case 'K':
        p = new Piece(King, white);
        break;
      case 'k':
        p = new Piece(King, black);
        break;
      case 'Q':
        p = new Piece(Queen, white);
        break;
      case 'q':
        p = new Piece(Queen, black);
        break;
      case 'B':
        p = new Piece(Bishop, white);
        break;
      case 'b':
        p = new Piece(Bishop, black);
        break;
      case 'N':
        p = new Piece(Knight, white);
        break;
      case 'n':
        p = new Piece(Knight, black);
        break;
      case 'R':
        p = new Piece(Rook, white);
        break;
      case 'r':
        p = new Piece(Rook, black);
        break;
      case 'P':
        p = new Piece(Pawn, white);
        break;
      case 'p':
        p = new Piece(Pawn, black);
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
        file += c - '0';
        if (file > h + 1)
        {
          cerr << "Read error: corrupt input " << endl;
          return -1;
        }
        break;
      case '/':
        file = a;
        rank--;
        if (rank < 1)
        {
          cerr << "Read error: corrupt input" << endl;
          return -1;
        }
        break;
      default:
        cerr << "Read error: unexpected character" << c << endl;
        return -1;
    }
    if (p)
    {
      if (file > h)
        cerr << "Read error: corrupt input" << endl;
      _game.put_piece(p, file, rank);
      file++;
    }
  } // end of for-loop

  // Read the trailing information
  istringstream is(FEN_string);
  string str;
  //skip until trailing info
  getline(is, str, ' ');
  string col_to_move;
  is >> col_to_move;
  //cout << "col_to_move = " << col_to_move << endl;
  if (col_to_move == "w")
  {
    _game.set_col_to_move(white);
    _game.set_col_to_start(white);
  }
  else if (col_to_move == "b")
  {
    _game.set_col_to_move(black);
    _game.set_col_to_start(black);
  }
  else
  {
    cerr << "Read Error: unexpected color" << endl;
    return -1;
  }
  Castling_state cs;
  is >> cs;
  cout << "cs = " << cs << endl;
  _game.set_castling_state(cs);
  string eps; // en passant string;
  is >> eps;
  cout << "x" << eps << "x" << endl;
  if (eps.size() > 2)
  {
    cerr << "Read Error: unexpected en passant character" << endl;
    return -1;
  }
  if (eps.size() == 1 && eps != "-")
  {
    cerr << "Read Error: unexpected en passant character" << endl;
    return -1;
  }
  if (eps.size() == 2 && (eps[0] < 'a' || eps[0] > 'h'))
  {
    cerr << "Read Error: unexpected en passant character" << endl;
    return -1;
  }
  if (eps.size() == 2 && (eps[1] != '3' && eps[1] != '6'))
  {
    cerr << "Read Error: unexpected en passant character" << endl;
    return -1;
  }
  if (eps.size() == 2)
    _game.set_en_passant_square(eps[0] - 'a', eps[1] - '1' + 1);

  int half_move_counter;
  is >> half_move_counter;
  _game.set_half_move_counter(half_move_counter);

  int move_number;
  is >> move_number;
  _game.set_moveno(move_number);
  return 0;
}
}
