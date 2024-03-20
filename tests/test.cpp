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
#include "../src/uci.hpp"

using namespace C2_chess;
namespace fs = std::filesystem;
namespace // file private namespace
{
Current_time steady_clock;
std::ofstream ofs(get_logfile_name());
Shared_ostream& logfile = *(Shared_ostream::get_instance(ofs, ofs.is_open()));
const fs::path project_root_dir(""s + getenv("HOME") + "/eclipse-workspace/C2-chessengine");
const fs::path test_pos_filename(project_root_dir.string() + "/tests/test_positions/FEN_test_positions.txt");
const fs::path perft_suite_filename(project_root_dir.string() + "/tests/test_positions/perftsuite.epd");

// void cd_project_root_dir()
// {
//   // setting current working directory
//   fs::current_path(project_root_dir);
//   REQUIRE(fs::current_path().string() == project_root_dir);
// }

std::string get_FEN_test_position(unsigned int n)
{
  // auto path = std::filesystem::current_path(); // getting path

  // set current working directory
  //cd_project_root_dir();

  std::ifstream ifs(test_pos_filename);
  REQUIRE(ifs.is_open());
  unsigned int testnum = 1;
  std::string line;
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
      // Match the first 6 tokens in the std::string line.
      std::vector<std::string> matches;
      regexp_grep(line, "^([^\\s]+\\s){5}[^\\s]+", matches);
      return matches[0];
    }
    testnum++;
  }
  return "";
}

void make_and_takeback_move(Game& game, const std::string& uci_move)
{
  game.write_chessboard(std::cout, Color::White);
  auto& bwu = game.get_chessboard();
  auto saved_hash_tag = bwu.get_hash_tag();
  auto saved_side_to_move = bwu.get_side_to_move();
  auto saved_move_number = bwu.get_move_number();
  auto saved_castling_rights = bwu.get_castling_rights();
  auto saved_half_move_counter = bwu.get_half_move_counter();
  auto saved_has_castled_0 = bwu.has_castled(Color::White);
  auto saved_has_castled_1 = bwu.has_castled(Color::Black);
  auto saved_ep_square = bwu.get_ep_square();
  auto saved_material_diff = bwu.get_material_diff();
  auto saved_latest_move = bwu.get_latest_move();
  auto saved_all_pieces = bwu.get_all_pieces();
  auto saved_white_pieces = bwu.get_white_pieces();
  auto saved_black_pieces = bwu.get_black_pieces();
  auto saved_own = bwu.get_own();
  auto saved_other = bwu.get_other();
  auto saved_game_history = bwu.get_game_history();

  Takeback_state tb_state;
  game.make_move(uci_move, tb_state);
  game.write_chessboard(std::cout, Color::White);

  game.takeback_latest_move(tb_state);
  game.write_chessboard(std::cout, Color::White);

  REQUIRE(game.get_castling_rights() == saved_castling_rights);
  REQUIRE(game.get_material_diff() == saved_material_diff);

  REQUIRE(bwu.get_hash_tag() == saved_hash_tag);
  REQUIRE(bwu.get_side_to_move() == saved_side_to_move);
  REQUIRE(bwu.get_move_number() == saved_move_number);
  REQUIRE(bwu.get_castling_rights() == saved_castling_rights);
  REQUIRE(bwu.get_half_move_counter() == saved_half_move_counter);
  REQUIRE(bwu.has_castled(Color::White) == saved_has_castled_0);
  REQUIRE(bwu.has_castled(Color::Black) == saved_has_castled_1);
  REQUIRE(bwu.get_ep_square() == saved_ep_square);
  REQUIRE(bwu.get_material_diff() == saved_material_diff);
  REQUIRE(bwu.get_latest_move() == saved_latest_move);
  REQUIRE(bwu.get_all_pieces() == saved_all_pieces);
  REQUIRE(bwu.get_white_pieces() == saved_white_pieces);
  REQUIRE(bwu.get_black_pieces() == saved_black_pieces);
  REQUIRE(bwu.get_own() == saved_own);
  REQUIRE(bwu.get_other() == saved_other);
  REQUIRE(bwu.get_game_history() == saved_game_history);
}

} // End of fileprivate namespace

