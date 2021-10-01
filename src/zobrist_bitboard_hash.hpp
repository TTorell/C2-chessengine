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

struct bitboard_hash_element
{
  int best_move_index;
  float evaluation;
  int level;
};

class Zobrist_bitboard_hash
{
  private:
    friend class Bitboard;
    std::unordered_map<unsigned long, bitboard_hash_element> _hash_map;
    unsigned long _random_table[64][2][6]; // square_index, piece_color, piece_type
    unsigned long _en_passant_file[8];
    unsigned long _castling_rights[4];
    unsigned long _black_to_move;

    void init_random_numbers()
    {
      std::random_device rd;
      std::mt19937 mt(rd());
      std::uniform_real_distribution<double> dist(0, pow(2,64));
      for (int square_index = 0; square_index < 64; square_index++)
        for (int rank = 1; rank <= 8; rank++)
          for (int col = index(col::white); col <= index(col::black); col++)
            for (int type = index(piecetype::King); type <= index(piecetype::Pawn); type++)
              _random_table[square_index][col][type] = round(dist(mt));

      for (int i = 0; i < 4; i++)
        _castling_rights[i] = round(dist(mt));

      for (int i = 0; i < 8; i++)
        _en_passant_file[i] = round(dist(mt));
      _black_to_move = round(dist(mt));
    }

  public:

    Zobrist_bitboard_hash()
    {
      init_random_numbers();
    }

    ~Zobrist_bitboard_hash()
    {
    }

    // find(hash_tag) returns a reference to the hash_element
    // matching the hash_tag. If no such element can be found
    // it returns a reference to a new empty (default allocated)
    // hash_element connected to the hash_tag.
    // So it can be used for both searching and inserting.
    bitboard_hash_element& find(unsigned long hash_tag)
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

