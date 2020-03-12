//    Battleship game assignment for MSU CSCI 366
//    Copyright (C) 2020    Mike P. Wittie
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.


#include "common.hpp"
#include "Server.hpp"

using namespace std ;


/**
 * Calculate the length of a file (helper function)
 *
 * @param file - the file whose length we want to query
 * @return length of the file in bytes
 */
int get_file_length ( ifstream *file )
{
    file -> seekg ( 0 , ios :: end ) ;
	
    int l = file -> tellg ( ) ;
	
    return l ;
}


void Server :: initialize ( unsigned int board_size , string p1_setup_board, string p2_setup_board )
{
	
	// open board files and verify sizes:
		
	this -> p1_setup_board . open ( p1_setup_board , ifstream::in ) ;
	
	if ( ! this -> p1_setup_board . is_open ( ) )
	{
		throw ServerException ( "Bad player 1 board file name: " + p1_setup_board + "!" ) ;
	}
	
	this -> p2_setup_board . open ( p2_setup_board , ifstream::in ) ;
	
	if ( ! this -> p2_setup_board . is_open ( ) )
	{
		throw ServerException ( "Bad player 2 board file name: " + p2_setup_board + "!" ) ;
	}
	
	// get file lengths:
	
	int l1 = get_file_length ( & this -> p1_setup_board ) ;
	int l2 = get_file_length ( & this -> p2_setup_board ) ;
	
	// check that file lengths match board size:
	
	int	z = ( board_size + 1 ) * board_size ;
	
	if ( l1 != z )
	{
		throw ServerException ( "Wrong player 1 board size!\n" ) ;
	}
	
	if ( l2 != z )
	{
		throw ServerException ( "Wrong player 2 board size!\n" ) ;
	}
	
	// copy board_size:
	
	this -> board_size = board_size ;	
}

// get char value at given coordinates:
char get_val ( ifstream *board , int board_size , unsigned int x , unsigned int y )
{
	// calculate the linear point 'p' that we wish to read:
	
	unsigned int p = ( board_size + 1 ) * y + x ;
	
	// seek to p:
	
	board -> seekg ( p ) ;

	// get the correct char at p:
	
	char c ;	
	board -> get ( c ) ;				
	
	// return char:
	
	return c ;
}


int Server :: evaluate_shot ( unsigned int player , unsigned int x , unsigned int y )
{
	// check if valid player number:
	
	if ( player <= 0 )
	{
		throw ServerException ( "Player number too low!\n" ) ;
	}
	
	if ( player > MAX_PLAYERS )
	{
		throw ServerException ( "Player number too high!\n" ) ;
	}	
	
	// check if shot is within bounds:
	
	if ( x < 0 )
	{
		return OUT_OF_BOUNDS ;
	}
	
	if ( x >= board_size )
	{
		return OUT_OF_BOUNDS ;
	}
	
	if ( y < 0 )
	{
		return OUT_OF_BOUNDS ;
	}
	
	if ( y >= board_size )
	{
		return OUT_OF_BOUNDS ;
	}
	
	// determine result:
	
	char res ;
	if ( player == 1 )
	{	
		res = get_val ( & this -> p2_setup_board , board_size , x , y ) ;
	}
	else if ( player == 2 )
	{	
		res = get_val ( & this -> p1_setup_board , board_size , x , y ) ;
	}
	
	
	
	// return result:
	
	if ( res == 'C' || res == 'B' || res == 'R' || res == 'S' || res == 'D' )
	{
		return HIT ;
	}
	else
	{
		return MISS ;
	}		
}


int Server :: process_shot ( unsigned int player )
{
	
	// open json file:
	
	string file_in_name ;
	if ( player == 1 )
	{		
		file_in_name = "player_1.shot.json" ;
	}
	else if ( player == 2 )
	{
		file_in_name = "player_2.shot.json" ;
	}
	else
	{
		throw ServerException ( "Bad player number!\n" ) ;
	}	

	// create an input file stream:
	
	ifstream file_in ( file_in_name ) ; 								
	
	// if file exists:
	
	if ( file_in )						
	{
		// initialize an archive on the file:
		
		cereal :: JSONInputArchive archive_in ( file_in ) ;			
		
		// read json file:
		
		unsigned int x ;
		unsigned int y ;
		
		// deserialize the array:
		
		archive_in ( x , y ) ; 									
		
		//close file:
		
		file_in . close ( ) ;								
		
		// remove shot file:
		
		remove ( file_in_name . c_str ( ) ) ;				
		
		// pass to evaluate_shot:
		
		int res ;
		if ( player == 1 )
		{		
			res = evaluate_shot ( 1 , x , y ) ;
		}
		else if ( player == 2 )
		{
			res = evaluate_shot ( 2 , x , y ) ;
		}
		
		// write to result file:
		
		string file_out_name ;		
		if ( player == 1 )
		{		
			file_out_name = "player_1.result.json" ;
		}
		else if ( player == 2 )
		{
			file_out_name = "player_2.result.json" ;
		}
		
		// remove any old serialization files:
		
		remove ( file_out_name . c_str ( ) ) ;										
		
		// create an output file stream:
		
		ofstream file_out ( file_out_name ) ; 

		// must include braces so destructor of archive is called before we close the file:
		{
			// initialize an archive on the file:
			
			cereal :: JSONOutputArchive archive_out ( file_out ) ; 			
			
			// serialize the data giving it a name:
			
			archive_out ( cereal :: make_nvp ( "result" , res ) ) ; 				
		}
		// destructor will have been called
		
		// close the file:
		
		file_out . close ( ) ; 	

		// return status code:
		
		return SHOT_FILE_PROCESSED ;
	}
	
	// brach here if file does not exist:
	
	// close file:
	
	file_in . close ( ) ;								
	
	// remove shot file:
	
	remove ( file_in_name . c_str ( ) ) ;				
		
	// return error result:
	
	return NO_SHOT_FILE ;
}