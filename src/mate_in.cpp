/*
#include "chesstypes.hpp"
#include <unistd.h> // for access()
#include <stdlib.h> // for exit()
int main( int argc, char** argv )
{
	Board chessboard;
	ifstream position_file;
	int make_move;
	char posfile_name[40];
	if ( argc < 2 )
	{
		cout << "Usage: mate_in number <position_file> " << endl;
		exit(1);
	}
	if ( argc > 2 )
	{
		strcpy( posfile_name, argv[2] );
	}
	else
	{
		strcpy( posfile_name, "c:\\torsten\\c++\\c2\\testpos");
		cout << " Using default: position_file " << posfile_name << endl;
	}
	if (access(posfile_name,04)==0) //Check for read
		cout << "Reading position from the file: " << posfile_name << endl;
	else
	{
		cout << "Error: Can't read positionfile " << posfile_name << endl;
		exit(1);
	}
	position_file.open( posfile_name, ios::in);
	chessboard.clear();
	col col_to_move;
	chessboard.read_position(position_file, col_to_move);
	cout << "col_to_move=" << col_to_move << endl;
	position_file.close();
	chessboard.init(col_to_move);
	chessboard.check_moves(col_to_move);
	//chessboard.write("print_file");
	Player p(computer,col_to_move,chessboard);
	//Player c(computer,col_to_move==white?black:white,chessboard);
	int moves;
	if ( argc >1 )
		moves = atoi( argv[1] );
	for (int i=1; i<=moves; i++)
	{
		cout << "calculating mate in " << i << endl;
		if (p.mate_in(i,chessboard,0,make_move))
		{
			cout << "mate in " <<  i << " with: ";
			int move_number = 1;
			chessboard.make_move( make_move, move_number, p.get_colour(), false );
			exit(0);
		}
		cout << "i= " << i << endl;
	}
	cout << "No mate in " << moves << "." << endl;
	exit(1);
}
*/


