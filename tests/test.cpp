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
#include "../src/game.hpp"
#include "../src/current_time.hpp"

using namespace C2_chess;

namespace // file private namespace
{
Current_time steady_clock;

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

TEST_CASE("perft_test")
{
  // An example-line from the file perftsuite.epd looks like this:
  // "101k7/8/7p/8/8/6P1/8/K7 b - - 0 1 ;D1 4 ;D2 16 ;D3 101 ;D4 637 ;D5 4354 ;D6 29679"
  // A three characters long line-number directly followed by the FEN-string of the position,
  // followed by a blank and a semicolon. Then comes the number of nodes searced for
  // depth 1 to 6.
  // We can skip the first three characters and the last blank in each token, if we add an
  // extra blank to the line and split the line with semicolon as delimiter).

  size_t max_depth = 4; // Should be 7 to run all pert-test, but that takes time.
  uint64_t timediff;
  const bool init_pieces = true;
  const bool same_line = true;
  std::string line;
  std::string filename = "tests/test_positions/perftsuite.epd";
  std::ifstream ifs(filename);
  REQUIRE(ifs.is_open());
  std::vector<unsigned int> failed_testcases;
  std::vector<std::string> input_vector;
  unsigned int testnum = 1;

  while (std::getline(ifs, line))
  {
    bool failed = false;

    // Skip empty lines and lines starting with a blank.
    if (line.empty())
      continue;
    // Add a blank to get symmetrical tokens.
    line += ' ';
    input_vector = split(line, ';');
    // Remove the first three characters and the last blank
    // from each string in the vector.
    for (std::string& token : input_vector)
    {
      token = token.substr(3, token.size() - 4);
    }
    write_vector(input_vector, std::cout);

    Bitboard bb;
    bb.read_position(input_vector[0], init_pieces);
    bb.find_legal_moves(gentype::all);
    for (size_t max_search_plies = 2; max_search_plies <= max_depth; max_search_plies++)
    {
      bb.clear_search_info();
      steady_clock.tic();
      unsigned long int n_searched_nodes = bb.perft_test(0, max_search_plies);
      timediff = steady_clock.toc_us();
      if (n_searched_nodes != static_cast<unsigned int>(std::stol(input_vector[max_search_plies - 1])))
      {
        failed = true;
        std::cout << "PERFT-testcase " << testnum << " for depth = " << max_search_plies - 1
                  << " failed. "
                  << n_searched_nodes << " : " << input_vector[max_search_plies - 1]
                  << std::endl;
      }
      else
      {
        std::cout << "PERFT-testcase " << testnum << " for depth = " << max_search_plies - 1
                  << " passed. "
                  << "n_leaf_nodes = " << n_searched_nodes << ". It took " << timediff
                  << " micro seconds." << std::endl;
      }
      REQUIRE(n_searched_nodes  == static_cast<unsigned int>(std::stoi(input_vector[max_search_plies - 1])));
    }
    if (failed)
    {
      failed_testcases.push_back(testnum);
    }
    testnum++;
  }
  REQUIRE(failed_testcases.size() == 0);
  if (failed_testcases.size() == 0)
    std::cout << "All PERFT-testcases passed!" << std::endl;
  else
  {
    std::cout << "The following PERFT-testcases failed: " << std::endl;
    write_vector(failed_testcases, std::cout, same_line);
  }
}

TEST_CASE("Move_generation")
{
  uint64_t timediff;
  std::string arg = "";
  std::cout << "Please enter an argument to the Move_generation_test." << std::endl;
  std::cout << "You can enter the number of a specific test-position," << std::endl;
  std::cout << "or you can add a new test-position by entering its" << std::endl;
  std::cout << "filename, testpos72.pgn for instance." << std::endl;
  std::cout << "Entering all will test all test-positions. " << std::endl;
  std::cout << "Your choice: ";
  std::cin >> arg;
  steady_clock.tic();
  const char* const preferred_test_exec_dir = "/home/torsten/eclipse-workspace/C2-chessengine";
  REQUIRE(check_execution_dir(preferred_test_exec_dir));
  Bitboard_with_utils chessboard;
  unsigned int single_testnum = 0;
  if (arg != "" && arg != "all")
  {
    if (regexp_match(arg, "^testpos[0-9]+.pgn$")) // matches e.g. testpos23.pgn
    {
      REQUIRE(chessboard.add_mg_test_position(arg) == 0);
    }
    else if (regexp_match(arg, "^[0-9]+$")) // matches e.g 23
      single_testnum = static_cast<unsigned int>(std::stoi(arg));
    else
    {
      std::cerr << "ERROR: Unknown input argument: " << arg << std::endl;
      exit(-1);
    }
  }
  int result = chessboard.test_move_generation(single_testnum);
  if (result)
  {
    std::cout << "TESTS FAILED!" << std::endl;
  }
  REQUIRE(result == 0);
  timediff = steady_clock.toc_ns();
  std::cout << "It took " << timediff << " nanoseconds." << std::endl;
}

TEST_CASE("Castling_wrights")
{
// Load test position 71
  std::string FEN_string = get_FEN_test_position(71);
  Bitboard_with_utils chessboard;
  REQUIRE(chessboard.read_position(FEN_string) == 0);
  REQUIRE(chessboard.get_castling_rights() == castling_rights_all);
  chessboard.find_legal_moves(gentype::all);
  chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
  chessboard.write_movelist(std::cout, true) << std::endl;

  SECTION("Rh1xh8 taking Rook at h8 etc")
  {
// Rh1xh8: Both sides looses KS castling
    chessboard.make_UCI_move("h1h8");
    chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BQ | castling_right_WQ));

// Black King steps away loosing right to casle QS as well.
    chessboard.make_UCI_move("e8f7");
    chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_right_WQ);
