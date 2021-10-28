#ifndef _GAME
#define _GAME

#include "player.hpp"
#include "board.hpp"
#include "movelist.hpp"
#include "pgn_info.hpp"

namespace C2_chess
{
class Config_params;
class Castling_state;

class Game {
  protected:
    bool _is_first_position;
    Movelog _move_log;
    Board _chessboard;
    Player _player1;
    Player _player2;
    Player* _player[2];
    int _moveno = 0;
    col _col_to_move;
    float _score = 0.0;
    int _half_move_counter = 0;
    PGN_info _pgn_info;
    Config_params& _config_params;
    bool _playing;
  public:
    Game(Config_params& config_params);
    Game(col c,
         Config_params& config_params);
    Game(const Game&) = delete;
    Game(col c,
         playertype pt1,
         playertype pt2,
         Config_params& config_params);
    ~Game();
    Game operator=(const C2_chess::Game&) = delete;
    void init();
    void clear_chessboard();
    void clear_move_log();
    void setup_pieces();
    void init_board_hash_tag();
    void actions_after_a_move();
    void start();
    Move engine_go(const Config_params& config_params, const std::string& max_search_time);
    void start_timer_thread(const std::string& max_search_time);
    bool has_time_left();
    void set_time_left(bool value);
    void save() const;
    col get_col_to_move() const;
    playertype get_playertype(const col& color) const;
    void set_col_to_move(col c);
    void set_move_log_col_to_start(col c);
    void set_castling_state(const Castling_state &cs);
    void put_piece(Piece* const p, int file, int rank);
    void set_en_passant_square(int file, int rank);
    void set_half_move_counter(int half_move_counter);
    void set_moveno(int moveno);
    std::ostream& write_chessboard(std::ostream& os, outputtype ot, col from_perspective) const;
    std::ostream& write_diagram(std::ostream& os) const;
    Shared_ostream& write_diagram(Shared_ostream& sos) const;
    void play_on_cmd_line(Config_params& config_params);
    void figure_out_last_move(const Board& new_position, col col_to_move, int half_move_counter, int moveno);
    void start_new_game(col col_to_move, int half_move_counter, int move_no);
};
} // namespace C2_chess
#endif

