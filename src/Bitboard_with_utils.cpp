/*
 * Utils.cpp
 *
 *  Created on: 17 juli 2021
 *      Author: torsten
 */

#include <vector>
#include "Bitboard_with_utils.hpp"
#include "chessfuncs.hpp"
#include "current_time.hpp"

namespace C2_chess
{

// Inits the Bitboard position from a text string (Forsyth-Edwards Notation)
int Bitboard_with_utils::read_position(const std::string& FEN_string)
{
  //std::cout << "\"" << FEN_string << "\"" << std::endl;
  _W_King = 0L;
  _W_Queens = 0L;
  _W_Rooks = 0L;
  _W_Bishops = 0L;
  _W_Knights = 0L;
  _W_Pawns = 0L;
  _B_King = 0L;
  _B_Queens = 0L;
  _B_Rooks = 0L;
  _B_Bishops = 0L;
  _B_Knights = 0L;
  _B_Pawns = 0L;

  std::vector<std::string> FEN_tokens = split(FEN_string, ' ');
  if (FEN_tokens.size() != 6)
  {
    std::cerr << "Read error: Number of elements in FEN string should be 6, but there are " << FEN_tokens.size() << std::endl;
    std::cerr << "FEN-string: " << FEN_string << std::endl;
    return -1;
  }

  int r = 8;
  int f = a;
  //char ch = '0';
  //unsigned int i;
  //for (i = 0; i < FEN_string.size() && ch != ' '; i++)
  for (const char& ch : FEN_tokens[0])
  {
    switch (ch)
    {
      case ' ':
        break;
      case 'K':
        _W_King_file_index = f;
        _W_King_rank_index = r;
        _W_King_file = file[f];
        _W_King_rank = rank[r];
        _W_King = _W_King_file & _W_King_rank;
        _W_King_diagonal = diagonal[8 - r + f];
        _W_King_anti_diagonal = anti_diagonal[f + r - 1];
        break;
      case 'k':
        _B_King_file_index = f;
        _B_King_rank_index = r;
        _B_King_file = file[f];
        _B_King_rank = rank[r];
        _B_King = _B_King_file & _B_King_rank;
        _B_King_diagonal = diagonal[8 - r + f];
        _B_King_anti_diagonal = anti_diagonal[f + r - 1];
        break;
      case 'Q':
        _W_Queens |= (file[f] & rank[r]);
        break;
      case 'q':
        _B_Queens |= (file[f] & rank[r]);
        break;
      case 'B':
        _W_Bishops |= (file[f] & rank[r]);
        break;
      case 'b':
        _B_Bishops |= (file[f] & rank[r]);
        break;
      case 'N':
        _W_Knights |= (file[f] & rank[r]);
        break;
      case 'n':
        _B_Knights |= (file[f] & rank[r]);
        break;
      case 'R':
        _W_Rooks |= (file[f] & rank[r]);
        break;
      case 'r':
        _B_Rooks |= (file[f] & rank[r]);
        break;
      case 'P':
        _W_Pawns |= (file[f] & rank[r]);
        break;
      case 'p':
        _B_Pawns |= (file[f] & rank[r]);
        break;
      case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        f += ch - '1';
        break;
      case '/':
        f = a;
        r--;
        if (r < 1)
        {
          std::cerr << "Read error: corrupt FEN input (rank)" << "\n";
          std::cerr << "FEN-string: " << FEN_string << std::endl;
          return -1;
        }
        continue; // without increasing f
      default:
        std::cerr << "Read error: unexpected character in FEN input" << ch << "\n";
        std::cerr << "FEN-string: " << FEN_string << std::endl;
        return -1;
    } // end of case-statement
    if (f > h)
    {
      std::cerr << "Read error: corrupt FEN input (file)" << "\n";
      std::cerr << "FEN-string: " << FEN_string << std::endl;
      return -1;
    }
    f++;
  } // end of for-loop
  _col_to_move = (FEN_tokens[1] == "w") ? col::white : col::black;
  std::string castling_rights = FEN_tokens[2];
  _castling_rights = castling_rights_none;
  for (const char& cr : castling_rights)
  {
    switch (cr)
    {
      case '-':
        _castling_rights = castling_rights_none;
        break;
      case 'K':
        _castling_rights |= castling_right_WK;
        break;
      case 'Q':
        _castling_rights |= castling_right_WQ;
        break;
      case 'k':
        _castling_rights |= castling_right_BK;
        break;
      case 'q':
        _castling_rights |= castling_right_BQ;
        break;
      default:
        std::cerr << "Read error: Strange castling character \'" << cr << "\' in FEN-string." << std::endl;
        std::cerr << "FEN-string: " << FEN_string << std::endl;
        return -1;
    }
  }
  std::string ep_string = FEN_tokens[3];
  if (!regexp_match(ep_string, "^-|([a-h][36])$")) // Either a "-" or a square on rank 3 or 6-
  {
    std::cerr << "Read error: Strange en_passant_square characters \"" << ep_string << "\" in FEN-string." << std::endl;
    std::cerr << "FEN-string: " << FEN_string << std::endl;
    return -1;
  }
  _ep_square = 0L;
  if (ep_string.length() == 2)
  {
    _ep_square = file[ep_string[0] - 'a'] & rank[ep_string[1] - '0'];
  }
  //std::cout << "ep_square: " << std::endl << to_binary_board(_ep_square) << std::endl;

  return 0;
}

// rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Kkq - 0 1
// rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
inline std::ostream& Bitboard_with_utils::write_piece(std::ostream& os, uint64_t square) const
{
  if (square & _W_King)
    os << "\u2654";
  else if (square & _W_Queens)
    os << "\u2655";
  else if (square & _W_Rooks)
    os << "\u2656";
  else if (square & _W_Bishops)
    os << "\u2657";
  else if (square & _W_Knights)
    os << "\u2658";
  else if (square & _W_Pawns)
    os << "\u2659";
  else if (square & _B_King)
    os << "\u265A";
  else if (square & _B_Queens)
    os << "\u265B";
  else if (square & _B_Rooks)
    os << "\u265C";
  else if (square & _B_Bishops)
    os << "\u265D";
  else if (square & _B_Knights)
    os << "\u265E";
  else if (square & _B_Pawns)
    os << "\u265F";
  return os;
}

std::ostream& Bitboard_with_utils::write(std::ostream& os, outputtype wt, col from_perspective) const
{
  uint64_t _W_pieces = _W_King | _W_Queens | _W_Rooks | _W_Bishops | _W_Knights | _W_Pawns;
  uint64_t _B_pieces = _B_King | _B_Queens | _B_Rooks | _B_Bishops | _B_Knights | _B_Pawns;
  uint64_t all_pieces = _W_pieces | _B_pieces;
  switch (wt)
  {
    case outputtype::cmd_line_diagram:
      if (from_perspective == col::white)
      {
        //os << "\n";
        for (int i = 8; i >= 1; i--)
        {
          for (int j = a; j <= h; j++)
          {
            uint64_t square = file[j] & rank[i];
            os << iso_8859_1_to_utf8("|");
            if (square & all_pieces)
              write_piece(os, square);
            else
              os << ("\u25a1");
          }
          os << iso_8859_1_to_utf8("|") << iso_8859_1_to_utf8(std::to_string(i)) << std::endl;
        }
        os << iso_8859_1_to_utf8(" a b c d e f g h ") << std::endl;
      }
      else // From blacks point of view
      {
        //os << "\n";
        for (int i = 1; i <= 8; i++)
        {
          for (int j = h; j >= a; j--)
          {
            uint64_t square = file[j] & rank[i];
            os << iso_8859_1_to_utf8("|");
            if (square & all_pieces)
              write_piece(os, square);
            else
              os << ("\u25a1");
          }
          os << iso_8859_1_to_utf8("|") << iso_8859_1_to_utf8(std::to_string(i)) << std::endl;
        }
        os << iso_8859_1_to_utf8(" h g f e d c b a ") << std::endl;
      }
      break;
    default:
      os << iso_8859_1_to_utf8("Sorry: Output type not implemented yet.") << std::endl;
  }
  return os;
}

std::ostream& operator <<(std::ostream& os, const BitMove& m)
{
  switch (m.piece_type)
  {
    case piecetype::King:
      {
      if (abs((long int) (m.from_f_index() - m.to_f_index())) == 2)
      {
        if (m.to_f_index() == g)
          os << "0-0";
        else
          os << "0-0-0";
        return os;
      }
      os << "K";
      break;
    }
    case piecetype::Queen:
      os << "Q";
      break;
    case piecetype::Rook:
      os << "R";
      break;
    case piecetype::Bishop:
      os << "B";
      break;
    case piecetype::Knight:
      os << "N";
      break;
    default:
      break;
  }
  Position from(m.from_f_index(), m.from_r_index());
  Position to(m.to_f_index(), m.to_r_index());
  os << from;
  if (m.properties & move_props_capture)
    os << "x";
  else
    os << "-";
  os << to;
  if (m.properties & move_props_en_passant)
    os << " " << "e.p.";
  if (m.properties & move_props_promotion)
  {
    switch (m.promotion_piece_type)
    {
      case piecetype::Queen:
        os << "=Q";
        break;
      case piecetype::Rook:
        os << "=R";
        break;
      case piecetype::Bishop:
        os << "=B";
        break;
      case piecetype::Knight:
        os << "=N";
        break;
      default:
        break;
    }
  }
  if (m.properties & move_props_check)
    os << "+";
  if (m.properties & move_props_mate)
    os << " mate";
  if (m.properties & move_props_stalemate)
    os << " stalemate";
  return os;
}

std::ostream& Bitboard_with_utils::write_movelist(std::ostream& os, bool same_line)
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

void Bitboard_with_utils::add_mg_test_position(const std::string& filename)
{
  std::string line, FEN_string;
  std::ifstream ifs;
  while (!ifs.is_open())
  {
    if (regexp_match(filename, "^testpos[0-9]+.pgn$")) // matches e.g. testpos23.pgn
      ifs.open("test_positions/" + filename);
    else
    {
      ifs.open(user_input("Enter the name of the new test position file: "));
    }
    if (!ifs.is_open())
      std::cerr << "ERROR: Couldn't open file " << filename << std::endl;
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
  Bitboard_with_utils chessboard;
  chessboard.read_position(FEN_string);
  chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
  chessboard.find_all_legal_moves();
  std::cout << "Possible moves are:" << std::endl;
  chessboard.write_movelist(std::cout);
  if (question_to_user("Is this correct?\nIf so, would you like to add the position\nas a new test case? [y/n]: ", "^[yY].*$"))
  {
    std::ofstream ofs;
    ofs.open("test_positions/FEN_test_positions.txt", std::ios::app);
    if (!ofs.is_open())
    {
      std::cerr << "ERROR: Couldn't open file FEN_test_positions.txt" << std::endl;
      return;
    }
    ofs << FEN_string << " ";
    chessboard.write_movelist(ofs, true);
    ofs.close();
  }
}

bool Bitboard_with_utils::run_mg_test_case(int testnum,
                                           const std::string& FEN_string,
                                           const std::vector<std::string>& ref_moves_vector,
                                           const std::string& testcase_info)
{
  CurrentTime now;
  std::cout << "-- Test " << testnum << " " << testcase_info << " --" << std::endl;

  Bitboard_with_utils chessboard;
  if (chessboard.read_position(FEN_string) != 0)
  {
    std::cerr << "Couldn't read FEN string." << std::endl;
    return false;
  }
  //chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);

  uint64_t start = now.nanoseconds();

  chessboard.find_all_legal_moves();

  uint64_t stop = now.nanoseconds();
  std::cout << "It took " << stop - start << " nanoseconds." << std::endl;

  // Compare output and reference moves, first build a vector of "output-moves".
  std::stringstream out_moves;
  chessboard.write_movelist(out_moves);
  std::vector<std::string> out_moves_vector;
  std::string out_move = "";
  while (std::getline(out_moves, out_move))
    out_moves_vector.push_back(out_move);
  if (!compare_move_lists(out_moves_vector, ref_moves_vector))
  {
    std::cout << "ERROR: Test " << testnum << " " << testcase_info << " failed!" << std::endl;
    chessboard.write(std::cout, outputtype::cmd_line_diagram, col::white);
    std::cout << "Comparing program output with test case reference:" << std::endl;
    chessboard.write_movelist(std::cout, true);
    bool first = true;
    for (const std::string& Move : ref_moves_vector)
    {
      if (first)
        first = false;
      else
        std::cout << " ";
      std::cout << Move;
    }
    std::cout << std::endl;
    return false;
  }
  return true;
}

int Bitboard_with_utils::test_move_generation(unsigned int single_testnum)
{
  std::string line;
  std::string filename = "test_positions/FEN_test_positions.txt";
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
      ref_moves_vector.erase(ref_moves_vector.begin(), ref_moves_vector.begin() + 6);
      // fix that "e.p." has been treated as a separate token
      // (because it's separated from the move by a blank-character.
      for (unsigned int i = 0; i < ref_moves_vector.size(); i++)
        if (ref_moves_vector[i] == "e.p.")
        {
          ref_moves_vector[i - 1] += " e.p.";
          ref_moves_vector.erase(ref_moves_vector.begin() + i);
        }

      // Run the testcase for the position:
      if (!run_mg_test_case(testnum, FEN_string, ref_moves_vector, "for inital color"))
        failed_testcases.push_back(testnum);

      // Run the same test-case with reversed colors.
      std::string reversed_FEN_string = reverse_FEN_string(matches[0]);
      //std::cout << "FEN_string2: " << reversed_FEN_string << std::endl;
      std::vector<std::string> reversed_ref_moves_vector = reverse_moves(ref_moves_vector);
      if (!run_mg_test_case(testnum, reversed_FEN_string, reversed_ref_moves_vector, "for other color"))
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
  }
  else if (failed_testcases.size() == 1)
  {
    std::cout << "FAILURE: Test case " << failed_testcases[0] << " failed." << std::endl;
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
  Bitboard_with_utils chessboard;
  unsigned int single_testnum = 0;
  if (!check_execution_dir(preferred_exec_dir))
    return false;
  if (arg != "")
  {
    if (regexp_match(arg, "^testpos[0-9]+.pgn$")) // matches e.g. testpos23.pgn
      chessboard.add_mg_test_position(arg);
    else if (regexp_match(arg, "^[0-9]+$")) // matches e.g 23
      single_testnum = std::stoi(arg);
    else
    {
      std::cerr << "ERROR: Unknown input argument: " << arg << std::endl;
      return false;
    }
  }
  chessboard.test_move_generation(single_testnum);
  return true;
}

} // End namespace C2_chess

