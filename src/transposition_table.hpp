/*
 * zobrist_bitboard_hash.hpp
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
//#include "Bitboard.hpp"

namespace C2_chess
{

struct TT_element
{
    int best_move_index;
    float evaluation;
    int search_ply;
};

class Transposition_table
{
  private:
    friend class Bitboard;
    friend class Bitboard_with_utils;
    std::unordered_map<unsigned long, TT_element> _hash_map;
    unsigned long _random_table[64][2][6]; // square_index, piece_color, piece_type
    unsigned long _en_passant_file[8];
    unsigned long _castling_rights[16]; // we only use index 1, 2, 4 and 8
    unsigned long _black_to_move;

    void init_random_numbers()
    {
      std::random_device rd;
      std::mt19937 mt(rd());
      std::uniform_real_distribution<double> dist(0, pow(2, 64));
      for (int square_index = 0; square_index < 64; square_index++)
        for (int rank = 1; rank <= 8; rank++)
          for (int col = index(col::white); col <= index(col::black); col++)
            for (int type = index(piecetype::Queen); type <= index(piecetype::King); type++)
              _random_table[square_index][col][type] = round(dist(mt));

      _castling_rights[1] = round(dist(mt));
      _castling_rights[2] = round(dist(mt));
      _castling_rights[4] = round(dist(mt));
      _castling_rights[8] = round(dist(mt));

      for (int i = 0; i < 8; i++)
        _en_passant_file[i] = round(dist(mt));
      _black_to_move = round(dist(mt));
    }

  public:

    Transposition_table() :
        _hash_map(),
        _black_to_move(0L)
    {
      init_random_numbers();
    }

    ~Transposition_table()
    {
    }

    // find(hash_tag) returns a reference to the hash_element
    // matching the hash_tag. If no such element can be found
    // it returns a reference to a new empty (default allocated)
    // hash_element connected to the hash_tag.
    // So it can be used for both searching and inserting.
    TT_element& find(unsigned long hash_tag)
    {
      return _hash_map[hash_tag];
    }

    void clear()
    {
      _hash_map.clear();
    }
};

} // End namespace C2_chess

#endif

