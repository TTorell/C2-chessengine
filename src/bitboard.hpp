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
#include <functional>
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
    // Static declarations

    // transposition_table is a hash-table storing evaluations and best-moves for
    // already searched positions during a search-operation, so the evaluation of
    // a position doesn't have to be recalculated if it appears again during the
    // search. The information is also used to retrieve the PV-moves used in the
    // move-ordering, essential for the pruning of the "search-tree".
    static Transposition_table transposition_table;

    // history stores the move-history in the game as well as during a search-operation.
    // Needed for instance to decide "threefold-draw".
    static Game_history history;

    // search_cash keeps track of moves which have increased alpha during the search.
    // The information is used during move-ordering.
    static int alpha_move_cash[2][7][64]; // [color][piecetype][square_index]

    // Thread-safe boolean, telling if there's more time to think
    // or if a move must be returned as fast as possible.
    static std::atomic<bool> time_left;

    uint64_t _hash_tag; // Unique Zobrist hash-tag of the position.
    Color _side_to_move = Color::White;
    uint16_t _move_number;
    uint8_t _castling_rights = castling_rights_none;
    bool _has_castled[2];
    uint64_t _ep_square = zero; // Square to which an en passant move is possible.
    float _material_diff; // Holds the material evaluation of the position.
    Bitmove _latest_move;

    uint8_t _search_ply;
    uint64_t _checkers;
    uint64_t _pinners;
    uint64_t _pinned_pieces;
    Bitmove _previous_search_best_move;
    Bitmove _beta_killers[2][N_SEARCH_PLIES_DEFAULT];

    uint64_t _all_pieces;
    Bitpieces _white_pieces;
    Bitpieces _black_pieces;
    Bitpieces* _own;
    Bitpieces* _other;
    uint8_t _half_move_counter;

    // ### Protected basic Bitboard_functions ###
    // ------------------------------------------
    void assemble_pieces();

    void init_piece_state();

    void add_position_to_game_history()
    {
      history.add_position(_hash_tag);
    }

    inline void sort_moves(list_ref movelist) const;

    inline void add_move(list_ref movelist, Piecetype p_type, Piecetype capture_p_type, uint16_t move_props, uint64_t from_square, uint64_t to_square, Piecetype promotion_p_type = Piecetype::Queen) const;

    bool is_in_movelist(list_ref movelist, const Bitmove& m) const;

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
    void find_long_castling(list_ref moelist) const;

    void find_short_castling(list_ref movelist) const;

    inline uint64_t find_blockers(uint64_t sq, uint64_t mask, uint64_t all_pieces) const;

    inline uint64_t find_other_color_blockers(uint64_t sq, uint64_t mask) const;

    uint64_t find_legal_squares(uint64_t sq, uint64_t mask) const;

    void find_Queen_Rook_and_Bishop_moves(list_ref movelist, Gentype gt) const;

    void find_legal_moves_for_pinned_pieces(list_ref movelist, Gentype gt) const;

    void find_Knight_moves(list_ref movelist, Gentype gt) const;

    void find_Pawn_moves(list_ref movelist, Gentype gt) const;

    void find_normal_legal_moves(list_ref movelist, Gentype gt) const;

    void find_Knight_moves_to_square(list_ref movelist, const uint64_t to_square) const;

    bool check_if_other_pawn_is_pinned_ep(uint64_t other_pawn_square, uint64_t own_pawn_square) const;

    void try_adding_ep_pawn_move(list_ref movelist, uint64_t from_square) const;

    void add_pawn_move_check_promotion(list_ref movelist, uint64_t from_square, uint64_t to_square) const;

    void find_pawn_moves_to_empty_square(uint64_t to_square, Gentype gt);

    void find_moves_to_square(uint64_t to_square, Gentype gt);

    void find_moves_after_check(Gentype gt);

    void find_pawn_moves_to_empty_square(list_ref movelist, uint64_t to_square, Gentype gt) const;

    void find_moves_to_square(list_ref movelist, uint64_t to_square, Gentype gt) const;

    void find_moves_after_check(list_ref movelist, Gentype gt) const;

    void find_checkers_and_pinned_pieces();

    bool square_is_threatened(uint64_t square, bool King_is_asking) const;

    bool square_is_threatened2(uint64_t to_square, bool King_is_asking) const;

    inline void find_king_moves(list_ref movelist, Gentype gt) const;

    // ### Protected methods for making a move ###
    // ---------------------------------------

    //inline void touch_piece(const uint64_t square, const Color color);

    inline void remove_taken_piece(const uint64_t square, const Color color, const Piecetype piece_type);

    //inline void remove_pawn(const uint64_t square, const Color color);

    inline void move_piece(uint64_t from_square, uint64_t to_square, Piecetype p_type);

    inline void remove_castling_right(uint8_t cr);

    inline void place_piece(const uint64_t square, const Color color, const Piecetype p_type);

    inline void clear_ep_square();

    inline void set_ep_square(uint64_t ep_square);

    void update_hash_tag(uint64_t square, Color p_color, Piecetype p_type);

    inline void update_hash_tag(uint64_t square1, uint64_t square2, Color p_color, Piecetype type);

    void update_side_to_move();

    void update_half_move_counter();

    inline void update_state_after_king_move(const Bitmove& m);

    void takeback_promotion(const Bitmove& m, const Color moving_side);

    void takeback_en_passant(const Bitmove& m, const Color moving_side);

    void takeback_castling(const Bitmove& m, const Color moving_side);

    void takeback_normal_move(const Bitmove& m, const Color moving_side);

    void save_in_takeback_state(Takeback_state& tb_state) const;

    void takeback_from_state(const Takeback_state& state);

    void takeback_latest_move(const Takeback_state& tb_state, const bool takeback_from_history = true);

    // ### Protected methods for searching for the best move ###
    // ---------------------------------------------------------

    static void start_timer(double time);

    void count_pawns_in_centre(float& sum, float weight) const;

    void count_castling(float& sum, float weight) const;

    void count_development(float& sum, float weight) const;

    //int count_QRB_threats_to_center_square(uint64_t to_square, Color side) const;

    int count_threats_to_center_squares(const Bitpieces& pieces, const uint64_t pawn_pattern) const;

    int walk_into_center(const uint64_t from_sq, const int n_center_sqs, std::function<void(uint64_t&)> shift_func) const;

    //int count_QRB_threats_to_center_squares(const Bitpieces& pieces) const;

    //int count_threats_to_square(uint64_t square, Color side) const;

    float evaluate_empty_movelist(int search_ply) const;

    float evaluate_position() const;

    void clear_PV_table();

