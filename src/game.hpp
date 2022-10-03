#ifndef _GAME_HPP
#define _GAME_HPP

#include "chesstypes.hpp"
#include "bitboard_with_utils.hpp"
#include "movelog.hpp"
#include "pgn_info.hpp"
#include "shared_ostream.hpp"
#include "game_history.hpp"

namespace // fileprivate namespace
{
}

namespace C2_chess
{
class Config_params;
struct Go_params;
struct Position_params;
class Castling_state;

class Game
{
  protected:
    bool _is_first_position;
    Movelog _move_log;
    Bitboard_with_utils _chessboard;
    Playertype _player_type[2];
    float _score = 0.0;
    //    PGN_info _pgn_info;
    Config_params& _config_params;
    bool _playing;
  public:
    Game(Config_params& config_params);
    Game(Color side, Config_params& config_params);
    Game(const Game&) = delete;
    Game(Playertype pt1,
         Playertype pt2,
         Config_params& config_params);
    ~Game();
    Game operator=(const C2_chess::Game&) = delete;


    int read_position(const Position_params& params);
    int read_position_FEN(const std::string& FEN_string);

    void init();
    void clear_chessboard();
    void clear_move_log(Color col_to_move, uint16_t move_number);
    void setup_pieces();
    void init_board_hash_tag();
    void actions_after_a_move(const bool movelist_is_empty);
    void start();
    Bitmove find_best_move(float& score, unsigned int max_search_ply);
    Bitmove incremental_search(const double movetime, unsigned int max_depth = N_SEARCH_BOARDS_DEFAULT/2);
    Bitmove engine_go(const Config_params& config_params, const Go_params& go_params, const bool apply_max_search_depth = false);
    //void start_timer_thread(const std::string& max_search_time);
    bool has_time_left();
    void set_time_left(bool value);
    void save() const;
    Color get_side_to_move() const;
    uint64_t get_hash_tag() const;
    float get_material_diff() const;
    Playertype get_playertype(const Color& side) const;
    History_state get_game_history_state()
    {
      return _chessboard.get_history_state();
    }
    void set_move_log_col_to_start(Color c);
    void set_castling_state(const Castling_state& cs);
    void set_en_passant_square(int file, int rank);
    void set_half_move_counter(int half_move_counter);
    void set_moveno(int moveno);
    std::ostream& write_chessboard(std::ostream& os, const Color from_perspective) const;
    std::ostream& write_diagram(std::ostream& os) const;
    Shared_ostream& write_diagram(Shared_ostream& sos) const;
    std::ostream& write_movelog(std::ostream& os) const;
    std::ostream& write_movelist(std::ostream& os) const;
    void play_on_cmd_line(Config_params& config_params);
    void figure_out_last_move(const Bitboard& new_position);
    void start_new_game();
    int read_position(const std::string& filename);
    int make_a_move(float& score, const uint8_t max_search_ply);
    void make_move(const std::string& move);
    void takeback_latest_move();
};

} // namespace C2_chess
#endif

