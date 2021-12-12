#ifndef _MOVELIST
#define _MOVELIST

#include <iostream>
#include <deque>
#include <vector>
#include "chesstypes.hpp"
#include "bitboard.hpp"

namespace C2_chess
{

class BitMove;
class Square;

//class Movelist
//{
//  protected:
//    enum
//    {
//      _LISTMAX = 1000
//    };
//    std::deque<Move*> _list;
//    int _listindex;
//    public:
//    Movelist();
//    Movelist(const Movelist&);
//    virtual ~Movelist();
//    Movelist& operator=(const Movelist&);
//    Move* first() const;
//    Move* next() const;
//    void into(Move* const newmove);
//    void into_as_first(Move* const newmove);
//    void into_as_last(Move* const newmove);
//    void out(Move* const rubbish);
//    int size() const;
//    bool empty() const;
//    bool in_list(const Move& m, int& index) const;
//    void clear();
//    Move* operator[](int) const;
//    virtual std::ostream& write(std::ostream& os, bool same_line = false) const;
//    friend std::ostream& operator<<(std::ostream& os, const Movelist& ml)
//    {
//      ml.write(os);
//      return os;
//    }
//};

class Movelog
{
  protected:
    col _col_to_start = col::white;
    int _first_moveno = 1;
    std::vector<BitMove> _list;
    public:
    Movelog() :
        _col_to_start(col::white),
        _first_moveno(1)
    {
    }

    Movelog(col col_to_start, int first_moveno) :
        _col_to_start(col_to_start),
        _first_moveno(first_moveno)
    {
    }

    void set_col_to_start(col color)
    {
      _col_to_start = color;
    }

    void set_first_moveno(int moveno)
    {
      _first_moveno = moveno;
    }

    std::ostream& write(std::ostream& os) const;

    void push_back(const BitMove& move)
    {
      _list.push_back(move);
    }

    void clear()
    {
      _list.clear();
    }
};
}
#endif

