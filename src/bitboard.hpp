/*

 * Bitboard.hpp
 *
 *  Created on: Apr 3, 2021
 *      Author: torsten
 */

#ifndef __BITBOARD
#define __BITBOARD

#include <cstdint>
#include <cstring>
#include <cmath>
#include <cassert>
#include <ostream>
#include <sstream>
#include <bit>
#include <bitset>
#include <deque>
#include <atomic>
#include "chesstypes.hpp"
#include "chessfuncs.hpp"
#include "transposition_table.hpp"
#include "game_history.hpp"
#include "shared_ostream.hpp"

namespace C2_chess
{

class Bitboard
{
  protected:
    // Static declarations, incomplete type.
    static Transposition_table transposition_table;
    static Game_history history;
    static Bitboard search_boards[];
    static struct Takeback_element takeback_list[];
    static std::atomic<bool> time_left;

    uint64_t _hash_tag;
    Color _side_to_move = Color::White;
    uint16_t _move_number;
    uint8_t _castling_rights = castling_rights_none;
    bool _has_castled[2];
    uint64_t _ep_square = zero;
    float _material_diff;
    Bitmove _last_move;
    uint64_t _checkers;
    uint64_t _pinners;
    uint64_t _pinned_pieces;
    uint64_t _all_pieces;
    Bitpieces _white_pieces;
    Bitpieces _black_pieces;
    Bitpieces* _own;
    Bitpieces* _other;
    uint8_t _half_move_counter;

    // ### Protected basic Bitboard_functions ###
    // ------------------------------------------

    void init_piece_state();

    inline void sort_moves(std::deque<Bitmove>& movelist) const;

    inline void add_move(std::deque<Bitmove>& movelist,
                         Piecetype p_type,
                         uint16_t move_props,
                         uint64_t from_square,
                         uint64_t to_square,
                         Piecetype promotion_p_type = Piecetype::Queen) const;

    bool is_in_movelist(std::deque<Bitmove>& movelist, const Bitmove& m) const;

    inline float get_piece_value(Piecetype p_type) const;

    inline float get_piece_value(uint64_t square) const;

    bool is_empty_square(uint64_t square) const
    {
      return square & ~_all_pieces;
    }

    bool is_not_pinned(uint64_t square) const
    {
      return square & ~_pinned_pieces;
    }

    bool is_promotion_square(uint64_t square) const
    {
      return (_side_to_move == Color::White) ? square & row_8 : square & row_1;
    }

    // ### Protected Methods for move-generation
    // -----------------------------------------
    void find_long_castling(std::deque<Bitmove>& moelist) const;

    void find_short_castling(std::deque<Bitmove>& movelist) const;

    inline uint64_t find_blockers(uint64_t sq, uint64_t mask, uint64_t all_pieces) const;

    inline uint64_t find_other_color_blockers(uint64_t sq, uint64_t mask) const;

    uint64_t find_legal_squares(uint64_t sq, uint64_t mask) const;

    void find_Queen_Rook_and_Bishop_moves(std::deque<Bitmove>& movelist, Gentype gt) const;

    void find_legal_moves_for_pinned_pieces(std::deque<Bitmove>& movelist, Gentype gt) const;

    void find_Knight_moves(std::deque<Bitmove>& movelist, Gentype gt) const;

    void find_Pawn_moves(std::deque<Bitmove>& movelist, Gentype gt) const;

    void find_normal_legal_moves(std::deque<Bitmove>& movelist, Gentype gt) const;

    void find_Knight_moves_to_square(std::deque<Bitmove>& movelist, const uint64_t to_square) const;

    bool check_if_other_pawn_is_pinned_ep(uint64_t other_pawn_square, uint64_t own_pawn_square) const;

    void try_adding_ep_pawn_move(std::deque<Bitmove>& movelist, uint64_t from_square) const;

    void add_pawn_move_check_promotion(std::deque<Bitmove>& movelist, uint64_t from_square, uint64_t to_square) const;