TEST_CASE("perft_test") // A thorough move-generation test with public test-data from the web.
{
  // An example-line from the file perftsuite.epd looks like this:
  // "101k7/8/7p/8/8/6P1/8/K7 b - - 0 1 ;D1 4 ;D2 16 ;D3 101 ;D4 637 ;D5 4354 ;D6 29679"
  // A three characters long line-number directly followed by the FEN-string of the position,
  // followed by a blank and a semicolon. Then comes the number of nodes searched for
  // depth 1 to 6.
  // We can skip the first three characters and the last blank in each token, if we add an
  // extra blank to the line and split the line with semicolon as delimiter).
  // max_depth should be 6 to fully run all pert-test, but that takes time,
  // so for a regression test I set it to 4 after testing through all 6 depths.
  int max_depth = 4;
  uint64_t timediff;
  //nst bool init_pieces = true;
  const bool same_line = true;
  std::cout << perft_suite_filename.string() << std::endl;
  std::ifstream ifs(perft_suite_filename.string());
  REQUIRE(ifs.is_open());
  std::vector<unsigned int> failed_testcases;
  std::vector<std::string> input_vector;
  unsigned int testnum = 1;

  std::string line;
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
    bb.init();
    bb.read_position(input_vector[0], init_pieces);
    //bb.find_legal_moves(*bb.get_movelist(0), Gentype::All);
    for (int max_search_depth = 2; max_search_depth <= max_depth; max_search_depth++)
    {
      bb.clear_search_info();
      steady_clock.tic();
      unsigned long int n_searched_nodes = bb.perft_test(0, max_search_depth);
      timediff = steady_clock.toc_us();
      if (n_searched_nodes != static_cast<unsigned int>(std::stol(input_vector[static_cast<std::size_t>(max_search_depth - 1)])))
      {
        failed = true;
        std::cout << "PERFT-testcase " << testnum << " for depth = " << max_search_depth - 1 << " failed. " << n_searched_nodes << " : " << input_vector[static_cast<std::size_t>(max_search_depth - 1)]
                  << std::endl;
      }
      else
      {
        std::cout << "PERFT-testcase " << testnum << " for depth = " << max_search_depth - 1 << " passed. " << "n_leaf_nodes = " << n_searched_nodes << ". It took " << timediff
                  << " micro seconds." << std::endl;
      }
      REQUIRE(n_searched_nodes == static_cast<unsigned int>(std::stoi(input_vector[static_cast<std::size_t>(max_search_depth - 1)])));
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

TEST_CASE("move_generation")
{
  uint64_t timediff;
  std::string answer;
  std::cout << "Please enter an argument to the Move_generation_test." << std::endl;
  std::cout << "You can enter the number of a specific test-position," << std::endl;
  std::cout << "or you can add a new test-position by entering its" << std::endl;
  std::cout << "filename, testpos72.pgn for instance." << std::endl;
  std::cout << "Entering all will test all test-positions. " << std::endl;
  std::cout << "Your choice: ";
  //std::getline(std::cin>>std::ws, answer);
  std::cout << "What" << std::endl;
  std::cin >> std::ws >> answer;
  steady_clock.tic();
  Bitboard_with_utils chessboard;
  answer = "testpos101.pgn";
  chessboard.init();
  unsigned int single_testnum = 0;
  if (answer != "" && answer != "all")
  {
    if (regexp_match(answer, "^testpos[0-9]+.pgn$")) // matches e.g. testpos23.pgn
    {
      REQUIRE(chessboard.add_mg_test_position(answer) == 0);
    }
    else if (regexp_match(answer, "^[0-9]+$")) // matches e.g 23
      single_testnum = static_cast<unsigned int>(std::stoi(answer));
    else
    {
      std::cerr << "ERROR: Unknown input argument: " << answer << std::endl;
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

TEST_CASE("move-ordering")
{
  std::string FEN_string = get_FEN_test_position(75);
  Bitboard_with_utils chessboard;
  chessboard.init();
  REQUIRE(chessboard.read_position(FEN_string, init_pieces) == 0);
  chessboard.write(std::cout, Color::White);
  chessboard.write_movelist(std::cout, true) << std::endl;
  list_t movelist;
  chessboard.find_legal_moves(movelist, Gentype::All);
  std::cout << "-------" << std::endl;
  for (const auto& move : movelist)
    std::cout << move._evaluation << std::endl;
  REQUIRE(is_sorted_descending(movelist));
}

TEST_CASE("history")
{
  Bitboard_with_utils bwu;
  bwu.clear_game_history();
  for (auto hash_tag = zero; hash_tag < 256; hash_tag++)
  {
    bwu.add_position_to_game_history(hash_tag);
    REQUIRE(bwu.get_history_state()._is_threefold_repetiotion == zero);
    REQUIRE(bwu.get_history_state()._n_repeated_positions == zero);
    REQUIRE(bwu.get_history_state()._n_plies == static_cast<int>(hash_tag + 1));
  }
  for (int n_plies = 256; n_plies > 0; n_plies--)
  {
    bwu.takeback_from_game_history();
    REQUIRE(bwu.get_history_state()._is_threefold_repetiotion == zero);
    REQUIRE(bwu.get_history_state()._n_repeated_positions == zero);
    REQUIRE(bwu.get_history_state()._n_plies == n_plies - 1);
  }

  for (auto hash_tag = zero; hash_tag < 256; hash_tag++)
  {
    bwu.add_position_to_game_history(hash_tag);
    bwu.add_position_to_game_history(hash_tag);
    REQUIRE(bwu.get_history_state()._is_threefold_repetiotion == zero);
    REQUIRE(bwu.get_history_state()._n_repeated_positions == static_cast<int>(hash_tag + 1));
    REQUIRE(bwu.get_history_state()._n_plies ==  static_cast<int>(2 * (hash_tag + 1)));
  }
  int tmp = 256;
  for (int n_plies = 256; n_plies > 0; n_plies--)
  {
    bwu.takeback_from_game_history();
    REQUIRE(bwu.get_history_state()._n_plies == 256 + n_plies - 1);
    REQUIRE(bwu.get_history_state()._is_threefold_repetiotion == zero);
    if (n_plies % 2 == 0)
    {
      tmp--;
    }
    REQUIRE(bwu.get_history_state()._n_repeated_positions == tmp);
  }

  auto saved_history_state = bwu.get_history_state();
  bwu.add_position_to_game_history(709870987);
  bwu.add_position_to_game_history(709870988);
  bwu.add_position_to_game_history(709870989);
  bwu.add_position_to_game_history(709870990);
  bwu.reset_history_state(saved_history_state);
  REQUIRE(bwu.get_history_state()._is_threefold_repetiotion == zero);
  REQUIRE(bwu.get_history_state()._n_repeated_positions == 128);
  REQUIRE(bwu.get_history_state()._n_plies == 256);
  // Or a little simpler:
  REQUIRE(bwu.get_history_state() == saved_history_state);
}

TEST_CASE("history_three-fold_repetition")
{
  std::string FEN_string = get_FEN_test_position(82);
  Bitboard_with_utils chessboard;
  chessboard.init();
  REQUIRE(chessboard.read_position(FEN_string) == 0);
  chessboard.clear_game_history();
  chessboard.add_position_to_game_history(chessboard.get_hash_tag());
  Takeback_state tb_state_dummy;
  //chessboard.find_legal_moves(*chessboard.get_movelist(0), Gentype::All);
  chessboard.make_UCI_move("a6b6", tb_state_dummy);
  chessboard.make_UCI_move("a8b8", tb_state_dummy);
  REQUIRE(chessboard.is_threefold_repetition() == false);
  chessboard.make_UCI_move("b6a6", tb_state_dummy);
  chessboard.make_UCI_move("b8a8", tb_state_dummy);
  REQUIRE(chessboard.is_threefold_repetition() == false);
  chessboard.make_UCI_move("a6b6", tb_state_dummy);
  chessboard.make_UCI_move("a8b8", tb_state_dummy);
  REQUIRE(chessboard.is_threefold_repetition() == false);
  chessboard.make_UCI_move("b6a6", tb_state_dummy);
  chessboard.make_UCI_move("b8a8", tb_state_dummy);
  REQUIRE(chessboard.is_threefold_repetition() == true);
  chessboard.takeback_from_game_history();
  REQUIRE(chessboard.is_threefold_repetition() == false);
}

TEST_CASE("castling_rights")
{
  // Load test position 71
  Takeback_state tb_state_dummy;
  std::string FEN_string = get_FEN_test_position(71);
  Bitboard_with_utils chessboard;
  chessboard.init();
  REQUIRE(chessboard.read_position(FEN_string) == 0);
  REQUIRE(chessboard.get_castling_rights() == castling_rights_all);
  //chessboard.find_legal_moves(*chessboard.get_movelist(0), Gentype::All);
  chessboard.write(std::cout, Color::White);
  chessboard.write_movelist(std::cout, on_same_line) << std::endl;

  SECTION("Rh1xh8 taking Rook at h8 etc")
  {
    // Rh1xh8: Both sides looses KS castling
    chessboard.make_UCI_move("h1h8");
    chessboard.write(std::cout, Color::White);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BQ | castling_right_WQ));

    // Black King steps away loosing right to casle QS as well.
    chessboard.make_UCI_move("e8f7");
    chessboard.write(std::cout, Color::White);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_right_WQ);
    // White castles queen-side
    chessboard.make_UCI_move("e1c1");
    chessboard.write(std::cout, Color::White);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
  }

  SECTION("Both Kings castles")
  {
    SECTION("Queenside-Kingside")
    {
      chessboard.make_UCI_move("e1c1"); // 0-0-0
      chessboard.write(std::cout, Color::White);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ));
      chessboard.make_UCI_move("e8g8"); // ...0-0
      chessboard.write(std::cout, Color::White);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    }

    SECTION("Kingside-Queenside")
    {
      chessboard.make_UCI_move("e1g1"); // 0-0
      chessboard.write(std::cout, Color::White);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ));
      chessboard.make_UCI_move("e8c8"); // ... 0-0-0
      chessboard.write(std::cout, Color::White);
      chessboard.write_movelist(std::cout, true) << std::endl;
      REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    }
  }

  SECTION("Rook moves, but no capture")
  {
    chessboard.make_UCI_move("a1b1"); // Ra1-b1
    chessboard.write(std::cout, Color::White);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ | castling_right_WK));
    chessboard.make_UCI_move("h8g8"); // ...Rh8-g8
    chessboard.write(std::cout, Color::White);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BQ | castling_right_WK));
    chessboard.make_UCI_move("h1g1"); // ...Rh1-g1
    chessboard.write(std::cout, Color::White);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_right_BQ);
    chessboard.make_UCI_move("a8b8"); // ...Ra8-b8
    chessboard.write(std::cout, Color::White);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
  }

  SECTION("King moves")
  {
    chessboard.make_UCI_move("e1d1"); // Ke1-d1
    chessboard.write(std::cout, Color::White);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == (castling_right_BK | castling_right_BQ));
    chessboard.make_UCI_move("e8e7"); // ... Ke8-e7
    chessboard.write(std::cout, Color::White);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
