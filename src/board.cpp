#include <memory>
#include <sstream>
#include "board.hpp"
#include "square.hpp"
#include "piece.hpp"
#include "current_time.hpp"
#include "chesstypes.hpp"
#include "chessfuncs.hpp"

extern "C"
{
#include <string.h>
}

namespace
{
C2_chess::CurrentTime current_time;
}

namespace C2_chess
{

using std::unique_ptr;
using std::stringstream;

// To reach level 15 is but a dream, but to be on the safe side ...
Board Board::level_boards[15]; // definition, complete type

Board::Board() :
    _last_move(), _possible_moves(), _castling_state(), _en_passant_square(0)
{
  // cout << "Board default constr" << endl;
  _file[a].set_name('a');
  _file[b].set_name('b');
  _file[c].set_name('c');
  _file[d].set_name('d');
  _file[e].set_name('e');
  _file[f].set_name('f');
  _file[g].set_name('g');
  _file[h].set_name('h');

  _rank[0].set_name('0'); // not used
  _rank[1].set_name('1');
  _rank[2].set_name('2');
  _rank[3].set_name('3');
  _rank[4].set_name('4');
  _rank[5].set_name('5');
  _rank[6].set_name('6');
  _rank[7].set_name('7');
  _rank[8].set_name('8');

  _file[a][1] = new Square(a, 1, black);
  _file[a][2] = new Square(a, 2, white);
  _file[a][3] = new Square(a, 3, black);
  _file[a][4] = new Square(a, 4, white);
  _file[a][5] = new Square(a, 5, black);
  _file[a][6] = new Square(a, 6, white);
  _file[a][7] = new Square(a, 7, black);
  _file[a][8] = new Square(a, 8, white);

  _file[b][1] = new Square(b, 1, white);
  _file[b][2] = new Square(b, 2, black);
  _file[b][3] = new Square(b, 3, white);
  _file[b][4] = new Square(b, 4, black);
  _file[b][5] = new Square(b, 5, white);
  _file[b][6] = new Square(b, 6, black);
  _file[b][7] = new Square(b, 7, white);
  _file[b][8] = new Square(b, 8, black);

  _file[c][1] = new Square(c, 1, black);
  _file[c][2] = new Square(c, 2, white);
  _file[c][3] = new Square(c, 3, black);
  _file[c][4] = new Square(c, 4, white);
  _file[c][5] = new Square(c, 5, black);
  _file[c][6] = new Square(c, 6, white);
  _file[c][7] = new Square(c, 7, black);
  _file[c][8] = new Square(c, 8, white);

  _file[d][1] = new Square(d, 1, white);
  _file[d][2] = new Square(d, 2, black);
  _file[d][3] = new Square(d, 3, white);
  _file[d][4] = new Square(d, 4, black);
  _file[d][5] = new Square(d, 5, white);
  _file[d][6] = new Square(d, 6, black);
  _file[d][7] = new Square(d, 7, white);
  _file[d][8] = new Square(d, 8, black);

  _file[e][1] = new Square(e, 1, black);
  _file[e][2] = new Square(e, 2, white);
  _file[e][3] = new Square(e, 3, black);
  _file[e][4] = new Square(e, 4, white);
  _file[e][5] = new Square(e, 5, black);
  _file[e][6] = new Square(e, 6, white);
  _file[e][7] = new Square(e, 7, black);
  _file[e][8] = new Square(e, 8, white);

  _file[f][1] = new Square(f, 1, white);
  _file[f][2] = new Square(f, 2, black);
  _file[f][3] = new Square(f, 3, white);
  _file[f][4] = new Square(f, 4, black);
  _file[f][5] = new Square(f, 5, white);
  _file[f][6] = new Square(f, 6, black);
  _file[f][7] = new Square(f, 7, white);
  _file[f][8] = new Square(f, 8, black);

  _file[g][1] = new Square(g, 1, black);
  _file[g][2] = new Square(g, 2, white);
  _file[g][3] = new Square(g, 3, black);
  _file[g][4] = new Square(g, 4, white);
  _file[g][5] = new Square(g, 5, black);
  _file[g][6] = new Square(g, 6, white);
  _file[g][7] = new Square(g, 7, black);
  _file[g][8] = new Square(g, 8, white);

  _file[h][1] = new Square(h, 1, white);
  _file[h][2] = new Square(h, 2, black);
  _file[h][3] = new Square(h, 3, white);
  _file[h][4] = new Square(h, 4, black);
  _file[h][5] = new Square(h, 5, white);
  _file[h][6] = new Square(h, 6, black);
  _file[h][7] = new Square(h, 7, white);
  _file[h][8] = new Square(h, 8, black);

// *** FIX THE RANKS ***

  for (int rank = 1; rank <= 8; rank++)
  {
    for (int file = a; file <= h; file++)
    {
      _rank[rank][file] = _file[file][rank];
    }
  }
}

Board::Board(const Board& from) :
    Board()
{
  *this = from;
}

Board::~Board()
{
  clear(); // Deletes pieces, clears square-lists and deletes moves
  // The squares are "automatically" deleted in File::~File()
}

Board& Board::operator=(const Board& from)
{

  clear();

  _last_move = from._last_move;
  _possible_moves = from._possible_moves;
  _castling_state = from._castling_state;
  // Taking care of the pointer variables.
  // These must obviously point into their own Board-object.
  int tmp_file = from._king_square[white]->get_position().get_file();
  int tmp_rank = from._king_square[white]->get_position().get_rank();
  _king_square[white] = _file[tmp_file][tmp_rank];
  tmp_file = from._king_square[black]->get_position().get_file();
  tmp_rank = from._king_square[black]->get_position().get_rank();
  _king_square[black] = _file[tmp_file][tmp_rank];
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
      Piece* p = from._file[file][rank]->get_piece();
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
  _file[a][1]->contain_piece(new Piece(Rook, white));
  _file[b][1]->contain_piece(new Piece(Knight, white));
  _file[c][1]->contain_piece(new Piece(Bishop, white));
  _file[d][1]->contain_piece(new Piece(Queen, white));
  _file[e][1]->contain_piece(new Piece(King, white));
  _file[f][1]->contain_piece(new Piece(Bishop, white));
  _file[g][1]->contain_piece(new Piece(Knight, white));
  _file[h][1]->contain_piece(new Piece(Rook, white));
  for (int file = a; file <= h; file++)
    _file[file][2]->contain_piece(new Piece(Pawn, white));
  for (int file = a; file <= h; file++)
    _file[file][7]->contain_piece(new Piece(Pawn, black));
  _file[a][8]->contain_piece(new Piece(Rook, black));
  _file[b][8]->contain_piece(new Piece(Knight, black));
  _file[c][8]->contain_piece(new Piece(Bishop, black));
  _file[d][8]->contain_piece(new Piece(Queen, black));
  _file[e][8]->contain_piece(new Piece(King, black));
  _file[f][8]->contain_piece(new Piece(Bishop, black));
  _file[g][8]->contain_piece(new Piece(Knight, black));
  _file[h][8]->contain_piece(new Piece(Rook, black));
}

void Board::put_piece(Piece* const p, int file, int rank)
{
  _file[file][rank]->contain_piece(p);
}

bool Board::read_piece_type(piecetype& pt, char c) const
{
  // method not important for efficiency
  switch (c)
  {
    case 'P':
      pt = Pawn;
      break;
    case 'K':
      pt = King;
      break;
    case 'N':
      pt = Knight;
      break;
    case 'B':
      pt = Bishop;
      break;
    case 'Q':
      pt = Queen;
      break;
    case 'R':
      pt = Rook;
      break;
    default:
      return false;
  }
  return true;
}

ostream& Board::write(ostream& os, output_type wt, col from_perspective) const
{
  switch (wt)
  {
    case debug:
      os << "The latest move was: ";
      os << _last_move << endl;
      os << "Castling state is: " << _castling_state << endl;
      os << "En passant square is: " << _en_passant_square << endl;
      for (int fileindex = a; fileindex <= h; fileindex++)
        for (int rankindex = 1; rankindex <= 8; rankindex++)
          _file[fileindex][rankindex]->write_describing(os);
      os << endl << "*** Possible moves ***" << endl;
      for (int i = 0; i < _possible_moves.cardinal(); i++)
        os << *_possible_moves[i] << endl;
      os << endl;
      this->write(os, cmd_line_diagram, white) << endl;
      break;
    case cmd_line_diagram:
      if (from_perspective == white)
      {
        os << "###################" << endl;
        for (int i = 8; i >= 1; i--)
        {
          os << "#";
          for (int j = a; j <= h; j++)
          {
            os << "|";
            Piece* p = _file[j][i]->get_piece();
            if (p)
              p->write_diagram_style(os);
            else
              os << "-";
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
            Piece* p = _file[j][i]->get_piece();
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

Shared_ostream& Board::write(Shared_ostream& os, output_type wt, col from_perspective) const
{
  stringstream ss;
  write(ss, wt, from_perspective);
  os << ss.str();
  return os;
}

ostream& Board::write_possible_moves(ostream& os)
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
  col other_col = this_col == white ? black : white;
  int one_or_eight = (this_col == white ? 1 : 8);

  if (_king_square[this_col]->count_threats(other_col) == 0)
  {
    // The king is not in check
    if (_castling_state.is_kingside_castling_OK(this_col))
    {
      // Double-check that the king and rook are in the correct squares.
      // We can't always trust the castling status, e.g. if read from pgn-file.
      if (_king_square[this_col] == _file[e][one_or_eight] && _file[h][one_or_eight]->contains(this_col, Rook))
      {
        // Castling short? Check that the squares between the
        // King and the rook are free and that the squares
        // which the King must pass are not threatened
        if (!(_file[f][one_or_eight]->get_piece() || _file[g][one_or_eight]->get_piece()))
        {
          if (!(_file[f][one_or_eight]->count_threats(other_col) || _file[g][one_or_eight]->count_threats(other_col)))
            _king_square[this_col]->into_move(_file[g][one_or_eight]);
        }
      }
    }
    if (_castling_state.is_queenside_castling_OK(this_col))
    {
      if (_king_square[this_col] == _file[e][one_or_eight] && _file[a][one_or_eight]->contains(this_col, Rook))
      {
        // Castling long? Check that the squares between the
        // King and the rook are free and that the squares
        // which the King must pass are not threatened
        if (!(_file[b][one_or_eight]->get_piece() || _file[c][one_or_eight]->get_piece() || _file[d][one_or_eight]->get_piece()))
        {
          if (!(_file[c][one_or_eight]->count_threats(other_col) || _file[d][one_or_eight]->count_threats(other_col)))
            _king_square[this_col]->into_move(_file[c][one_or_eight]);
        }
      }
    }
  }
}

void Board::init_rook_or_queen(int file, int rank, Square* s, Piece* p)
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

void Board::init_bishop_or_queen(int file, int rank, Square* s, Piece* p)
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
  uint64_t nsec_start = current_time.nanoseconds();
  bool pieces_found = false;
  col other_col = col_to_move == white ? black : white;
  int tf;
  int tr;
  bool stop;
  for (int rank = 1; rank <= 8; rank++)
  {
    for (int file = a; file <= h; file++)
    {
      Piece* p = _file[file][rank]->get_piece();
      if (p)
      {
        pieces_found = true;
        Square* s = _file[file][rank];
        piecetype pt = p->get_type();
        switch (pt)
        {
          case King:
          {
            _king_square[p->get_color()] = s;
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
          case Queen:
            init_bishop_or_queen(file, rank, s, p);
            init_rook_or_queen(file, rank, s, p);
            break;

          case Rook:
            init_rook_or_queen(file, rank, s, p);
            break;

          case Bishop:
            init_bishop_or_queen(file, rank, s, p);
            break;

          case Knight:
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

          case Pawn:
            stop = false;
            if (p->get_color() == white)
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
            else // p->get_color() == black
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
  uint64_t nsec_stop = current_time.nanoseconds();
  int timediff = nsec_stop - nsec_start;
//  cout << "init_nsecs = " << timediff << endl;
  Square* king_square = _king_square[col_to_move];
  int king_file_index = king_square->get_fileindex();
  int king_rank_index = king_square->get_rankindex();
  tf = king_file_index;
  tr = king_rank_index;
  Square* own_piece_square = 0;
  while (allowed(++tf, ++tr))
  {
    Piece* p = _file[tf][tr]->get_piece();
    if (p)
    {
      if (p->is(other_col))
      {
        if (p->is(Bishop) || p->is(Queen))
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
    Piece* p = _file[tf][tr]->get_piece();
    if (p)
    {
      if (p->is(other_col))
      {
        if (p->is(Bishop) || p->is(Queen))
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
    Piece* p = _file[tf][tr]->get_piece();
    if (p)
    {
      if (p->is(other_col))
      {
        if (p->is(Bishop) || p->is(Queen))
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
    Piece* p = _file[tf][tr]->get_piece();
    if (p)
    {
      if (p->is(other_col))
      {
        if (p->is(Bishop) || p->is(Queen))
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
    Piece* p = _file[tf][tr]->get_piece();
    if (p)
    {
      if (p->get_color() == other_col)
      {
        if (p->is(Rook) || p->is(Queen))
          if (own_piece_square)
            fix_bound_piece_rank(king_square, own_piece_square, _file[tf][tr]);
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
    Piece* p = _file[tf][tr]->get_piece();
    if (p)
    {
      if (p->is(other_col))
      {
        if (p->is(Rook) || p->is(Queen))
          if (own_piece_square)
            fix_bound_piece_rank(king_square, own_piece_square, _file[tf][tr]);
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
    Piece* p = _file[tf][tr]->get_piece();
    if (p)
    {
      if (p->is(other_col))
      {
        if (p->is(Rook) || p->is(Queen))
          if (own_piece_square)
            fix_bound_piece_file(king_square, own_piece_square, _file[tf][tr]);
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
    Piece* p = _file[tf][tr]->get_piece();
    if (p)
    {
      if (p->is(other_col))
      {
        if (p->is(Rook) || p->is(Queen))
          if (own_piece_square)
            fix_bound_piece_file(king_square, own_piece_square, _file[tf][tr]);
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
  Square* temp_square;
  for (temp_square = _king_square[col_to_move]->first_move(); temp_square != 0; temp_square = _king_square[col_to_move]->next_move())
  {
    if (temp_square->get_piece())
    {
      if (temp_square->get_piece()->get_color() == other_col)
        if (temp_square->count_protections())
        {
          _king_square[col_to_move]->out_move(temp_square);
        }
    }
    else if (temp_square->count_threats(other_col))
    {
      _king_square[col_to_move]->out_move(temp_square);
    }
  }

  // fix the square(s) to which the King "can move", but will be threatened if
  // the King moves. (X-ray threat through the King)
  temp_square = _king_square[col_to_move]->first_threat();
  while (temp_square)
  {
    int kf = _king_square[col_to_move]->get_fileindex();
    int kr = _king_square[col_to_move]->get_rankindex();
    int tf = temp_square->get_fileindex();
    int tr = temp_square->get_rankindex();

    if (temp_square->same_rank(_king_square[col_to_move]))
    {
      if (tf == min(tf, kf))
        if (allowed(kf + 1, kr))
        {
          _king_square[col_to_move]->out_move(_file[kf + 1][kr]);
        }
      if (tf == max(tf, kf))
        if (allowed(kf - 1, kr))
        {
          _king_square[col_to_move]->out_move(_file[kf - 1][kr]);
        }
    }
    else if (temp_square->same_file(_king_square[col_to_move]))
    {
      if (tr == min(tr, kr))
      {
        if (allowed(kf, kr + 1))
        {
          _king_square[col_to_move]->out_move(_file[kf][kr + 1]);
        }
      }
      if (tr == max(tr, kr))
        if (allowed(kf, kr - 1))
        {
          _king_square[col_to_move]->out_move(_file[kf][kr - 1]);
        }
    }
    else if (temp_square->same_diagonal(_king_square[col_to_move]))
    {
      if (tr == min(tr, kr) && tf == min(tf, kf))
        if (allowed(kf + 1, kr + 1))
        {
          _king_square[col_to_move]->out_move(_file[kf + 1][kr + 1]);
        }
      if (tr == min(tr, kr) && tf == max(tf, kf))
        if (allowed(kf - 1, kr + 1))
        {
          _king_square[col_to_move]->out_move(_file[kf - 1][kr + 1]);
        }
      if (tr == max(tr, kr) && tf == min(tf, kf))
        if (allowed(kf + 1, kr - 1))
        {
          _king_square[col_to_move]->out_move(_file[kf + 1][kr - 1]);
        }
      if (tr == max(tr, kr) && tf == max(tf, kf))
        if (allowed(kf - 1, kr - 1))
        {
          _king_square[col_to_move]->out_move(_file[kf - 1][kr - 1]);
        }
    }
    temp_square = _king_square[col_to_move]->next_threat();
  }

  // What about Castling ?
  init_castling(col_to_move);
  // The following may not be necessary, because it probably affects nothing.
  // It's col_to_moves time to calculate possible moves.
  //init_castling(other_col);
  // cout << "End init" << endl;
  return 0;
}

void Board::fix_threat_prot(int file, int rank, Piece* p, Square* s)
{
  Square* temp_square = _file[file][rank];
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

void Board::fix_bound_piece_file(const Square* king_square, Square* own_piece_square, const Square* threat_square)
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
  for (Square* temp_square = own_piece_square->first_move(); temp_square != 0; temp_square = own_piece_square->next_move())
  {
    switch (pt)
    {
      case Bishop:
      case Knight:
        own_piece_square->clear_moves();
        break;
      case Pawn:
      case Rook:
      case Queen:
        if (!threat_square->get_position().same_file(*temp_square))
        {
          own_piece_square->out_move(temp_square);
        }
        break;
      default:
        cerr << "strange kind of own piece in fix_bound_piece_file " << pt << endl;
    }
  }
}

void Board::fix_bound_piece_rank(const Square* king_square, Square* own_piece_square, const Square* threat_square)
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
  for (Square* temp_square = own_piece_square->first_move(); temp_square != 0; temp_square = own_piece_square->next_move())
  {
    switch (pt)
    {
      case Pawn:
      case Bishop:
      case Knight:
        own_piece_square->clear_moves();
        break;
      case Rook:
      case Queen:
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

void Board::fix_bound_piece_diagonal(const Square* king_square, Square* own_piece_square, const Square* threat_square)
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
  for (Square* temp_square = own_piece_square->first_move(); temp_square != 0; temp_square = own_piece_square->next_move())
  {
    switch (pt)
    {
      case Queen:
      case Pawn:
        // Here we must check both squares.
        // If the queen moves as a rook it can end up on the other
        // diagonal of either the King_square or threat_square, but
        // never both. The same goes for a pawn.
        if (!threat_square->get_position().same_diagonal(*temp_square) || !king_square->get_position().same_diagonal(*temp_square))
        {
          own_piece_square->out_move(temp_square);
        }
        break;
      case Bishop:
        // The bishop can never reach the other diagonal of threat_square
        // or king_square, so it's enough to check against threat_square here.
        if (!threat_square->get_position().same_diagonal(*temp_square))
        {
          own_piece_square->out_move(temp_square);
        }
        break;
      case Rook:
      case Knight:
        own_piece_square->clear_moves();
        break;
      default:
        cerr << "strange kind of own piece in fix_bound_piece_diagonal" << endl;
    }
  }
}

void Board::fix_pawn_tp(int file, int rank, Piece* p, Square* s)
{
  Square* temp_square = _file[file][rank];
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

void Board::fix_en_passant(Square* s, Square* possible_ep_square)
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
//  uint64_t nsec_start = current_time.nanoseconds();
//  uint64_t nsec_stop = current_time.nanoseconds();
//  cout << "nsecs = " << nsec_stop - nsec_start << endl;
  Piece* piece;
  Square* from_square;
  switch (col_to_move)
  {
    case white:
      // cout << "white" << endl;
      if (rank > 2) // otherwise white can never move a pawn there.
      {
        from_square = _file[file][rank - 1];
        piece = from_square->get_piece();
        // Is there a piece and is it a white pawn?
        if (piece && piece->is(col_to_move, Pawn))
        {
          if (from_square->in_movelist(_file[file][rank]))
          {
            unique_ptr<Move> m(new Move(from_square, _file[file][rank]));
            _possible_moves.into(m.get());
            //cout << "nsecs = " << current_time.nanoseconds() - nsec_start << endl;
            return;
          }
        }
      }
      // White can only make two-squares-pawn-moves to rank 4
      // and if the square in between is free.
      if ((!piece) && rank == 4)
      {
        Square* from_square = _file[file][rank - 2];
        piece = from_square->get_piece();
        if (piece && piece->is(col_to_move, Pawn))
        {
          if (from_square->in_movelist(_file[file][rank]))
          {
            unique_ptr<Move> m(new Move(from_square, _file[file][rank]));
            _possible_moves.into(m.get());
            //cout << "nsecs = " << current_time.nanoseconds() - nsec_start << endl;
            return;
          }
        }
      }
      break;
    case black:
      // cout << "black" << endl;
      if (rank < 7) // otherwise black can never move a pawn there.
      {
        from_square = _file[file][rank + 1];
        piece = from_square->get_piece();
//        write(cout,
//              cmd_line_diagram);
//        cout << piece << endl;
//        if (piece)
//          cout << piece->get_type() << endl;
        // Is there a piece and is it a black pawn?
        if (piece && piece->is(col_to_move, Pawn))
        {
          if (from_square->in_movelist(_file[file][rank]))
          {
            unique_ptr<Move> m(new Move(from_square, _file[file][rank]));
            _possible_moves.into(m.get());
            //cout << "nsecs = " << current_time.nanoseconds() - nsec_start << endl;
            return;
          }
        }
      }
      if ((!piece) && rank == 5)
      {
        // It could be that a black pawn can move two steps to this square.
        from_square = _file[file][rank + 2];
        piece = from_square->get_piece();
        if (piece && piece->is(col_to_move, Pawn))
        {
          if (from_square->in_movelist(_file[file][rank]))
          {
            unique_ptr<Move> m(new Move(from_square, _file[file][rank]));
            _possible_moves.into(m.get());
            //cout << "nsecs = " << current_time.nanoseconds() - nsec_start << endl;
            return;
          }
        }
      }
      break;
  }
  //cout << "nsecs2 = " << current_time.nanoseconds() - nsec_start << endl;
}

void Board::calculate_moves_K_not_threatened(col col_to_move)
{
  // The king is not threatened. All remaining moves are allowed.
  Square* s;
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
  Square* temp_square = _file[i][j]->first_threat();
  while (temp_square)
  {
    if (temp_square->in_movelist(_file[i][j]))
    {
      if (temp_square->get_piece()->is(col_to_move))
      {
        if (!temp_square->get_piece()->is(King))
        {
          m.reset(new Move(temp_square, _file[i][j]));
          _possible_moves.into(m.get());
        }
      }
    }
    temp_square = _file[i][j]->next_threat();
  }
}

void Board::check_bishop_or_queen(Square* threat_square, Square* kings_square, col col_to_move)
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

void Board::check_rook_or_queen(Square* __restrict__ threat_square, Square* __restrict__ kings_square, col col_to_move)
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

void Board::check_if_threat_can_be_taken_en_passant(col col_to_move, Square* threat_square)
{
  unique_ptr<Move> m;
  int rank_increment = (col_to_move == white) ? 1 : -1;
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
        Square* s = _file[file][thr_rank];
        if (s->contains(col_to_move, Pawn))
        {
          if (s->in_movelist(_en_passant_square))
          {
            unique_ptr<Move> m(new Move(_file[file][thr_rank], _en_passant_square));
            _possible_moves.into_as_first(m.get());
          }
        }
      }
      file = thr_file + 1;
      if (allowed(file, thr_rank))
      {
        Square* s = _file[file][thr_rank];
        if (s->contains(col_to_move, Pawn))
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
  uint64_t nsec_start = current_time.nanoseconds();
  col other_col = col_to_move == white ? black : white;
  _possible_moves.clear();

  Square* temp_square;
  Square* kings_square = _king_square[col_to_move];
  // is the King threatened?
  if (kings_square->count_threats(other_col))
  {
    //cout << "king threatened" << endl;
    // All King moves to unthreatened squares are allowed. (Except moves to squares
    // which will be threatened if the King moves(X-ray threatened through the King)
    // and they will be removed from the lists later
    unique_ptr<Move> m;
    temp_square = kings_square->first_move();
    while (temp_square)
    {
      m.reset(new Move(kings_square, temp_square, King));
      _possible_moves.into(m.get());
      temp_square = kings_square->next_move();
    }

    Piece* threat_piece = 0;
    Square* threat_square = 0;

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
      unique_ptr<Move> m;
      Square* temp_square = threat_square->first_threat();
      while (temp_square)
      {
        if (temp_square != kings_square)
        {
          // Check that the piece isn't bound,
          // in which case it would not be allowed to move.
          if (temp_square->in_movelist(threat_square))
          {
            m.reset(new Move(temp_square, threat_square));
            _possible_moves.into(m.get());
          }
        }
        temp_square = threat_square->next_threat();
      }

      // Maybe the threatening piece is a pawn and can be taken en passant.
      if (_en_passant_square)
      {
        switch (col_to_move)
        {
          case white:
            if (kings_square->get_rankindex() == 4 && threat_piece->is(Pawn)) // we know it's black
            {
              check_if_threat_can_be_taken_en_passant(col_to_move, threat_square);
            }
            break;
          case black:
            if (kings_square->get_rankindex() == 5 && threat_piece->is(Pawn)) // we know it's white
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
        case Bishop:
          check_bishop_or_queen(threat_square, kings_square, col_to_move);
          break;
        case Rook:
          check_rook_or_queen(threat_square, kings_square, col_to_move);
          break;
        case Queen:
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
  Move* mo = _possible_moves.first();
  while (mo)
  {
    if (mo->get_promotion() && mo->get_promotion_piece_type() == Undefined)
    {
      require(col_to_move == white ? (mo->get_from_rankindex() == 7) : (mo->get_from_rankindex() == 2),
      __FILE__,
              __func__,
              __LINE__);
      _possible_moves.out(mo);
      fix_promotion_move(mo);
    }
    mo = _possible_moves.next();
  }
  uint64_t nsec_stop = current_time.nanoseconds();
  int timediff = nsec_stop - nsec_start;
//  cout << "calculate_moves_nsecs = " << timediff << endl;
}

void Board::fix_promotion_move(Move* m)
{
  unique_ptr<Move> um(new Move(m->get_from(), m->get_to(), m->get_piece_type(), m->get_take(), m->get_target_piece_type(), false, true, Queen, false));
  _possible_moves.into(um.get());
  um.reset(new Move(m->get_from(), m->get_to(), m->get_piece_type(), m->get_take(), m->get_target_piece_type(), false, true, Rook, false));
  _possible_moves.into(um.get());
  um.reset(new Move(m->get_from(), m->get_to(), m->get_piece_type(), m->get_take(), m->get_target_piece_type(), false, true, Bishop, false));
  _possible_moves.into(um.get());
  um.reset(new Move(m->get_from(), m->get_to(), m->get_piece_type(), m->get_take(), m->get_target_piece_type(), false, true, Knight, false));
  _possible_moves.into(um.get());
}

int Board::make_move(int i, int& move_no, col col_to_move, bool silent)
{
  col other_col = col_to_move == white ? black : white;
  Move* m = _possible_moves[i];
  //write(cout, cmd_line_diagram, white);
  if (m)
  {
    Position from = m->get_from();
    Position to = m->get_to();
    Square* from_square = _file[from.get_file()][from.get_rank()];
    Square* to_square = _file[to.get_file()][to.get_rank()];
    if (from_square->get_piece())
    {
      // MAKE THE MOVE
      Piece* p = from_square->release_piece();
      if (to_square->get_piece())
      {
        m->set_take(true);
      }
      to_square->contain_piece(p);
      if (!p->is(Pawn))
        _en_passant_square = 0;
      // SOME SPECIAL CASES
      switch (p->get_type())
      {
        // CASTLING
        case King:
          _king_square[col_to_move] = to_square;
          _castling_state.king_moved(col_to_move);
          if (col_to_move == black)
          {
            if ((from_square == _file[e][8]) && (to_square == _file[g][8]))
            {
              // Move the rook
              _file[f][8]->contain_piece(_file[h][8]->release_piece());
              _castling_state.king_rook_moved(black);
              _castling_state.set_has_castled(black);
            }
            if ((from_square == _file[e][8]) && (to_square == _file[c][8]))
            {
              // Move the rook
              _file[d][8]->contain_piece(_file[a][8]->release_piece());
              _castling_state.queen_rook_moved(black);
              _castling_state.set_has_castled(black);
            }
          }
          else
          {
            if ((from_square == _file[e][1]) && (to_square == _file[g][1]))
            {
              _file[f][1]->contain_piece(_file[h][1]->release_piece());
              _castling_state.king_rook_moved(white);
              _castling_state.set_has_castled(white);
            }
            if ((from_square == _file[e][1]) && (to_square == _file[c][1]))
            {
              _file[d][1]->contain_piece(_file[a][1]->release_piece());
              _castling_state.queen_rook_moved(white);
              _castling_state.set_has_castled(white);
            }
          }
          break;

          // SET STATUS IF ROOK HAS MOVED FROM ITS ORIGINAL SQUARE
          //    (FOR ROOKING)
        case Rook:
          if (col_to_move == white)
          {
            if (from_square == _file[a][1])
              _castling_state.queen_rook_moved(white);
            else if (from_square == _file[h][1])
              _castling_state.king_rook_moved(white);
          }
          else
          {
            if (from_square == _file[a][8])
              _castling_state.queen_rook_moved(black);
            else if (from_square == _file[h][8])
              _castling_state.king_rook_moved(black);
          }
          break;

          // PAWN PROMOTION AND EN PASSANT
        case Pawn:
          if (m->get_promotion())
          {
            Piece* newpiece = new Piece(m->get_promotion_piece_type(), p->get_color());
            // Attention! here we delete p
            to_square->contain_piece(newpiece);
          }
          else if (to_square == _en_passant_square)
          {
            m->set_en_passant(true);
            if (col_to_move == white)
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
      if (col_to_move == black)
      {
        move_no++;
      }
      _last_move = *m;
      // Check if the move is a check and init the board for the opponents
      // next move.
      clear(false);      // Clear board but don't remove pieces
      init(other_col);
      if (_king_square[other_col]->count_threats(col_to_move))
        _last_move.set_check(true);

      calculate_moves(other_col);
      return 0;
    }
  }
  cerr << "make_move error" << endl;
  return -1;
}

int Board::make_move(player_type pt, int& move_no, col col_to_move)
{
  unique_ptr<Move> m;
  if (pt != human)
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
    piecetype pt = Pawn;
    bool promotion = false;
    piecetype promotion_pt = Pawn;
    piecetype target_pt = Pawn;
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
    Square* from_square = _file[from.get_file()][from.get_rank()];
    Piece* p = from_square->get_piece();
    if (!p)
      continue;

    // Check take
    Square* to_square = _file[to.get_file()][to.get_rank()];
    Piece* p2 = to_square->get_piece();
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
    if (pt != Pawn)
      if (p->get_type() != pt)
        continue;
    pt = p->get_type();
    // Check promotion
    if (promotion) //It is supposed to be a promotion
    {
      if (col_to_move == white)
      {
        if (from.get_rank() != 7)
          continue;
        else if (_file[from.get_file()][from.get_rank()]->get_piece()->get_type() != Pawn)
          continue;
      }
      else //col_to_move==black
      {
        if (from.get_rank() != 2)
          continue;
        else if (_file[from.get_file()][from.get_rank()]->get_piece()->get_type() != Pawn)
          continue;
      }
    }
    if (pt == Pawn)
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
    return make_move(move_index, move_no, col_to_move, true);
  } // while not read
}

float Board::evaluate_position(col col_to_move, output_type ot, int level) const
{
  if (_possible_moves.cardinal() == 0)
  {
    if (_king_square[col_to_move]->count_threats() > 0)
    {
      // This is checkmate, we want to evaluate the quickest way to mate higher
      // so we add/subtract level.
      return (col_to_move == white) ? (eval_min + level) : (eval_max - level);
    }
    else
    {
      // This must be stalemate
      // cout << "STALEMATE " << col_to_move << endl;
      return 0;
    }
  }
  float sum = 0.0;
  count_material(sum, 0.95, col_to_move, ot);
  count_center_control(sum, 0.01, col_to_move, ot);
  //count_possible_moves(sum, 0.01, col_to_move);
  count_development(sum, 0.01, col_to_move, ot);
  count_pawns_in_centre(sum, 0.01, col_to_move, ot);
  count_castling(sum, 0.01, col_to_move, ot);
  //cout << "sum " << sum << endl << endl;
  return sum;
}

void Board::count_material(float& sum, float weight, col col_to_move, output_type ot) const
{
  float counter = 0.0;
  Piece* p;
  for (int i = 1; i <= 8; i++)
  {
    for (int j = a; j <= h; j++)
    {
      p = _file[j][i]->get_piece();
      if (p)
      {
        switch (p->get_type())
        {
          case Pawn:
            if (p->get_color() == black)
              counter -= 1;
            else
              counter += 1;
            break;
          case Rook:
            if (p->get_color() == black)
              counter -= 5;
            else
              counter += 5;
            break;
          case Knight:
            if (p->get_color() == black)
              counter -= 3;
            else
              counter += 3;
            break;
          case Bishop:
            if (p->get_color() == black)
              counter -= 3;
            else
              counter += 3;
            break;
          case Queen:
            if (p->get_color() == black)
              counter -= 9;
            else
              counter += 9;
            break;

          case King:
            break;
          default:
            cerr << "Error: Undefined Piece Type: " << p->get_type() << endl;
        }
      }
    }
  }
  if (ot == debug)
    cout << endl << "Material " << counter * weight << endl;
  sum += counter * weight;
}

void Board::count_center_control(float& sum, float weight, col col_to_move, output_type ot) const
{
  int counter = 0;
  counter += _file[d][4]->count_controls();
  counter += _file[d][5]->count_controls();
  counter += _file[e][4]->count_controls();
  counter += _file[e][5]->count_controls();
  if (ot == debug)
    cout << "Center control " << counter * weight << endl;
  sum += counter * weight;
}

void Board::count_pawns_in_centre(float& sum, float weight, col col_to_move, output_type ot) const
{
  int counter = 0;
  if (_file[d][4]->contains_piece(white, Pawn))
    counter++;
  if (_file[d][5]->contains_piece(white, Pawn))
    counter++;
  if (_file[e][4]->contains_piece(white, Pawn))
    counter++;
  if (_file[e][5]->contains_piece(white, Pawn))
    counter++;
  if (_file[d][4]->contains_piece(black, Pawn))
    counter--;
  if (_file[d][5]->contains_piece(black, Pawn))
    counter--;
  if (_file[e][4]->contains_piece(black, Pawn))
    counter--;
  if (_file[e][5]->contains_piece(black, Pawn))
    counter--;
  if (ot == debug)
    cout << "Pawns in centre " << counter * weight << endl;
  sum += counter * weight;
}

//void Board::count_possible_moves(float& sum, float weight, col col_to_move) const
//{
//  weight = (col_to_move == white) ? weight : -weight;
//  sum += weight * _possible_moves.cardinal();
//}

void Board::count_development(float& sum, float weight, col col_to_move, output_type ot) const
{
  int counter = 0;
  if (!_file[a][1]->contains_piece(white, Rook))
    counter++;
  if (!_file[b][1]->contains_piece(white, Knight))
    counter++;
  if (!_file[c][1]->contains_piece(white, Bishop))
    counter++;
  if (!_file[d][1]->contains_piece(white, Queen))
    counter++;
  // forget the king
  if (!_file[f][1]->contains_piece(white, Bishop))
    counter++;
  if (!_file[g][1]->contains_piece(white, Knight))
    counter++;
  if (!_file[h][1]->contains_piece(white, Rook))
    counter++;

  if (!_file[a][8]->contains_piece(black, Rook))
    counter--;
  if (!_file[b][8]->contains_piece(black, Knight))
    counter--;
  if (!_file[c][8]->contains_piece(black, Bishop))
    counter--;
  if (!_file[d][8]->contains_piece(black, Queen))
    counter--;
  if (!_file[f][8]->contains_piece(black, Bishop))
    counter--;
  if (!_file[g][8]->contains_piece(black, Knight))
    counter--;
  if (!_file[h][8]->contains_piece(black, Rook))
    counter--;
  if (ot == debug)
    cout << "Development " << counter * weight << endl;
  sum += counter * weight;
}

void Board::count_castling(float& sum, float weight, col col_to_move, output_type ot) const
{
  int counter = 0;
  if (_castling_state.has_castled(white))
    counter++;
  if (_castling_state.has_castled(black))
    counter--;
  if (ot == debug)
    cout << "Castling " << counter * weight << endl;
  sum += counter * weight;
}

bool Board::is_end_node() const
{
  if (_possible_moves.cardinal() == 0)
    return true;
  else
    return false;
}

float Board::max(int level, int move_no, float alpha, float beta, int& best_move_index, const int& max_search_level, bool use_pruning) const
{
  float max_value = -101.0; // Must be lower than lowest evaluation
  int dummy_index;
  best_move_index = -1;
  level++;
  if (is_end_node() || level == max_search_level)
  {
    return evaluate_position(white, silent, level);
  }
  else
  {
    for (int i = 0; i < _possible_moves.cardinal(); i++)
    {
      Board::level_boards[level] = *this;
      //if (level_boards[level].init(white) != 0)
      //{
      //  cout << "Error in " __FILE__ << ":" << __LINE__ << endl;
      //  return -100.0;
      //}
      //level_boards[level].calculate_moves(white);
      level_boards[level].make_move(i, move_no, white, true);

      float tmp_value = level_boards[level].min(level, move_no, alpha, beta, dummy_index, max_search_level, use_pruning);
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
    }
    return max_value;
  }
}

float Board::min(int level, int move_no, float alpha, float beta, int& best_move_index, const int& max_search_level, bool use_pruning) const
{
  float min_value = 101.0; // Must be higher than highest evaluation
  int dummy_index;

  best_move_index = -1;
  level++;
  if (is_end_node() || level == max_search_level)
  {
    return evaluate_position(black, silent, level);
  }
  else
  {
    for (int i = 0; i < _possible_moves.cardinal(); i++)
    {
      level_boards[level] = *this;
      //if (level_boards[level].init(black) != 0)
      //{
      //  cout << "Error in " << __FILE__ << ":" << __LINE__ << endl;
      //  return -100.0;
      //}
      //level_boards[level].calculate_moves(black);
      level_boards[level].make_move(i, move_no, black, true);
      float tmp_value = level_boards[level].max(level, move_no, alpha, beta, dummy_index, max_search_level, use_pruning);
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
    }
    return min_value;
  }
}
}