// White castles queen-side
    chessboard.make_UCI_move("e1c1");
    chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
  }

  SECTION("Both Kings castles")
  {
    SECTION("Queenside-Kingside")
    {
      chessboard.make_UCI_move("e1c1"); // 0-0-0
      chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ));
      chessboard.make_UCI_move("e8g8"); // ...0-0
      chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    }

    SECTION("Kingside-Queenside")
    {
      chessboard.make_UCI_move("e1g1"); // 0-0
      chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ));
      chessboard.make_UCI_move("e8c8"); // ... 0-0-0
      chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    }
  }

  SECTION("Rook moves, but no capture")
  {
    chessboard.make_UCI_move("a1b1"); // Ra1-b1
    chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ | castling_right_WK));
    chessboard.make_UCI_move("h8g8"); // ...Rh8-g8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BQ | castling_right_WK));
    chessboard.make_UCI_move("h1g1"); // ...Rh1-g1
    chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_right_BQ);
    chessboard.make_UCI_move("a8b8"); // ...Ra8-b8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
  }

  SECTION("King moves")
  {
    chessboard.make_UCI_move("e1d1"); // Ke1-d1
    chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ));
    chessboard.make_UCI_move("e8e7"); // ... Ke8-e7
    chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
// Kings move back
    chessboard.make_UCI_move("d1e1"); // Kd1-e1
    chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    chessboard.make_UCI_move("e7e8"); // ... Ke7-e8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);

    uint64_t htag = chessboard.get_hash_tag(); // first test of _hash_tag

// Make another King_move
    chessboard.make_UCI_move("e1e2"); // Ke1-e2
    chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    chessboard.make_UCI_move("e8d8"); // ... Ke8-d8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