// Kings move back
    chessboard.make_UCI_move("d1e1"); // Kd1-e1
    chessboard.write(std::cout, Color::White);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    chessboard.make_UCI_move("e7e8"); // ... Ke7-e8
    chessboard.write(std::cout, Color::White);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);

    uint64_t htag = chessboard.get_hash_tag(); // first test of _hash_tag

// Make another King_move
    chessboard.make_UCI_move("e1e2"); // Ke1-e2
    chessboard.write(std::cout, Color::White);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    chessboard.make_UCI_move("e8d8"); // ... Ke8-d8
    chessboard.write(std::cout, Color::White);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
// and move back again
    chessboard.make_UCI_move("e2e1"); // Ke2-e1
    chessboard.write(std::cout, Color::White);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);
    chessboard.make_UCI_move("d8e8"); // ... Kd8-e8
    chessboard.write(std::cout, Color::White);
    chessboard.write_movelist(std::cout, true) << std::endl;
    REQUIRE(chessboard.get_castling_rights() == castling_rights_none);

    REQUIRE(htag == chessboard.get_hash_tag());
  }
}

TEST_CASE("Bitboard between")
{
  Bitboard_with_utils chessboard;
  chessboard.init();
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
  uint8_t bit_idx = 0;
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
  chessboard.init();
  chessboard.read_position(start_position_FEN);
  //chessboard.find_legal_moves(*chessboard.get_movelist(0), Gentype::All);
  REQUIRE(fabsf(chessboard.evaluate_position()) < 0.01F);
  chessboard.make_UCI_move("e2e4");
  now.tic();
  evaluation = chessboard.evaluate_position();
  time_taken = now.toc_ns();
  std::cout << "evaluation-time: " << time_taken << " nanoseconds." << std::endl;
  REQUIRE(fabsf(evaluation - 0.05F) < 0.01F);
  chessboard.make_UCI_move("e7e5");
  REQUIRE(fabsf(chessboard.evaluate_position()) < 0.01F);
  chessboard.make_UCI_move("g1f3");
  now.tic();
  evaluation = chessboard.evaluate_position();
  time_taken = now.toc_ns();
  std::cout << "evaluation-time: " << time_taken << " nanoseconds." << std::endl;
  REQUIRE(fabsf(evaluation - 0.09F) < 0.01F);
  chessboard.make_UCI_move("b8c6");
  REQUIRE(fabsf(chessboard.evaluate_position()) < 0.01F);
  chessboard.make_UCI_move("d2d4");
  now.tic();
  evaluation = chessboard.evaluate_position();
  time_taken = now.toc_ns();
  std::cout << "evaluation-time: " << time_taken << " nanoseconds." << std::endl;
  REQUIRE(fabsf(evaluation - 0.07F) < 0.01F);
  chessboard.make_UCI_move("d7d5");
  REQUIRE(fabsf(chessboard.evaluate_position()) < 0.01F);
  chessboard.make_UCI_move("e4d5");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 0.99F) < 0.01F);
  chessboard.make_UCI_move("e5d4");
  REQUIRE(fabsf(chessboard.evaluate_position()) < 0.01F);
  chessboard.make_UCI_move("d5c6");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 2.95F) < 0.01F);
  chessboard.make_UCI_move("b7c6");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 2.03F) < 0.01F);
  chessboard.make_UCI_move("d1d4");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 3.14F) < 0.01F);
  chessboard.make_UCI_move("d8d4");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation + 5.94F) < 0.01F);
  chessboard.make_UCI_move("f3d4");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 3.08F) < 0.01F);
  chessboard.make_UCI_move("c8h3");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 3.03F) < 0.01F);
  chessboard.make_UCI_move("f1c4");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 3.10F) < 0.01F);
  chessboard.make_UCI_move("e8c8");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 2.91F) < 0.01F);
  chessboard.make_UCI_move("e1g1");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 3.06F) < 0.01F);
  chessboard.make_UCI_move("d8d4");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 0.06F) < 0.01F);
  chessboard.make_UCI_move("g2h3");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 2.96F) < 0.01F);
  chessboard.make_UCI_move("d4c4");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation + 0.06F) < 0.01F);
  chessboard.make_UCI_move("f1e1");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation + 0.02F) < 0.01F);
  chessboard.make_UCI_move("g8f6");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation + 0.11F) < 0.01F);
  chessboard.make_UCI_move("c1g5");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation + 0.06F) < 0.01F);
  chessboard.make_UCI_move("c4c2");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation + 1.02F) < 0.01F);
  chessboard.make_UCI_move("b1c3");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation + 0.93F) < 0.01F);
  chessboard.make_UCI_move("c2f2");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation + 1.93F) < 0.01F);
  chessboard.make_UCI_move("g1f2");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 3.07F) < 0.01F);
  chessboard.make_UCI_move("h7h6");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 3.07F) < 0.01F);
  chessboard.make_UCI_move("g5f6");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 6.15F) < 0.01F);
  chessboard.make_UCI_move("g7f6");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 3.15F) < 0.01F);
  chessboard.make_UCI_move("c3e4");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 3.09F) < 0.01F);
  chessboard.make_UCI_move("h6h5");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 3.09F) < 0.01F);
  chessboard.make_UCI_move("e4c5");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 3.13F) < 0.01F);
  chessboard.make_UCI_move("h8g8");
  evaluation = chessboard.evaluate_position();
  REQUIRE(fabsf(evaluation - 3.08F) < 0.01F);
  chessboard.make_UCI_move("e1e8");
  evaluation = chessboard.evaluate_empty_movelist(7);
  REQUIRE(is_close(evaluation, 93.0F));
  evaluation = chessboard.evaluate_empty_movelist(1);
  REQUIRE(is_close(evaluation, 99.0F));
}

