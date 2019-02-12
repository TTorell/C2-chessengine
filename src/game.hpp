#ifndef _GAME
#define _GAME
class Game
{
  protected:
    Movelog _move_log;
    Board _chessboard;
    Player _player1;
    Player _player2;
    Player* _player[2];
    int _moveno = 0;
    col _col_to_move;
    float _score = 0.0;
    int _half_move_counter = 0.0;
    PGN_info _pgn_info;

  public:
    Game();
    Game(col c);
    Game(col c, player_type pt1, player_type pt2);
    ~Game();
    void init()
    {
      _chessboard.init(_col_to_move);
      _chessboard.calculate_moves(_col_to_move);
    }
    void clear_chessboard()
    {
      _chessboard.clear();
    }
    void setup_pieces()
    {
      _chessboard.setup_pieces();
    }
    void start();
    Move engine_go(Shared_ostream& logfile);
    void save() const;
    col get_col_to_move() const
    {
      return _col_to_move;
    }
    void set_col_to_move(col c)
    {
      _col_to_move = c;
    }
    void set_col_to_start(col c)
    {
      _move_log.set_col_to_start(c);
    }
    void set_castling_state(const Castling_state &cs)
    {
      _chessboard.set_castling_state(cs);
    }
    void put_piece(Piece* const p, int file, int rank)
    {
      _chessboard.put_piece(p, file, rank);
    }
    void set_en_passant_square(int file, int rank)
    {
      _chessboard.set_enpassant_square(file, rank);
    }

    void set_half_move_counter(int half_move_counter)
    {
      _half_move_counter = half_move_counter;
    }

    void set_moveno(int moveno)
    {
      _moveno = moveno;
    }

    ostream& write_chessboard(ostream& os, output_type ot, col from_perspective) const
    {
      _chessboard.write(os, ot, from_perspective);
      return os;
    }
    ostream& write_diagram(ostream& os) const
    {
      if (_player[white]->get_type() == human)
        _chessboard.write(os, cmd_line_diagram, white) << endl;
      else if (_player[black]->get_type() == human)
        _chessboard.write(os, cmd_line_diagram, black) << endl;
      else
        // The computer is playing itself
        _chessboard.write(os, cmd_line_diagram, white) << endl;
      return os;
    }
    Shared_ostream& write_diagram(Shared_ostream& os) const
    {
      stringstream ss;
      write_diagram(ss);
      ss.flush();
      os << ss.str();
      return os;
    }
};
#endif

