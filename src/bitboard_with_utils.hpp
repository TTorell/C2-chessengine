/*
 * Bitboard_with_utils.hpp
 *
 *  Created on: 17 juli 2021
 *      Author: torsten
 */
#ifndef Bitboard_with_utils_HPP_
#define Bitboard_with_utils_HPP_

#include <deque>
#include <vector>
#include <string>
#include "bitboard.hpp"

namespace C2_chess
{

using list_t = std::deque<Bitmove>;

std::ostream& operator <<(std::ostream& os, const Bitmove& m);

class Bitboard_with_utils: public Bitboard
{
  protected:
    bool run_mg_test_case(const uint32_t testnum, const std::string& FEN_string, const std::vector<std::string>& reference_moves, const std::string& testcase_info,
                          const Gentype gt);

    bool run_mg_test_case(uint32_t testnum, const std::string& FEN_string);

    std::vector<std::string> convert_moves_to_UCI(const std::vector<std::string>& moves, Color col_to_move);

  public:

    Game_history& get_game_history()
    {
      return Bitboard::history;
    }

    uint8_t get_castling_rights() const
    {
      return _castling_rights;
    }

    uint64_t get_hash_tag() const
    {
      return _hash_tag;
    }

    bool has_castled(Color side) const
    {
      return _has_castled[index(side)];
    }

    uint64_t get_ep_square() const
    {
      return _ep_square;
    }

    Bitmove get_latest_move() const
    {
      return _latest_move;
    }

    uint64_t  get_all_pieces() const
    {
      return _all_pieces;
    }

    Bitpieces get_white_pieces() const
    {
      return _white_pieces;
    }

    Bitpieces get_black_pieces() const
    {
      return _black_pieces;
    }

    Bitpieces* get_own() const
    {
      return _own;
    }

    Bitpieces* get_other() const
    {
      return _other;
    }

    float get_material_diff() const
    {
      return _material_diff;
    }

    History_state get_history_state() const
    {
      return history.get_state();
    }

    void set_mate()
    {
      _latest_move.add_property(move_props_mate);
    }

    void set_stalemate()
    {
      _latest_move.add_property(move_props_stalemate);
    }

    void set_iteration_depth(const int iteration_depth)
    {
      _iteration_depth = iteration_depth;
    }

    void set_draw_by_repetition()
    {
      _latest_move.add_property(move_props_draw_by_repetition);
    }

    void set_draw_by_50_moves()
    {
      _latest_move.add_property(move_props_draw_by_50_moves);
    }

    std::ostream& write_cmdline_style(std::ostream& os, const Color from_perspective) const;

    int add_mg_test_position(const std::string& filename);

    int test_move_generation(uint32_t single_testnum);

    bool bitboard_tests(const std::string& arg);

    uint64_t find_legal_squares(uint64_t sq, uint64_t mask, uint64_t all_pieces, uint64_t other_pieces);

    void make_UCI_move(const std::string& UCI_move);

    void make_UCI_move(const std::string& UCI_move, Takeback_state& tb_state);

    float evaluate_empty_movelist(int search_ply) const;

    float evaluate_position() const;

    Bitmove get_first_move()
    {
      //TODO: Is this right:
      list_t movelist;
      find_legal_moves(movelist, Gentype::All);
      assert(movelist.size() > 0);
      return movelist[0];
    }

    int get_move_index(const Bitmove& move)
    {
      int idx = 0;
      //TODO: Is this right:
      list_t movelist;
      find_legal_moves(movelist, Gentype::All);
      for (const Bitmove& m : movelist)
      {
        if (m == move)
          return idx;
        idx++;
      }
      return -1;
    }

    int figure_out_last_move(const Bitboard& new_position, Bitmove& m) const;

    void clear_game_history()
    {
      history.clear();
    }

    void add_position_to_game_history(const uint64_t hash_tag);

    void takeback_latest_move(Takeback_state& tb_state);

    void takeback_from_game_history();

    void reset_history_state(const History_state& saved_history_state);

//    Takeback_state& get_takeback_state(size_t idx) const
//    {
//      return takeback_list[idx].state_S;
//    }
//
//    Takeback_state& get_takeback_state_Q(size_t idx) const
//    {
//      return takeback_list[idx].state_Q;
//    }

    std::ostream& write_movelist(std::ostream& os, const bool same_line);

};

} // End namespace C2_chess

#endif /* Bitboard_with_utils_HPP_ */
