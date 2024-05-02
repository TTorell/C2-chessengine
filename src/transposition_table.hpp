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
#include <iostream>
#include "chesstypes.hpp"

namespace C2_chess
{

struct TT_element
{
    Bitmove _best_move;
    int _search_depth;

    // An unused element will have best_move = UNDEFINED_MOVE.
    TT_element() :
        _best_move(UNDEFINED_MOVE),
        _search_depth(0)
    {
    }

    void set(unsigned long hash_tag,
             const Bitmove& move,
             float evaluation,
             int search_depth)
    {
       if (hash_tag == 0){};
       _best_move = move;
       _best_move._evaluation = evaluation;
       _search_depth = search_depth;
    }

    bool is_initialized() const
    {
      return !(_best_move == UNDEFINED_MOVE);
    }
};

class Transposition_table_base
{
  protected:
    unsigned long _toggle_side_to_move;
    unsigned long _random_table[64][2][6]; // square_index, piece_color, piece_type
    unsigned long _en_passant_file[8];
    unsigned long _castling_rights[16]; // we only use index 1, 2, 4 and 8

    void init_random_numbers();

  public:
    virtual TT_element& find(unsigned long hash_tag) = 0;
    virtual void clear() = 0;
};

inline void Transposition_table_base::init_random_numbers()
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<double> dist(0, pow(2, 64));
  for (int square_index = 0; square_index < 64; square_index++)
    for (int row = 1; row <= 8; row++)
      for (int col = index(Color::White); col <= index(Color::Black); col++)
        for (int type = index(Piecetype::Queen); type <= index(Piecetype::King); type++)
          _random_table[square_index][col][type] = static_cast<uint64_t>(round(dist(mt)));
  _castling_rights[1] = static_cast<uint64_t>(round(dist(mt)));
  _castling_rights[2] = static_cast<uint64_t>(round(dist(mt)));
  _castling_rights[4] = static_cast<uint64_t>(round(dist(mt)));
  _castling_rights[8] =static_cast<uint64_t>( round(dist(mt)));
  for (int i = 0; i < 8; i++)
    _en_passant_file[i] = static_cast<uint64_t>(round(dist(mt)));
  _toggle_side_to_move = static_cast<uint64_t>(round(dist(mt)));
}

using Hashmap = std::unordered_map<uint64_t, TT_element>;
class Hash_map_TT: public Transposition_table_base
{
  private:
    friend class Bitboard;

    Hashmap _hash_map;

  public:

    Hash_map_TT(std::size_t dummy_size)
    {
      if (dummy_size == 0){} // Avoid warning "unused...."
      init_random_numbers();
    }

    void operator=(const Hash_map_TT&) = delete;
    Hash_map_TT(const Hash_map_TT&) = delete;

    ~Hash_map_TT() noexcept
    {
    }

    // find(hash_tag) returns a reference to the hash_element
    // matching the hash_tag. If no such element can be found
    // it returns a reference to a new empty (default allocated)
    // hash_element connected to the hash_tag.
    // So find() can be used for both searching and inserting.
    TT_element& find(unsigned long hash_tag)
    {
        return _hash_map[hash_tag];
    }

    void clear()
    {
        _hash_map.clear();
    }
};

//////////////////////
// Second alternative:
//////////////////////

// The index of the element is hash_tag modulo the
// max number of elements in the cash.
// elements must contain the complete hash_tag itself
// to check agains the hash_tag in the find-method
struct TT_element_2:public TT_element
{
    //int best_move_index;
    unsigned long _hash_tag;

    // An unused element will have best_move = UNDEFINED_MOVE.
    TT_element_2() :
    TT_element(),
    _hash_tag(0)
    {
    }

    void set(unsigned long hash_tag,
             const Bitmove& move,
             float evaluation,
             int search_depth)
    {
       _hash_tag = hash_tag;
       _best_move = move;
       _best_move._evaluation = evaluation;
       _search_depth = search_depth;
    }

    bool is_valid(unsigned long tag) const
    {
      return (is_initialized() && tag == _hash_tag);
    }
};

class Classic_hash_list_TT: public Transposition_table_base
{
  private:
    friend class Bitboard;

    TT_element_2* _hash_table;
    std::size_t _hash_size;

  public:

    Classic_hash_list_TT() = delete;
    Classic_hash_list_TT(const Classic_hash_list_TT&) = delete;

    Classic_hash_list_TT(std::size_t n_elements) :
        _hash_table(nullptr),
        _hash_size(n_elements)
    {
      init_random_numbers();
      _hash_table = new TT_element_2[_hash_size];
      if (!_hash_table)
        std::cerr << "Error: Out of memory for hash-table" << std::endl;
    }

    ~Classic_hash_list_TT()
    {
      delete [] _hash_table;
    }

    void operator=(const Classic_hash_list_TT&) = delete;

    // find(hash_tag) returns a reference to the hash_element
    // matching the hash_tag modular hash_size.
    // So find() can be used for both searching and inserting.
    TT_element_2& find(unsigned long hash_tag) override
    {
      return _hash_table[hash_tag % _hash_size];
    }

    void clear()
    {
        std::memset((char*)_hash_table, 0, _hash_size* sizeof(TT_element_2));
        for (std::size_t i = 0; i < _hash_size; i++)
        {
          _hash_table[i]._best_move = UNDEFINED_MOVE;
        }
    }
};

} // end of namespace C2_chess
#endif