TEST_CASE("evaluation, mate and stalemate")
{
  float evaluation;
  // Load test position 72
  std::string FEN_string = get_FEN_test_position(72);
  Bitboard_with_utils chessboard;
  chessboard.init();
  REQUIRE(chessboard.read_position(FEN_string) == 0);
  //chessboard.find_legal_moves(*chessboard.get_movelist(0), Gentype::All);

  SECTION("mate")
  {
    chessboard.make_UCI_move("c8c1");
    evaluation = chessboard.evaluate_empty_movelist(6);
    REQUIRE(evaluation == Approx(-94.0F).margin(0.0001).epsilon(1e-12));
    evaluation = chessboard.evaluate_empty_movelist(1);
    REQUIRE(evaluation == Approx(-99.0F).margin(0.0001).epsilon(1e-12));
  }

  SECTION("stalemate")
  {
    chessboard.make_UCI_move("c8g8");
    evaluation = chessboard.evaluate_empty_movelist(1);
    REQUIRE(evaluation == Approx(0.0F).margin(0.0001).epsilon(1e-12));
  }
}

TEST_CASE("find_best_move")
{
  logfile << "TEST STARTED find_best_move"s << "\n";
  Config_params config_params;
  config_params.set_config_param("max_search_depth", "6");
  Game game(config_params);
  game.init();
  Go_params go_params; // All members in go_params are set to zero.

  SECTION("examining_giving_away_pawn") // fixed
  {
    std::string FEN_string = get_FEN_test_position(91);
    game.read_position_FEN(FEN_string);
    game.init();
    go_params.movetime = 10000000; // milliseconds
    Bitmove bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Be4-a8");
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "Be5-h8");
  }

  SECTION("examining_strange_threefold_repetition") // fixed
  {
    std::string FEN_string = get_FEN_test_position(98);
    game.read_position_FEN(FEN_string);
    game.init();
    go_params.movetime = 10000000; // milliseconds
    Bitmove bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Bc8-g4");
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "Bc1-g5");
  }

  SECTION("examining_strange_queen-move1") // fixed
  {
    std::string FEN_string = get_FEN_test_position(94);
    game.read_position_FEN(FEN_string);
    game.init();
    go_params.movetime = 10000000; // 10000 seconds should be enough even for debug-compiled executable
    Bitmove bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Ng8-h6");
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "Ng1-h3");
  }

  SECTION("mate_in_one")
  {
    std::string FEN_string = get_FEN_test_position(90);
    game.read_position_FEN(FEN_string);
    game.init();
    go_params.movetime = 100000; // milliseconds
    Bitmove bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Qd8-h4+ mate");
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
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
    go_params.movetime = 100000; // milliseconds
    Bitmove bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Qg3-g6");
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "Qg6-g3");
  }

  SECTION("examining_missing_a_mate") // fixed
  {
    std::string FEN_string = get_FEN_test_position(89);
    game.read_position_FEN(FEN_string);
    game.init();
    go_params.movetime = 100000; // milliseconds
    Bitmove bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Ke7-d8");
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "Ke2-d1");
  }

  SECTION("strangulation_mate")
  {
    std::string FEN_string = get_FEN_test_position(73);
    game.read_position_FEN(FEN_string);
    game.init();
    go_params.movetime = 100000; // milliseconds
    Bitmove bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    REQUIRE(game.get_game_history_state() == History_state{2, 0, 0});
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Nc7-a6+");
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "Nc2-a3+");
  }

  SECTION("examining_strange_queen-move")
  {
    std::string FEN_string = get_FEN_test_position(78);
    game.read_position_FEN(FEN_string);
    game.init();
    go_params.movetime = 100000; // milliseconds
    Bitmove bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Bc8-b7");
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "Bc1-b2");
  }

  SECTION("examining_strange_rook-move")
  {
    std::string FEN_string = get_FEN_test_position(80);
    game.read_position_FEN(FEN_string);
    game.init();
    go_params.movetime = 100000; // milliseconds
    Bitmove bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Nd3-c1+");
    std::cout << FEN_string << std::endl;
    std::cout << reverse_FEN_string(FEN_string) << std::endl;
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "Nd6-c8+");
  }

  SECTION("examining_strange_queen_move2")
  {
    std::string FEN_string = get_FEN_test_position(101);
    game.read_position_FEN(FEN_string);
    game.init();
    go_params.movetime = 100000; // milliseconds
    Bitmove bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    std::stringstream ss;
    ss << bestmove;
    REQUIRE(ss.str() == "Nf6-d7");
    std::cout << FEN_string << std::endl;
    std::cout << reverse_FEN_string(FEN_string) << std::endl;
    game.read_position_FEN(reverse_FEN_string(FEN_string));
    game.init();
    bestmove = game.engine_go(config_params, go_params, use_max_search_depth);
    std::cout << "Best move: " << bestmove << std::endl;
    ss.clear();
    ss.str("");
    ss << bestmove;
    REQUIRE(ss.str() == "Nf3-d2");
  }

}

