#ifndef _BOARD
#define _BOARD

#include "movelist.hpp"
#include "castling_state.hpp"
#include "move.hpp"
#include "file.hpp"
#include "rank.hpp"
#include "circular_fifo.hpp"
#include "zobrist_hash.hpp"

namespace C2_chess
{
class Piece;

class Board {
  protected:
    File _file[8];
    Rank _rank[9];  // _rank[0] not used
    Move _last_move;
    Movelist _possible_moves;
    Castling_state _castling_state;
    Square* _king_square[2];
    Square* _en_passant_square = 0;
    unsigned long _hash_tag;
    static Zobrist_hash hash_table;
    float _material_evaluation;
  public:
    static Board level_boards[];  // declaration, incomplete type
    Board();
    Board(const Board&);
    ~Board();
    void setup_pieces();
    void clear(bool remove_pieces = true);
    void init_castling(col this_col);
    void init_bishop_or_queen(int file, int rank, Square* s, Piece* p);
    void init_rook_or_queen(int file, int rank, Square* s, Piece* p);
    int init(col col_to_move);
    float evaluate_position(col col_to_move, outputtype ot, int level) const;
    std::ostream& write(std::ostream& os, outputtype wt, col from_perspective) const;
    std::ostream& write_cmdline_style(std::ostream &os, outputtype wt, col from_perspective) const;
    std::ostream& write_possible_moves(std::ostream& os);
    void read_position(std::ifstream& positionfile, col& col_to_move);
    void calculate_moves(col col_to_move);
    std::istream& operator>>(std::istream&);
    int make_move(int, int&, col col_to_move);
    int make_move(playertype, int&, col col_to_move);
    Board& operator=(const Board&);
    Board& operator[](int) const;
    void put_piece(Piece* const p, int file, int rank);
    Move get_possible_move(int index) const;
    Movelist get_possible_moves() const;
    Move get_last_move() const
    {
      return _last_move;
    }
    Castling_state& get_castling_state()
    {
      return _castling_state;
    }
    void set_castling_state(const Castling_state& cs)
    {
      _castling_state = cs;
    }
    void set_enpassant_square(int file, int rank)
    {
      _en_passant_square = _file[file][rank];
    }
    void set_mate(bool is_mate)
    {
      _last_move.set_mate(is_mate);
    }
    void set_stalemate(bool is_stalemate)
    {
      _last_move.set_stalemate(is_stalemate);
    }
    int no_of_moves() const
    {
      return _possible_moves.size();
    }
    unsigned long get_hash_tag()
    {
      return _hash_tag;
    }

    void start_timer_thread(const std::string& max_search_time);
    bool has_time_left();
    void set_time_left(bool value);

    float max(int level, int move_no, float alpha, float beta, int& best_move_index, const int max_search_level) const;
    float min(int level, int move_no, float alpha, float beta, int& best_move_index, const int max_search_level) const;
    float max_for_testing(int level, int move_no, float alpha, float beta, int &best_move_index, const int max_search_level, bool use_pruning, bool search_until_no_captures) const;
    float min_for_testing(int level, int move_no, float alpha, float beta, int &best_move_index, const int max_search_level, bool use_pruning, bool search_until_no_captures) const;

    void set_time_diff_sum(uint64_t value);
    uint64_t get_time_diff_sum();
    void init_board_hash_tag(col col_to_move);
    void clear_hash();
    void init_material_evaluation();
    float get_material_evaluation();
    int figure_out_last_move(const Board& new_position, Move& m);
    int get_move_index(const Move& m) const;
  private:
    bool read_piece_type(piecetype& pt, char c) const;
    bool en_passant(Piece*, Square*) const;
    void calculate_moves_K_not_threatened(col col_to_move);
    void fix_promotion_move(Move*);
    void fix_pawn_tp(int, int, Piece*, Square*);
    void fix_en_passant(Square* s, Square* possible_ep_square);
    void fix_threat_prot(int, int, Piece*, Square*);
    bool allowed_index(int fileindex, int rankindex);
    bool allowed_fileindex(int fileindex);
    void fix_bound_piece_file(Square* own_piece_square, const Square* threat_square);
    void fix_bound_piece_rank(Square* own_piece_square, const Square* threat_square);
    void fix_bound_piece_diagonal(const Square* king_square, Square* own_piece_square, const Square* threat_square);
    int min(int, int);
    int max(int, int);
    void count_center_control(float& sum, float weight, outputtype ot) const;
    void count_pawns_in_centre(float& sum, float weight, outputtype ot) const;
    //void count_possible_moves(float& sum, float weight, col _col_to_move) const;
    void count_development(float& sum, float weight, outputtype ot) const;
    void count_castling(float& sum, float weight, outputtype ot) const;
    bool is_end_node() const;
    void check_put_a_piece_on_square(int i, int j, col col_to_move);
    void check_put_a_pawn_on_square(int file, int rank, col col_to_move);
    void check_rook_or_queen(Square* threat_square, Square* kings_square, col col_to_move);
    void check_bishop_or_queen(Square* threat_square, Square* kings_square, col col_to_move);
    void check_if_threat_can_be_captured_en_passant(col col_to_move, Square* threat_square);
    void update_hash_tag_remove_en_passant_file();
    void update_hash_tag_en_passant_file(int fileindex);
    void update_hash_tag_change_color_to_move();
    void update_hash_tag_remove_castling_rights(col color);
    void update_hash_tag_remove_kingside_castling_rights(col color);
    void update_hash_tag_remove_queenside_castling_rights(col color);
    void update_hash_tag(const Square* square);
    void update_hash_tag(int fileindex, int rankindex, col color, piecetype type);
    void update_material_evaluation_capture(const col& color, const piecetype& type);
    void update_material_evaluation_promotion(const col& color, const piecetype& type);
    friend class Zobrist_hash;
};
}
#endif