// and move back again
    chessboard.make_UCI_move("e2e1"); // Ke2-e1
    chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    chessboard.make_UCI_move("d8e8"); // ... Kd8-e8
    chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
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
  for (uint8_t bit_idx = 0; bit_idx < 64; bit_idx++)
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
  uint64_t time_taken;
  float evaluation;
  Current_time now;
  Bitboard_with_utils chessboard;
  chessboard.read_position(initial_position);
  chessboard.find_legal_moves(gentype::all);
  REQUIRE(fabs(chessboard.evaluate_position(color::white, 7)) < 0.01);
  chessboard.make_UCI_move("e2e4");
  now.tic();
  evaluation = chessboard.evaluate_position(color::black, 7);
  time_taken = now.toc_ns();
  std::cout << "evaluation-time: " << time_taken << " nanoseconds." << std::endl;
  REQUIRE(fabs(evaluation - 0.05) < 0.01);
  chessboard.make_UCI_move("e7e5");
  REQUIRE(fabs(chessboard.evaluate_position(color::white, 7)) < 0.01);
  chessboard.make_UCI_move("g1f3");
  now.tic();
  evaluation = chessboard.evaluate_position(color::black, 7);
  time_taken = now.toc_ns();
  std::cout << "evaluation-time: " << time_taken << " nanoseconds." << std::endl;
  REQUIRE(fabs(evaluation - 0.09) < 0.01);
  chessboard.make_UCI_move("b8c6");
  REQUIRE(fabs(chessboard.evaluate_position(color::white, 7)) < 0.01);
  chessboard.make_UCI_move("d2d4");
  now.tic();
  evaluation = chessboard.evaluate_position(color::black, 7);
  time_taken = now.toc_ns();
  std::cout << "evaluation-time: " << time_taken << " nanoseconds." << std::endl;
  REQUIRE(fabs(evaluation - 0.07) < 0.01);
  chessboard.make_UCI_move("d7d5");
  REQUIRE(fabs(chessboard.evaluate_position(color::white, 7)) < 0.01);
  chessboard.make_UCI_move("e4d5");
  evaluation = chessboard.evaluate_position(color::black, 7);
  REQUIRE(fabs(evaluation - 1.03) < 0.01);
  chessboard.make_UCI_move("e5d4");
  REQUIRE(fabs(chessboard.evaluate_position(color::white, 7)) < 0.01);
  chessboard.make_UCI_move("d5c6");
  evaluation = chessboard.evaluate_position(color::black, 7);
  REQUIRE(fabs(evaluation - 2.99) < 0.01);
  chessboard.make_UCI_move("b7c6");
  evaluation = chessboard.evaluate_position(color::white, 7);
  REQUIRE(fabs(evaluation - 1.97) < 0.01);
  chessboard.make_UCI_move("d1d4");
  evaluation = chessboard.evaluate_position(color::black, 7);
  REQUIRE(fabs(evaluation - 3.03) < 0.01);
  chessboard.make_UCI_move("d8d4");
  evaluation = chessboard.evaluate_position(color::white, 7);
  REQUIRE(fabs(evaluation + 6.04) < 0.01);
  chessboard.make_UCI_move("f3d4");
  evaluation = chessboard.evaluate_position(color::black, 7);
  REQUIRE(fabs(evaluation - 2.98) < 0.01);
  chessboard.make_UCI_move("c8h3");
  evaluation = chessboard.evaluate_position(color::white, 7);
  REQUIRE(fabs(evaluation - 2.93) < 0.01);
  chessboard.make_UCI_move("f1c4");
  evaluation = chessboard.evaluate_position(color::black, 7);
  REQUIRE(fabs(evaluation - 3.0) < 0.01);
  chessboard.make_UCI_move("e8c8");
  evaluation = chessboard.evaluate_position(color::white, 7);
  REQUIRE(fabs(evaluation - 2.81) < 0.01);
  chessboard.make_UCI_move("e1g1");
  evaluation = chessboard.evaluate_position(color::black, 7);
  REQUIRE(fabs(evaluation - 2.96) < 0.01);
  chessboard.make_UCI_move("d8d4");
  evaluation = chessboard.evaluate_position(color::white, 7);
  REQUIRE(fabs(evaluation + 0.04) < 0.01);
  chessboard.make_UCI_move("g2h3");
  evaluation = chessboard.evaluate_position(color::black, 7);
  REQUIRE(fabs(evaluation - 2.96) < 0.01);
  chessboard.make_UCI_move("d4c4");
  evaluation = chessboard.evaluate_position(color::white, 7);
  REQUIRE(fabs(evaluation + 0.06) < 0.01);
  chessboard.make_UCI_move("f1e1");
  evaluation = chessboard.evaluate_position(color::black, 7);
  REQUIRE(fabs(evaluation + 0.02) < 0.01);
  chessboard.make_UCI_move("g8f6");
  evaluation = chessboard.evaluate_position(color::white, 7);
  REQUIRE(fabs(evaluation + 0.11) < 0.01);
  chessboard.make_UCI_move("c1g5");
  evaluation = chessboard.evaluate_position(color::black, 7);
  REQUIRE(fabs(evaluation + 0.06) < 0.01);
  chessboard.make_UCI_move("c4c2");
  evaluation = chessboard.evaluate_position(color::white, 7);
  REQUIRE(fabs(evaluation + 1.02) < 0.01);
  chessboard.make_UCI_move("b1c3");
  evaluation = chessboard.evaluate_position(color::black, 7);
  REQUIRE(fabs(evaluation + 0.93) < 0.01);
  chessboard.make_UCI_move("c2f2");
  evaluation = chessboard.evaluate_position(color::white, 7);
  REQUIRE(fabs(evaluation + 1.93) < 0.01);
  chessboard.make_UCI_move("g1f2");
  evaluation = chessboard.evaluate_position(color::black, 7);
  REQUIRE(fabs(evaluation - 3.07) < 0.01);
  chessboard.make_UCI_move("h7h6");
  evaluation = chessboard.evaluate_position(color::white, 7);
  REQUIRE(fabs(evaluation - 3.07) < 0.01);
  chessboard.make_UCI_move("g5f6");
  evaluation = chessboard.evaluate_position(color::black, 7);
  REQUIRE(fabs(evaluation - 6.15) < 0.01);
  chessboard.make_UCI_move("g7f6");
  evaluation = chessboard.evaluate_position(color::white, 7);
  REQUIRE(fabs(evaluation - 3.09) < 0.01);
  chessboard.make_UCI_move("c3e4");
  evaluation = chessboard.evaluate_position(color::black, 7);
  REQUIRE(fabs(evaluation - 3.03) < 0.01);
  chessboard.make_UCI_move("h6h5");
  evaluation = chessboard.evaluate_position(color::white, 7);
  REQUIRE(fabs(evaluation - 3.03) < 0.01);
  chessboard.make_UCI_move("e4c5");
  evaluation = chessboard.evaluate_position(color::black, 7);
  REQUIRE(fabs(evaluation - 3.07) < 0.01);
  chessboard.make_UCI_move("h8g8");
  evaluation = chessboard.evaluate_position(color::white, 7);
  REQUIRE(fabs(evaluation - 3.02) < 0.01);
  chessboard.make_UCI_move("e1e8");
  evaluation = chessboard.evaluate_position(color::black, 7);
  REQUIRE(is_close(evaluation, 93.0F));
  evaluation = chessboard.evaluate_position(color::black, 1);
  REQUIRE(is_close(evaluation, 99.0F));
}

