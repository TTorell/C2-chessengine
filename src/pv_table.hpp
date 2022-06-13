/*
 * PV_table.hpp
 *
 *  Created on: 28 maj 2022
 *      Author: torsten
 */

#include "bitboard.hpp"
#include "chesstypes.hpp"
#include "movelog.hpp"

#ifndef SRC_PV_TABLE_HPP_
#define SRC_PV_TABLE_HPP_

namespace C2_chess
{

struct PV_entry
{
    uint64_t _hash_key;
    uint32_t _move;
};

class PV_table
{
  private:
    PV_entry *_table;
    int _size;

  public:
    PV_table(const int size):
        _table(nullptr),
        _size(size)
    {
      _table = new PV_entry[_size];
    }

    PV_table(const PV_table& from):
        _table(nullptr),
        _size(from._size)
    {
      // Just makes a new empty table of the same size as from.
      _table = new PV_entry[_size];
      assert(_table);
    }

    ~PV_table()
    {
      if (_table)
        delete[] _table;
    }

    PV_table& operator =(const PV_table& from)
    {
      // Just removes current content and makes
      // a new empty table of the same size as from.
      if (&from != this)
      {
        _size = from._size;
        if (_table)
          delete[] _table;
        _table = new PV_entry[from._size];
        assert(_table);
      }
      return *this;
    }

    void clear()
    {
      memset(_table, 0, _size * sizeof(PV_entry));
    }

    void store_move(uint64_t hash_key, uint32_t move)
    {
      const int idx = hash_key % _size;
      assert(idx >= 0 && idx < _size);
      _table[idx]._hash_key = hash_key;
      _table[idx]._move = move;
    }

    bool is_PV_move(uint64_t hash_key)
    {
      const int idx = hash_key % _size;
      assert(idx >= 0 && idx < _size);
      if (_table[idx]._hash_key == hash_key)
        return true;
      return false;
    }

};
} // namespace C2_chess
#endif /* SRC_PV_TABLE_HPP_ */
