/*
 * Bitboard_with_utils.hpp
 *
 *  Created on: 17 juli 2021
 *      Author: torsten
 */
#ifndef Bitboard_with_utils_HPP_
#define Bitboard_with_utils_HPP_

#include <fstream>
#include "Bitboard.hpp"
namespace C2_chess
{

const char* const preferred_exec_dir = "/home/torsten/eclipse-workspace/C2-chessengine";

std::ostream& operator <<(std::ostream& os, const BitMove& m);

class Bitboard_with_utils: public Bitboard
{
  protected:
    bool run_mg_test_case(int testnum, const std::string& FEN_string);

  public:
    int read_position(const std::string& FEN_string);
    std::ostream& write_piece(std::ostream& os, uint64_t square) const;
    std::ostream& write(std::ostream& os, outputtype wt, col from_perspective) const;
    std::ostream& write_movelist(std::ostream& os, bool same_line = false);
    void add_mg_test_position(const std::string& filename);
    int test_move_generation(unsigned int single_testnum);
    bool bitboard_tests(const std::string& arg);
};

} // End namespace C2_chess
#endif /* Bitboard_with_utils_HPP_ */