TEST_CASE("evaluation, mate and stalemate")
{
  float evaluation;
// Load test position 72
  std::string FEN_string = get_FEN_test_position(72);
  Bitboard_with_utils chessboard;
  REQUIRE(chessboard.read_position(FEN_string) == 0);
  chessboard.find_legal_moves(gentype::all);

  SECTION("mate")
  {
    chessboard.make_UCI_move("c8c1");
    evaluation = chessboard.evaluate_position(color::white, 7);
    REQUIRE(evaluation == Approx(-93.0F).margin(0.0001).epsilon(1e-12));
    evaluation = chessboard.evaluate_position(color::white, 1);
    REQUIRE(evaluation == Approx(-99.0F).margin(0.0001).epsilon(1e-12));
  }

  SECTION("stalemate")
  {
    chessboard.make_UCI_move("c8g8");
    evaluation = chessboard.evaluate_position(color::white, 7);
    REQUIRE(evaluation == Approx(0.0F).margin(0.0001).epsilon(1e-12));
  }
}

TEST_CASE("find_best_move")
{
  std::ofstream ofs(get_logfile_name());
  Shared_ostream& logfile = *(Shared_ostream::get_instance(ofs, ofs.is_open()));
  logfile << "TEST STARTED" << "\n";
  Config_params config_params;
  Game game(config_params);

  SECTION("examining:_strange_queen-move1")
  {
    std::string FEN_string = get_FEN_test_position(94);
    game.read_position_FEN(FEN_string);
    game.init();
    Bitmove bestmove = game.engine_go(config_params, "10000");
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Ng8-h6");
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, "10000");
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "d2-d4");
  }

  SECTION("mate in one")
  {
    std::string FEN_string = get_FEN_test_position(90);
    game.read_position_FEN(FEN_string);
    game.init();
    Bitmove bestmove = game.engine_go(config_params, "");
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Qd8-h4+ mate");
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, "");
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "Qd1-h5+ mate");
  }

  SECTION("Qg6_mate_in_two")
  {
    std::string FEN_string = get_FEN_test_position(93);
    game.read_position_FEN(FEN_string);
    game.init();
    Bitmove bestmove = game.engine_go(config_params, "");
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Qg3-g6");
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, "");
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "Qg6-g3");
  }

  SECTION("examining: giving away pawn")
  {
    std::string FEN_string = get_FEN_test_position(91);
    game.read_position_FEN(FEN_string);
    game.init();
    Bitmove bestmove = game.engine_go(config_params, "");
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Ke5-e6");
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, "");
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "Ke4-d5");
  }

  SECTION("examining: missing a mate")
  {
    std::string FEN_string = get_FEN_test_position(89);
    game.read_position_FEN(FEN_string);
    game.init();
    Bitmove bestmove = game.engine_go(config_params, "");
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Ke7-d8");
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, "");
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "Ke2-d1");
  }

  SECTION("strangulation mate")
  {
    std::string FEN_string = get_FEN_test_position(73);
    game.read_position_FEN(FEN_string);
    game.init();
    Bitmove bestmove = game.engine_go(config_params, "");
    REQUIRE(game.get_game_history_state() == History_state{2, 0, 0});
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Nc7-a6+");
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, "");
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "Nc2-a3+");
  }

  SECTION("examining: strange queen-move")
  {
    std::string FEN_string = get_FEN_test_position(78);
    game.read_position_FEN(FEN_string);
    game.init();
    Bitmove bestmove = game.engine_go(config_params, "");
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Nb8-d7");
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, "");
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "Nb1-d2");
  }

  SECTION("examining: strange rook-move")
  {
    std::string FEN_string = get_FEN_test_position(80);
    game.read_position_FEN(FEN_string);
    game.init();
    Bitmove bestmove = game.engine_go(config_params, "");
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Nd3-c1+");
    std::cout << FEN_string << std::endl;
    std::cout << reverse_FEN_string(FEN_string) << std::endl;
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, "");
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "Nd6-c4");
  }

}