//    inline Takeback_state& get_takeback_state(size_t idx) const
//    {
//      return takeback_list[idx].state_S;
//    }
//
//    inline Takeback_state& get_takeback_state_Q(size_t idx) const
//    {
//      return takeback_list[idx].state_Q;
//    }

    float Quiesence_search(float alpha, float beta, uint8_t max_search_ply);

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
    }

    // Reads the position from a text-string, with some
    // syntactic error-checking.
    // Returns -1 if the text-string wasn't accepted.
    int read_position(const std::string& FEN_string, const bool initialyse = false);

    // Puts all legal moves of the position in _movelist.
    // (Naturally only the moves for the player who's in
    // turn to make a move.)
    void find_legal_moves(list_ref movelist, Gentype gt);

    // ### Public methods for make move ###
    // ------------------------------------

    // Looks up the i:th move in movelist and makes it.
    void make_move(const size_t i, Takeback_state& tb_state);

    // Only for the command-line interface, where the user
    // is prompted to enter the new move on the keyboard,
    // if he's on turn.
    int make_move(Playertype player_type);

//    // This make_move() doesn't require a generated movelist.
//    void make_move(const Bitmove& m, Takeback_state& tb_state, Takeback_state& next_tb_state, const Gentype gt, const bool add_to_history = true);

    void new_make_move(const Bitmove& m, Takeback_state& tb_state, const bool add_to_history = true);

    // All properties of a move are not decided immediately,
    // the "check" property is set after the move has been made
    // and the position has been initialized for the other player.
    // So we save the move, until then, in _latest_move.
    // The check-property is mostly for the presentation of the
    // move in text, and unnecessary for other purposes.
//    Bitmove get_latest_move() const
//    {
//      return _latest_move;
//    }

    // The chess-engine keeps track of the game, which is
    // not necessary, but nice to see in the log-files.
    // For guessing the opponents last move (from the
    // FEN-string in the "set position" UCI-command) we need
    // to compare two positions and the following methods
    // came in handy.
    uint64_t get_all_pieces() const
    {
      return _all_pieces;
    }

    Bitpieces* get_own_pieces() const
    {
      return _own;
    }

    Bitpieces* get_other_pieces() const
    {
      return _other;
    }

    uint16_t get_move_number() const
    {
      return _move_number;
    }

    inline uint8_t get_half_move_counter() const
    {
      return _half_move_counter;
    }

    // Probably a somewhat slow way of determining the piecetype of
    // a piece on given square. Should not be used inside the search-loop,
    // but that's hard to avoid.
    Piecetype get_piece_type(uint64_t square) const;

    // ### Public methods for the search of best move. ###
    // ---------------------------------------------------

    void get_pv_line(std::vector<Bitmove>& pv_line) const;

    void set_time_left(bool value);

    bool has_time_left();

    void start_timer_thread(const double time);

    void clear_transposition_table(map_tag map = map_tag::Current);

    void switch_tt_tables();

    inline Color get_side_to_move() const
    {
      return _side_to_move;
    }

    bool is_draw_by_50_moves() const;

    inline bool is_threefold_repetition() const
    {
      return history.is_threefold_repetition();
    }

    void init_material_evaluation();

    // Search function
    float negamax_with_pruning(float alpha, float beta, Bitmove& best_move, const uint8_t max_search_ply);

    unsigned int perft_test(uint8_t search_ply, uint8_t max_search_plies);

    std::ostream& write_piece_diagram_style(std::ostream& os, C2_chess::Piecetype p_type, C2_chess::Color side) const;
    std::ostream& write_piece(std::ostream& os, uint64_t square) const;
    std::ostream& write(std::ostream& os, const Color from_perspective) const;
    std::ostream& write_movelist(const list_ref movelist, std::ostream& os, bool same_line = false) const;
    Shared_ostream& write_search_info(Shared_ostream& logfile) const;

    void clear_search_info();
    void clear_search_vars();
    Search_info& get_search_info() const;

};

} // namespace C2_chess
#endif
