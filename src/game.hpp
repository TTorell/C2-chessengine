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
    float _score = 0.0;
    //    PGN_info _pgn_info;
    Config_params& _config_params;
    bool _playing;
  public:
    Game(Config_params& config_params);
    Game(color side, Config_params& config_params);
    Game(const Game&) = delete;
    Game(playertype pt1,
         playertype pt2,
         Config_params& config_params);
    ~Game();
    Game operator=(const C2_chess::Game&) = delete;


    int read_position_FEN(const std::string& FEN_string);

    void init();
    void clear_chessboard();
    void clear_move_log(color col_to_move, uint16_t move_number);
    void setup_pieces();
    void init_board_hash_tag();
    void actions_after_a_move();
    void start();
    Bitmove find_best_move(float& score, unsigned int max_search_ply);
    Bitmove incremental_search(const std::string& max_search_time, unsigned int max_search_level);
    Bitmove engine_go(const Config_params& config_params, const std::string& max_search_time);
    void start_timer_thread(const std::string& max_search_time);
    bool has_time_left();
    void set_time_left(bool value);
    void save() const;
    color get_col_to_move() const;
    playertype get_playertype(const color& side) const;
    void set_move_log_col_to_start(color c);
    void set_castling_state(const Castling_state& cs);
    void set_en_passant_square(int file, int rank);
    void set_half_move_counter(int half_move_counter);
    void set_moveno(int moveno);
    std::ostream& write_chessboard(std::ostream& os, outputtype ot, color from_perspective) const;
    std::ostream& write_diagram(std::ostream& os) const;
    Shared_ostream& write_diagram(Shared_ostream& sos) const;
    std::ostream& write_movelog(std::ostream& os) const;
    std::ostream& write_movelist(std::ostream& os) const;
    void play_on_cmd_line(Config_params& config_params);
    void figure_out_last_move(const Bitboard& new_position);
    void start_new_game();
    int read_position(const std::string& filename);
    int make_a_move(float& score, const uint8_t max_search_level);
    void make_move(const std::string& move);
    History_state get_game_history_state()
    {
      return _chessboard.get_history_state();
    }
};

} // namespace C2_chess
#endif