    void find_pawn_moves_to_empty_square(std::deque<Bitmove>& movelist, uint64_t to_square, Gentype gt) const;

    void find_moves_to_square(std::deque<Bitmove>& movelist, uint64_t to_square, Gentype gt) const;

    void find_moves_after_check(std::deque<Bitmove>& movelist, Gentype gt) const;

    void find_checkers_and_pinned_pieces();

    bool square_is_threatened(uint64_t square, bool King_is_asking) const;

    bool square_is_threatened2(uint64_t to_square, bool King_is_asking) const;

    inline void find_king_moves(std::deque<Bitmove>& movelist, Gentype gt) const;

    // ### Protected methods for making a move ###
    // ---------------------------------------

    inline void add_promotion_piece(Piecetype p_type);

    //inline void touch_piece(const uint64_t square, const Color color);

    inline void remove_taken_piece(const uint64_t square, const Color color);

    //inline void remove_pawn(const uint64_t square, const Color color);

    inline void move_piece(uint64_t from_square,
                           uint64_t to_square,
                           Piecetype p_type);

    inline void remove_castling_right(uint8_t cr);

    inline void place_piece(Piecetype p_type, const uint64_t square, Color color);

    inline void clear_ep_square();

    inline void set_ep_square(uint64_t ep_square);

    void update_hash_tag(uint64_t square, Color p_color, Piecetype p_type);

    inline void update_hash_tag(uint64_t square1, uint64_t square2, Color p_color, Piecetype type);

    void update_side_to_move();

    inline void update_state_after_king_move(const Bitmove& m);

    void takeback_en_passant(const Bitmove& m, const Color moving_side);

    void takeback_castling(const Bitmove& m, const Color moving_side);

    void takeback_from_state(Takeback_state& state);

    // ### Protected methods for searching for the best move ###
    // ---------------------------------------------------------

    void count_pawns_in_centre(float& sum, float weight) const;

    void count_castling(float& sum, float weight) const;

    void count_development(float& sum, float weight) const;

    void count_center_control(float& sum, float weight) const;

    int count_threats_to_square(uint64_t square, Color side) const;

    float evaluate_position(const bool movelist_is_empty, Color for_side, uint8_t search_ply, bool evaluate_zero_moves = true) const;

    void get_pv_line(std::vector<Bitmove>& pv_line) const;

  public:

    Bitboard();
    Bitboard(const Bitboard& b);

    Bitboard& operator=(const Bitboard& from);

    void init_board_hash_tag();

    void init();

    // Clears things which could matter when reading a new position.
    void clear()
    {
      std::memset(_own, 0, sizeof(Bitpieces));
      std::memset(_other, 0, sizeof(Bitpieces));
      std::memset(_has_castled, 0, 2 * sizeof(bool));
      _ep_square = zero;
      _castling_rights = castling_rights_none;
      _material_diff = 0.0;
      //TODO: REMOVE movelist.clear();
    }

    // Reads the position from a text-string, with some
    // syntactic error-checking.
    // Returns -1 if the text-string wasn't accepted.
    int read_position(const std::string& FEN_string, const bool initialyse = false);

    // Puts all legal moves of the position in _movelist.
    // (Naturally only the moves for the player who's in
    // turn to make a move.)
    void find_legal_moves(std::deque<Bitmove>& movelist, Gentype gt);

    // ### Public methods for make move ###
    // ------------------------------------

    // Looks up the i:th move in movelist and makes it.
    // move_no just keeps track of the full move number
    // in the game.
    void make_move(std::deque<Bitmove>& movelist, uint8_t i, Gentype gt = Gentype::All, bool update_history = true);

    // Only for the command-line interface, where the user
    // is prompted to enter the new move on the keyboard,
    // if he's on turn.
    int make_move(Playertype player_type);

    // This make_move() doesn't require a generated movelist.
    void make_move(std::deque<Bitmove>& next_movelist, const Bitmove& m, Gentype gt = Gentype::All, bool update_history = true);

