#ifndef _POSITION
#define _POSITION
class Position
{
	protected:
		int _file;
		int _rank;

	public:
		Position();
		Position(int file, int rank);
		Position(const Position& p);
		~Position();
		Position& operator=(const Position& p);
		bool operator==(const Square& s) const;
		bool operator==(const Position& p) const;
		int get_file() const {return _file;};
		int get_rank() const {return _rank;};
		bool set_file(char filechar) {_file=filechar-'a'; return (filechar >='a' && filechar <='h');};
		bool set_file(int fileindex) {_file=fileindex; return (fileindex >=a && fileindex <=h);};
		bool set_rank(char rankchar) {_rank=rankchar + 1 - '1'; return (rankchar >='1' && rankchar <='8');};
		bool set_rank(int rankindex) {_rank=rankindex; return (rankindex >=1 && rankindex <=8);};
		bool same_file(const Position& p) const {return _file==p._file;};
		bool same_rank(const Position& p) const {return _rank==p._rank;};
		bool same_diagonal(const Position& p) const;
  //	  	bool same_file(const Square& s) const;
  //		bool same_rank(const Square& s) const;
  //		bool same_diagonal(const Square& s) const;
		friend ostream& operator<<(ostream& os, const Position& p);
};
#endif



