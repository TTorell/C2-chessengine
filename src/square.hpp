#ifndef _SQUARE
#define _SQUARE
#include "position.hpp"
class Square {
  protected:
    Piece* _piece;
    Position _position;
    col _colour;
    //Rank* _rank;
    //File* _file;
    //Diagonal* _diagonal[3];
    Squarelist _moves;
    Squarelist _threats;
    Squarelist _protections;

  private:
    Square();

  public:
    Square(int file, int rank, col thiscolour, piecetype pt = Undefined, col pc = white);
    Square(const Square& s);
    ~Square();
    Square& operator=(const Square& s);
    void clear(bool delete_piece = true);
    //void belong_to_file(File&);
    //void belong_to_rank(Rank&);
    //void belong_to_diagonals(Diagonal&,Diagonal&);
    void contain_piece(Piece*);
    bool contains_piece(col c, piecetype pt) const;
    Piece* release_piece();
    void remove_piece();
    Piece* get_piece() const
    {
      return _piece;
    }
    col get_colour() const
    {
      return _colour;
    }
    //File* get_file() const {return _file;};
    //Rank* get_rank() const {return _rank;};
    //Diagonal* get_diagonal1() const {return _diagonal[1];};
    //Diagonal* get_diagonal2() const {return _diagonal[2];};
    void into_threat(Square* const);
    void into_protection(Square* const);
    void into_move(Square* const);
    void out_threat(Square* const rubbish);
    void out_protection(Square* const);
    void out_move(Square* const);
    int count_threats(col) const;
    int count_threats() const;
    int count_protections() const;
    int count_moves() const;
    void clear_threats();
    void clear_protections();
    void clear_moves();
    Square* first_threat() const
    {
      return _threats.first();
    }
    Square* first_protection() const
    {
      return _protections.first();
    }
    Square* first_move() const
    {
      return _moves.first();
    }
    Square* next_move() const
    {
      return _moves.next();
    }
    Square* next_protection() const
    {
      return _protections.next();
    }
    Square* next_threat() const
    {
      return _threats.next();
    }
    ostream& write_threats(ostream& os) const;
    ostream& write_protections(ostream& os) const;
    ostream& write_moves(ostream& os) const;
    ostream& write_describing(ostream& os) const;
    ostream& write_name(ostream& os) const;
    ostream& write_move_style(ostream& os) const;
    bool in_movelist(Square* s) const;
    bool same_file(Square* const s) const;
    bool same_rank(Square* const s) const;
    bool same_diagonal(Square* const s) const;
    int count_controls() const;
    Position get_position() const
    {
      return _position;
    }
    int get_fileindex() const
    {
      return _position.get_file();
    }
    int get_rankindex() const
    {
      return _position.get_rank();
    }
    operator const Position&() const
    {
      return _position;
    }
};
#endif

