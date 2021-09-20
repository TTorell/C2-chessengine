#ifndef _POSITION
#define _POSITION

#include <iostream>

namespace C2_chess
{

class Square;

class Position {
  protected:
    int _file;
    int _rank;

  public:
    Position();
    Position(int file, int rank);
    Position(const Position& p);
    ~Position();
    Position& operator=(const Position& p);
    bool operator==(const Square& s) const;
    bool operator==(const Position& p) const;
    int get_file() const;
    int get_rank() const;
    bool set_file(char filechar);
    bool set_file(int fileindex);
    bool set_rank(char rankchar);
    bool set_rank(int rankindex);
    bool same_file(const Position& p) const;
    bool same_rank(const Position& p) const;
    bool same_diagonal(const Position& p) const;
    friend std::ostream& operator<<(std::ostream& os, const Position& p);
};
}
#endif

