#include <memory>
#include <sstream>
#include <chrono>
#include <atomic>
#include "board.hpp"
#include "square.hpp"
#include "piece.hpp"
#include "current_time.hpp"
#include "chesstypes.hpp"
#include "chessfuncs.hpp"

extern "C" {
#include <string.h>
}


namespace
{
C2_chess::CurrentTime current_time;
uint64_t time_diff_sum = 0;
}

namespace C2_chess
{

using std::unique_ptr;
using std::stringstream;
using namespace std::chrono;

std::atomic<bool> time_left(false);

Board Board::level_boards[38]; // definition, complete type

Board::Board() :
    _last_move(), _possible_moves(), _castling_state(), _en_passant_square(0)
{
  for (int file = a; file <= h; file++)
  {
    _file[a].set_name('a' + (char) file);
    for (int rank = 1; rank <= 8; rank++)
    {
      _file[file][rank] = new Square(file, rank);
    }
  }
  // fix the ranks
  for (int rank = 1; rank <= 8; rank++)
  {
    _rank[rank].set_name('0' + (char) rank);
    for (int file = a; file <= h; file++)
    {
      _rank[rank][file] = _file[file][rank];
    }
  }
}

Board::Board(const Board &from) :
    Board()
{
  *this = from;
}

Board::~Board()
{
  clear(); // Deletes pieces, clears square-lists and deletes moves
  // The squares are "automatically" deleted in File::~File()
}

Board& Board::operator=(const Board &from)
{

  clear();

  _last_move = from._last_move;
  _possible_moves = from._possible_moves;
  _castling_state = from._castling_state;
  // Taking care of the pointer variables.
  // These must obviously point into their own Board-object.
  int tmp_file = from._king_square[static_cast<int>(col::white)]->get_position().get_file();
  int tmp_rank = from._king_square[static_cast<int>(col::white)]->get_position().get_rank();
  _king_square[static_cast<int>(col::white)] = _file[tmp_file][tmp_rank];
  tmp_file = from._king_square[static_cast<int>(col::black)]->get_position().get_file();
  tmp_rank = from._king_square[static_cast<int>(col::black)]->get_position().get_rank();
  _king_square[static_cast<int>(col::black)] = _file[tmp_file][tmp_rank];
  if (from._en_passant_square)
  {
    tmp_file = from._en_passant_square->get_position().get_file();
    tmp_rank = from._en_passant_square->get_position().get_rank();
    _en_passant_square = _file[tmp_file][tmp_rank];
  }
  else
  {
    _en_passant_square = 0;
  }
  for (int file = a; file <= h; file++)
  {
    for (int rank = 1; rank <= 8; rank++)
    {
      Piece *p = from._file[file][rank]->get_piece();
      if (p)
      {
        _file[file][rank]->contain_piece(new Piece(*p));
      }
    }
  }
  return *this;
}

// setup_pieces should be called after default constructor.
// Creates pieces and puts them on their original squares.
void Board::setup_pieces()
{
  _file[a][1]->contain_piece(new Piece(piecetype::Rook, col::white));
  _file[b][1]->contain_piece(new Piece(piecetype::Knight, col::white));
  _file[c][1]->contain_piece(new Piece(piecetype::Bishop, col::white));
  _file[d][1]->contain_piece(new Piece(piecetype::Queen, col::white));
  _file[e][1]->contain_piece(new Piece(piecetype::King, col::white));
  _file[f][1]->contain_piece(new Piece(piecetype::Bishop, col::white));
  _file[g][1]->contain_piece(new Piece(piecetype::Knight, col::white));
  _file[h][1]->contain_piece(new Piece(piecetype::Rook, col::white));
  for (int file = a; file <= h; file++)
    _file[file][2]->contain_piece(new Piece(piecetype::Pawn, col::white));
  for (int file = a; file <= h; file++)
    _file[file][7]->contain_piece(new Piece(piecetype::Pawn, col::black));
  _file[a][8]->contain_piece(new Piece(piecetype::Rook, col::black));
  _file[b][8]->contain_piece(new Piece(piecetype::Knight, col::black));
  _file[c][8]->contain_piece(new Piece(piecetype::Bishop, col::black));
  _file[d][8]->contain_piece(new Piece(piecetype::Queen, col::black));
  _file[e][8]->contain_piece(new Piece(piecetype::King, col::black));
  _file[f][8]->contain_piece(new Piece(piecetype::Bishop, col::black));
  _file[g][8]->contain_piece(new Piece(piecetype::Knight, col::black));
  _file[h][8]->contain_piece(new Piece(piecetype::Rook, col::black));
}

void Board::put_piece(Piece *const p, int file, int rank)
{
  _file[file][rank]->contain_piece(p);
}

Move Board::get_possible_move(int index) const
{
  return *_possible_moves[index];
}

bool Board::read_piece_type(piecetype &pt, char ch) const
{
  // method not important for efficiency
  switch (ch)
  {
    case 'P':
      pt = piecetype::Pawn;
      break;
    case 'K':
      pt = piecetype::King;
      break;
    case 'N':
      pt = piecetype::Knight;
      break;
    case 'B':
      pt = piecetype::Bishop;
      break;
    case 'Q':
      pt = piecetype::Queen;
      break;
    case 'R':
      pt = piecetype::Rook;
      break;
    default:
      return false;
  }
  return true;
}

ostream& Board::write(ostream &os, outputtype wt, col from_perspective) const
{
  switch (wt)
  {
    case outputtype::debug:
      os << "The latest move was: ";
      os << _last_move << endl;
      os << "Castling state is: " << _castling_state << endl;
      os << "En passant square is: " << _en_passant_square << endl;
      for (int fileindex = a; fileindex <= h; fileindex++)
        for (int rankindex = 1; rankindex <= 8; rankindex++)
          _file[fileindex][rankindex]->write_describing(os);
      os << endl << "*** Possible moves ***" << endl;
      for (int i = 0; i < _possible_moves.size(); i++)
        os << *_possible_moves[i] << endl;
      os << endl;
      this->write(os, outputtype::cmd_line_diagram, col::white) << endl;
      break;
    case outputtype::cmd_line_diagram:
      if (from_perspective == col::white)
      {
        os << "###################" << endl;
        for (int i = 8; i >= 1; i--)
        {
          os << "#";
          for (int j = a; j <= h; j++)
          {
            os << "|";
            Piece *p = _file[j][i]->get_piece();
            if (p)
              p->write_diagram_style(os);
            else
              os << "\u2662";
          }
          os << "|#" << " " << i << endl;
        }
        os << "###################" << endl;
        os << "  a b c d e f g h " << endl;
      }
      else // From blacks point of view
      {
        os << "###################" << endl;
        for (int i = 1; i <= 8; i++)
        {
          os << "#";
          for (int j = h; j >= a; j--)
          {
            os << "|";
            Piece *p = _file[j][i]->get_piece();
            if (p)
              p->write_diagram_style(os);
            else
              os << "-";
          }
          os << "|#" << " " << i << endl;
        }
        os << "###################" << endl;
        os << "  h g f e d c b a " << endl;
      }
      break;
    default:
      os << "Sorry: Output type not implemented yet." << endl;
  }
  return os;
}

Shared_ostream& Board::write(Shared_ostream &os, outputtype wt, col from_perspective) const
{
  stringstream ss;
  write(ss, wt, from_perspective);
  os << ss.str();
  return os;
}

ostream& Board::write_possible_moves(ostream &os)
{
  os << _possible_moves << endl;
  return os;
}

void Board::clear(bool remove_pieces)
{
  // Clear square-lists and, depending on the input value, delete pieces.
  // remove-pieces is default true, if left out.
  for (int rankindex = 1; rankindex <= 8; rankindex++)
    for (int fileindex = a; fileindex <= h; fileindex++)
      _file[fileindex][rankindex]->clear(remove_pieces);
  // Delete moves and clear move-list.
  _possible_moves.clear();
}

void Board::init_castling(col this_col)
{
  col other_col = this_col == col::white ? col::black : col::white;
  int one_or_eight = (this_col == col::white ? 1 : 8);

  if (_king_square[static_cast<int>(this_col)]->count_threats(other_col) == 0)
  {
    // The king is not in check
    if (_castling_state.is_kingside_castling_OK(this_col))
    {
      // Double-check that the king and rook are in the correct squares.
      // We can't always trust the castling status, e.g. if read from pgn-file.
      if (_king_square[static_cast<int>(this_col)] == _file[e][one_or_eight] && _file[h][one_or_eight]->contains(this_col, piecetype::Rook))
      {
        // Castling short? Check that the squares between the
        // King and the rook are free and that the squares
        // which the King must pass are not threatened
        if (!(_file[f][one_or_eight]->get_piece() || _file[g][one_or_eight]->get_piece()))
        {
          if (!(_file[f][one_or_eight]->count_threats(other_col) || _file[g][one_or_eight]->count_threats(other_col)))
            _king_square[static_cast<int>(this_col)]->into_move(_file[g][one_or_eight]);
        }
      }
    }
    if (_castling_state.is_queenside_castling_OK(this_col))
    {
      if (_king_square[static_cast<int>(this_col)] == _file[e][one_or_eight] && _file[a][one_or_eight]->contains(this_col, piecetype::Rook))
      {
        // Castling long? Check that the squares between the
        // King and the rook are free and that the squares
        // which the King must pass are not threatened
        if (!(_file[b][one_or_eight]->get_piece() || _file[c][one_or_eight]->get_piece() || _file[d][one_or_eight]->get_piece()))
        {
          if (!(_file[c][one_or_eight]->count_threats(other_col) || _file[d][one_or_eight]->count_threats(other_col)))
            _king_square[static_cast<int>(this_col)]->into_move(_file[c][one_or_eight]);
        }
      }
    }
  }
}

void Board::init_rook_or_queen(int file, int rank, Square *s, Piece *p)
{
  int tf = file;
  int tr = rank;
  while (allowed(++tf, tr))
  {
    fix_threat_prot(tf, tr, p, s);
    if (_file[tf][tr]->get_piece())
      break;
  }
  tf = file;
  tr = rank;
  while (allowed(--tf, tr))
  {
    fix_threat_prot(tf, tr, p, s);
    if (_file[tf][tr]->get_piece())
      break;
  }
  tf = file;
  tr = rank;
  while (allowed(tf, ++tr))
  {
    fix_threat_prot(tf, tr, p, s);
    if (_file[tf][tr]->get_piece())
      break;
  }
  tf = file;
  tr = rank;
  while (allowed(tf, --tr))
  {
    fix_threat_prot(tf, tr, p, s);
    if (_file[tf][tr]->get_piece())
      break;
  }
}

void Board::init_bishop_or_queen(int file, int rank, Square *s, Piece *p)
{
  int tf = file;
  int tr = rank;
  while (allowed(++tf, ++tr))
  {
    fix_threat_prot(tf, tr, p, s);
    if (_file[tf][tr]->get_piece())
      break;
  }
  tf = file;
  tr = rank;
  while (allowed(--tf, --tr))
  {
    fix_threat_prot(tf, tr, p, s);
    if (_file[tf][tr]->get_piece())
      break;
  }
  tf = file;
  tr = rank;
  while (allowed(--tf, ++tr))
  {
    fix_threat_prot(tf, tr, p, s);
    if (_file[tf][tr]->get_piece())
      break;
  }
  tf = file;
  tr = rank;
  while (allowed(++tf, --tr))
  {
    fix_threat_prot(tf, tr, p, s);
    if (_file[tf][tr]->get_piece())
      break;
  }
}

int Board::init(col col_to_move)
{
  // Attention! no clear is made here.
  //  uint64_t nsec_start = current_time.nanoseconds();
  bool pieces_found = false;
  col other_col = col_to_move == col::white ? col::black : col::white;
  int tf;
  int tr;
  bool stop;
  for (int rank = 1; rank <= 8; rank++)
  {
    for (int file = a; file <= h; file++)
    {
      Piece *p = _file[file][rank]->get_piece();
      if (p)
      {
        pieces_found = true;
        Square *s = _file[file][rank];
        piecetype pt = p->get_type();
        switch (pt)
        {
          case piecetype::King:
          {
            _king_square[static_cast<int>(p->get_color())] = s;
            if (allowed(file + 1, rank))
              fix_threat_prot(file + 1, rank, p, s);
            if (allowed(file + 1, rank + 1))
              fix_threat_prot(file + 1, rank + 1, p, s);
            if (allowed(file, rank + 1))
              fix_threat_prot(file, rank + 1, p, s);
            if (allowed(file - 1, rank + 1))
              fix_threat_prot(file - 1, rank + 1, p, s);
            if (allowed(file - 1, rank))
              fix_threat_prot(file - 1, rank, p, s);
            if (allowed(file - 1, rank - 1))
              fix_threat_prot(file - 1, rank - 1, p, s);
            if (allowed(file, rank - 1))
              fix_threat_prot(file, rank - 1, p, s);
            if (allowed(file + 1, rank - 1))
              fix_threat_prot(file + 1, rank - 1, p, s);
            break;
          }
          case piecetype::Queen:
            init_bishop_or_queen(file, rank, s, p);
            init_rook_or_queen(file, rank, s, p);
            break;

          case piecetype::Rook:
            init_rook_or_queen(file, rank, s, p);
            break;

          case piecetype::Bishop:
            init_bishop_or_queen(file, rank, s, p);
            break;

          case piecetype::Knight:
            if (allowed(file + 2, rank + 1))
              fix_threat_prot(file + 2, rank + 1, p, s);
            if (allowed(file + 2, rank - 1))
              fix_threat_prot(file + 2, rank - 1, p, s);
            if (allowed(file + 1, rank - 2))
              fix_threat_prot(file + 1, rank - 2, p, s);
            if (allowed(file - 1, rank - 2))
              fix_threat_prot(file - 1, rank - 2, p, s);
            if (allowed(file - 2, rank - 1))
              fix_threat_prot(file - 2, rank - 1, p, s);
            if (allowed(file - 2, rank + 1))
              fix_threat_prot(file - 2, rank + 1, p, s);
            if (allowed(file - 1, rank + 2))
              fix_threat_prot(file - 1, rank + 2, p, s);
            if (allowed(file + 1, rank + 2))
              fix_threat_prot(file + 1, rank + 2, p, s);
            break;

          case piecetype::Pawn:
            stop = false;
            if (p->get_color() == col::white)
            {
              if (allowed(file, rank + 1))
              {
                if (!_file[file][rank + 1]->get_piece())
                  s->into_move(_file[file][rank + 1]);
                else
                  stop = true;
                if ((rank == 2) && !stop)
                  if (!_file[file][rank + 2]->get_piece())
                    s->into_move(_file[file][rank + 2]);
              }
              if (allowed(file - 1, rank + 1))
              {
                fix_pawn_tp(file - 1, rank + 1, p, s);
                fix_en_passant(s, _file[file - 1][rank + 1]);
              }
              if (allowed(file + 1, rank + 1))
              {
                fix_pawn_tp(file + 1, rank + 1, p, s);
                fix_en_passant(s, _file[file + 1][rank + 1]);
              }
            }
            else // p->get_color() == col::black
            {
              if (allowed(file, rank - 1))
              {
                if (!(_file[file][rank - 1]->get_piece()))
                  s->into_move(_file[file][rank - 1]);
                else
                  stop = true;
                if ((rank == 7) && !stop)
                  if (!_file[file][rank - 2]->get_piece())
                    s->into_move(_file[file][rank - 2]);
              }
              if (allowed(file - 1, rank - 1))
              {
                fix_pawn_tp(file - 1, rank - 1, p, s);
                fix_en_passant(s, _file[file - 1][rank - 1]);
              }
              if (allowed(file + 1, rank - 1))
              {
                fix_pawn_tp(file + 1, rank - 1, p, s);
                fix_en_passant(s, _file[file + 1][rank - 1]);
              }
            }
            break;
          default:
            cout << "Error: Undefined piecetype in Board::init()" << endl;
            return -1;
        } //end switch
      } // end if (p)
    } // end for rank index
  } // end for file index
  if (!pieces_found)
  {
    cout << "Error: No pieces on the board!" << endl;
    return -1;
  }
  //  uint64_t nsec_stop = current_time.nanoseconds();
  //  int timediff = nsec_stop - nsec_start;
  //  cout << "init_nsecs = " << timediff << endl;
  Square *king_square = _king_square[static_cast<int>(col_to_move)];
  int king_file_index = king_square->get_fileindex();
  int king_rank_index = king_square->get_rankindex();
  tf = king_file_index;
  tr = king_rank_index;
  Square *own_piece_square = 0;
  while (allowed(++tf, ++tr))
  {
    Piece *p = _file[tf][tr]->get_piece();
    if (p)
    {
      if (p->is(other_col))
      {
        if (p->is(piecetype::Bishop) || p->is(piecetype::Queen))
        {
          if (own_piece_square)
            fix_bound_piece_diagonal(king_square, own_piece_square, _file[tf][tr]);
          break; // Even if it's not a Bishop or a queen
        }
      }
      else //p is of col_to_move
      {
        if (own_piece_square) // Two own pieces between King and Threat
          break;
        own_piece_square = _file[tf][tr]; // Remember own piece square and continue
      }
    }
  }
  // Restart from the kings position again and move out in another direction
  tf = king_file_index;
  tr = king_rank_index;
  own_piece_square = 0;
  while (allowed(--tf, --tr))
  {
    Piece *p = _file[tf][tr]->get_piece();
    if (p)
    {
      if (p->is(other_col))
      {
        if (p->is(piecetype::Bishop) || p->is(piecetype::Queen))
        {
          if (own_piece_square)
            fix_bound_piece_diagonal(king_square, own_piece_square, _file[tf][tr]);
          break; // Even if it's not a Bishop or a Queen
        }
      }
      else //p is of col_to_move
      {
        if (own_piece_square) // Two own pieces between King and Threat
          break;
        own_piece_square = _file[tf][tr]; // Remember own piece square and continue
      }
    }
  }

  // Start from the kings position again and move out in another direction
  tf = king_file_index;
  tr = king_rank_index;
  own_piece_square = 0;
  while (allowed(++tf, --tr))
  {
    Piece *p = _file[tf][tr]->get_piece();
    if (p)
    {
      if (p->is(other_col))
      {
        if (p->is(piecetype::Bishop) || p->is(piecetype::Queen))
        {
          if (own_piece_square)
            fix_bound_piece_diagonal(king_square, own_piece_square, _file[tf][tr]);
          break; // Even if it's not a Bishop or a Queen
        }
      }
      else //p is of col_to_move
      {
        if (own_piece_square) // Two own pieces between King and Threat
          break;
        own_piece_square = _file[tf][tr]; // Remember own piece square and continue
      }
    }
  }
  // Start from the kings position again and move out in another direction
  tf = king_file_index;
  tr = king_rank_index;
  own_piece_square = 0;
  while (allowed(--tf, ++tr))
  {
    Piece *p = _file[tf][tr]->get_piece();
    if (p)
    {
      if (p->is(other_col))
      {
        if (p->is(piecetype::Bishop) || p->is(piecetype::Queen))
        {
          if (own_piece_square)
            fix_bound_piece_diagonal(king_square, own_piece_square, _file[tf][tr]);
          break; // Even if it's not a Bishop or a Queen
        }
      }
      else //p is of col_to_move
      {
        if (own_piece_square) // Two own pieces between King and Threat
          break;
        own_piece_square = _file[tf][tr]; // Remember own piece square and continue
      }
    }
  }
  tf = king_file_index;
  tr = king_rank_index;
  own_piece_square = 0;
  while (allowed(--tf, tr))
  {
    Piece *p = _file[tf][tr]->get_piece();
    if (p)
    {
      if (p->get_color() == other_col)
      {
        if (p->is(piecetype::Rook) || p->is(piecetype::Queen))
          if (own_piece_square)
            fix_bound_piece_rank(own_piece_square, _file[tf][tr]);
        break; // Even if it's not a Queen or Rook
      }
      else // p is of col_to_move
      {
        if (own_piece_square) // Two own pieces between King and Threat
          break;
        own_piece_square = _file[tf][tr]; // Remember own piece square and continue
      }
    }
  }
  tf = king_file_index;
  tr = king_rank_index;
  own_piece_square = 0;
  while (allowed(++tf, tr))
  {
    Piece *p = _file[tf][tr]->get_piece();
    if (p)
    {
      if (p->is(other_col))
      {
        if (p->is(piecetype::Rook) || p->is(piecetype::Queen))
          if (own_piece_square)
            fix_bound_piece_rank(own_piece_square, _file[tf][tr]);
        break; // Even if it's not a Queen or Rook
      }
      else // p is of col_to_move
      {
        if (own_piece_square) // Two own pieces between King and Threat
          break;
        own_piece_square = _file[tf][tr]; // Remember own piece square and continue
      }
    }
  }
  tf = king_file_index;
  tr = king_rank_index;
  own_piece_square = 0;
  while (allowed(tf, ++tr))
  {
    Piece *p = _file[tf][tr]->get_piece();
    if (p)
    {
      if (p->is(other_col))
      {
        if (p->is(piecetype::Rook) || p->is(piecetype::Queen))
          if (own_piece_square)
            fix_bound_piece_file(own_piece_square, _file[tf][tr]);
        break; // Even if it's not a Queen or Rook
      }
      else // p is of col_to_move
      {
        if (own_piece_square) // Two own pieces between King and Threat
          break;
        own_piece_square = _file[tf][tr]; // Remember own piece square and continue
      }
    }
  }
  tf = king_file_index;
  tr = king_rank_index;
  own_piece_square = 0;
  while (allowed(tf, --tr))
  {
    Piece *p = _file[tf][tr]->get_piece();
    if (p)
    {
      if (p->is(other_col))
      {
        if (p->is(piecetype::Rook) || p->is(piecetype::Queen))
          if (own_piece_square)
            fix_bound_piece_file(own_piece_square, _file[tf][tr]);
        break; // Even if it's not a Queen or Rook
      }
      else // p is of col_to_move
      {
        if (own_piece_square) // Two own pieces between King and Threat
          break;
        own_piece_square = _file[tf][tr]; // Remember own piece square and continue
      }
    }
  }

  // Fix the Kings forbidden moves
  // Moves to an empty square that's threatened by any of the opponents
  // pieces are not allowed, and it's not allowed for the king to take
  // a piece if it's protected by another one of the opponents pieces.
  // Such moves will be removed from the move-list of the king_square
  Square *temp_square;
  for (temp_square = _king_square[static_cast<int>(col_to_move)]->first_move(); temp_square != 0; temp_square = _king_square[static_cast<int>(col_to_move)]->next_move())
  {
    if (temp_square->get_piece())
    {
      if (temp_square->get_piece()->get_color() == other_col)
        if (temp_square->count_protections())
        {
          _king_square[static_cast<int>(col_to_move)]->out_move(temp_square);
        }
    }
    else if (temp_square->count_threats(other_col))
    {
      _king_square[static_cast<int>(col_to_move)]->out_move(temp_square);
    }
  }

  // fix the square(s) to which the King "can move", but will be threatened if
  // the King moves. (X-ray threat through the King)
  temp_square = _king_square[static_cast<int>(col_to_move)]->first_threat();
  while (temp_square)
  {
    int kf = _king_square[static_cast<int>(col_to_move)]->get_fileindex();
    int kr = _king_square[static_cast<int>(col_to_move)]->get_rankindex();
    int to_f = temp_square->get_fileindex();
    int to_r = temp_square->get_rankindex();

    if (temp_square->same_rank(_king_square[static_cast<int>(col_to_move)]))
    {
      if (to_f == min(to_f, kf))
        if (allowed(kf + 1, kr))
        {
          _king_square[static_cast<int>(col_to_move)]->out_move(_file[kf + 1][kr]);
        }
      if (to_f == max(to_f, kf))
        if (allowed(kf - 1, kr))
        {
          _king_square[static_cast<int>(col_to_move)]->out_move(_file[kf - 1][kr]);
        }
    }
    else if (temp_square->same_file(_king_square[static_cast<int>(col_to_move)]))
    {
      if (to_r == min(to_r, kr))
      {
        if (allowed(kf, kr + 1))
        {
          _king_square[static_cast<int>(col_to_move)]->out_move(_file[kf][kr + 1]);
        }
      }
      if (to_r == max(to_r, kr))
        if (allowed(kf, kr - 1))
        {
          _king_square[static_cast<int>(col_to_move)]->out_move(_file[kf][kr - 1]);
        }
    }
    else if (temp_square->same_diagonal(_king_square[static_cast<int>(col_to_move)]))
    {
      if (to_r == min(to_r, kr) && to_f == min(to_f, kf))
        if (allowed(kf + 1, kr + 1))
        {
          _king_square[static_cast<int>(col_to_move)]->out_move(_file[kf + 1][kr + 1]);
        }
      if (to_r == min(to_r, kr) && to_f == max(to_f, kf))
        if (allowed(kf - 1, kr + 1))
        {
          _king_square[static_cast<int>(col_to_move)]->out_move(_file[kf - 1][kr + 1]);
        }
      if (to_r == max(to_r, kr) && to_f == min(to_f, kf))
        if (allowed(kf + 1, kr - 1))
        {
          _king_square[static_cast<int>(col_to_move)]->out_move(_file[kf + 1][kr - 1]);
        }
      if (to_r == max(to_r, kr) && to_f == max(to_f, kf))
        if (allowed(kf - 1, kr - 1))
        {
          _king_square[static_cast<int>(col_to_move)]->out_move(_file[kf - 1][kr - 1]);
        }
    }
    temp_square = _king_square[static_cast<int>(col_to_move)]->next_threat();
  }

  // What about Castling ?
  init_castling(col_to_move);
  // The following may not be necessary, because it probably affects nothing.
  // It's col_to_moves time to calculate possible moves.
  //init_castling(other_col);
  // cout << "End init" << endl;
  return 0;
}

void Board::fix_threat_prot(int file, int rank, Piece *p, Square *s)
{
  Square *temp_square = _file[file][rank];
  if (temp_square->get_piece())
  {
    if (temp_square->get_piece()->get_color() == p->get_color())
      temp_square->into_protection(s);
    else
    {
      temp_square->into_threat(s);
      s->into_move(temp_square);
    }
  }
  else
  {
    temp_square->into_threat(s);
    s->into_move(temp_square);
  }
}

void Board::fix_bound_piece_file(Square *own_piece_square, const Square *threat_square)
{
  // Requirements:
  // King_square, own_piece_square and threat_square is on the
  // same file. The "own_King" is placed on King_square
  // On own_piece_square there is a piece of the same colour as the king
  // On threat_square there is an "enemy-Rook" or "enemy-Queen".
  // own_piece_square is located between king_square and threat_square.

  // Get the type of Own_piece
  piecetype pt = own_piece_square->get_piece()->get_type();

  // Go through all moves that own piece can make and
  // check if they are allowed (=along the same file).
  // Remove them from the movelist of own_piece_square.
  // For Bishops and Nights no moves are allowed, because
  // all moves make the piece step out of the file.
  // For Rooks and Queens there are some moves which leaves
  // the piece on the original file.
  // For pawns all moves except "takes" will be allowed.
  for (Square *temp_square = own_piece_square->first_move(); temp_square != 0; temp_square = own_piece_square->next_move())
  {
    switch (pt)
    {
      case piecetype::Bishop:
      case piecetype::Knight:
        own_piece_square->clear_moves();
        break;
      case piecetype::Pawn:
      case piecetype::Rook:
      case piecetype::Queen:
        if (!threat_square->get_position().same_file(*temp_square))
        {
          own_piece_square->out_move(temp_square);
        }
        break;
      default:
        cerr << "strange kind of own piece in fix_bound_piece_file " << static_cast<int>(pt) << endl;
    }
  }
}

void Board::fix_bound_piece_rank(Square *own_piece_square, const Square *threat_square)
{
  // Requirements:
  // King_square, own_piece_square and threat_square is on the
  // same rank. The "own_King" is placed on King_square.
  // On own_piece_square there is a piece of the same colour as the king
  // On threat_square there is an "enemy-Rook" or "enemy-Queen".
  // own_piece_square is located between king_square and threat_square.

  // Get the type of Own_piece
  piecetype pt = own_piece_square->get_piece()->get_type();

  // Go through all moves that own piece can make and
  // check if they are allowed (=along the same rank).
  // Remove the forbidden moves from the movelist of own_piece_square.
  // For Pawns, Bishops and Nights no moves are allowed, because
  // all moves make the piece step out of the rank.
  // For Rooks and Queens there are some moves which leaves
  // the piece on the original rank.
  for (Square *temp_square = own_piece_square->first_move(); temp_square != 0; temp_square = own_piece_square->next_move())
  {
    switch (pt)
    {
      case piecetype::Pawn:
      case piecetype::Bishop:
      case piecetype::Knight:
        own_piece_square->clear_moves();
        break;
      case piecetype::Rook:
      case piecetype::Queen:
        if (!threat_square->get_position().same_rank(*temp_square))
        {
          own_piece_square->out_move(temp_square);
        }
        break;
      default:
        cerr << "strange kind of own piece in fix_bound_piece_rank" << endl;
    }
  }
}

void Board::fix_bound_piece_diagonal(const Square *king_square, Square *own_piece_square, const Square *threat_square)
{
  // Requirements:
  // King_square, own_piece_square and threat_square is on the
  // same diagonal. The "own_King" is placed on King_square
  // On own_piece_square there is a piece of the same colour as the king
  // On threat_square there is an "enemy-Bishop" or "enemy-Queen".
  // own_piece_square is located between king_square and threat_square.

  // Get the type of Own_piece
  piecetype pt = own_piece_square->get_piece()->get_type();

  // Go through all moves that own piece can make and
  // check if they are allowed (=along the same diagonal).
  // Remove forbidden moves from own_piece_squares movelist.
  // For Rooks and Nights no moves are allowed, because
  // all moves make the piece step out of the diagonal.
  // For Bishops and Queen there are some moves which leaves
  // the piece on the original diagonal.      case Knight:

  // For pawns there might be the possibility to take the
  // threatening piece and stay on the same diagonal
  for (Square *temp_square = own_piece_square->first_move(); temp_square != 0; temp_square = own_piece_square->next_move())
  {
    switch (pt)
    {
      case piecetype::Queen:
      case piecetype::Pawn:
        // Here we must check both squares.
        // If the queen moves as a rook it can end up on the other
        // diagonal of either the King_square or threat_square, but
        // never both. The same goes for a pawn.
        if (!threat_square->get_position().same_diagonal(*temp_square) || !king_square->get_position().same_diagonal(*temp_square))
        {
          own_piece_square->out_move(temp_square);
        }
        break;
      case piecetype::Bishop:
        // The bishop can never reach the other diagonal of threat_square
        // or king_square, so it's enough to check against threat_square here.
        if (!threat_square->get_position().same_diagonal(*temp_square))
        {
          own_piece_square->out_move(temp_square);
        }
        break;
      case piecetype::Rook:
      case piecetype::Knight:
        own_piece_square->clear_moves();
        break;
      default:
        cerr << "strange kind of own piece in fix_bound_piece_diagonal" << endl;
    }
  }
}

void Board::fix_pawn_tp(int file, int rank, Piece *p, Square *s)
{
  Square *temp_square = _file[file][rank];
  if (temp_square->get_piece())
  {
    if (temp_square->get_piece()->is(p->get_color()))
      temp_square->into_protection(s);
    else
    {
      temp_square->into_threat(s);
      s->into_move(temp_square);
    }
  }
  else
  {
    temp_square->into_threat(s);
  }
}

void Board::fix_en_passant(Square *s, Square *possible_ep_square)
{
  if (_en_passant_square == possible_ep_square)
  {
    // Avoid e.g Pg2 going to g3 if g3 is possible_ep_square
    if (s->get_position().get_rank() == 4 || s->get_position().get_rank() == 5)
    {
      s->into_move(_en_passant_square);
    }
  }
}

bool Board::allowed(int fileindex, int rankindex)
{
  bool temp = true;
  if (fileindex < a)
    temp = false;
  if (fileindex > h)
    temp = false;
  if (rankindex < 1)
    temp = false;
  if (rankindex > 8)
    temp = false;
  return temp;
}

int Board::min(int i, int j)
{
  if (i < j)
    return i;
  else
    return j;
}

int Board::max(int i, int j)
{
  if (i > j)
    return i;
  else
    return j;
}

void Board::check_put_a_pawn_on_square(int file, int rank, col col_to_move)
{
  Square *from_square;
  Piece *piece;
  switch (col_to_move)
  {
    case col::white:
      // Check for one-square-pawn-moves
      if (rank > 2) // otherwise white can never move a pawn there.
      {
        from_square = _file[file][rank - 1];
        piece = from_square->get_piece();
        // Is there a piece and is it a white pawn?
        if (piece && piece->is(col_to_move, piecetype::Pawn))
        {
          // Check that the pawn isn't pinned
          if (from_square->in_movelist(_file[file][rank]))
          {
            unique_ptr<Move> m(new Move(from_square, _file[file][rank]));
            _possible_moves.into(m.get());
            return;
          }
        }
      }
      // White can only make two-squares-pawn-moves to rank 4
      // and if the square in between is free.
      if (rank == 4)
      {
        from_square = _file[file][rank - 2];
        piece = from_square->get_piece();
        // Is there a piece and is it a white pawn?
        if (piece && piece->is(col_to_move, piecetype::Pawn))
        {
          if (from_square->in_movelist(_file[file][rank]))
          {
            unique_ptr<Move> m(new Move(from_square, _file[file][rank]));
            _possible_moves.into(m.get());
            return;
          }
        }
      }
      break;
    case col::black:
      // Check for one-square-pawn-moves
      if (rank < 7) // otherwise black can never move a pawn there.
      {
        from_square = _file[file][rank + 1];
        piece = from_square->get_piece();
        // Is there a piece and is it a black pawn?
        if (piece && piece->is(col_to_move, piecetype::Pawn))
        {
          // Check that the pawn isn't pinned
          if (from_square->in_movelist(_file[file][rank]))
          {
            unique_ptr<Move> m(new Move(from_square, _file[file][rank]));
            _possible_moves.into(m.get());
            return;
          }
        }
      }
      if (rank == 5)
      {
        // It could be that a black pawn can move two steps to this square.
        from_square = _file[file][rank + 2];
        piece = from_square->get_piece();
        // Is there a piece and is it a black pawn?
        if (piece && piece->is(col_to_move, piecetype::Pawn))
        {
          // Check that the pawn isn't pinned
          if (from_square->in_movelist(_file[file][rank]))
          {
            unique_ptr<Move> m(new Move(from_square, _file[file][rank]));
            _possible_moves.into(m.get());
            return;
          }
        }
      }
      break;
  }
}

void Board::calculate_moves_K_not_threatened(col col_to_move)
{
  // The king is not threatened. All remaining moves are allowed.
  Square *s;
  unique_ptr<Move> m;
  for (int i = a; i <= h; i++)
  {
    for (int j = 1; j <= 8; j++)
    {
      if (_file[i][j]->get_piece())
      {
        if (_file[i][j]->get_piece()->is(col_to_move))
        {
          s = _file[i][j]->first_move();
          while (s)
          {
            m.reset(new Move(_file[i][j], s));
            _possible_moves.into(m.get());
            s = _file[i][j]->next_move();
          }
        }
      }
    }
  }
}

void Board::check_put_a_piece_on_square(int i, int j, col col_to_move)
{
  unique_ptr<Move> m;
  Square *temp_square = _file[i][j]->first_threat();
  while (temp_square)
  {
    // Check that the piece isn't pinned
    if (temp_square->in_movelist(_file[i][j]))
    {
      if (temp_square->get_piece()->is(col_to_move))
      {
        if (!temp_square->get_piece()->is(piecetype::King))
        {
          m.reset(new Move(temp_square, _file[i][j]));
          _possible_moves.into(m.get());
        }
      }
    }
    temp_square = _file[i][j]->next_threat();
  }
}

void Board::check_bishop_or_queen(Square *threat_square, Square *kings_square, col col_to_move)
{
  if (threat_square->same_diagonal(kings_square))
  {
    //cout << "same_diagonal" << endl;
    int tf = threat_square->get_fileindex();
    int tr = threat_square->get_rankindex();
    int kf = kings_square->get_fileindex();
    int kr = kings_square->get_rankindex();
    //cout << "tf=" << tf << "tr=" << tr << "kf=" << kf << "kr=" << kr << endl;
    if (tf == min(tf, kf) && tr == min(tr, kr))
    {
      for (int i = tf + 1, j = tr + 1; i < kf; i++, j++)
      {
        check_put_a_piece_on_square(i, j, col_to_move);
        check_put_a_pawn_on_square(i, j, col_to_move);
      }
    }
    else if (tf == min(tf, kf) && tr == max(tr, kr))
    {
      for (int i = tf + 1, j = tr - 1; i < kf; i++, j--)
      {
        check_put_a_piece_on_square(i, j, col_to_move);
        check_put_a_pawn_on_square(i, j, col_to_move);
      }
    }
    else if (tf == max(tf, kf) && tr == min(tr, kr))
    {
      for (int i = tf - 1, j = tr + 1; i > kf; i--, j++)
      {
        check_put_a_piece_on_square(i, j, col_to_move);
        check_put_a_pawn_on_square(i, j, col_to_move);
      }
    }
    else if (tf == max(tf, kf) && tr == max(tr, kr))
    {
      for (int i = tf - 1, j = tr - 1; i > kf; i--, j--)
      {
        check_put_a_piece_on_square(i, j, col_to_move);
        check_put_a_pawn_on_square(i, j, col_to_move);
      }
    }
  }
}

void Board::check_rook_or_queen(Square *threat_square, Square *kings_square, col col_to_move)
{
  if (threat_square->same_rank(kings_square))
  {
    int rank = kings_square->get_rankindex();
    int from = min(kings_square->get_fileindex(), threat_square->get_fileindex());
    int to = max(kings_square->get_fileindex(), threat_square->get_fileindex());
    for (int i = from + 1; i < to; i++)
    {
      check_put_a_piece_on_square(i, rank, col_to_move);
      check_put_a_pawn_on_square(i, rank, col_to_move);
    }
  }
  else if (threat_square->same_file(kings_square))
  {
    int file = kings_square->get_fileindex();
    int from = min(kings_square->get_rankindex(), threat_square->get_rankindex());
    int to = max(kings_square->get_rankindex(), threat_square->get_rankindex());
    for (int i = from + 1; i < to; i++)
    {
      check_put_a_piece_on_square(file, i, col_to_move);
      // No need to check for pawns, not even en passants,
      // because the king is on the same file as the threatening piece.
    }
  }
}

void Board::check_if_threat_can_be_taken_en_passant(col col_to_move, Square *threat_square)
{
  unique_ptr<Move> m;
  int rank_increment = (col_to_move == col::white) ? 1 : -1;
  int ep_file = _en_passant_square->get_fileindex();
  int ep_rank = _en_passant_square->get_rankindex();
  int thr_file = threat_square->get_fileindex();
  int thr_rank = threat_square->get_rankindex();
  if (ep_rank == thr_rank + 1)
  {
    if (ep_file == thr_file && ep_rank == thr_rank + rank_increment)
    {
      int file = thr_file - 1;
      if (allowed(file, thr_rank))
      {
        Square *s = _file[file][thr_rank];
        if (s->contains(col_to_move, piecetype::Pawn))
        {
          // Check that the pawn isn't pinned
          if (s->in_movelist(_en_passant_square))
          {
            unique_ptr<Move> move(new Move(_file[file][thr_rank], _en_passant_square));
            _possible_moves.into_as_first(move.get());
          }
        }
      }
      file = thr_file + 1;
      if (allowed(file, thr_rank))
      {
        Square *s = _file[file][thr_rank];
        if (s->contains(col_to_move, piecetype::Pawn))
        {
          if (s->in_movelist(_en_passant_square))
          {
            m.reset(new Move(_file[file][thr_rank], _en_passant_square));
            _possible_moves.into_as_first(m.get());
          }
        }
      }
    }
  }
}

void Board::calculate_moves(col col_to_move)
{
//  uint64_t nsec_start = current_time.nanoseconds();
  col other_col = col_to_move == col::white ? col::black : col::white;
  _possible_moves.clear();

  Square *temp_square;
  Square *kings_square = _king_square[static_cast<int>(col_to_move)];
  // is the King threatened?
  if (kings_square->count_threats(other_col)) //TODO: speed up
  {
    //cout << "king threatened" << endl;
    // All King moves to unthreatened squares are allowed. (Except moves to squares
    // which will be threatened if the King moves(X-ray threatened through the King)
    // and they will be removed from the lists later
    unique_ptr<Move> m;
    temp_square = kings_square->first_move();
    while (temp_square)
    {
      m.reset(new Move(kings_square, temp_square, piecetype::King));
      _possible_moves.into(m.get());
      temp_square = kings_square->next_move();
    }

    Piece *threat_piece = 0;
    Square *threat_square = 0;

    // Is the king threatened only once? (not double check)
    if (kings_square->count_threats(other_col) == 1)
    {
      //cout << "King is threatened once" << endl;
      threat_square = kings_square->first_threat();
      threat_piece = threat_square->get_piece();
      if (threat_piece == 0)
      {
        cerr << "no-piece error in check_moves()\n";
      }

      // The piece that checks the king maybe can be taken.
      // If it is the king that can take it, that was covered
      // when the king's moves were calculated
      Square *tmp_square = threat_square->first_threat();
      while (tmp_square)
      {
        if (tmp_square != kings_square)
        {
          // Check that the piece isn't pinned,
          // in which case it would not be allowed to move.
          if (tmp_square->in_movelist(threat_square))
          {
            unique_ptr<Move> move;
            move.reset(new Move(tmp_square, threat_square));
            _possible_moves.into(move.get());
          }
        }
        tmp_square = threat_square->next_threat();
      }

      // Maybe the threatening piece is a pawn and can be taken en passant.
      if (_en_passant_square)
      {
        switch (col_to_move)
        {
          case col::white:
            if (kings_square->get_rankindex() == 4 && threat_piece->is(piecetype::Pawn)) // we know it's black
            {
              check_if_threat_can_be_taken_en_passant(col_to_move, threat_square);
            }
            break;
          case col::black:
            if (kings_square->get_rankindex() == 5 && threat_piece->is(piecetype::Pawn)) // we know it's col::white
            {
              check_if_threat_can_be_taken_en_passant(col_to_move, threat_square);
            }
            break;
          default:
            cerr << "Error: unknown color" << endl;
            break;
        }
      }

      // Can we put some piece in between threat-piece and our king?
      switch (threat_piece->get_type())
      {
        case piecetype::Bishop:
          check_bishop_or_queen(threat_square, kings_square, col_to_move);
          break;
        case piecetype::Rook:
          check_rook_or_queen(threat_square, kings_square, col_to_move);
          break;
        case piecetype::Queen:
          check_bishop_or_queen(threat_square, kings_square, col_to_move);
          check_rook_or_queen(threat_square, kings_square, col_to_move);
          break;
        default:
          // If it's a Knight or a pawn that threatens our king, then we can't
          // Put anything in between.
          break;
      }
    }
  }
  else
  {
    calculate_moves_K_not_threatened(col_to_move);
  }

// ATTENTION! WIll THIS WORK? into_as_first if promotion is a take.
// What about _list_index in into as first? increment?

// Duplicate "promotion moves" for all possible promotions (Q,R,B,N)
  Move *mo = _possible_moves.first();
  while (mo)
  {
    if (mo->get_promotion() && mo->get_promotion_piece_type() == piecetype::Undefined)
    {
      require(col_to_move == col::white ? (mo->get_from_rankindex() == 7) : (mo->get_from_rankindex() == 2),
      __FILE__,
              __func__,
              __LINE__);
      _possible_moves.out(mo);
      fix_promotion_move(mo);
    }
    mo = _possible_moves.next();
  }
//  uint64_t nsec_stop = current_time.nanoseconds();
//  int timediff = nsec_stop - nsec_start;
//  cout << "calculate_moves_nsecs = " << timediff << endl;
}

void Board::fix_promotion_move(Move *m)
{
  unique_ptr<Move> um(new Move(m->get_from(), m->get_to(), m->get_piece_type(), m->get_take(), m->get_target_piece_type(), false, true, piecetype::Queen, false));
  _possible_moves.into(um.get());
  um.reset(new Move(m->get_from(), m->get_to(), m->get_piece_type(), m->get_take(), m->get_target_piece_type(), false, true, piecetype::Rook, false));
  _possible_moves.into(um.get());
  um.reset(new Move(m->get_from(), m->get_to(), m->get_piece_type(), m->get_take(), m->get_target_piece_type(), false, true, piecetype::Bishop, false));
  _possible_moves.into(um.get());
  um.reset(new Move(m->get_from(), m->get_to(), m->get_piece_type(), m->get_take(), m->get_target_piece_type(), false, true, piecetype::Knight, false));
  _possible_moves.into(um.get());
}

// First make a move, then init the board and possible moves for other_col
int Board::make_move(int i, int &move_no, col col_to_move)
{
  col other_col = col_to_move == col::white ? col::black : col::white;
  Move *m = _possible_moves[i];
  //write(cout, cmd_line_diagram, col::white);
  if (m)
  {
    Position from = m->get_from();
    Position to = m->get_to();
    Square *from_square = _file[from.get_file()][from.get_rank()];
    Square *to_square = _file[to.get_file()][to.get_rank()];
    if (from_square->get_piece())
    {
      // MAKE THE MOVE
      Piece *p = from_square->release_piece();
      if (to_square->get_piece())
      {
        m->set_take(true);
      }
      to_square->contain_piece(p);
      if (!p->is(piecetype::Pawn))
        _en_passant_square = 0;
      // SOME SPECIAL CASES
      switch (p->get_type())
      {
        // CASTLING
        case piecetype::King:
          _king_square[static_cast<int>(col_to_move)] = to_square;
          _castling_state.king_moved(col_to_move);
          if (col_to_move == col::black)
          {
            if ((from_square == _file[e][8]) && (to_square == _file[g][8]))
            {
              // Move the rook
              _file[f][8]->contain_piece(_file[h][8]->release_piece());
              _castling_state.king_rook_moved(col::black);
              _castling_state.set_has_castled(col::black);
            }
            if ((from_square == _file[e][8]) && (to_square == _file[c][8]))
            {
              // Move the rook
              _file[d][8]->contain_piece(_file[a][8]->release_piece());
              _castling_state.queen_rook_moved(col::black);
              _castling_state.set_has_castled(col::black);
            }
          }
          else
          {
            if ((from_square == _file[e][1]) && (to_square == _file[g][1]))
            {
              _file[f][1]->contain_piece(_file[h][1]->release_piece());
              _castling_state.king_rook_moved(col::white);
              _castling_state.set_has_castled(col::white);
            }
            if ((from_square == _file[e][1]) && (to_square == _file[c][1]))
            {
              _file[d][1]->contain_piece(_file[a][1]->release_piece());
              _castling_state.queen_rook_moved(col::white);
              _castling_state.set_has_castled(col::white);
            }
          }
          break;

          // SET STATUS IF ROOK HAS MOVED FROM ITS ORIGINAL SQUARE
          //    (FOR ROOKING)
        case piecetype::Rook:
          if (col_to_move == col::white)
          {
            if (from_square == _file[a][1])
              _castling_state.queen_rook_moved(col::white);
            else if (from_square == _file[h][1])
              _castling_state.king_rook_moved(col::white);
          }
          else
          {
            if (from_square == _file[a][8])
              _castling_state.queen_rook_moved(col::black);
            else if (from_square == _file[h][8])
              _castling_state.king_rook_moved(col::black);
          }
          break;

          // PAWN PROMOTION AND EN PASSANT
        case piecetype::Pawn:
          if (m->get_promotion())
          {
            Piece *newpiece = new Piece(m->get_promotion_piece_type(), p->get_color());
            // Attention! here we delete p
            to_square->contain_piece(newpiece);
          }
          else if (to_square == _en_passant_square)
          {
            m->set_en_passant(true);
            if (col_to_move == col::white)
              _file[to.get_file()][to.get_rank() - 1]->remove_piece();
            else
              _file[to.get_file()][to.get_rank() + 1]->remove_piece();
          }
          // If the pawn moves two squares we have to set the _en_passant_square.
          if (to.get_rank() - from.get_rank() == 2)
          {
            _en_passant_square = _file[to.get_file()][to.get_rank() - 1];
          }
          else if (to.get_rank() - from.get_rank() == -2)
          {
            _en_passant_square = _file[to.get_file()][to.get_rank() + 1];
          }
          else
          {
            _en_passant_square = 0;
          }
          break;

        default:
          break;
      }
      if (col_to_move == col::black)
      {
        move_no++;
      }
      _last_move = *m;
      // Check if the move is a check and init the board for the opponents
      // next move.
      clear(false);      // Clear board but don't remove pieces
      init(other_col);
      if (_king_square[static_cast<int>(other_col)]->count_threats(col_to_move))
        _last_move.set_check(true);

      calculate_moves(other_col);
      return 0;
    }
  }
  cerr << "make_move error" << endl;
  return -1;
}

// This method is only for the cmd-line interface
int Board::make_move(playertype player, int &move_no, col col_to_move)
{
  unique_ptr<Move> m;
  if (player != playertype::human)
  {
    cout << "Error: Using wrong make_move() method for computer." << endl;
    return -1;
  }
  //this->write("testfile.doc");
  //   cerr<<"*** Reading the latest move ***\n";
  Position from;
  Position to;
  char st[100];
  bool first = true;
  while (true)
  {
    //cout << "inside while" << endl;
    if (!first)
      cout << "The Move you entered is impossible!" << endl << endl;
    first = false;
    cout << "Your move: ";
    cin >> st;
    bool from_file_read = false;
    bool from_rank_read = false;
    bool to_file_read = false;
    bool to_rank_read = false;
    bool ep_checked = false;
    bool en_passant = false;
    bool check = false;
    bool take = false;
    piecetype pt = piecetype::Pawn;
    bool promotion = false;
    piecetype promotion_pt = piecetype::Pawn;
    piecetype target_pt = piecetype::Pawn;
    int i;
    for (i = 0; i < (int) strlen(st); i++)
    {
      if (!from_file_read)
      {
        if (from.set_file(st[i]))
          from_file_read = true;
        else if (!read_piece_type(pt, st[i]))
          break;
        continue;
      };
      if (!from_rank_read)
      {
        if (from.set_rank(st[i]))
          from_rank_read = true;
        else
          break;
        continue;
      }
      if (!to_file_read)
      {
        if (to.set_file(st[i]))
          to_file_read = true;
        else if (st[i] != '-' && st[i] != 'x')
          break;
        else if (st[i] == 'x')
          take = true;
        continue;
      }
      if (!to_rank_read)
      {
        if (to.set_rank(st[i]))
          to_rank_read = true;
        else
          break;
        continue;
      }
      if (!ep_checked)
      {
        if (st[i] == 'e')
          en_passant = true;
        else if (read_piece_type(promotion_pt, st[i]))
        {

        }
        else if (st[i] == '+')
          check = true;
        else
          break;
        ep_checked = true;
        continue;
      }
      if (en_passant)
        if (st[i] == '.' || st[i] == 'p')
          continue;
      if (!check)
      {
        if (st[i] == '+')
          check = true;
      }
    } // end of for loop
    if (i < (int) strlen(st))
      continue;
    //this->write(cout, debug);
    Square *from_square = _file[from.get_file()][from.get_rank()];
    Piece *p = from_square->get_piece();
    if (!p)
      continue;

    // Check take
    Square *to_square = _file[to.get_file()][to.get_rank()];
    Piece *p2 = to_square->get_piece();
    if (!p2)
    {
      if (take == true)
        continue;
    }
    else if (p2->get_color() == p->get_color())
      continue;
    else
    {
      take = true;
      target_pt = p2->get_type();
    }
    // Check piece_type
    if (pt != piecetype::Pawn)
      if (p->get_type() != pt)
        continue;
    pt = p->get_type();
    // Check promotion
    if (promotion) //It is supposed to be a promotion
    {
      if (col_to_move == col::white)
      {
        if (from.get_rank() != 7)
          continue;
        else if (_file[from.get_file()][from.get_rank()]->get_piece()->get_type() != piecetype::Pawn)
          continue;
      }
      else //col_to_move==col::black
      {
        if (from.get_rank() != 2)
          continue;
        else if (_file[from.get_file()][from.get_rank()]->get_piece()->get_type() != piecetype::Pawn)
          continue;
      }
    }
    if (pt == piecetype::Pawn)
    {
      if (_en_passant_square)
        if (_en_passant_square->get_position() == to)
          en_passant = true;
    }
    m.reset(new Move(from, to, pt, take, target_pt, en_passant, promotion, promotion_pt, check));
    // find index of move
    int move_index;
    if (!_possible_moves.in_list(m.get(), &move_index))
    {
      continue;
    }
    //  Move is OK,make it
    return make_move(move_index, move_no, col_to_move);
  } // while not read
}

float Board::evaluate_position(col col_to_move, outputtype ot, int level) const
{
  if (_possible_moves.size() == 0)
  {
    if (_king_square[static_cast<int>(col_to_move)]->count_threats() > 0)
    {
      // This is checkmate, we want to evaluate the quickest way to mate higher
      // so we add/subtract level.
      return (col_to_move == col::white) ? (eval_min + level) : (eval_max - level);
    }
    else
    {
      // This must be stalemate
      return 0.0;
    }
  }
  uint64_t nsec_start = current_time.nanoseconds();

  // Start with a very small number in sum, just so we don't return 0.0 in an
  // equal position. 0.0 is reserved for stalemate.
  float sum = epsilon;
  count_material(sum, 0.95F, ot);
  count_center_control(sum, 0.02F, ot);
  //count_possible_moves(sum, 0.01F, col_to_move); // of doubtful value.
  count_development(sum, 0.05F, ot);
  count_pawns_in_centre(sum, 0.03F, ot);
  count_castling(sum, 0.10F, ot);
  time_diff_sum += current_time.nanoseconds() - nsec_start;
  return sum;
}

void Board::count_material(float &sum, float weight, outputtype ot) const
{
  float counter = 0.0;
  Piece *p;
  for (int i = 1; i <= 8; i++)
  {
    for (int j = a; j <= h; j++)
    {
      p = _file[j][i]->get_piece();
      if (p)
      {
        switch (p->get_type())
        {
          case piecetype::Pawn:
            if (p->get_color() == col::black)
              counter -= 1;
            else
              counter += 1;
            break;
          case piecetype::Rook:
            if (p->get_color() == col::black)
              counter -= 5;
            else
              counter += 5;
            break;
          case piecetype::Knight:
            if (p->get_color() == col::black)
              counter -= 3;
            else
              counter += 3;
            break;
          case piecetype::Bishop:
            if (p->get_color() == col::black)
              counter -= 3;
            else
              counter += 3;
            break;
          case piecetype::Queen:
            if (p->get_color() == col::black)
              counter -= 9;
            else
              counter += 9;
            break;

          case piecetype::King:
            break;
          default:
            cerr << "Error: Undefined Piece Type: " << static_cast<int>(p->get_type()) << endl;
        }
      }
    }
  }
  if (ot == outputtype::debug)
    cout << endl << "Material " << counter * weight << endl;
  sum += counter * weight;
}

void Board::count_center_control(float &sum, float weight, outputtype ot) const
{
  int counter = 0;
  counter += _file[d][4]->count_controls();
  counter += _file[d][5]->count_controls();
  counter += _file[e][4]->count_controls();
  counter += _file[e][5]->count_controls();
  if (ot == outputtype::debug)
    cout << "Center control " << counter * weight << endl;
  sum += counter * weight;
}

void Board::count_pawns_in_centre(float &sum, float weight, outputtype ot) const
{
  int counter = 0;
  if (_file[d][4]->contains_piece(col::white, piecetype::Pawn))
    counter++;
  if (_file[d][5]->contains_piece(col::white, piecetype::Pawn))
    counter++;
  if (_file[e][4]->contains_piece(col::white, piecetype::Pawn))
    counter++;
  if (_file[e][5]->contains_piece(col::white, piecetype::Pawn))
    counter++;
  if (_file[d][4]->contains_piece(col::black, piecetype::Pawn))
    counter--;
  if (_file[d][5]->contains_piece(col::black, piecetype::Pawn))
    counter--;
  if (_file[e][4]->contains_piece(col::black, piecetype::Pawn))
    counter--;
  if (_file[e][5]->contains_piece(col::black, piecetype::Pawn))
    counter--;
  if (ot == outputtype::debug)
    cout << "Pawns in center " << counter * weight << endl;
  sum += counter * weight;
}

//void Board::count_possible_moves(float& sum, float weight, col col_to_move) const
//{
//  weight = (col_to_move == col::white) ? weight : -weight;
//  sum += weight * _possible_moves.size();
//}

void Board::count_development(float &sum, float weight, outputtype ot) const
{
  int counter = 0;
  if (!_file[a][1]->contains_piece(col::white, piecetype::Rook))
    counter++;
  if (!_file[b][1]->contains_piece(col::white, piecetype::Knight))
    counter++;
  if (!_file[c][1]->contains_piece(col::white, piecetype::Bishop))
    counter++;
  // if (!_file[d][1]->contains_piece(col::white, piecetype::Queen))
  //  counter++;
  // forget the king
  if (!_file[f][1]->contains_piece(col::white, piecetype::Bishop))
    counter++;
  if (!_file[g][1]->contains_piece(col::white, piecetype::Knight))
    counter++;
  if (!_file[h][1]->contains_piece(col::white, piecetype::Rook))
    counter++;

  if (!_file[a][8]->contains_piece(col::black, piecetype::Rook))
    counter--;
  if (!_file[b][8]->contains_piece(col::black, piecetype::Knight))
    counter--;
  if (!_file[c][8]->contains_piece(col::black, piecetype::Bishop))
    counter--;
  // if (!_file[d][8]->contains_piece(col::black, piecetype::Queen))
  //  counter--;
  if (!_file[f][8]->contains_piece(col::black, piecetype::Bishop))
    counter--;
  if (!_file[g][8]->contains_piece(col::black, piecetype::Knight))
    counter--;
  if (!_file[h][8]->contains_piece(col::black, piecetype::Rook))
    counter--;
  if (ot == outputtype::debug)
    cout << "Development " << counter * weight << endl;
  sum += counter * weight;
}

void Board::count_castling(float &sum, float weight, outputtype ot) const
{
  int counter = 0;
  if (_castling_state.has_castled(col::white))
    counter++;
  if (_castling_state.has_castled(col::black))
    counter--;
  if (ot == outputtype::debug)
    cout << "Castling " << counter * weight << endl;
  sum += counter * weight;
}

bool Board::is_end_node() const
{
  if (_possible_moves.size() == 0)
    return true;
  return false;
}

// This method will run in the timer_thread.
static void start_timer(const string& max_search_time)
{
  double time = stod(max_search_time);
  time_left = true;
  while (time_left)
  {
    uint64_t nsec_start = current_time.nanoseconds();
    this_thread::sleep_for(milliseconds(10));
    uint64_t nsec_stop = current_time.nanoseconds();
    uint64_t timediff = nsec_stop - nsec_start;
    time -= (double)timediff/1e6;
    if (time <= 0.0)
    {
      time_left = false;
      break;
    }
  }
}

void Board::start_timer_thread(const string& max_search_time)
{
  thread timer_thread(start_timer, max_search_time);
  timer_thread.detach();
}

bool Board::has_time_left()
{
  return time_left;
}

void Board::set_time_left(bool value)
{
  time_left = value;
}


// This method is for testing such functionality as turning of pruning and
// continuing the search until no more captures area available."
float Board::max_for_testing(int level, int move_no, float alpha, float beta, int &best_move_index, const int &max_search_level, bool use_pruning, bool search_until_no_captures) const
{
  float max_value = -101.0; // Must be lower than lowest evaluation
  int dummy_index;
  best_move_index = -1;
  level++;
//  cout << "level = " << level << ":" << _last_move << endl;
  if (is_end_node() || (level >= max_search_level && !_last_move.get_take()))
  {
    return evaluate_position(col::black, outputtype::silent, level);
  }
  else
  {
    bool takes = false;
    for (int i = 0; i < _possible_moves.size(); i++)
    {
      // Continue after max_search_level, but only if the move is a capture.
      if (search_until_no_captures)
      {
          if (level >= max_search_level && !_possible_moves[i]->get_take())
            continue;
          else
            takes = true;
      }
      Board::level_boards[level] = *this;
      level_boards[level].make_move(i, move_no, col::white);
      float tmp_value = level_boards[level].min_for_testing(level, move_no, alpha, beta, dummy_index, max_search_level, use_pruning, search_until_no_captures);
      if (tmp_value > max_value)
      {
        max_value = tmp_value;
        best_move_index = i;
      }
      if (use_pruning)
      {
        if (tmp_value >= beta)
        {
          //clean up and prune
          //cout << "pruning in max" << endl;
          //level_boards[level].clear();
          return max_value;          // same as tmp_value
        }
        if (tmp_value > alpha)
        {
          alpha = tmp_value;
        }
      }
      //level_boards[level].clear();
      if (!time_left)
      {
        best_move_index = -1;
        return 0.0;
      }
    }
    if (search_until_no_captures)
    {
      if (!takes) // There were no captures in the possible move lisst
        return evaluate_position(col::black, outputtype::silent, level);
    }
    return max_value;
  }
}

// This method is for testing such functionality as turning of pruning and
// continue searching until no more "takes" area available."
float Board::min_for_testing(int level, int move_no, float alpha, float beta, int &best_move_index, const int &max_search_level, bool use_pruning, bool search_until_no_captures) const
{
  float min_value = 101.0; // Must be higher than highest evaluation
  int dummy_index;

  best_move_index = -1;
  level++;
  //  cout << "level = " << level << ":" << _last_move << endl;
  if (is_end_node() || (level >= max_search_level && !_last_move.get_take()))
  {
    return evaluate_position(col::black, outputtype::silent, level);
  }
  else
  {
    bool takes = false;
    for (int i = 0; i < _possible_moves.size(); i++)
    {
      // Continue after max_search_level, but only if the move is a capture.
      if (search_until_no_captures)
      {
        if (level >= max_search_level && !_possible_moves[i]->get_take())
          continue;
        else
          takes = true;
      }
      // save current board in list
      level_boards[level] = *this;
      level_boards[level].make_move(i, move_no, col::black);
      float tmp_value = level_boards[level].max_for_testing(level, move_no, alpha, beta, dummy_index, max_search_level, use_pruning, search_until_no_captures);
      if (tmp_value < min_value)
      {
        min_value = tmp_value;
        best_move_index = i;
      }
      if (use_pruning)
      {
        if (tmp_value <= alpha)
        {
          //clean up and prune
          //cout << "pruning in min" << endl;
          //level_boards[level].clear();
          return min_value;
        }
        if (tmp_value < beta)
        {
          beta = tmp_value;
        }
      }
      //level_boards[level].clear();
      if (!time_left)
      {
        best_move_index = -1;
        return 0.0;
      }
    }
    if (search_until_no_captures)
    {
      if (!takes)
        return evaluate_position(col::black, outputtype::silent, level);
    }
    return min_value;
  }
}

float Board::max(int level, int move_no, float alpha, float beta, int &best_move_index, const int &max_search_level) const
{
  float max_value = -101.0; // Must be lower than lowest evaluation
  int dummy_index; // best_move_index is only an output parameter,
                   // it doesn't matter what you put in.
  best_move_index = -1;
  level++;

  // If there are no possible moves, the evaluation will check for such things as
  // mate or stalemate which may happen before max_search_level has been reached.
  if (level >= max_search_level || _possible_moves.size() == 0)
  {
    return evaluate_position(col::black, outputtype::silent, level);
  }
  else
  {
    // Collect the best value from all possible moves
    for (int i = 0; i < _possible_moves.size(); i++)
    {
      // Copy current board into the preallocated board for this level.
      Board::level_boards[level] = *this;
      // Make the selected move on the "level-board" and ask min() to evaluate it further.
      level_boards[level].make_move(i, move_no, col::white);
      float tmp_value = level_boards[level].min(level, move_no, alpha, beta, dummy_index, max_search_level);
      // Save the value if it is the "best", so far, from max() point of view.
      if (tmp_value > max_value)
      {
        max_value = tmp_value;
        best_move_index = i;
      }
      // Pruning:
      if (tmp_value >= beta)
      {
        // Look no further. Skip the rest of the branches.
        // They wont produce a better result.
        return max_value;
      }
      if (tmp_value > alpha)
      {
        // Update alpha value for min();
        alpha = tmp_value;
      }
      // Somewhere we have to check if time is up, to interrupt the search.
      // Why not do it here?
      if (!time_left)
      {
        best_move_index = -1;
        return 0.0;
      }
    }
    // Return the best value among the possible moves.
    return max_value;
  }
}

// min() is naturally very similar to max, but occasionally reversed,
// so I've not supplied any comments on this function. See max().
float Board::min(int level, int move_no, float alpha, float beta, int &best_move_index, const int &max_search_level) const
{
  float min_value = 101.0;
  int dummy_index;
  best_move_index = -1;
  level++;
  if (level >= max_search_level || _possible_moves.size() == 0)
  {
    return evaluate_position(col::black, outputtype::silent, level);
  }
  else
  {
    for (int i = 0; i < _possible_moves.size(); i++)
    {
      level_boards[level] = *this;
      level_boards[level].make_move(i, move_no, col::black);
      float tmp_value = level_boards[level].max(level, move_no, alpha, beta, dummy_index, max_search_level);
      if (tmp_value < min_value)
      {
        min_value = tmp_value;
        best_move_index = i;
      }
      if (tmp_value <= alpha)
      {
        return min_value;
      }
      if (tmp_value < beta)
      {
        beta = tmp_value;
      }
      if (!time_left)
      {
        best_move_index = -1;
        return 0.0;
      }
    }
    return min_value;
  }
}

void Board::set_time_diff_sum(uint64_t value)
{
  time_diff_sum = value;
}

uint64_t Board::get_time_diff_sum()
{
  return time_diff_sum;
}

} // namespace C2_chess