TEST_CASE("50-moves-rule")
{
  std::string FEN_string = get_FEN_test_position(82);
  Bitboard_with_utils chessboard;
  chessboard.init();
  REQUIRE(chessboard.read_position(FEN_string, init_pieces) == 0);
  REQUIRE(chessboard.get_half_move_counter() == 49);
  REQUIRE(chessboard.is_draw_by_50_moves() == false);
  //chessboard.find_legal_moves(*chessboard.get_movelist(0), Gentype::All);

  SECTION("pawn_move")
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

  SECTION("other_move")
  {
    chessboard.make_UCI_move("a6b6");
    REQUIRE(chessboard.get_half_move_counter() == 50);
    REQUIRE(chessboard.is_draw_by_50_moves() == true);
  }

  SECTION("some_moves")
  {
    chessboard.make_UCI_move("a6b6");
    chessboard.make_UCI_move("a8b8");
    chessboard.make_UCI_move("b6a6");
    chessboard.make_UCI_move("b8a8");
    REQUIRE(chessboard.get_half_move_counter() == 53);
    REQUIRE(chessboard.is_draw_by_50_moves() == true);
  }

}

TEST_CASE("figure_out_last_move_1")
{
  logfile << "\n";
  Config_params config_params;
  Game game(config_params);
  game.init();
  std::string FEN_string = get_FEN_test_position(62);
  REQUIRE(game.read_position_FEN(FEN_string) == 0);
  game.write_diagram(std::cout);
  game.write_movelist(std::cout);

  SECTION("En_passant")
  {
    FEN_string = get_FEN_test_position(83);
    REQUIRE(game.read_position_FEN(FEN_string) == 0);
    std::stringstream ss;
    game.write_movelog(ss);
    game.write_movelog(std::cout);
    REQUIRE("kallee5xf6 e.p.kkk"s.find("e5xf6 e.p.", 0) != std::string::npos);
    REQUIRE(ss.str().find("1.e5xf6 e.p.") != std::string::npos);
  }

  SECTION("Normal_move")
  {
    FEN_string = get_FEN_test_position(84);
    REQUIRE(game.read_position_FEN(FEN_string) == 0);
    std::stringstream ss;
    game.write_movelog(ss);
    game.write_movelog(std::cerr);
    REQUIRE(ss.str().starts_with("1.Kd4-d5"));
  }

  // TODO:test promotions
}

