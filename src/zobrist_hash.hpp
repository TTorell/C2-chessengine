/*
 * zobrist_hash.cpp
 *
 *  Created on: Mar 9, 2021
 *      Author: torsten
 */
#ifndef _ZOBRIST_HASH
#define _ZOBRIST_HASH

#include <unordered_map>
#include <random>
#include <cmath>
#include "chesstypes.hpp"
#include "board.hpp"

namespace C2_chess
{

struct hash_element
{
    int best_move_index;
    float evaluation;
    int level;
};

class Zobrist_hash
{
  private:
    friend class Board;
    std::unordered_map<unsigned long, hash_element> _hash_map;
    unsigned long _random_table[8][9][2][6];
    unsigned long _en_passant_file[8];
    unsigned long _castling_rights[4];
    unsigned long _black_to_move;

    void init_random_numbers()
    {
      std::random_device rd;
      std::mt19937 mt(rd());
      std::uniform_real_distribution<double> dist(0, pow(2, 64));
      for (int file_idx = a; file_idx <= h; file_idx++)
        for (int rank_idx = 1; rank_idx <= 8; rank_idx++)
          for (int col = index(col::white); col <= index(col::black); col++)
            for (int type = index(piecetype::King); type <= index(piecetype::Pawn); type++)
              _random_table[file_idx][rank_idx][col][type] = round(dist(mt));

      for (int i = 0; i < 4; i++)
        _castling_rights[i] = round(dist(mt));

      for (int i = 0; i < 8; i++)
        _en_passant_file[i] = round(dist(mt));
      _black_to_move = round(dist(mt));
    }

  public:

    Zobrist_hash() :
        _hash_map(),
        _black_to_move(0L)
    {
      init_random_numbers();
    }

    ~Zobrist_hash()
    {
    }

    // find(hash_tag) returns a reference to the hash_element
    // matching the hash_tag. If no such element can be found
    // it returns a reference to a new empty (default allocated)
    // hash_element connected to the hash_tag.
    // So it can be used for both searching and inserting.
    hash_element& find(unsigned long hash_tag)
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
