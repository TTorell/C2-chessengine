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

struct PV_statistics
{
    uint32_t _n_insertions;
    uint32_t _n_hash_conflicts;

    friend std::ostream& operator <<(std::ostream& os, const PV_statistics& stats)
    {
      os << "inserted_entries: " << stats._n_insertions << "\nhash_conflicts: " <<
          stats._n_hash_conflicts << std::endl;
      return os;
    }
};

class PV_table
{
  private:
    PV_entry *_table;
    uint64_t _size;
    PV_statistics _statistics;

    PV_table();

  public:
    PV_table(long unsigned int size):
        _table(nullptr),
        _size(size),
        _statistics{0,0}
    {
      _table = new PV_entry[_size];
      clear();
    }

    PV_table(const PV_table& from):
        _table(nullptr),
        _size(from._size),
        _statistics(from._statistics)
    {
      _table = new PV_entry[_size];
      assert(_table);
      std::memcpy(from._table, _table, _size*sizeof(PV_entry));
    }

    ~PV_table()
    {
      if (_table)
      {
        delete[] _table;
        _table = nullptr;
      }
    }

    int get_size() const
    {
      return _size;
    }

    PV_table& operator =(const PV_table& pvt)
    {
      assert(&pvt != this);
      _size = pvt._size;
      _statistics = pvt._statistics;
      if (_table)
      {
        delete[] _table;
        _table = nullptr;
      }
      _table = new PV_entry[pvt._size];
      assert(_table);
      std::memcpy(pvt._table, _table, _size*sizeof(PV_entry));
      return *this;
    }

    void clear()
    {
      memset(_table, 0, _size * sizeof(PV_entry));
      _statistics = {0,0};
    }

    void store_move(uint64_t hash_key, uint32_t move)
    {
      const uint64_t idx = hash_key % _size;
      // std::cerr << hash_key << " : " << move << std::endl;
      assert( idx < _size);
      if (_table[idx]._hash_key != 0 && _table[idx]._hash_key != hash_key)
        _statistics._n_hash_conflicts++;
      _table[idx] = {hash_key, move};
      _statistics._n_insertions++;
    }

    uint32_t get_move(uint64_t hash_key) const
    {
      const uint64_t idx = hash_key % _size;
      if (_table[idx]._hash_key == hash_key)
      {
        return _table[idx]._move;
      }
      else
      {
        return 0;
      }
    }

    PV_statistics get_statistics() const
    {
      return _statistics;
    }

    bool is_PV_move(uint64_t hash_key, uint32_t move) const
    {
      const uint64_t idx = hash_key % _size;
      assert(idx < _size);
      if (_table[idx]._hash_key == hash_key)
        return _table[idx]._move == move;
      return false;
    }

};
} // namespace C2_chess
#endif /* SRC_PV_TABLE_HPP_ */
