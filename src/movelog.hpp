#ifndef _MOVELOG
#define _MOVELOG

#include <iostream>
#include <deque>
#include <vector>
#include "chesstypes.hpp"

namespace C2_chess
{

class Movelog
{
  protected:
    Color _col_to_start;
    uint16_t _first_moveno;
    std::vector<Bitmove> _list;

    public:
    Movelog() :
        _col_to_start(Color::White),
        _first_moveno(1),
        _list { }
    {
    }

    Movelog(Color col_to_start, uint16_t first_moveno):
        _col_to_start(col_to_start),
        _first_moveno(first_moveno),
        _list { }
    {
    }

    void push_back(const Bitmove& move)
    {
      _list.push_back(move);
    }

    void pop()
    {
      assert(!_list.empty());
      _list.pop_back();
      if(_list.empty())
        _first_moveno = 1;
    }

    void clear_and_init(Color col_to_start, uint16_t first_moveno)
    {
      _col_to_start = col_to_start;
      _first_moveno = first_moveno;
      _list.clear();
    }

    std::ostream& write(std::ostream& os) const;

    friend std::ostream& operator <<(std::ostream& os, const Movelog&);
};
}
#endif

