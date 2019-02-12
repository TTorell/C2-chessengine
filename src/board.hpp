#ifndef _BOARD
#define _BOARD

class Board
{
  protected:
		File _file[8];
		Rank _rank[9];  // _rank[0] not used
		Move _last_move;
		Movelist _possible_moves;
		Castling_state _castling_state;
		Square* _king_square[2];
		Square* _en_passant_square = 0;
	public:
        static Board level_boards[];// declaration, incomplete type
		Board();
		Board(const Board&);
		~Board();
		void clear(bool remove_pieces=true);
		void init_castling(col this_col);
		void init_bishop_or_queen(int file, int rank, Square* s, Piece* p);
		void init_rook_or_queen(int file, int rank, Square* s, Piece* p);
		int init(col col_to_move);
		void write(char* filename) const;
		ostream& write(ostream& os, output_type wt, col from_perspective) const;
		Shared_ostream& write(Shared_ostream& os, output_type wt, col from_perspective) const;
		void read_position(ifstream& positionfile, col& col_to_move);
		void calculate_moves(col col_to_move);
		int no_of_moves() const;
		istream& operator>>(istream&);
		int make_move(int,int&,col col_to_move,bool silent = true);
		int make_move(player_type,int&, col col_to_move);
		Board& operator=(const Board&);
		Board& operator[](int) const;
		Move get_last_move() const;
        float evaluate_position(col col_to_move, output_type ot, int level) const;
        void put_piece(Piece* const p,int file,int rank);
        Castling_state& get_castling_state() {return _castling_state;};
        void set_castling_state(const Castling_state& cs){_castling_state = cs;};
        void set_enpassant_square(int file, int rank)
        {
          _en_passant_square = _file[file][rank];
        }
        void set_mate(bool is_mate){_last_move.set_mate(is_mate);}
        void set_stalemate(bool is_stalemate){_last_move.set_stalemate(is_stalemate);}
        float max(int level, int move_no, float alpha, float beta, int& best_move_index, const int& search_level, bool prune) const;
        float min(int level, int move_no, float alpha, float beta, int& best_move_index, const int& search_level, bool prune) const;
        void setup_pieces();

	private:
		bool read_piece_type(piecetype& pt,char c) const;
		bool en_passant(Piece*,Square*) const;
        void calculate_moves_K_not_threatened(col col_to_move);
		void fix_promotion_move(Move*);
		void fix_pawn_tp(int,int,Piece*,Square*);
		void fix_en_passant(Square* s, Square* possible_ep_square);
		void fix_threat_prot(int,int,Piece*,Square*);
		bool allowed(int,int);
		void fix_bound_piece_file(const Square* king_square,
										  Square* own_piece_square,
										  const Square* threat_square);
		void fix_bound_piece_rank(const Square* king_square,
										  Square* own_piece_square,
										  const Square* threat_square);
		void fix_bound_piece_diagonal(const Square* king_square,
												Square* own_piece_square,
												const Square* threat_square);
		int min(int,int);
		int max(int,int);
		void count_material(float& sum, float weight, col _col_to_move, output_type ot) const;
		void count_center_control(float& sum, float weight, col _col_to_move, output_type ot) const;
		void count_pawns_in_centre(float& sum, float weight, col col_to_move, output_type ot) const;
		//void count_possible_moves(float& sum, float weight, col _col_to_move) const;
		void count_development(float& sum, float weight, col col_to_move, output_type ot) const;
		void count_castling(float& sum, float weight, col col_to_move, output_type ot) const;
        bool is_end_node() const;
        void check_put_a_piece_on_square(int i, int j, col col_to_move);
        void check_put_a_pawn_on_square(int file, int rank, col col_to_move);
        void check_rook_or_queen(Square* threat_square, Square* kings_square, col col_to_move);
        void check_bishop_or_queen(Square* threat_square,Square* kings_square, col col_to_move);
};
#endif
