#ifndef _RANK
#define _RANK
class Rank
{
	protected:
		Square* _file[8];
		char _name;

	public:
		Rank();
		Rank(char name);
		~Rank();
		char get_name() {return _name;};
		int get_rankindex() {return ((int)_name-48);};
		void set_name(char name){_name=name;};
		bool is_included(Square*) const;
		Square*& operator[](int) const;
		friend ostream& operator<<(ostream& os, const Rank& m);
};
#endif
