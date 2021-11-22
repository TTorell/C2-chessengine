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
      Bitboard dummy_board;
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

int FEN_reader::parse_FEN_string(const std::string& FEN_string, Bitboard& board) const
{
  board.read_position(FEN_string);
  return 0;
}

} // End namespace 
