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
#include "Client.hpp"

using namespace std ;

string ascii ;

Client::~Client()
{
	// remove action board
	
	remove ( board_name . c_str ( ) ) ;
	
}


void Client::initialize(unsigned int player, unsigned int board_size)
{
	// set data: 
	this -> player 		= player ;
	this -> board_size	= board_size ;
	
	// create action board (player_#.action_board.json:
	
	// create row vector:
	
	vector < int > row ( board_size , 0 ) ;
	
	// create vector of vectors:
	
	vector < vector < int > > action_board ( board_size , row ) ;
	
	// write to file
	
	if ( player == 1 )
	{		
		board_name = "player_1.action_board.json" ;
	}
	else if ( player == 2 )
	{
		board_name = "player_2.action_board.json" ;
	}
	
	// remove any old serialization files:
	
	remove ( board_name . c_str ( ) ) ;									
	
	// create an output file stream :
	
	ofstream file_out ( board_name ) ; 	

	// must include braces so destructor of archive is called before we close the file:
	{
		// initialize an archive on the file:
		
		cereal :: JSONOutputArchive archive_out ( file_out ) ; 	
		
		// serialize the data giving it a name:
		
		archive_out ( cereal :: make_nvp ( "board" , action_board ) ) ; 			
	}
	// destructor will have been called
	
	// close the file:
	
	file_out . close ( ) ; 		

	// resize string for ascii :
	
	ascii . resize ( ( board_size + 1 ) * board_size ) ;

	this -> initialized = true ;
}


void Client::fire(unsigned int x, unsigned int y)
{
	// fires shot
	
	// creates player_#.shot.json file:
	
	string file_out_name ;		
	if ( player == 1 )
	{		
		file_out_name = "player_1.shot.json" ;
	}
	else if ( player == 2 )
	{
		file_out_name = "player_2.shot.json" ;
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
		
		archive_out ( cereal :: make_nvp ( "x" , x ) , cereal :: make_nvp ( "y" , y ) ) ; 				
	}
	// destructor will have been called
	
	// close the file:
	
	file_out . close ( ) ; 	
	
}


bool Client::result_available()
{
	// check is result is available:
	
	string file_in_name ;
	if ( player == 1 )
	{		
		file_in_name = "player_1.result.json" ;
	}
	else if ( player == 2 )
	{
		file_in_name = "player_2.result.json" ;
	}
	
	// create an input file stream:
	
	ifstream file_in ( file_in_name ) ;
	
	// if file exists:
	
	if ( file_in )						
	{
		//close file:
		
		file_in . close ( ) ;	
		
		// return true :
		
		return true ;
	}
	
	// otherwise return false:
	
	return false ;
}


int Client::get_result()
{
	// get result from player_#.result.json file:
	
	// open json file:
	
	string file_in_name ;
	if ( player == 1 )
	{		
		file_in_name = "player_1.result.json" ;
	}
	else if ( player == 2 )
	{
		file_in_name = "player_2.result.json" ;
	}
	
	// create an input file stream:
	
	ifstream file_in ( file_in_name ) ;

	if ( file_in )						
	{
		// initialize an archive on the file:
		
		cereal :: JSONInputArchive archive_in ( file_in ) ;		

		// read result:
		
		int res;
		
		archive_in ( res ) ;
		
		//close file:
		
		file_in . close ( ) ;								
		
		// remove result file:
		
		remove ( file_in_name . c_str ( ) ) ;	
		
		if ( res != HIT && res != MISS && res != OUT_OF_BOUNDS )
		{
			throw ClientException ( "Bad result!\n" ) ;
		}
		
		return res ;		
	}
	
	throw ClientException ( "No result file!\n" ) ;
	
	return 0 ;
	
	// return HIT MISS or OUT_OF_BOUNDS
	
	return OUT_OF_BOUNDS ;
}



void Client::update_action_board(int result, unsigned int x, unsigned int y)
{
	// update internal representation of player_#.action_board.json
	
	// create row vector:
	
	vector < int > row ( board_size , 0 ) ;
	
	// create vector of vectors:
	
	vector < vector < int > > action_board ( board_size , row ) ;
	
	// read old action board:
	
	ifstream file_in ( board_name ) ;

	if ( file_in )						
	{
		// initialize an archive on the file:
		
		cereal :: JSONInputArchive archive_in ( file_in ) ;		
		
		// deserialize data:
		
		archive_in ( action_board ) ;
	}
	
	//close file:
	
	file_in . close ( ) ;
	
	// remove any old serialization files:
	
	remove ( board_name . c_str ( ) ) ;	

	// modify vector:
	
	action_board [ x ] [ y ] = result ;
	
	// create an output file stream :
	
	ofstream file_out ( board_name ) ; 	

	// must include braces so destructor of archive is called before we close the file:
	{
		// initialize an archive on the file:
		
		cereal :: JSONOutputArchive archive_out ( file_out ) ; 	
		
		// serialize the data giving it a name:
		
		archive_out ( cereal :: make_nvp ( "board" , action_board ) ) ; 			
	}
	// destructor will have been called
	
	// close the file:
	
	file_out . close ( ) ; 		
}


string Client::render_action_board()
{
	// format ASCII representation of player_#.action_board.json
	
	// create row vector:
	
	vector < int > row ( board_size , 0 ) ;
	
	// create vector of vectors:
	
	vector < vector < int > > action_board ( board_size , row ) ;
	
	// read old action board:
	
	ifstream file_in ( board_name ) ;

	if ( file_in )						
	{
		// initialize an archive on the file:
		
		cereal :: JSONInputArchive archive_in ( file_in ) ;		
		
		// deserialize data:
		
		archive_in ( action_board ) ;
	}
	
	file_in . close ( ) ;
	
	unsigned int x = 0 ;
	unsigned int y = 0 ;
	unsigned int i = 0 ;
	
	while ( y < board_size - 1 )
	{
		while ( x < board_size )
		{
			int d = action_board [ x ] [ y ] ;
			
			// water:
			if ( d == 0 )
			{
				ascii[i] = '-' ;
			}
			
			// miss:
			if ( d == -1 )
			{
				ascii[i] = 'X' ;
			}
			
			// hit:
			if ( d == 1 )
			{
				ascii[i] = '*' ;
			}						
			i ++ ;
			x ++ ;
		}		
		
		ascii[i] = '\n' ;
		i ++ ;
		
		x = 0 ;
		y ++ ;
	}
	
	// return string version of board
	
	return ascii ;
	
	
}



