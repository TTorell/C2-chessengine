/*
 * transposition_table.hpp
 *
 *  Created on: Sep 26, 2021
 *      Author: torsten
 */

#ifndef _ZOBRIST_BITBOARD_HASH
#define _ZOBRIST_BITBOARD_HASH

#include <unordered_map>
#include <random>
#include <cmath>
#include <cstring>
#include "chesstypes.hpp"

namespace C2_chess
{

struct TT_element
{
    //int best_move_index;
    Bitmove _best_move;
    int _search_ply;

    // An unused element will have best_move = UNDEFINED_MOVE.
    TT_element() :
        _best_move(UNDEFINED_MOVE),
        _search_ply(0)
    {
    }

    void set(unsigned long hash_tag,
             const Bitmove& move,
             float evaluation,
             int search_ply)
    {
       if (hash_tag == 0){};
       _best_move = move;
       _best_move._evaluation = evaluation;
       _search_ply = search_ply;
    }

    bool is_initialized() const
    {
      return !(_best_move == UNDEFINED_MOVE);
    }

    bool is_valid(unsigned long dummy_tag) const
    {
      if (dummy_tag == 0){}; // Avoid warning "unused ..."
      return is_initialized();
    }
};

enum class Table
{
  Current,
  Previous,
  Both
};

using Hashmap = std::unordered_map<uint64_t, TT_element>;

class Transposition_table
{
  private:
    friend class Bitboard;

    Hashmap _hash_map[2];
    Hashmap* _current_searchdepth_map;
    Hashmap* _previous_searchdepth_map;
    unsigned long _toggle_side_to_move;
    unsigned long _random_table[64][2][6]; // square_index, piece_color, piece_type
    unsigned long _en_passant_file[8];
    unsigned long _castling_rights[16]; // we only use index 1, 2, 4 and 8

    void init_random_numbers();

  public:

    Transposition_table(std::size_t dummy_size) :
        _current_searchdepth_map(&_hash_map[0]),
        _previous_searchdepth_map(&_hash_map[1]),
        _toggle_side_to_move(0L)
    {
      if (dummy_size == 0){} // Avoid warning "unused...."
      init_random_numbers();
    }

    void operator=(const Transposition_table&) = delete;
    Transposition_table(const Transposition_table&) = delete;

    ~Transposition_table()
    {
    }

    // find(hash_tag) returns a reference to the hash_element
    // matching the hash_tag. If no such element can be found
    // it returns a reference to a new empty (default allocated)
    // hash_element connected to the hash_tag.
    // So find() can be used for both searching and inserting.
    TT_element& find(unsigned long hash_tag, Table map = Table::Current)
    {
      if (map == Table::Current)
        return (*_current_searchdepth_map)[hash_tag];
      else
        return (*_previous_searchdepth_map)[hash_tag];
    }

    void clear(const Table map = Table::Current)
    {
      switch (map)
      {
      case Table::Current:
        _current_searchdepth_map->clear();
        break;
      case Table::Previous:
        _previous_searchdepth_map->clear();
        break;
      case Table::Both:
        default:
        _current_searchdepth_map->clear();
        _previous_searchdepth_map->clear();
      }
    }

    void switch_maps()
    {
      Hashmap* tmp_map = _previous_searchdepth_map;
      _previous_searchdepth_map->clear();
      _previous_searchdepth_map = _current_searchdepth_map;
      _current_searchdepth_map = tmp_map;
    }
};

inline void Transposition_table::init_random_numbers()
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<double> dist(0, pow(2, 64));
  for (int square_index = 0; square_index < 64; square_index++)
    for (int row = 1; row <= 8; row++)
      for (int col = index(Color::White); col <= index(Color::Black); col++)
        for (int type = index(Piecetype::Queen); type <= index(Piecetype::King); type++)
          _random_table[square_index][col][type] = round(dist(mt));
  _castling_rights[1] = round(dist(mt));
  _castling_rights[2] = round(dist(mt));
  _castling_rights[4] = round(dist(mt));
  _castling_rights[8] = round(dist(mt));
  for (int i = 0; i < 8; i++)
    _en_passant_file[i] = round(dist(mt));
  _toggle_side_to_move = round(dist(mt));
}

//Second alternative
struct TT_element_2
{
    //int best_move_index;
    unsigned long _hash_tag;
    Bitmove _best_move;
    int _search_ply;

    // An unused element will have best_move = UNDEFINED_MOVE.
    TT_element_2() :
        _hash_tag(0),
        _best_move(UNDEFINED_MOVE),
        _search_ply(0)
    {
    }