TEST_CASE("move-ordering")
{
  std::string FEN_string = get_FEN_test_position(75);
  Bitboard_with_utils chessboard;
  REQUIRE(chessboard.read_position(FEN_string) == 0);
  chessboard.find_legal_moves(gentype::all);
  chessboard.write(std::cout, outputtype::cmd_line_diagram, color::white);
  chessboard.write_movelist(std::cout, true) << std::endl;
}

TEST_CASE("50-moves-rule")
{
  std::string FEN_string = get_FEN_test_position(82);
  Bitboard_with_utils chessboard;
  REQUIRE(chessboard.read_position(FEN_string) == 0);
  REQUIRE(chessboard.get_half_move_counter() == 49);
  REQUIRE(chessboard.is_draw_by_50_moves() == false);
  chessboard.find_legal_moves(gentype::all);

  SECTION("pawn move")
  {
    chessboard.make_UCI_move("h3h4");
    REQUIRE(chessboard.get_half_move_counter() == 0);
    REQUIRE(chessboard.is_draw_by_50_moves() == false);
  }

  SECTION("capture")
  {
    chessboard.make_UCI_move("h7g5");
    REQUIRE(chessboard.get_half_move_counter() == 0);
    REQUIRE(chessboard.is_draw_by_50_moves() == false);
  }

  SECTION("other move")
  {
    chessboard.make_UCI_move("a6b6");
    REQUIRE(chessboard.get_half_move_counter() == 50);
    REQUIRE(chessboard.is_draw_by_50_moves() == true);
  }

  SECTION("some moves")
  {
    chessboard.make_UCI_move("a6b6");
    chessboard.make_UCI_move("a8b8");
    chessboard.make_UCI_move("b6a6");
    chessboard.make_UCI_move("b8a8");
    REQUIRE(chessboard.get_half_move_counter() == 53);
    REQUIRE(chessboard.is_draw_by_50_moves() == true);
  }

}

