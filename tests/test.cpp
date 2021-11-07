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
  int result = chessboard.bitboard_tests("");
  if (result != 0)
    std::cout << "TESTS FAILED!" << std::endl;
  REQUIRE(result == 0);
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
    // White castles queen-side
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
      chessboard.make_move(0); // ... 0-0
      chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    }
  }

  SECTION("Rook moves, but no capture")
  {
    chessboard.make_move(9); // Ra8-b8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ | castling_right_WK));
    chessboard.make_move(17); // ...Rh8-g8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BQ | castling_right_WK));
    chessboard.make_move(15); // ...Rh1-g1
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_right_BQ);
    chessboard.make_move(6); // ...Ra8-b8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
  }

  SECTION("King moves")
  {
    chessboard.make_move(8); // Ke1-d1
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ));
    chessboard.make_move(7); // ... Ke8-e7
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    // Kings move back
    chessboard.make_move(5); // Kd1-e1
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    chessboard.make_move(3); // ... Ke7-e8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);

    uint64_t htag = chessboard.get_hash_tag(); // first test of _hash_tag

    // Make another King_move
    chessboard.make_move(3); // Ke1-e2
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    chessboard.make_move(3); // ... Ke8-d8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    // and move back again
    chessboard.make_move(8); // Ke2-e1
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    chessboard.make_move(2); // ... Kd8-e8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);

    REQUIRE(htag == chessboard.get_hash_tag());
  }
}

TEST_CASE("Bitboard between")
{
  uint64_t squares = between(e8_square, e1_square, e_file | row_8);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (e_file ^ (e8_square | e1_square)));
  squares = between(e1_square, e8_square, e_file | row_1);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (e_file ^ (e8_square | e1_square)));
  squares = between(a1_square, a8_square, a_file | row_1);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (a_file ^ (a8_square | a1_square)));
  squares = between(a8_square, a1_square, a_file | row_8);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (a_file ^ (a8_square | a1_square)));
  squares = between(a8_square, a7_square, a_file | row_8);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == zero);
  squares = between(a7_square, a8_square, a_file | row_7);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == zero);
  squares = between(a1_square, b1_square, a_file | row_1);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == zero);
  squares = between(b1_square, a1_square, b_file | row_1);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == zero);
  squares = between(a1_square, h1_square, a_file | row_1);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (row_1 ^ (a1_square | h1_square)));
  squares = between(h1_square, a1_square, h_file | row_1);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (row_1 ^ (a1_square | h1_square)));
  squares = between(c7_square, e7_square, c_file | row_7);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == d7_square);
  squares = between(e7_square, c7_square, e_file | row_7);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == d7_square);
  uint64_t a1_diag = to_diagonal(a1_square);
  REQUIRE(a1_diag == diagonal[7]);
  uint64_t a1_anti_diag = to_anti_diagonal(a1_square);
  REQUIRE(a1_anti_diag == anti_diagonal[0]);
  squares = between(a1_square, h8_square, to_diagonal(a1_square) | to_anti_diagonal(a1_square), true);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (diagonal[7] ^ (a1_square | h8_square)));
  squares = between(h8_square, a1_square, to_diagonal(h8_square) | to_anti_diagonal(h8_square), true);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (diagonal[7] ^ (a1_square | h8_square)));
  squares = between(a8_square, h1_square, to_diagonal(a8_square) | to_anti_diagonal(a8_square), true);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (anti_diagonal[7] ^ (a8_square | h1_square)));
  squares = between(h1_square, a8_square, to_diagonal(h1_square) | to_anti_diagonal(h1_square), true);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (anti_diagonal[7] ^ (a8_square | h1_square)));
  squares = between(h1_square, g2_square, to_diagonal(h1_square) | to_anti_diagonal(h1_square), true);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == zero);
  squares = between(g2_square, h1_square, to_diagonal(g2_square) | to_anti_diagonal(g2_square), true);
  std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == zero);
}

TEST_CASE("popright_square")
{
  uint64_t squares = whole_board;
  uint64_t saved_squares = squares;
  for (int8_t bit_idx = 0; bit_idx < 64; bit_idx++)
  {
    std::cout << to_binary(squares) << std::endl;
    std::cout << to_binary(square(bit_idx)) << std::endl;
    REQUIRE(popright_square(squares) == square(bit_idx));
    REQUIRE((squares ^ square(bit_idx)) == saved_squares);
    saved_squares = squares;
  }
  std::cout << "" << std::endl;
  squares = diagonal[7];
  saved_squares = squares;
  uint64_t bit_idx = 0;
  while (squares)
  {
    std::cout << to_binary(squares) << std::endl;
    REQUIRE(popright_square(squares) == square(bit_idx));
    REQUIRE((squares ^ square(bit_idx)) == saved_squares);
    saved_squares = squares;
    bit_idx += 9;
  }
}

TEST_CASE("ortogonal_squares")
{
  REQUIRE((ortogonal_squares(e4_square) ^ (e_file | row_4)) == zero);
  REQUIRE((ortogonal_squares(a1_square) ^ (a_file | row_1)) == zero);
  REQUIRE((ortogonal_squares(h8_square) ^ (h_file | row_8)) == zero);
  REQUIRE((ortogonal_squares(a8_square) ^ (a_file | row_8)) == zero);

}

TEST_CASE("find_blockers")
{
  Bitboard_with_utils chessboard;
  uint64_t all_pieces = a1_square | b1_square | d1_square | f1_square | h1_square;
  uint64_t sq = e1_square;
  uint64_t my_rank = row_1;
  uint64_t blockers = chessboard.find_blockers(sq, my_rank, all_pieces);
  std::cout << to_binary(blockers) << std::endl;
  REQUIRE(blockers == (d1_square | f1_square));
}
