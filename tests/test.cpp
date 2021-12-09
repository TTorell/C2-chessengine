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
#include <cmath>
#include "../src/bitboard_with_utils.hpp"
#include "../src/chessfuncs.hpp"
#include "../src/current_time.hpp"

using namespace C2_chess;

namespace // file private namespace
{
std::string initial_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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
  uint64_t start, stop;
  CurrentTime now;
  start = now.nanoseconds();
  const char* const preferred_test_exec_dir = "/home/torsten/eclipse-workspace/C2-chessengine";
  REQUIRE(check_execution_dir(preferred_test_exec_dir));
  Bitboard_with_utils chessboard;
  int result = chessboard.bitboard_tests("");
  if (result != 0)
    std::cout << "TESTS FAILED!" << std::endl;
  REQUIRE(result == 0);
  stop = now.nanoseconds();
  std::cout << "It took " << stop - start << " nanoseconds." << std::endl;
}

TEST_CASE("Castling_wrights")
{
  // Load test position 71
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
    chessboard.make_move("h1h8");
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BQ | castling_right_WQ));

    // Black King steps away loosing right to casle QS as well.
    chessboard.make_move("e8f7");
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_right_WQ);
    // White castles queen-side
    chessboard.make_move("e1c1");
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
  }

  SECTION("Both Kings castles")
  {
    SECTION("Queenside-Kingside")
    {
      chessboard.make_move("e1c1"); // 0-0-0
      chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ));
      chessboard.make_move("e8g8"); // ...0-0
      chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    }

    SECTION("Kingside-Queenside")
    {
      chessboard.make_move("e1g1"); // 0-0
      chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ));
      chessboard.make_move("e8c8"); // ... 0-0-0
      chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    }
  }

  SECTION("Rook moves, but no capture")
  {
    chessboard.make_move("a1b1"); // Ra1-b1
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ | castling_right_WK));
    chessboard.make_move("h8g8"); // ...Rh8-g8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BQ | castling_right_WK));
    chessboard.make_move("h1g1"); // ...Rh1-g1
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_right_BQ);
    chessboard.make_move("a8b8"); // ...Ra8-b8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
  }

  SECTION("King moves")
  {
    chessboard.make_move("e1d1"); // Ke1-d1
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ));
    chessboard.make_move("e8e7"); // ... Ke8-e7
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    // Kings move back
    chessboard.make_move("d1e1"); // Kd1-e1
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    chessboard.make_move("e7e8"); // ... Ke7-e8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);

    uint64_t htag = chessboard.get_hash_tag(); // first test of _hash_tag

    // Make another King_move
    chessboard.make_move("e1e2"); // Ke1-e2
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    chessboard.make_move("e8d8"); // ... Ke8-d8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    // and move back again
    chessboard.make_move("e2e1"); // Ke2-e1
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    chessboard.make_move("d8e8"); // ... Kd8-e8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);

    REQUIRE(htag == chessboard.get_hash_tag());
  }
}

TEST_CASE("Bitboard between")
{
  Bitboard_with_utils chessboard;
  uint64_t squares = between(e8_square, e1_square, e_file | row_8);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (e_file ^ (e8_square | e1_square)));
  squares = between(e1_square, e8_square, e_file | row_1);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (e_file ^ (e8_square | e1_square)));
  squares = between(a1_square, a8_square, a_file | row_1);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (a_file ^ (a8_square | a1_square)));
  squares = between(a8_square, a1_square, a_file | row_8);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (a_file ^ (a8_square | a1_square)));
  squares = between(a8_square, a7_square, a_file | row_8);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == zero);
  squares = between(a7_square, a8_square, a_file | row_7);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == zero);
  squares = between(a1_square, b1_square, a_file | row_1);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == zero);
  squares = between(b1_square, a1_square, b_file | row_1);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == zero);
  squares = between(a1_square, h1_square, a_file | row_1);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (row_1 ^ (a1_square | h1_square)));
  squares = between(h1_square, a1_square, h_file | row_1);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (row_1 ^ (a1_square | h1_square)));
  squares = between(c7_square, e7_square, c_file | row_7);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == d7_square);
  squares = between(e7_square, c7_square, e_file | row_7);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == d7_square);
  uint64_t a1_diag = to_diagonal(a1_square);
  REQUIRE(a1_diag == diagonal[7]);
  uint64_t a1_anti_diag = to_anti_diagonal(a1_square);
  REQUIRE(a1_anti_diag == anti_diagonal[0]);
  squares = between(a1_square, h8_square, to_diagonal(a1_square) | to_anti_diagonal(a1_square), true);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (diagonal[7] ^ (a1_square | h8_square)));
  squares = between(h8_square, a1_square, to_diagonal(h8_square) | to_anti_diagonal(h8_square), true);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (diagonal[7] ^ (a1_square | h8_square)));
  squares = between(a8_square, h1_square, to_diagonal(a8_square) | to_anti_diagonal(a8_square), true);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (anti_diagonal[7] ^ (a8_square | h1_square)));
  squares = between(h1_square, a8_square, to_diagonal(h1_square) | to_anti_diagonal(h1_square), true);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == (anti_diagonal[7] ^ (a8_square | h1_square)));
  squares = between(h1_square, g2_square, to_diagonal(h1_square) | to_anti_diagonal(h1_square), true);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == zero);
  squares = between(g2_square, h1_square, to_diagonal(g2_square) | to_anti_diagonal(g2_square), true);
  // std::cout << to_binary_board(squares) << std::endl;
  REQUIRE(squares == zero);
}

