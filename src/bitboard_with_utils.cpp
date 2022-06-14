/*
 * Utils.cpp
 *
 *  Created on: 17 juli 2021
 *      Author: torsten
 */

#include <vector>

#include "bitboard_with_utils.hpp"
#include "chessfuncs.hpp"
#include "current_time.hpp"

namespace C2_chess
{

std::ostream& operator<<(std::ostream& os, const BitMove& m)
{
  std::stringstream sstr;
  switch (m.piece_type())
  {
    case piecetype::King:
      {
      if (abs(static_cast<int64_t>(file_idx(m.from()) - file_idx(m.to()))) == 2)
      {
        if (file_idx(m.to()) == g)
          sstr << "0-0";
        else
          sstr << "0-0-0";
        os << sstr.str();
        return os;
      }
      sstr << "K";
      break;
    }
    case piecetype::Queen:
      sstr << "Q";
      break;
    case piecetype::Rook:
      sstr << "R";
      break;
    case piecetype::Bishop:
      sstr << "B";
      break;
    case piecetype::Knight:
      sstr << "N";
      break;
    default:
      break;
  }
  sstr << static_cast<char>('a' + file_idx(m.from())) << static_cast<int>(rank_idx(m.from()));
  if (m.properties() & move_props_capture)
    sstr << "x";
  else
    sstr << "-";
  sstr << static_cast<char>('a' + file_idx(m.to())) << static_cast<int>(rank_idx(m.to()));
  if (m.properties() & move_props_en_passant)
    sstr << " " << "e.p.";
  if (m.properties() & move_props_promotion)
  {
    switch (m.promotion_piece_type())
    {
      case piecetype::Queen:
        sstr << "=Q";
        break;
      case piecetype::Rook:
        sstr << "=R";
        break;
      case piecetype::Bishop:
        sstr << "=B";
        break;
      case piecetype::Knight:
        sstr << "=N";
        break;
      default:
        break;
    }
  }
  if (m.properties() & move_props_check)
    sstr << "+";
  if (m.properties() & move_props_mate)
    sstr << " mate";
  if (m.properties() & move_props_stalemate)
    sstr << " stalemate";
  os << sstr.str();
  return os;
}

std::ostream& Bitboard_with_utils::write_movelist(std::ostream& os, bool same_line) const
{
  if (_movelist.size() > 0)
  {
    bool first = true;
    for (const BitMove& m : _movelist)
    {
      if (same_line)
      {
        if (!first)
          os << " ";
        first = false;
      }
      else
      {
        if (!first)
          os << std::endl;
        first = false;
      }
      os << m;
    }
    os << std::endl;
  }
  return os;
}

std::vector<std::string> Bitboard_with_utils::convert_moves_to_UCI(const std::vector<std::string>& moves, col col_to_move)
{
  std::vector<std::string> stripped_moves;
  for (const std::string& move : moves)
  {
    if (move == "0-0")
    {
      (col_to_move == col::white) ? stripped_moves.push_back("e1g1") : stripped_moves.push_back("e8g8");
    }
    else if (move == "0-0-0")
    {
      (col_to_move == col::white) ? stripped_moves.push_back("e1c1") : stripped_moves.push_back("e8c8");
    }
    else
    {
      std::string stripped_move = "";
      bool promotion = false;
      for (const char& ch : move)
      {
        if (std::isspace(ch))
          break;
        if (std::isdigit(ch) || (std::islower(ch) && ch != 'x'))
        {
          stripped_move.push_back(ch);
        }
        if (promotion)
          stripped_move.push_back(std::tolower(ch));
        if (ch == '=')
          promotion = true;
      }
      stripped_moves.push_back(stripped_move);
    }
  }
  return stripped_moves;
}

void Bitboard_with_utils::make_move(const std::string& UCI_move)
{
  std::stringstream out_moves;
  write_movelist(out_moves);
  std::vector<std::string> out_moves_vector;
  std::string out_move = "";
  while (std::getline(out_moves, out_move))
    out_moves_vector.push_back(out_move);
  out_moves_vector = convert_moves_to_UCI(out_moves_vector, _col_to_move);
  bool found = false;
  int i = 0;
  for (const std::string& s : out_moves_vector)
  {
    if (s == UCI_move)
    {
      found = true;
      break;
    }
    i++;
  }
  if (found == true)
    Bitboard::make_move(i);
  else
    assert(false);
}

int Bitboard_with_utils::add_mg_test_position(const std::string& filename)
{
  std::string line, FEN_string;
  std::ifstream ifs;
  while (!ifs.is_open())
  {
    if (regexp_match(filename, "^testpos[0-9]+.pgn$")) // matches e.g. testpos23.pgn
      ifs.open("tests/test_positions/" + filename);
    else
    {
      ifs.open(user_input("Enter the name of the new test position file: "));
    }
    if (!ifs.is_open())
    {
      std::cerr << "ERROR: Couldn't open file " << "tests/test_positions/" + filename << std::endl;
      return -1;
    }
  }
  while (getline(ifs, line))
  {
    if (regexp_grep(line, "^.FEN "))
      break;
  }
  ifs.close();
  std::cout << "The following FEN-string was found: " << std::endl;
  FEN_string = cut(line, '"', 2);
  std::cout << FEN_string << std::endl;

  read_position(FEN_string);
  init_piece_state();
  write(std::cout, outputtype::cmd_line_diagram, col::white);
  find_legal_moves(gentype::all);
  std::cout << "Possible moves are:" << std::endl;
  write_movelist(std::cout);
  if (question_to_user("Is this correct?\nIf so, would you like to add the position\nas a new test case? [y/n]: ", "^[yY].*$"))
  {
    std::ofstream ofs;
    ofs.open("tests/test_positions/FEN_test_positions.txt", std::ios::app);
    if (!ofs.is_open())
    {
      std::cerr << "ERROR: Couldn't open file FEN_test_positions.txt" << std::endl;
      return -1;
    }
    ofs << FEN_string << " ";
    write_movelist(ofs, true);
    ofs.close();
  }
  return 0;
}

bool Bitboard_with_utils::run_mg_test_case(int testnum,
                                           const std::string& FEN_string,
                                           const std::vector<std::string>& ref_moves_vector,
                                           const std::string& testcase_info,
                                           const gentype gt)
{
  CurrentTime now;
  std::cout << "-- Test " << testnum << " " << testcase_info << " --" << std::endl;

  if (read_position(FEN_string) != 0)
  {
    std::cerr << "Couldn't read FEN string." << std::endl;
    return false;
  }
//chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);

  uint64_t start = now.nanoseconds();

  find_legal_moves(gt);

  uint64_t stop = now.nanoseconds();
  std::cout << "It took " << stop - start << " nanoseconds." << std::endl;

// Compare output and reference moves, first build a vector of "output-moves".
  std::stringstream out_moves;
  write_movelist(out_moves);
  std::vector<std::string> out_moves_vector;
  std::string out_move = "";
  while (std::getline(out_moves, out_move))
    out_moves_vector.push_back(out_move);
  std::vector<std::string> filtered_ref_moves_vector;
  switch (gt)
  {
    case gentype::captures:
      for (const std::string& m : ref_moves_vector)
        if (m.find("x") != std::string::npos || m.find("=") != std::string::npos)
          filtered_ref_moves_vector.push_back(m);
      break;
    case gentype::all:
      filtered_ref_moves_vector = ref_moves_vector;
      break;
    default:
      assert(false); // Unsupported gentype;
  }
  if (!compare_move_lists(out_moves_vector, filtered_ref_moves_vector))
  {
    std::cout << "ERROR: Test " << testnum << " " << testcase_info << " failed!" << std::endl;
    write(std::cout, outputtype::cmd_line_diagram, col::white);
    std::cout << "Comparing program output with test case reference:" << std::endl;
    std::cout << "OUT: " << std::endl;
    write_movelist(std::cout, true);
    std::cout << "REF: " << std::endl;
    bool first = true;
    for (const std::string& m : filtered_ref_moves_vector)
    {
      if (first)
        first = false;
      else
        std::cout << " ";
      std::cout << m;
    }
    std::cout << std::endl;
    return false;
  }
  return true;
}

int Bitboard_with_utils::test_move_generation(unsigned int single_testnum)
{
  std::string line;
  std::string filename = "tests/test_positions/FEN_test_positions.txt";
  std::ifstream ifs(filename);
  if (!ifs.is_open())
  {
    std::cerr << "Couldn't open file " << filename << std::endl;
    return -1;
  }
  std::vector<unsigned int> failed_testcases;
  unsigned int testnum = 1;
  while (std::getline(ifs, line))
  {
    // Skip empty lines and lines starting with a blank.
    //std::cout << "single_testnum: " << single_testnum << ", " << "testnum: " << testnum << std::endl;
    if (single_testnum == 0 || single_testnum == testnum)
    {
      if ((line.empty()) || line[0] == ' ')
        continue;

      // In this case each line in FEN_test_positions.txt contains the FEN-string
      // followed by a list of all legal moves in the position at the end.
      // Extract the actual FEN_string:
      // Match the first 6 tokens in the string.
      std::vector<std::string> matches;
      regexp_grep(line, "^([^\\s]+\\s){5}[^\\s]+", matches);
      std::string FEN_string = matches[0];
      //std::cout << "FEN_string1: " << FEN_string << std::endl;
      // Put all the reference moves in a string-vector
      std::vector<std::string> ref_moves_vector = split(line, ' ');
      // Erase the actual FEN_string tokens from the vector.
      ref_moves_vector.erase(ref_moves_vector.begin(), static_cast<std::vector<std::string>::iterator>(ref_moves_vector.begin() + 6));
      // fix that "e.p." has been treated as a separate token
      // (because it's separated from the move by a blank-character.
      for (unsigned int i = 0; i < ref_moves_vector.size(); i++)
        if (ref_moves_vector[i] == "e.p.")
        {
          ref_moves_vector[i - 1] += " e.p.";
          ref_moves_vector.erase(static_cast<std::vector<std::string>::iterator>(ref_moves_vector.begin() + i));
        }

      // Run the testcase for the position:
      if (!run_mg_test_case(testnum, FEN_string, ref_moves_vector, "for inital color", gentype::all))
        failed_testcases.push_back(testnum);

      // Run the same test-case with reversed colors.
      std::string reversed_FEN_string = reverse_FEN_string(matches[0]);
      //std::cout << "FEN_string2: " << reversed_FEN_string << std::endl;
      std::vector<std::string> reversed_ref_moves_vector = reverse_moves(ref_moves_vector);
      if (!run_mg_test_case(testnum, reversed_FEN_string, reversed_ref_moves_vector, "for other color", gentype::all))
        failed_testcases.push_back(testnum);

      // Run the above two testcases, but generate only captures
      if (!run_mg_test_case(testnum, FEN_string, ref_moves_vector, "inital color only captures", gentype::captures))
        failed_testcases.push_back(testnum);

      if (!run_mg_test_case(testnum, reversed_FEN_string,
                            reversed_ref_moves_vector,
                            "other color only captures",
                            gentype::captures))
        failed_testcases.push_back(testnum);

    }
    testnum++;
  }
  if (failed_testcases.size() > 1)
  {
    std::cout << "FAILURE: The following " << failed_testcases.size() << " test cases failed:";
    for (unsigned int tn : failed_testcases)
      std::cout << " " << tn;
    std::cout << std::endl;
    return -1;
  }
  else if (failed_testcases.size() == 1)
  {
    std::cout << "FAILURE: Test case " << failed_testcases[0] << " failed." << std::endl;
    return -1;
  }
  else
  {
    std::cout << "SUCCES: ";
    if (single_testnum == 0)
      std::cout << "All test cases passed!" << std::endl;
    else
      std::cout << "Test case " << single_testnum << " passed!" << std::endl;
  }
  ifs.close();
  return 0;
}

bool Bitboard_with_utils::bitboard_tests(const std::string& arg)
{
  unsigned int single_testnum = 0;
  if (arg != "" && arg != "all")
  {
    if (regexp_match(arg, "^testpos[0-9]+.pgn$")) // matches e.g. testpos23.pgn
    {
      if (add_mg_test_position(arg) != 0)
        return false;
    }
    else if (regexp_match(arg, "^[0-9]+$")) // matches e.g 23
      single_testnum = std::stoi(arg);
    else
    {
      std::cerr << "ERROR: Unknown input argument: " << arg << std::endl;
      return false;
    }
  }
  return test_move_generation(single_testnum) == 0;
}

uint64_t Bitboard_with_utils::find_legal_squares(uint64_t sq, uint64_t mask, uint64_t all_pieces, uint64_t other_pieces)
{
  _all_pieces = all_pieces;
  _other->pieces = other_pieces;
  return Bitboard::find_legal_squares(sq, mask);
}

void Bitboard_with_utils::init_board_hash_tag()
{
  uint64_t pieces, piece;

  transposition_table.clear();
  _hash_tag = zero;
  pieces = _all_pieces;
  while (pieces)
  {
    piece = popright_square(pieces);
    piecetype p_type = get_piece_type(piece);
    col p_color = (piece & _own->pieces) ? _col_to_move : other_color(_col_to_move);
    update_hash_tag(piece, p_color, p_type);
  }

  if ((_castling_rights & castling_right_WK) == 0)
    _hash_tag ^= transposition_table._castling_rights[castling_right_WK];
  if ((_castling_rights & castling_right_WQ) == 0)
    _hash_tag ^= transposition_table._castling_rights[castling_right_WQ];
  if ((_castling_rights & castling_right_BK) == 0)
    _hash_tag ^= transposition_table._castling_rights[castling_right_BK];
  if ((_castling_rights & castling_right_BQ) == 0)
    _hash_tag ^= transposition_table._castling_rights[castling_right_BQ];

  if (_ep_square)
    _hash_tag ^= transposition_table._en_passant_file[file_idx(_ep_square)];

  if (_col_to_move == col::black)
    _hash_tag ^= transposition_table._black_to_move;
}

float Bitboard_with_utils::evaluate_position(col col_to_move, uint8_t level) const
{
  return Bitboard::evaluate_position(col_to_move, level);
}

int Bitboard_with_utils::figure_out_last_move(const Bitboard& new_position, BitMove& m) const
{
  uint64_t from_square;
  uint64_t to_square;
  uint16_t move_props = move_props_none;
  piecetype p_type;
  piecetype promotion_pt;
  // TODO: 0-0, 0-0-0 and e.p
  uint64_t new_all_pieces = new_position.all_pieces();
  uint64_t piece_diff = _all_pieces ^ new_all_pieces;

  if (new_position.get_half_move_counter() != 0 && new_position.get_half_move_counter() != get_half_move_counter() + 1)
  {
    return -1;
  }

  if (new_position.get_move_number() < get_move_number() ||
      new_position.get_move_number() - get_move_number() > 1)
  {
    return -2;
  }

  if (new_position.get_color_to_move() == get_color_to_move())
  {
    return -3;
  }

  // TODO: Check move_number (if it is moved here from the Game class.

  switch (std::popcount(piece_diff))
  {
    case 1:
      // Must be a capture
      move_props |= move_props_capture;
      from_square = piece_diff;
      p_type = get_piece_type(from_square);
      if ((from_square & _own->Pawns) && ((rank_idx(from_square) == ((_col_to_move == col::black) ? 2 : 7))))
        move_props |= move_props_promotion;
      //to_square = _own->pieces ^ new_position.other()->pieces; // TODO: _other->pieces ^ new_position.own()->pieces ?
      to_square =_other->pieces ^ new_position.own()->pieces;
      break;
    case 2:
      // not a capture
      from_square = _all_pieces & piece_diff;
      to_square = piece_diff ^ from_square;
      p_type = get_piece_type(from_square);
      if ((from_square & _own->Pawns) && ((rank_idx(from_square) == ((_col_to_move == col::black) ? 2 : 7))))
        move_props |= move_props_promotion;
      break;
    case 3:
      // Could be en_passant
      if (_ep_square & piece_diff)
      {
        to_square = _ep_square;
        if (_own->Pawns & piece_diff)
          from_square = _own->Pawns & piece_diff;
        else
          return -4;
        p_type = piecetype::Pawn;
        move_props |= move_props_capture | move_props_en_passant;
      }
      else
        return -5;
      break;
    case 4:
      // Could be 0-0 or 0-0-0
      if (_col_to_move == col::black)
      {
        if (_own->King & e8_square & piece_diff)
        {
          from_square = e8_square;
          if (_own->Rooks & a8_square & piece_diff)
            to_square = c8_square;
          else if (_own->Rooks & h8_square & piece_diff)
            to_square = g8_square;
          else
            return -6;
          p_type = piecetype::King;
          move_props |= move_props_castling;
        }
        else
          return -7;
      }
      else
      {
        if (_own->King & e1_square & piece_diff)
        {
          from_square = e1_square;
          if (_own->Rooks & a1_square & piece_diff)
            to_square = c1_square;
          else if (_own->Rooks & h1_square & piece_diff)
            to_square = g1_square;
          else
            return -8;
          p_type = piecetype::King;
          move_props |= move_props_castling;
        }
        else
          return -9;
      }
      break;
    default:
      return -10;
  }
  promotion_pt = new_position.get_piece_type(to_square);
  if (move_props & move_props_promotion)
  {
    promotion_pt = new_position.get_piece_type(to_square);
    m = BitMove(p_type, move_props, from_square, to_square, promotion_pt);
  }
  else
  {
    m = BitMove(p_type, move_props, from_square, to_square);
  }
  return 0;
}

} // End namespace C2_chess

