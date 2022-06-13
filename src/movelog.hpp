#ifndef _MOVELIST
#define _MOVELIST

#include <iostream>
#include <deque>
#include <vector>
#include "chesstypes.hpp"

namespace C2_chess
{

class Movelog
{
  protected:
    col _col_to_start;
    uint16_t _first_moveno;
    std::vector<BitMove> _list;

    public:
    Movelog() :
        _col_to_start(col::white),
        _first_moveno(1),
        _list { }
    {
    }

    Movelog(col col_to_start, uint16_t first_moveno):
        _col_to_start(col_to_start),
        _first_moveno(first_moveno),
        _list { }
    {
    }

    void push_back(const BitMove& move)
    {
      _list.push_back(move);
    }

    void clear_and_init(col col_to_start, uint16_t first_moveno)
    {
      _col_to_start = col_to_start;
      _first_moveno = first_moveno;
      _list.clear();
    }

    std::ostream& write(std::ostream& os) const;

};
}
#endif