TEST_CASE("popright_square")
{
  uint64_t squares = whole_board;
  uint64_t saved_squares = squares;
  for (int8_t bit_idx = 0; bit_idx < 64; bit_idx++)
  {
    //    std::cout << to_binary(squares) << std::endl;
    //    std::cout << to_binary(square(bit_idx)) << std::endl;
    REQUIRE(popright_square(squares) == square(bit_idx));
    REQUIRE((squares ^ square(bit_idx)) == saved_squares);
    saved_squares = squares;
  }
  //  std::cout << "" << std::endl;
  squares = diagonal[7];
  saved_squares = squares;
  uint64_t bit_idx = 0;
  while (squares)
  {
    //    std::cout << to_binary(squares) << std::endl;
    REQUIRE(popright_square(squares) == square(bit_idx));
    REQUIRE((squares ^ square(bit_idx)) == saved_squares);
    saved_squares = squares;
    bit_idx += 9;
  }
}

TEST_CASE("ortogonal_squares")
{
  Bitboard_with_utils chessboard;
  REQUIRE((ortogonal_squares(e4_square) ^ (e_file | row_4)) == zero);
  REQUIRE((ortogonal_squares(a1_square) ^ (a_file | row_1)) == zero);
  REQUIRE((ortogonal_squares(h8_square) ^ (h_file | row_8)) == zero);
  REQUIRE((ortogonal_squares(a8_square) ^ (a_file | row_8)) == zero);
}

TEST_CASE("find_legal_squares")
{
  uint64_t other_pieces, legal_squares, all_pieces;
  Bitboard_with_utils chessboard;
  uint64_t sq = e1_square;
  uint64_t my_rank = row_1;

  SECTION("more than one pawn on each side")
  {
    all_pieces = a1_square | b1_square | d1_square | e1_square | f1_square | h1_square;

    SECTION("both blockers of other color")
    {
      other_pieces = a1_square | b1_square | d1_square | f1_square | h1_square;
      legal_squares = chessboard.find_legal_squares(sq, my_rank, all_pieces, other_pieces);
//  std::cout << to_binary(blockers) << std::endl;
      REQUIRE(legal_squares == (d1_square | f1_square));
    }
    SECTION("one blocker of other color")
    {
      other_pieces = a1_square | b1_square | f1_square | h1_square;
      legal_squares = chessboard.find_legal_squares(sq, my_rank, all_pieces, other_pieces);
      REQUIRE(legal_squares == (f1_square));
    }

    SECTION("No blocker of other color")
    {
      other_pieces = a1_square | b1_square | h1_square;
      legal_squares = chessboard.find_legal_squares(sq, my_rank, all_pieces, other_pieces);
      REQUIRE(legal_squares == zero);
    }
  }

  SECTION("no pawns to the left")
  {
    all_pieces = e1_square | f1_square | h1_square;

    SECTION("right blockers of other color")
    {
      other_pieces = f1_square;
      legal_squares = chessboard.find_legal_squares(sq, my_rank, all_pieces, other_pieces);
      REQUIRE(legal_squares == ((row_1 & not_g_h_files) ^ e1_square));
    }
    SECTION("right blocker of same color")
    {
      other_pieces = zero;
      legal_squares = chessboard.find_legal_squares(sq, my_rank, all_pieces, other_pieces);
      REQUIRE(legal_squares == (row_1 & ~(e_file | f_file | g_file | h_file))); // @suppress("C-Style cast instead of C++ cast")
    }
  }
  SECTION("no blockers to the right")
  {
    all_pieces = a1_square | b1_square | d1_square | e1_square;

    SECTION("left blocker of other color")
    {
      other_pieces = d1_square;
      legal_squares = chessboard.find_legal_squares(sq, my_rank, all_pieces, other_pieces);
      REQUIRE(legal_squares == (row_1 & (d_file | f_file | g_file | h_file))); // @suppress("C-Style cast instead of C++ cast")
    }
    SECTION("left blocker of same color")
    {
      other_pieces = zero;
      legal_squares = chessboard.find_legal_squares(sq, my_rank, all_pieces, other_pieces);
      REQUIRE(legal_squares == (row_1 & (f_file | g_file | h_file))); // @suppress("C-Style cast instead of C++ cast")
    }
  }
  SECTION("no blockers at all")
  {
    all_pieces = e1_square;
    other_pieces = zero;
    legal_squares = chessboard.find_legal_squares(sq, my_rank, all_pieces, other_pieces);
    REQUIRE(legal_squares == (row_1 ^ e1_square)); // @suppress("C-Style cast instead of C++ cast")
  }
}