TEST_CASE("three-fold repetition")
{
  std::string FEN_string = get_FEN_test_position(82);
  Bitboard_with_utils chessboard;
  REQUIRE(chessboard.read_position(FEN_string) == 0);
  chessboard.clear_game_history();
  chessboard.add_position_to_game_history();
  chessboard.find_legal_moves(gentype::all);
  chessboard.make_UCI_move("a6b6");
  chessboard.make_UCI_move("a8b8");
  REQUIRE(chessboard.is_threefold_repetition() == false);
  chessboard.make_UCI_move("b6a6");
  chessboard.make_UCI_move("b8a8");
  REQUIRE(chessboard.is_threefold_repetition() == false);
  chessboard.make_UCI_move("a6b6");
  chessboard.make_UCI_move("a8b8");
  REQUIRE(chessboard.is_threefold_repetition() == false);
  chessboard.make_UCI_move("b6a6");
  chessboard.make_UCI_move("b8a8");
  REQUIRE(chessboard.is_threefold_repetition() == true);
}

TEST_CASE("figure_out_last_move_1")
{
  std::ofstream ofs(get_logfile_name());
  Shared_ostream& logfile = *(Shared_ostream::get_instance(ofs, ofs.is_open()));
  logfile << "\n";
  Config_params config_params;
  Game game(config_params);
  std::string FEN_string = get_FEN_test_position(62);
  REQUIRE(game.read_position_FEN(FEN_string) == 0);
  game.write_diagram(std::cout);
  game.write_movelist(std::cout);

  SECTION("En passant")
  {
    FEN_string = get_FEN_test_position(83);
    REQUIRE(game.read_position_FEN(FEN_string) == 0);
    std::stringstream ss;
    game.write_movelog(ss);
    REQUIRE(ss.str().starts_with("1.e5xf6 e.p."));
  }

  SECTION("Normal move")
  {
    FEN_string = get_FEN_test_position(84);
    REQUIRE(game.read_position_FEN(FEN_string) == 0);
    std::stringstream ss;
    game.write_movelog(ss);
    REQUIRE(ss.str().starts_with("1.Kd4-d5"));
  }

  // TODO:test promotions
}

