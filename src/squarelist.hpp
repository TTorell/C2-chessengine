#ifndef SQUARELIST
#define SQUARELIST

namespace C2_chess
{

class Square;

const int SQUARELISTMAX=30;

class Squarelist
{
	protected:
		Square* _list[SQUARELISTMAX];
		int _no_of_elements;
		int _listindex; //Changes to _listindex is not considered
					    //to change the object status,
					    //So member functions can change _listindex but still
					    //be declared const. See first() and next().
	public:
		Squarelist();
		~Squarelist();
		Square* first() const; //pseudo const
		Square* next() const;  //----||------
		void into(Square* const);
		void out(Square* const rubbish);
		int cardinal() const;
		bool empty() const;
		bool full()const;
		bool in_list(Square* const) const;
		void clear();
		Square* operator[](int) const;
};
}
#endif