TEST_CASE("figure_out_last_move_2")
{
  logfile << "\n";
  Config_params config_params;
  Game game(config_params);
  game.init();
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

  SECTION("Short_castling")
  {
    FEN_string = get_FEN_test_position(87);
    REQUIRE(game.read_position_FEN(FEN_string) == 0);
    std::stringstream ss;
    game.write_movelog(ss);
    REQUIRE(ss.str().starts_with("1.0-0"));
  }

  SECTION("long_castling")
  {
    FEN_string = get_FEN_test_position(88);
    REQUIRE(game.read_position_FEN(FEN_string) == 0);
    std::stringstream ss;
    game.write_movelog(ss);
    REQUIRE(ss.str().starts_with("1.0-0-0"));
  }
}

TEST_CASE("size_align")
{
  REQUIRE(size_align(8, 17) == 24);
  REQUIRE(size_align(8, 23) == 24);
  REQUIRE(size_align(8, 16) == 16);
}

TEST_CASE("sizeof_Bitmove")
{
  REQUIRE(sizeof(Bitmove) == 8);
  REQUIRE(alignof(Bitmove) == 4);
}
;

TEST_CASE("takeback_promotion")
{
  logfile << "TEST STARTED takeback_promotion" << "\n";
  Config_params config_params;
  Game game(config_params);
  game.init();
  std::string FEN_string = get_FEN_test_position(95);
  game.read_position_FEN(FEN_string);
  game.init();

  SECTION("Queen promotion on empty square")
  {
    make_and_takeback_move(game, "g7g8q");
  }

  SECTION("Bishop promotion on empty square")
  {
    make_and_takeback_move(game, "g7g8b");
  }

  SECTION("Knight promotion taking a Queen")
  {
    make_and_takeback_move(game, "g7h8n");
  }

  SECTION("Rook promotion taking a rook")
  {
    make_and_takeback_move(game, "g7f8r");
  }

  SECTION("Bishop promotion taking a knight")
  {
    make_and_takeback_move(game, "c7b8q");
  }

  SECTION("Queen promotion taking a bishop")
  {
    make_and_takeback_move(game, "c7d8q");
  }
}