    void take_back_move(std::deque<Bitmove>& movelist, const Bitmove& m, const Gentype gt, const bool add_to_history = true);

    // All properties of a move are not decided immediately,
    // but some of them (check for instance) are set after the
    // move has been made and the position has been initialized
    // for the other player. So we save the move, until then,
    // in _last_move. These move-properties are mostly for the
    // presentation of the move in text, and unnecessary for
    // other purposes.
    Bitmove last_move() const
    {
      return _last_move;
    }

    void set_mate()
    {
      _last_move.add_property(move_props_mate);
    }

    void set_stalemate()
    {
      _last_move.add_property(move_props_stalemate);
    }

    void set_draw_by_repetition()
    {
      _last_move.add_property(move_props_draw_by_repetition);
    }

    void set_draw_by_50_moves()
    {
      _last_move.add_property(move_props_draw_by_50_moves);
    }

    // The chess-engine keeps track of the game, which is
    // not necessary, but nice to see in the log-files.
    // For guessing the opponents last move (from the
    // FEN-string in the "set position" UCI-command) we need
    // to compare two positions and the following methods
    // came in handy.
    uint64_t all_pieces() const
    {
      return _all_pieces;
    }

    Bitpieces* other() const
    {
      return _other;
    }

    Bitpieces* own() const
    {
      return _own;
    }

    // A slow way of determining the piecetype of a piece on
    // given square. Should not be used inside the search-loop.
    Piecetype get_piece_type(uint64_t square) const;

    uint16_t get_move_number() const
    {
      return _move_number;
    }

    void set_move_number(uint16_t move_number)
    {
      _move_number = move_number;
    }

    // ### Public methods for the search of best move. ###
    // ---------------------------------------------------

    void set_time_left(bool value);

    static void start_timer(double time);

    void start_timer_thread(const double time);

    bool has_time_left();

    void clear_transposition_table(map_tag map = map_tag::Current);

    void switch_tt_tables();

    void clear_PV_table();

    //TODO: REMOVE inline void clear_movelist(std::deque<Bitmove>& movelist);

    void update_half_move_counter();

    void set_half_move_counter(uint8_t half_move_counter);

    uint8_t get_half_move_counter() const
    {
      return _half_move_counter;
    }

    Color get_side_to_move() const
    {
      return _side_to_move;
    }

    inline std::deque<Bitmove>* get_movelist(size_t idx) const
    {
      return takeback_list[idx].state_S.movelist;
    }

    inline std::deque<Bitmove>* get_movelist_Q(size_t idx) const
    {
      return takeback_list[idx].state_Q.movelist;
    }

    bool is_draw_by_50_moves() const;

    void add_position_to_game_history()
    {
      history.add_position(_hash_tag);
    }

    inline bool is_threefold_repetition() const
    {
      return history.is_threefold_repetition();
    }

    void init_material_evaluation();

    History_state get_history_state() const
    {
      return history.get_state();
    }

    float Quiesence_search(uint8_t search_ply, float alpha, float beta, uint8_t max_search_ply);

    // Search function
    float negamax_with_pruning(uint8_t level, float alpha, float beta, Bitmove& best_move, const uint8_t max_search_ply);

    std::ostream& write_piece_diagram_style(std::ostream& os, C2_chess::Piecetype p_type, C2_chess::Color side) const;
    std::ostream& write_piece(std::ostream& os, uint64_t square) const;
    std::ostream& write(std::ostream& os, const Color from_perspective) const;
    std::ostream& write_movelist( const std::deque<Bitmove>& movelist, std::ostream& os, bool same_line = false) const;
    Shared_ostream& write_search_info(Shared_ostream& logfile) const;

    void clear_search_info();
    Search_info& get_search_info() const;
    unsigned int perft_test(uint8_t search_ply, uint8_t max_search_plies) const;

    static void init_search_boards();
};

} // namespace C2_chess
#endif
