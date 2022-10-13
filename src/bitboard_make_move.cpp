/*
 * Bitboard_make_move.cpp
 *
 *  Created on: Nov 3, 2021
 *      Author: torsten
 */

#include "bitboard.hpp"
#include "chesstypes.hpp"
#include "game_history.hpp"
#include "transposition_table.hpp"

namespace C2_chess
{

Transposition_table Bitboard::transposition_table;

Piecetype Bitboard::get_piece_type(uint64_t square) const
{
  if ((square & _all_pieces) == zero)
    return Piecetype::Undefined;

  auto& pieces = (_white_pieces.pieces & square) ? _white_pieces : _black_pieces;
  if (pieces.Pawns & square)
  {
    return Piecetype::Pawn;
  }
  else if (pieces.Queens & square)
  {
    return Piecetype::Queen;
  }
  else if (pieces.Rooks & square)
  {
    return Piecetype::Rook;
  }
  else if (pieces.Bishops & square)
  {
    return Piecetype::Bishop;
  }
  else if (pieces.Knights & square)
  {
    return Piecetype::Knight;
  }
  else if (pieces.King & square)
  {
    return Piecetype::King;
  }
  std::cerr << "Error: Unknown Piecetype." << std::endl;
  return Piecetype::Undefined;
}

inline void Bitboard::remove_taken_piece(const uint64_t square, const Color piece_color)
{
  assert(std::has_single_bit(square));

  // The reference "&" is very important here!
  // Otherwise we'll only update a copied instance.
  auto& pieces = (piece_color == Color::White) ? _white_pieces : _black_pieces;
  auto piecetype = Piecetype::Undefined;

  if (pieces.Pawns & square)
  {
    pieces.Pawns ^= square, piecetype = Piecetype::Pawn;
  }
  else if (pieces.Rooks & square)
  {
    pieces.Rooks ^= square, piecetype = Piecetype::Rook;
    // update castling rights if needed
    switch (square)
    {
      case h8_square:
        remove_castling_right(castling_right_BK);
        break;
      case a8_square:
        remove_castling_right(castling_right_BQ);
        break;
      case h1_square:
        remove_castling_right(castling_right_WK);
        break;
      case a1_square:
        remove_castling_right(castling_right_WQ);
        break;
      default:
        ;
    }
  }
  else if (pieces.Bishops & square)
  {
    pieces.Bishops ^= square, piecetype = Piecetype::Bishop;
  }
  else if (pieces.Knights & square)
  {
    pieces.Knights ^= square, piecetype = Piecetype::Knight;
  }
  else if (pieces.Queens & square)
  {
    pieces.Queens ^= square, piecetype = Piecetype::Queen;
  }
  assert(piecetype != Piecetype::Undefined);
  update_hash_tag(square, piece_color, piecetype);
  _material_diff += (piece_color == Color::White) ? -piece_values[index(piecetype)] : piece_values[index(piecetype)];
}

inline void Bitboard::place_piece(Piecetype p_type, const uint64_t square, Color color)
{
  auto pieces = (color == Color::White) ? _white_pieces : _black_pieces;
  switch (p_type)
  {

    case Piecetype::Pawn:
      pieces.Pawns |= square;
      break;
    case Piecetype::Queen:
      pieces.Queens |= square;
      break;
    case Piecetype::King:
      pieces.King |= square;
      break;
    case Piecetype::Rook:
      pieces.Rooks |= square;
      break;
    case Piecetype::Knight:
      pieces.Knights |= square;
      break;
    case Piecetype::Bishop:
      pieces.Bishops |= square;
      break;
    default:
      ;
  }
}

// Move the piece and update the hash_tag.
inline void Bitboard::move_piece(uint64_t from_square, uint64_t to_square, Piecetype p_type)
{
  Bitpieces& pieces = (_side_to_move == Color::White) ? _white_pieces : _black_pieces;
  switch (p_type)
  {
    case Piecetype::Pawn:
      pieces.Pawns ^= (from_square | to_square);
      break;
    case Piecetype::Queen:
      pieces.Queens ^= (from_square | to_square);
      break;
    case Piecetype::King:
      pieces.King ^= (from_square | to_square);
      break;
    case Piecetype::Rook:
      pieces.Rooks ^= (from_square | to_square);
      break;
    case Piecetype::Knight:
      pieces.Knights ^= (from_square | to_square);
      break;
    case Piecetype::Bishop:
      pieces.Bishops ^= (from_square | to_square);
      break;
    default:
      ;
  }
  update_hash_tag(from_square, to_square, _side_to_move, p_type);
}

// Preconditions: Remove One castling_right at the time
inline void Bitboard::remove_castling_right(uint8_t cr)
{
  if (_castling_rights & cr)
  {
    _castling_rights ^= cr;
    _hash_tag ^= transposition_table._castling_rights[cr];
  }
}

inline void Bitboard::clear_ep_square()
{
  _hash_tag ^= transposition_table._en_passant_file[file_idx(_ep_square)];
  _ep_square = zero;
}

inline void Bitboard::set_ep_square(uint64_t ep_square)
{
  assert(std::has_single_bit(ep_square));
  _ep_square = ep_square;
  _hash_tag ^= transposition_table._en_passant_file[file_idx(_ep_square)];
}

// Set a "unique" hash tag for the position after
// adding or removing one piece from a square.
void Bitboard::update_hash_tag(uint64_t square, Color p_color, Piecetype p_type)
{
  assert(std::has_single_bit(square));
  _hash_tag ^= transposition_table._random_table[bit_idx(square)][index(p_color)][index(p_type)];
}

// Set a "unique" hash tag for the position after
// removing one piece from a square and putting
// the same piece on an empty square.
inline void Bitboard::update_hash_tag(uint64_t square1, uint64_t square2, Color p_color, Piecetype p_type)
{
  _hash_tag ^= transposition_table._random_table[bit_idx(square1)][index(p_color)][index(p_type)];
  _hash_tag ^= transposition_table._random_table[bit_idx(square2)][index(p_color)][index(p_type)];
}

void Bitboard::update_side_to_move()
{
  _side_to_move = other_color(_side_to_move);
  if (_side_to_move == Color::White)
  {
    _own = &_white_pieces;
    _other = &_black_pieces;
  }
  else
  {
    _own = &_black_pieces;
    _other = &_white_pieces;
  }
  _hash_tag ^= transposition_table._toggle_side_to_move;
}

inline void Bitboard::update_state_after_king_move(const Bitmove& m)
{
  uint64_t from_square = m.from();
  uint64_t to_square = m.to();

  if (from_square & e1_square) // TODO: Kan inte fel sidas kung flytta från rutan?
  {
    remove_castling_right(castling_right_WK);
    remove_castling_right(castling_right_WQ);
  }
  else if (from_square & e8_square)
  {
    remove_castling_right(castling_right_BK);
    remove_castling_right(castling_right_BQ);
  }
  // Move the rook if it's actually a castling move.
  if (m.properties() & move_props_castling)
  {
    if (from_square > to_square)
    {
      // Castling king side.
      if (_side_to_move == Color::White)
      {
        _white_pieces.Rooks ^= (h1_square | f1_square);
        update_hash_tag(h1_square, f1_square, _side_to_move, Piecetype::Rook);
        _has_castled[index(Color::White)] = true;
      }
      else
      {
        _black_pieces.Rooks ^= (h8_square | f8_square);
        update_hash_tag(h8_square, f8_square, _side_to_move, Piecetype::Rook);
        _has_castled[index(Color::Black)] = true;
      }
    }
    else
    {
      // Castling queen side.
      if (_side_to_move == Color::White)
      {
        _white_pieces.Rooks ^= (a1_square | d1_square);
        update_hash_tag(a1_square, d1_square, _side_to_move, Piecetype::Rook);
        _has_castled[index(Color::White)] = true;
      }
      else
      {
        _black_pieces.Rooks ^= (a8_square | d8_square);
        update_hash_tag(a8_square, d8_square, _side_to_move, Piecetype::Rook);
        _has_castled[index(Color::Black)] = true;
      }
    }
  }
}

void Bitboard::make_move(list_ref movelist, uint8_t i, Takeback_state& tb_state, Gentype gt, bool add_to_history)
{
  assert(i < movelist.size());
  make_move(movelist, movelist[i], tb_state, gt, add_to_history);
}

// The move must be valid, but doesn't have to be in _movelist.
// _movelist may be empty, not generated yet.
void Bitboard::make_move(list_ref next_movelist, const Bitmove& m, Takeback_state& tb_state, Gentype gt, bool add_to_history)
{
  assert((_own->pieces & _other->pieces) == zero);
  uint64_t to_square = m.to();
  uint64_t from_square = m.from();

  // Save some takeback values
  save_in_takeback_state(tb_state, get_piece_type(to_square));

  // Clear _ep_square
  uint64_t tmp_ep_square = _ep_square;
  if (_ep_square)
    clear_ep_square();

  // Remove possible piece of other color on to_square and
  // Then make the move (updates hashtag)
  if (to_square & _other->pieces)
    remove_taken_piece(to_square, other_color(_side_to_move));
  move_piece(from_square, to_square, m.piece_type()); // updates hash-tag

// OK we have moved the piece, now we must
// look at some special cases.
  if (m.piece_type() == Piecetype::King)
  {
    update_state_after_king_move(m);
  }
  else if (m.piece_type() == Piecetype::Rook)
  {
    // Clear castling rights for one side if applicable.
    if (to_square & _white_pieces.Rooks)
    {
      if (from_square & a1_square)
        remove_castling_right(castling_right_WQ);
      else if (from_square & h1_square)
        remove_castling_right(castling_right_WK);
    }
    else
    {
      // Black rook
      if (from_square & a8_square)
        remove_castling_right(castling_right_BQ);
      else if (from_square & h8_square)
        remove_castling_right(castling_right_BK);
    }
  }
  else if (m.piece_type() == Piecetype::Pawn)
  {
    if (m.properties() & move_props_promotion)
    {
      // Remove the pawn from promotion square
      // subtract 1 from the normal piece-values
      // because the pawn disappears from the board.
      (_side_to_move == Color::White) ? _white_pieces.Pawns ^= to_square : _black_pieces.Pawns ^= to_square;
      update_hash_tag(to_square, _side_to_move, Piecetype::Pawn);
      switch (m.promotion_piece_type())
      {
        case Piecetype::Queen:
          (_side_to_move == Color::White) ? (_material_diff += 8.0, _white_pieces.Queens |= to_square) : (_material_diff -= 8.0, _black_pieces.Queens |= to_square);
          update_hash_tag(to_square, _side_to_move, Piecetype::Queen);
          break;
        case Piecetype::Rook:
          (_side_to_move == Color::White) ? (_material_diff += 4.0, _white_pieces.Rooks |= to_square) : (_material_diff -= 4.0, _black_pieces.Rooks |= to_square);
          update_hash_tag(to_square, _side_to_move, Piecetype::Rook);
          break;
        case Piecetype::Knight:
          (_side_to_move == Color::White) ? (_material_diff += 2.0, _white_pieces.Knights |= to_square) : (_material_diff -= 2.0, _black_pieces.Knights |= to_square);
          update_hash_tag(to_square, _side_to_move, Piecetype::Knight);
          break;
        case Piecetype::Bishop:
          (_side_to_move == Color::White) ? (_material_diff += 2.0, _white_pieces.Bishops |= to_square) : (_material_diff -= 2.0, _black_pieces.Bishops |= to_square);
          update_hash_tag(to_square, _side_to_move, Piecetype::Bishop);
          break;
        default:
          ;
      }
    }
    else if (m.properties() & move_props_en_passant)
    {
      // Remove the pawn taken e.p.
      if (_side_to_move == Color::White)
      {
        _black_pieces.Pawns ^= tmp_ep_square << 8, _material_diff += 1.0;
        update_hash_tag(tmp_ep_square << 8, Color::Black, Piecetype::Pawn);
      }
      else
      {
        _white_pieces.Pawns ^= tmp_ep_square >> 8, _material_diff -= 1.0;
        update_hash_tag(tmp_ep_square >> 8, Color::White, Piecetype::Pawn);
      }
    }
    else if ((_side_to_move == Color::White) && (from_square & row_2) && (to_square & row_4))
    {
      // Check if there is a pawn of other color alongside to_square.
      if (((to_square & not_a_file) && ((to_square << 1) & _other->Pawns)) || ((to_square & not_h_file) && ((to_square >> 1) & _other->Pawns)))
        set_ep_square(to_square << 8);
    }
    else if ((_side_to_move == Color::Black) && (from_square & row_7) && (to_square & row_5))
    {
      if (((to_square & not_a_file) && ((to_square << 1) & _other->Pawns)) || ((to_square & not_h_file) && ((to_square >> 1) & _other->Pawns)))
        set_ep_square(to_square >> 8);
    }
  }
  // Set up the board for other player:
  _latest_move = m;
  update_side_to_move();
  if (_side_to_move == Color::White)
    _move_number++;
  if (square_is_threatened(_own->King, no_xray_threats_through_king))
    _latest_move.add_property(move_props_check);
  update_half_move_counter();
  // add_position to game_history
  if (add_to_history)
  {
    history.add_position(_hash_tag);
  }
  find_legal_moves(next_movelist, gt);
}

void Bitboard::takeback_en_passant(const Bitmove& m, const Color moving_side)
{
  // Keeping the definition of to_square and from_square from the move,
  // even if the move will be reversed.
  // _other points to the pieces of the moving side.
  const auto to_square = m.to();
  const auto taken_pawn_square = (moving_side == Color::White) ? to_square << 8 : to_square >> 8;
  set_ep_square(to_square);
  // Put the pawns back.
  _other->Pawns ^= (to_square | m.from()); // moving side
  _own->Pawns |= taken_pawn_square;
  // Update the resulting material diff
  _material_diff += (moving_side == Color::White) ? -pawn_value : pawn_value;
}

void Bitboard::takeback_promotion(const Bitmove& m, const Color moving_side, const Piecetype taken_piece_t)
{
  // Keeping the definition of to_square and from_square from the move,
  // even if the move will be reversed.
  // _other points to the pieces of the moving side.
  const auto to_square = m.to();
  const auto from_square = m.from();

  // Put back the Pawn that has promoted:
  _other->Pawns |= from_square;
  // Remove the promoted piece:
  switch (m.promotion_piece_type())
  {
    case Piecetype::Queen:
      _other->Queens ^= to_square;
      break;
    case Piecetype::Rook:
      _other->Rooks ^= to_square;
      break;
    case Piecetype::Knight:
      _other->Knights ^= to_square;
      break;
    case Piecetype::Bishop:
      _other->Bishops ^= to_square;
      break;
    default:
      ;
  }
  if (taken_piece_t != Piecetype::Undefined)
  {
    switch (taken_piece_t)
    {
      case Piecetype::Queen:
        _own->Queens ^= to_square;
        break;
      case Piecetype::Rook:
        _own->Rooks ^= to_square;
        break;
      case Piecetype::Knight:
        _own->Knights ^= to_square;
        break;
      case Piecetype::Bishop:
        _own->Bishops ^= to_square;
        break;
      case Piecetype::Pawn:
        _own->Pawns ^= to_square;
        break;
      default:
        ;
    }
  }
  // Update the resulting material diff
  float material_diff_w = pawn_value - piece_values[index(m.promotion_piece_type())] - piece_values[index(taken_piece_t)];
  _material_diff += (moving_side == Color::White) ? material_diff_w : -material_diff_w;
}

void Bitboard::takeback_castling(const Bitmove& m, const Color moving_side)
{
  uint64_t rook_squares;
  auto king_squares = (moving_side == Color::White) ? (e1_square | m.to()) : (e8_square | m.to());
  if (file_idx(m.to()) == g)
  {
    rook_squares = (moving_side == Color::White) ? (f1_square | h1_square) : (f8_square | h8_square);
  }
  else
  {
    rook_squares = (moving_side == Color::White) ? (d1_square | a1_square) : (d8_square | a8_square);
  }
  _other->King ^= king_squares;
  _other->Rooks ^= rook_squares;
  // No change in material_diff.
}

void Bitboard::take_back_latest_move()
{
  takeback_latest_move(*get_movelist(0), Gentype::All, takeback_list[0].state_S);
}

void Bitboard::takeback_normal_move(const Bitmove& m, const Color moving_side, const Piecetype taken_piece_type)
{
  // Move the piece back:
  auto piece_squares = m.to() | m.from();
  switch (m.piece_type())
  {
    case Piecetype::Queen:
      _other->Queens ^= piece_squares;
      break;
    case Piecetype::Rook:
      _other->Rooks ^= piece_squares;
      break;
    case Piecetype::Knight:
      _other->Knights ^= piece_squares;
      break;
    case Piecetype::Bishop:
      _other->Bishops ^= piece_squares;
      break;
    case Piecetype::Pawn:
      _other->Pawns ^= piece_squares;
      break;
    case Piecetype::King:
      _other->King ^= piece_squares;
      break;
    default:
      ;
  }
  // Put back the taken piece:
  if (taken_piece_type != Piecetype::Undefined)
  {
    assert(m.properties() & move_props_capture);
    switch (taken_piece_type)
    {
      case Piecetype::Queen:
        _own->Queens ^= m.to();
        break;
      case Piecetype::Rook:
        _own->Rooks ^= m.to();
        break;
      case Piecetype::Knight:
        _own->Knights ^= m.to();
        break;
      case Piecetype::Bishop:
        _own->Bishops ^= m.to();
        break;
      case Piecetype::Pawn:
        _own->Pawns ^= m.to();
        break;
      default:
        assert(false);
    }
  }
  auto material_diff_w = -piece_values[index(taken_piece_type)];
  _material_diff += (moving_side == Color::White)? material_diff_w : -material_diff_w;
}

void Bitboard::takeback_latest_move(list_ref movelist, Gentype gt, const Takeback_state& tb_state, const bool takeback_from_history)
{
  assert((_own->pieces & _other->pieces) == zero);

  // Keeping the definition of to_square and from_square from the move,
  // even if the move will be reversed.
  // _side_to_move is the side that didn't make the move.

  //  std::cout << "takeback_move: " << m << std::endl << to_binary_board(from_square) << std::endl << to_binary_board(to_square) << std::endl <<
  //  to_binary_board(_other->pieces) << std::endl;

  auto m = _latest_move;
  if (m.properties() & move_props_en_passant)
  {
    takeback_en_passant(m, other_color(_side_to_move));
  }
  else if (m.properties() & move_props_castling)
  {
    takeback_castling(m, other_color(_side_to_move));
  }
  else if (m.properties() & move_props_promotion)
  {
    takeback_promotion(m, other_color(_side_to_move), tb_state._taken_piece_type);
  }
  else
  {
    takeback_normal_move(m, other_color(_side_to_move), tb_state._taken_piece_type);
  }

  //  Set up the board for other player:
  update_side_to_move();
  if (_side_to_move == Color::Black)
    _move_number--;
  if (takeback_from_history)
  {
    history.takeback_latest_move();
  }

  takeback_from_state(tb_state);
  _hash_tag = tb_state._hash_tag;
  _latest_move = tb_state._latest_move;
  _castling_rights = tb_state._castling_rights;
  _half_move_counter = tb_state._half_move_counter;
  _has_castled[index(Color::White)] = tb_state._has_castled_w;
  _has_castled[index(Color::Black)] = tb_state._has_castled_b;
  // TODO: Can movelist be taken from takeback_state(search-ply - 1)?
  find_legal_moves(movelist, gt);
}

} // namespace C2_chess