TEST_CASE("takeback_en_passant")
{
  logfile << "TEST STARTED takeback_en_passant" << "\n";
  Config_params config_params;
  Game game(config_params);
  game.init();
  std::string FEN_string = get_FEN_test_position(96);
  game.read_position_FEN(FEN_string);
  game.init();

  SECTION("d5xe6_ep")
  {
    make_and_takeback_move(game, "d5e6");
  }
}

TEST_CASE("takeback_castling")
{
  logfile << "TEST STARTED takeback_castling" << "\n";
  Config_params config_params;
  Game game(config_params);
  game.init();
  std::string FEN_string = get_FEN_test_position(97);
  game.read_position_FEN(FEN_string);
  game.init();

  SECTION("short_castling")
  {
    make_and_takeback_move(game, "e1g1");
  }

  SECTION("long_castling")
  {
    make_and_takeback_move(game, "e1c1");
  }
}

TEST_CASE("takeback_normal_move")
{
  logfile << "TEST STARTED takeback_normal" << "\n";
  Config_params config_params;
  Game game(config_params);
  game.init();
  std::string FEN_string = get_FEN_test_position(97);
  game.read_position_FEN(FEN_string);
  game.init();

  SECTION("pawn_move")
  {
    make_and_takeback_move(game, "e2e3");
  }

  SECTION("long_castling")
  {
    make_and_takeback_move(game, "d2d4");
  }
}