TEST_CASE("evaluation")
{
  uint64_t start, stop;
  CurrentTime now;
  start = now.nanoseconds();
  Bitboard_with_utils chessboard;
  chessboard.read_position(initial_position);
  chessboard.find_all_legal_moves();
  REQUIRE(fabs(chessboard.evaluate_position(col::white, outputtype::silent, 7)) < 0.01);
  chessboard.make_move("e2e4");
  float evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 0.05) < 0.01);
  chessboard.make_move("e7e5");
  REQUIRE(fabs(chessboard.evaluate_position(col::white, outputtype::silent, 7)) < 0.01);
  chessboard.make_move("g1f3");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 0.09) < 0.01);
  chessboard.make_move("b8c6");
  REQUIRE(fabs(chessboard.evaluate_position(col::white, outputtype::silent, 7)) < 0.01);
  chessboard.make_move("d2d4");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 0.07) < 0.01);
  chessboard.make_move("d7d5");
  REQUIRE(fabs(chessboard.evaluate_position(col::white, outputtype::silent, 7)) < 0.01);
  chessboard.make_move("e4d5");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 1.03) < 0.01);
  chessboard.make_move("e5d4");
  REQUIRE(fabs(chessboard.evaluate_position(col::white, outputtype::silent, 7)) < 0.01);
  chessboard.make_move("d5c6");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 2.99) < 0.01);
  chessboard.make_move("b7c6");
  evaluation = chessboard.evaluate_position(col::white, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 1.97) < 0.01);
  chessboard.make_move("d1d4");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 3.03) < 0.01);
  chessboard.make_move("d8d4");
  evaluation = chessboard.evaluate_position(col::white, outputtype::silent, 7);
  REQUIRE(fabs(evaluation + 6.04) < 0.01);
  chessboard.make_move("f3d4");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 2.98) < 0.01);
  chessboard.make_move("c8h3");
  evaluation = chessboard.evaluate_position(col::white, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 2.93) < 0.01);
  chessboard.make_move("f1c4");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 3.0) < 0.01);
  chessboard.make_move("e8c8");
  evaluation = chessboard.evaluate_position(col::white, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 2.81) < 0.01);
  chessboard.make_move("e1g1");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 2.96) < 0.01);
  chessboard.make_move("d8d4");
  evaluation = chessboard.evaluate_position(col::white, outputtype::silent, 7);
  REQUIRE(fabs(evaluation + 0.04) < 0.01);
  chessboard.make_move("g2h3");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 2.96) < 0.01);
  chessboard.make_move("d4c4");
  evaluation = chessboard.evaluate_position(col::white, outputtype::silent, 7);
  REQUIRE(fabs(evaluation + 0.06) < 0.01);
  chessboard.make_move("f1e1");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation + 0.02) < 0.01);
  chessboard.make_move("g8f6");
  evaluation = chessboard.evaluate_position(col::white, outputtype::silent, 7);
  REQUIRE(fabs(evaluation + 0.11) < 0.01);
  chessboard.make_move("c1g5");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation + 0.06) < 0.01);
  chessboard.make_move("c4c2");
  evaluation = chessboard.evaluate_position(col::white, outputtype::silent, 7);
  REQUIRE(fabs(evaluation + 1.02) < 0.01);
  chessboard.make_move("b1c3");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation + 0.93) < 0.01);
  chessboard.make_move("c2f2");
  evaluation = chessboard.evaluate_position(col::white, outputtype::silent, 7);
  REQUIRE(fabs(evaluation + 1.93) < 0.01);
  chessboard.make_move("g1f2");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 3.07) < 0.01);
  chessboard.make_move("h7h6");
  evaluation = chessboard.evaluate_position(col::white, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 3.07) < 0.01);
  chessboard.make_move("g5f6");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 6.15) < 0.01);
  chessboard.make_move("g7f6");
  evaluation = chessboard.evaluate_position(col::white, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 3.09) < 0.01);
  chessboard.make_move("c3e4");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 3.03) < 0.01);
  chessboard.make_move("h6h5");
  evaluation = chessboard.evaluate_position(col::white, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 3.03) < 0.01);
  chessboard.make_move("e4c5");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 3.07) < 0.01);
  chessboard.make_move("h8g8");
  evaluation = chessboard.evaluate_position(col::white, outputtype::silent, 7);
  REQUIRE(fabs(evaluation - 3.02) < 0.01);
  chessboard.make_move("e1e8");
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 7);
  REQUIRE(evaluation == 93.0F);
  evaluation = chessboard.evaluate_position(col::black, outputtype::silent, 1);
  REQUIRE(evaluation == 99.0F);


  stop = now.nanoseconds();
  std::cout << "It took " << stop - start << " nanoseconds." << std::endl;
}