    void set(unsigned long hash_tag,
             const Bitmove& move,
             float evaluation,
             int search_ply)
    {
       _hash_tag = hash_tag;
       _best_move = move;
       _best_move._evaluation = evaluation;
       _search_ply = search_ply;
    }

    bool is_initialized() const
    {
      return !(_best_move == UNDEFINED_MOVE);
    }

    bool is_valid(unsigned long tag) const
    {
      return (is_initialized() && tag == _hash_tag);
    }
};

class Transposition_table_2
{
  private:
    friend class Bitboard;

    TT_element_2* _hash_table[2];
    TT_element_2* _current_searchdepth_table;
    TT_element_2* _previous_searchdepth_table;
    unsigned long _toggle_side_to_move;
    std::size_t _hash_size;
    unsigned long _random_table[64][2][6]; // square_index, piece_color, piece_type
    unsigned long _en_passant_file[8];
    unsigned long _castling_rights[16]; // we only use index 1, 2, 4 and 8

    void init_random_numbers();

  public:

    Transposition_table_2() = delete;

    Transposition_table_2(size_t n_elements) :
        _current_searchdepth_table(nullptr),
        _previous_searchdepth_table(nullptr),
        _toggle_side_to_move(0L),
        _hash_size(n_elements)
    {

      init_random_numbers();
      _hash_table[0] = new TT_element_2[_hash_size];
      if (!_hash_table[0])
        std::cerr << "Error: Out of memory for hash-table 0"<< std::endl;
      _hash_table[1] = new TT_element_2[_hash_size];
      if (!_hash_table[1])
        std::cerr << "Error: Out of memory for hash-table 1"<< std::endl;
      _current_searchdepth_table = _hash_table[0];
      _previous_searchdepth_table = _hash_table[1];
    }

    Transposition_table_2(const Transposition_table_2&) = delete;

    ~Transposition_table_2()
    {
      delete [] _hash_table[0];
      delete [] _hash_table[1];
    }

    void operator=(const Transposition_table_2&) = delete;

    // find(hash_tag) returns a reference to the hash_element
    // matching the hash_tag modular hash_size.
    // So find() can be used for both searching and inserting.
    TT_element_2& find(unsigned long hash_tag, Table table = Table::Current)
    {
      if (table == Table::Current)
        return _current_searchdepth_table[hash_tag % _hash_size];
      else
        return _previous_searchdepth_table[hash_tag % _hash_size];
    }

    void clear(const Table table = Table::Current)
    {
      switch (table)
      {
      case Table::Current:
        std::memset((char*)_current_searchdepth_table, 0, _hash_size* sizeof(TT_element_2));
        for (std::size_t i = 0; i < _hash_size; i++)
        {
          _current_searchdepth_table[i]._best_move = UNDEFINED_MOVE;
        }
        break;
      case Table::Previous:
        std::memset((char*)_previous_searchdepth_table, 0, _hash_size* sizeof(TT_element_2));
        for (std::size_t i = 0; i < _hash_size; i++)
        {
          _previous_searchdepth_table[i]._best_move = UNDEFINED_MOVE;
        }
        break;
      case Table::Both:
        default:
          std::memset((char*)_current_searchdepth_table, 0, _hash_size* sizeof(TT_element_2));
          std::memset((char*)_previous_searchdepth_table, 0, _hash_size* sizeof(TT_element_2));
          for (std::size_t i = 0; i < _hash_size; i++)
          {
            _current_searchdepth_table[i]._best_move = UNDEFINED_MOVE;
            _previous_searchdepth_table[i]._best_move = UNDEFINED_MOVE;
          }
      }
    }

    void switch_maps()
    {
      clear(Table::Previous);
      TT_element_2* tmp_table = _previous_searchdepth_table;
      _previous_searchdepth_table = _current_searchdepth_table;
      _current_searchdepth_table = tmp_table;
    }
};

inline void Transposition_table_2::init_random_numbers()
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<double> dist(0, pow(2, 64));
  for (int square_index = 0; square_index < 64; square_index++)
    for (int row = 1; row <= 8; row++)
      for (int col = index(Color::White); col <= index(Color::Black); col++)
        for (int type = index(Piecetype::Queen); type <= index(Piecetype::King); type++)
          _random_table[square_index][col][type] = round(dist(mt));
  _castling_rights[1] = round(dist(mt));
  _castling_rights[2] = round(dist(mt));
  _castling_rights[4] = round(dist(mt));
  _castling_rights[8] = round(dist(mt));
  for (int i = 0; i < 8; i++)
    _en_passant_file[i] = round(dist(mt));
  _toggle_side_to_move = round(dist(mt));
}

} // End namespace C2_chess

#endif

