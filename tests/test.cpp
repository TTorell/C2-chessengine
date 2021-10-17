/*
 * test.cpp
 *
 *  Created on: Oct 15, 2021
 *      Author: torsten
 */

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <iostream>
#include <string>
#include "../src/bitboard_with_utils.hpp"
#include "../src/chessfuncs.hpp"

using namespace C2_chess;

namespace // file private namespace
{

std::string get_FEN_test_position(unsigned int n)
{
  std::string line;
  std::string filename = "tests/test_positions/FEN_test_positions.txt";
  std::ifstream ifs(filename);
  REQUIRE(ifs.is_open());
  unsigned int testnum = 1;
  while (std::getline(ifs, line))
  {
    // Skip empty lines and lines starting with a blank.
    if ((line.empty()) || line[0] == ' ')
      continue;
    if (n == testnum)
    {
      // In this case each line in FEN_test_positions.txt contains the FEN-string
      // followed by a list of all legal moves in the position at the end.
      // Extract the actual FEN_string:
      // Match the first 6 tokens in the string.
      std::vector<std::string> matches;
      regexp_grep(line, "^([^\\s]+\\s){5}[^\\s]+", matches);
      return matches[0];
    }
    testnum++;
  }
  return "";
}

} // End of fileprivate namespace

TEST_CASE("Move_generation")
{
  const char* const preferred_test_exec_dir = "/home/torsten/eclipse-workspace/C2-chessengine";
  REQUIRE(check_execution_dir(preferred_test_exec_dir));
  Bitboard_with_utils chessboard;
  bool result = chessboard.bitboard_tests("");
  if (!result)
    std::cout << "TESTS FAILED!" << std::endl;
  REQUIRE(result);
}

TEST_CASE("Castling_wrights")
{
  // Load testposition 71
  std::string FEN_string = get_FEN_test_position(71);
  Bitboard_with_utils chessboard;
  REQUIRE(chessboard.read_position(FEN_string) == 0);
  REQUIRE(chessboard.get_castling_rights() == castling_rights_all);
  chessboard.find_all_legal_moves();
  chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
  chessboard.write_movelist(std::cout, true) << std::endl;

  SECTION("Rh1xh8 taking Rook at h8 etc")
  {
    // Rh1xh8: Both colors looses KS castling
    chessboard.make_move(0);
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BQ | castling_right_WQ));

    // Black King steps away loosing right to casle QS as well.
    chessboard.make_move(0);
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_right_WQ);
    // White castles queenside
    chessboard.make_move(1);
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
  }

  SECTION("Both Kings castles")
  {
    SECTION("Queenside-Kingside")
    {
      chessboard.make_move(1); // 0-0-0
      chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ));
      chessboard.make_move(1); // ...0-0
      chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    }

    SECTION("Kingside-Queenside")
    {
      chessboard.make_move(2); // 0-0
      chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ));
      chessboard.make_move(0); // 0-0
      chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    }
  }

  SECTION("Rook moves, but nocapture")
  {
    chessboard.make_move(9); // Ra8-b8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ | castling_right_WK));
    chessboard.make_move(17); // ...Rh8-g8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_WK));
  }

}

