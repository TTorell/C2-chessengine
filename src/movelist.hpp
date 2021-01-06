#ifndef _MOVELIST
#define _MOVELIST

#include <iostream>
#include <deque>
#include "chesstypes.hpp"

namespace C2_chess
{

using std::ostream;
using std::deque;

class Move;
class Square;

class Movelist
{
 protected:
   enum
    {
      _LISTMAX = 1000
    };
    deque<Move*> _list;
    int _listindex;
  public:
    Movelist();
    Movelist(const Movelist&);
    virtual ~Movelist();
    Movelist& operator=(const Movelist&);
    Move* first() const;
    Move* next() const;
    void into(Move* const newmove);
    void into_as_first(Move* const newmove);
    void into_as_last(Move* const newmove);
    void out(Move* const rubbish);
    int cardinal() const;
    bool empty() const;
    bool in_list(Move* m,int* index) const;
    void clear();
    Move* operator[](int) const;
    virtual ostream& write(ostream& os) const;
    friend ostream& operator<<(ostream& os, const Movelist& ml)
    {
      ml.write(os);
      return os;
    }
};

class Movelog: public Movelist
{
  protected:
    col _col_to_start = white;
    int _first_moveno = 1;
  public:
    Movelog() :
        Movelist()
    {
      _col_to_start = white;
      _first_moveno = 1;
    }

    Movelog(col col_to_start, int first_moveno) :
        Movelist()
    {
      _col_to_start = col_to_start;
      _first_moveno = first_moveno;
    }


    void set_col_to_start(col c)
    {
      _col_to_start = c;
    }

    void set_first_moveno(int moveno)
    {
      _first_moveno = moveno;
    }

    virtual ostream& write(ostream& os) const;
};
}
#endif

