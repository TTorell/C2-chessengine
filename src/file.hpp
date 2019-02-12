#ifndef _FIL
#define _FIL
class File
{
	protected:
		Square* _rank[9];
		char _name;
	public:
		File();
		File(char name);
		~File();
		char get_filename() {return _name;};
		void set_name(char name) {_name=name;};
		int get_fileindex() {return ((int)_name - 97);};
		bool is_included(Square*) const;
		Square*& operator[](int) const;
		friend ostream& operator<<(ostream& os, const File& m);
};
#endif