//TEST_CASE("timing basic functions")
//{
//  CurrentTime now;
//  uint8_t b_idx = 0;
//  uint64_t sq = a1_square;
//  uint64_t x = 0;
//
//  uint64_t start = now.nanoseconds();
//
//  for (int k = 0; k < 100000; k++)
//  {
//    sq = a1_square;
//    for (uint8_t i = 0; i < 64; i++)
//    {
//      sq = popright_square(sq);
//      uint8_t z = bit_idx(sq);
//      b_idx = file_idx(z);
//      x ^= file[b_idx] ^ square(b_idx);
//    }
//  }
//
//  uint64_t stop = now.nanoseconds();
//  std::cout << "It took " << stop - start << " nanoseconds." << std::endl;
//  start = now.nanoseconds();
//
//  for (int k = 0; k < 100000; k++)
//  {
//    sq = a1_square;
//    for (uint8_t i = 0; i < 64; i++)
//    {
//      sq = popright_square2(sq);
//      uint8_t z = bit_idx(sq);
//      b_idx = file_idx(z);
//      x ^= file[b_idx] ^ square(b_idx);
//    }
//  }
//
//  stop = now.nanoseconds();
//  std::cout << "It took " << stop - start << " nanoseconds." << std::endl;
//  start = now.nanoseconds();
//
//  for (int k = 0; k < 100000; k++)
//  {
//    sq = a1_square;
//    for (uint8_t i = 0; i < 64; i++)
//    {
//      sq = popright_square(sq);
//      uint8_t z = bit_idx(sq);
//      b_idx = file_idx(z);
//      x ^= file[b_idx] ^ square(b_idx);
//    }
//  }
//
//  stop = now.nanoseconds();
//  std::cout << "It took " << stop - start << " nanoseconds." << std::endl;
//  start = now.nanoseconds();
//
//  for (int k = 0; k < 100000; k++)
//  {
//    sq = a1_square;
//    for (uint8_t i = 0; i < 64; i++)
//    {
//      sq = popright_square2(sq);
//      uint8_t z = bit_idx(sq);
//      b_idx = file_idx(z);
//      x ^= file[b_idx] ^ square(b_idx);
//    }
//  }
//
//  stop = now.nanoseconds();
//  std::cout << "It took " << stop - start << " nanoseconds." << std::endl;
//  std::cout << x << b_idx << sq << std::endl;
//
//}
