/*
 * Bitboard_with_utils.hpp
 *
 *  Created on: 17 juli 2021
 *      Author: torsten
 */
#ifndef Bitboard_with_utils_HPP_
#define Bitboard_with_utils_HPP_

#include <fstream>
#include <vector>
#include <string>
#include "bitboard.hpp"

namespace C2_chess
{

std::ostream& operator <<(std::ostream& os, const BitMove& m);

class Bitboard_with_utils: public Bitboard
{
  protected:
    bool run_mg_test_case(const uint32_t testnum,
                          const std::string& FEN_string,
                          const std::vector<std::string>& reference_moves,
                          const std::string& testcase_info,
                          const gentype gt);

    bool run_mg_test_case(uint32_t testnum, const std::string& FEN_string);

    std::vector<std::string> convert_moves_to_UCI(const std::vector<std::string>& moves, col col_to_move);

  public:

    uint8_t get_castling_rights() const
    {
      return _castling_rights;
    }

    uint64_t get_hash_tag() const
    {
      return _hash_tag;
    }

    std::ostream& write_cmdline_style(std::ostream& os, outputtype ot, col from_perspective) const;

    int add_mg_test_position(const std::string& filename);

    int test_move_generation(uint32_t single_testnum);

    bool bitboard_tests(const std::string& arg);

    uint64_t find_legal_squares(uint64_t sq, uint64_t mask, uint64_t all_pieces, uint64_t other_pieces);

    void make_move(const std::string& UCI_move);

    float evaluate_position(col col_to_move, uint8_t level) const;

    col get_col_to_move() const
    {
      return _col_to_move;
    }

    int no_of_moves()
    {
      return _movelist.size();
    }

    void make_move(uint8_t move_index)
    {
      Bitboard::make_move(move_index);
    }

    int figure_out_last_move(const Bitboard& new_position, BitMove& m) const;

    int get_move_index(const BitMove& move) const
    {
      int idx = 0;
      for (const BitMove& m : _movelist)
      {
        if (m == move)
          return idx;
        idx++;
      }
      return -1;
    }

    void clear_game_history()
    {
      history.clear();
    }

};

} // End namespace C2_chess

#endif /* Bitboard_with_utils_HPP_ */
