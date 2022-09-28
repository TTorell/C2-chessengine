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
#include "config_param.hpp"

namespace C2_chess
{

std::ostream& operator <<(std::ostream& os, const Bitmove& m);

class Bitboard_with_utils: public Bitboard
{
  protected:
    bool run_mg_test_case(const uint32_t testnum,
                          const std::string& FEN_string,
                          const std::vector<std::string>& reference_moves,
                          const std::string& testcase_info,
                          const Gentype gt);

    bool run_mg_test_case(uint32_t testnum, const std::string& FEN_string);

    std::vector<std::string> convert_moves_to_UCI(const std::vector<std::string>& moves, Color col_to_move);

  public:

    uint8_t get_castling_rights() const
    {
      return _castling_rights;
    }

    uint64_t get_hash_tag() const
    {
      return _hash_tag;
    }

    std::ostream& write_cmdline_style(std::ostream& os, const Color from_perspective) const;

    int add_mg_test_position(const std::string& filename);

    int test_move_generation(uint32_t single_testnum);

    bool bitboard_tests(const std::string& arg);

    uint64_t find_legal_squares(uint64_t sq, uint64_t mask, uint64_t all_pieces, uint64_t other_pieces);

    void make_UCI_move(const std::string& UCI_move);

    float evaluate_position(Color col_to_move, uint8_t level, bool evaluate_zero_moves = true) const;

    Bitmove get_first_move()
    {
      //TODO: Is this right:
      auto movelist = get_movelist(0);
      assert(movelist->size() > 0);
      return (*movelist)[0];
    }

    int get_move_index(const Bitmove& move) const
    {
      int idx = 0;
      //TODO: Is this right:
      for (const Bitmove& m : *get_movelist(0))
      {
        if (m == move)
          return idx;
        idx++;
      }
      return -1;
    }

    int figure_out_last_move(const Bitboard& new_position, Bitmove& m) const;

    void clear_game_history()
    {
      history.clear();
    }

    std::ostream& write_movelist(std::ostream& os, const bool same_line) const;

};

} // End namespace C2_chess

#endif /* Bitboard_with_utils_HPP_ */