TEST_CASE("figure_out_last_move_2")
{
  std::ofstream ofs(get_logfile_name());
  Shared_ostream& logfile = *(Shared_ostream::get_instance(ofs, ofs.is_open()));
  logfile << "\n";
  Config_params config_params;
  Game game(config_params);
  std::string FEN_string = get_FEN_test_position(85);
  REQUIRE(game.read_position_FEN(FEN_string) == 0);
  game.write_diagram(std::cout);
  game.write_movelist(std::cout);

  SECTION("Capture")
  {
    FEN_string = get_FEN_test_position(86);
    REQUIRE(game.read_position_FEN(FEN_string) == 0);
    std::stringstream ss;
    game.write_movelog(ss);
    REQUIRE(ss.str().starts_with("1.Rh1xh7+"));
  }

  SECTION("Short castling")
  {
    FEN_string = get_FEN_test_position(87);
    REQUIRE(game.read_position_FEN(FEN_string) == 0);
    std::stringstream ss;
    game.write_movelog(ss);
    REQUIRE(ss.str().starts_with("1.0-0"));
  }

  SECTION("long castling")
  {
    FEN_string = get_FEN_test_position(88);
    REQUIRE(game.read_position_FEN(FEN_string) == 0);
    std::stringstream ss;
    game.write_movelog(ss);
    REQUIRE(ss.str().starts_with("1.0-0-0"));
  }
}

//TEST_CASE("PV_table")
//{
//  PV_table pv_table(PV_TABLE_SIZE_DEFAULT);
//  std::vector<BitMove> ref_vector;
//  REQUIRE(sizeof(pv_table) == size_align(8, static_cast<int>(sizeof(PV_entry*) + sizeof(int) + sizeof(PV_statistics))));
//  REQUIRE(pv_table.get_size() == PV_TABLE_SIZE_DEFAULT);
//  Bitboard_with_utils chessboard;
//  chessboard.read_position(initial_position, true);
//  chessboard.init();
//  Bitboard_with_utils initial_board = chessboard;
//  REQUIRE(initial_board.get_hash_tag() == chessboard.get_hash_tag());
//  uint64_t hash_tag = chessboard.get_hash_tag();
//  chessboard.make_move("e2e4");
//  pv_table.store_move(hash_tag, chessboard.last_move()._move);
//  ref_vector.push_back(chessboard.last_move());
//  hash_tag = chessboard.get_hash_tag();
//  chessboard.make_move("e7e5");
//  pv_table.store_move(hash_tag, chessboard.last_move()._move);
//  ref_vector.push_back(chessboard.last_move());
//  hash_tag = chessboard.get_hash_tag();
//  chessboard.make_move("f2f4");
//  pv_table.store_move(hash_tag, chessboard.last_move()._move);
//  ref_vector.push_back(chessboard.last_move());
//  hash_tag = chessboard.get_hash_tag();
//  chessboard.make_move("e5f4");
//  pv_table.store_move(hash_tag, chessboard.last_move()._move);
//  ref_vector.push_back(chessboard.last_move());
//  hash_tag = chessboard.get_hash_tag();
//  chessboard.make_move("g1f3");
//  pv_table.store_move(hash_tag, chessboard.last_move()._move);
//  ref_vector.push_back(chessboard.last_move());
//  REQUIRE(pv_table.get_statistics()._n_insertions == 5);
//  REQUIRE(pv_table.get_statistics()._n_hash_conflicts == 0);
//  std::vector<BitMove> pv_line;
//  initial_board.get_pv_line(pv_line, pv_table);
//  REQUIRE(pv_line.size() == 5);
//  write_vector(pv_line, std::cout, true);
//  REQUIRE(pv_line == ref_vector);
//  BitMove move_d2_d4(piecetype::Pawn, move_props_none, d2_square, d4_square);
//  pv_table.store_move(initial_board.get_hash_tag(), move_d2_d4._move);
//  REQUIRE(pv_table.get_statistics()._n_hash_conflicts == 0);
//  pv_line.clear();
//  ref_vector.clear();
//  ref_vector.push_back(move_d2_d4);
//  initial_board.get_pv_line(pv_line, pv_table);
//  REQUIRE(pv_line == ref_vector);
//
//}

TEST_CASE("size_align")
{
  REQUIRE(size_align(8, 17) == 24);
  REQUIRE(size_align(8, 23) == 24);
  REQUIRE(size_align(8, 16) == 16);
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
