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
#include "chesstypes.hpp"

namespace C2_chess
{

struct TT_element
{
    //int best_move_index;
    Bitmove best_move;
    int search_ply;

    // An unused element will have best_move = UNDEFINED_MOVE.
    TT_element() :
        best_move(UNDEFINED_MOVE),
        search_ply(0)
    {
    }

    bool is_initialized() const
    {
      return !(best_move == UNDEFINED_MOVE);
    }
};

enum class map_tag
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

    Transposition_table() :
        _current_searchdepth_map(&_hash_map[0]),
        _previous_searchdepth_map(&_hash_map[1]),
        _toggle_side_to_move(0L)
    {
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
    // So find can be used for both searching and inserting.
    TT_element& find(unsigned long hash_tag, map_tag map = map_tag::Current)
    {
      if (map == map_tag::Current)
        return (*_current_searchdepth_map)[hash_tag];
      else
        return (*_previous_searchdepth_map)[hash_tag];
    }

    void clear(const map_tag map = map_tag::Current)
    {
      switch (map)
      {
      case map_tag::Current:
        _current_searchdepth_map->clear();
        break;
      case map_tag::Previous:
        _previous_searchdepth_map->clear();
        break;
      case map_tag::Both:
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

} // End namespace C2_chess

#endif

