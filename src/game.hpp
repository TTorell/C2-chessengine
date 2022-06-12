#ifndef _GAME_HPP
#define _GAME_HPP

#include "chesstypes.hpp"
#include "bitboard_with_utils.hpp"
#include "movelist.hpp"
#include "pgn_info.hpp"
#include "shared_ostream.hpp"

namespace // fileprivate namespace
{
  std::string initial_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
}

namespace C2_chess
{
class Config_params;
class Castling_state;

class Game
{
  protected:
    bool _is_first_position;
    Movelog _move_log;
    Bitboard_with_utils _chessboard;
    playertype _player_type[2];
//    uint8_t _moveno = 0;
    float _score = 0.0;
    //    PGN_info _pgn_info;
    Config_params& _config_params;
    bool _playing;
  public:
    Game(Config_params& config_params);
    Game(col c, Config_params& config_params);
    Game(const Game&) = delete;
    Game(playertype pt1,
         playertype pt2,
         Config_params& config_params);
    ~Game();
    Game operator=(const C2_chess::Game&) = delete;

    int find_best_move_index(float& score,
                             int max_search_level);

    int read_position_FEN(const std::string& FEN_string)
    {
      // TODO: read new position to temporary board, so
      // we can call figure_out_last_move().
      Bitboard new_position;
      if (new_position.read_position(FEN_string, true)) // true means init_piece_state().
        return -1;
      if (!_is_first_position)
        figure_out_last_move(new_position);
      else
        _chessboard.read_position(FEN_string, true);
      _chessboard.find_legal_moves(gentype::all);
      return 0;
    }

    void init();
    void clear_chessboard();
    void clear_move_log();
    void setup_pieces();
    void init_board_hash_tag();
    void actions_after_a_move();
    void start();
    BitMove incremental_search(const std::string& max_search_time, int max_search_level);
    BitMove engine_go(const Config_params& config_params, const std::string& max_search_time);
    void start_timer_thread(const std::string& max_search_time);
    bool has_time_left();
    void set_time_left(bool value);
    void save() const;
    col get_col_to_move() const;
    playertype get_playertype(const col& color) const;
    void set_move_log_col_to_start(col c);
    void set_castling_state(const Castling_state& cs);
    void set_en_passant_square(int file, int rank);
    void set_half_move_counter(int half_move_counter);
    void set_moveno(int moveno);
    std::ostream& write_chessboard(std::ostream& os, outputtype ot, col from_perspective) const;
    std::ostream& write_diagram(std::ostream& os) const;
    Shared_ostream& write_diagram(Shared_ostream& sos) const;
    void play_on_cmd_line(Config_params& config_params);
    void figure_out_last_move(const Bitboard& new_position);
    void start_new_game(col col_to_move);
    int read_position(const std::string& filename);
    int make_a_move(float& score, const uint8_t max_search_level);
};

} // namespace C2_chess
#endif

