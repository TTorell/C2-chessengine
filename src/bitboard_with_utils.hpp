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
    bool run_mg_test_case(int testnum,
                          const std::string& FEN_string,
                          const std::vector<std::string>& reference_moves,
                          const std::string& testcase_info);
    bool run_mg_test_case(int testnum, const std::string& FEN_string);
    std::vector<std::string> convert_moves_to_UCI(const std::vector<std::string>& moves, col col_to_move);
  public:
    uint8_t get_castling_rights() const {return _castling_rights;}
    uint64_t get_hash_tag() const { return _hash_tag;}
    std::ostream& write_piece(std::ostream& os, uint64_t square) const;
    std::ostream& write(std::ostream& os, outputtype wt, col from_perspective) const;
    std::ostream& write_movelist(std::ostream& os, bool same_line = false);
    void add_mg_test_position(const std::string& filename);
    int test_move_generation(unsigned int single_testnum);
    bool bitboard_tests(const std::string& arg);
    uint64_t find_legal_squares(uint64_t sq, uint64_t mask, uint64_t all_pieces, uint64_t other_pieces);
    void make_move(const std::string& UCI_move);
};

} // End namespace C2_chess
#endif /* Bitboard_with_utils_HPP_ */