TEST_CASE("print_patterns")
{
  logfile << "TEST STARTED print_patterns" << "\n";
  std::cout << "pawn_center_control_W_pattern" << std::endl;
  std::cout << to_binary_board(pawn_center_control_W_pattern) << std::endl;
  std::cout << "pawn_center_control_B_pattern" << std::endl;
  std::cout << to_binary_board(pawn_center_control_B_pattern) << std::endl;
  std::cout << "king_center_control_pattern1" << std::endl;
  std::cout << to_binary_board(king_center_control_pattern1) << std::endl;
  std::cout << "king_center_control_pattern2" << std::endl;
  std::cout << to_binary_board(king_center_control_pattern2) << std::endl;
  std::cout << "knight_center_control_pattern1" << std::endl;
  std::cout << to_binary_board(knight_center_control_pattern1) << std::endl;
  std::cout << "knight_center_control_pattern2" << std::endl;
  std::cout << to_binary_board(knight_center_control_pattern2) << std::endl;
  std::cout << "rook_center_control_pattern" << std::endl;
  std::cout << to_binary_board(rook_center_control_pattern) << std::endl;

  std::cout << "west_of_center" << std::endl;
  std::cout << to_binary_board(west_of_center) << std::endl;
  std::cout << "east_of_center" << std::endl;
  std::cout << to_binary_board(east_of_center) << std::endl;
  std::cout << "south_of_center" << std::endl;
  std::cout << to_binary_board(south_of_center) << std::endl;
  std::cout << "north_of_center" << std::endl;
  std::cout << to_binary_board(north_of_center) << std::endl;

  std::cout << "north_west_of_center" << std::endl;
  std::cout << to_binary_board(north_west_of_center) << std::endl;
  std::cout << "north_east_of_center" << std::endl;
  std::cout << to_binary_board(north_east_of_center) << std::endl;
  std::cout << "south_west_of_center" << std::endl;
  std::cout << to_binary_board(south_west_of_center) << std::endl;
  std::cout << "south_east_of_center" << std::endl;
  std::cout << to_binary_board(south_east_of_center) << std::endl;

  std::cout << "bishop_north_west_of_center" << std::endl;
  std::cout << to_binary_board(bishop_north_west_of_center) << std::endl;
  std::cout << "bishop_north_east_of_center" << std::endl;
  std::cout << to_binary_board(bishop_north_east_of_center) << std::endl;
  std::cout << "bishop_south_west_of_center" << std::endl;
  std::cout << to_binary_board(bishop_south_west_of_center) << std::endl;
  std::cout << "bishop_south_east_of_center" << std::endl;
  std::cout << to_binary_board(bishop_south_east_of_center) << std::endl;

  std::cout << "bishop_center_control_pattern1" << std::endl;
  std::cout << to_binary_board(bishop_center_control_pattern1) << std::endl;
  std::cout << "bishop_center_control_pattern2" << std::endl;
  std::cout << to_binary_board(bishop_center_control_pattern2) << std::endl;
  for (int i = a; i <= h; i++)
  {
    std::cout << "isolated_pawn_pattern[" << i << "]" << std::endl;
    std::cout << to_binary_board(isolani_pattern[i]) << std::endl;
  }
  std::cout << "passed_pawn_pattern_w" << std::endl;
  std::cout << to_binary_board(passed_pawn_pattern_W) << std::endl;
  std::cout << "passed_pawn_pattern_B" << std::endl;
  std::cout << to_binary_board(passed_pawn_pattern_B) << std::endl;
  std::cout << "d4-adjusted passed_pawn_pattern_B" << std::endl;
  std::cout << to_binary_board(adjust_passer_pattern(passed_pawn_pattern_B, d4_square, bit_idx(e7_square))) << std::endl;
  std::cout << "d4-adjusted passed_pawn_pattern_W" << std::endl;
  std::cout << to_binary_board(adjust_passer_pattern(passed_pawn_pattern_W, d4_square, bit_idx(e2_square))) << std::endl;

}

TEST_CASE("print_evaluations")
{
  std::string line;
  auto filename = "tests/test_positions/FEN_test_positions.txt"s;
  std::ifstream ifs(filename);
  if (!ifs.is_open())
  {
    std::cerr << "Couldn't open file " << filename << std::endl;
    return;
  }
  std::vector<unsigned int> failed_testcases;
  unsigned int testnum = 1;

  while (std::getline(ifs, line))
  {
    // Skip empty lines and lines starting with a blank.
    //std::cout << "single_testnum: " << single_testnum << ", " << "testnum: " << testnum << std::endl;
    if ((line.empty()) || line[0] == ' ')
      continue;

    // In this case each line in FEN_test_positions.txt contains the FEN-string
    // followed by a list of all legal moves in the position at the end.
    // Extract the actual FEN_string:
    // Match the first 6 tokens in the string.
    std::vector<std::string> matches;
    regexp_grep(line, "^([^\\s]+\\s){5}[^\\s]+", matches);
    std::string FEN_string = matches[0];
    // std::cout << "FEN_string1: " << FEN_string << std::endl;
    Config_params config_params;
    Game game(config_params);
    game.read_position_FEN(FEN_string);
    game.init();
    auto evaluation1 = game.get_chessboard().evaluate_position();
    std::cout << "testpos" << (int)testnum << ".pgn" << std::endl;
    std::cout << evaluation1 << std::endl;
    std::string reversed_FEN_string = reverse_FEN_string(matches[0]);
    game.read_position_FEN(reversed_FEN_string);
    game.init();
    auto evaluation2 = game.get_chessboard().evaluate_position();
    std::cout << "testpos" << (int)testnum << ".pgn reversed" << std::endl;
    std::cout << evaluation2 << std::endl;
    CHECK(is_close(evaluation1, -evaluation2, 1e-5F));

    testnum++;
  }
}

//  Game game(config_params);
//  game.init();
//  Go_params go_params; // All members in go_params are set to zero.
//
//  std::string FEN_string = get_FEN_test_position(95);
//  game.read_position_FEN(FEN_string);
//  game.init();
//
//}
//
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
