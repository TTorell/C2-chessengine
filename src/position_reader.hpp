#ifndef _POSITION_READER
#define _POSITION_READER

#include <iostream>
#include <string>

namespace C2_chess
{

class Bitboard;

class Game;
class Board;

class Position_reader {
  protected:
    Game& _game;

  public:
    Position_reader(Game& game);

    virtual int read_position(const std::string& inputfile) = 0;

    virtual std::istream& read_position(std::istream& is) = 0;

    virtual ~Position_reader()
    {
    }
};

class FEN_reader: public Position_reader {
  public:
    FEN_reader(Game& game);
    ~FEN_reader();

    int parse_FEN_string(const std::string& FEN_string, Bitboard& board) const;
    int read_position(const std::string& inputfile);
    std::istream& read_position(std::istream& is);
    int parse_FEN_string(const std::string& FEN_string, Board& b) const;

  private:
    const std::string get_infotext(const std::string& line);
    void parse_string();
};
}
#endif
