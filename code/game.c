/*	mipslabwork.c

	This file written 2015 by F Lundevall

	This file should be changed by YOU! So add something here:

	String Game
	By us,
	Jarl Silvén and Simon Hellberg

	This file modified 2016-12-09 by Jarl Silvén and Simon Hellberg

	Latest update 2016-12-09 by Jarl Silvén and Simon Hellberg

	For copyright and licensing, see file COPYING */

#include <stdint.h>   // Declarations of uint_32 and the like
#include <pic32mx.h>  // Declarations of system-specific addresses etc
#include "headers.h"  // Declarations for these labs
void *stdout;

int timeoutcount = 0;

int start = 1;		// is player in starting screen?
int pos = 0;		// how far is the player currently?
int score = 0;		// player score
int crashed = 0;	// flag for crash check
int dspot = 1;		// player position
int pspot = 1;		// player's previous position
int star = 0;		// is player in starmode?
int stardur = 0;	// how long has player been in starmode?
int player = 1;		// is player currently default symbol?
int fader = 0;		// how long until next player symbol change?

#define length 1000

// rows of game world
char row0[length] = "   ";
char row1[length] = "   ";
char row2[length] = "   ";
char row3[length] = "   ";

// collected world
char * rows[] = {row0,row1,row2,row3};

// current positions in rows
char * p0 = row0;
char * p1 = row1;
char * p2 = row2;
char * p3 = row3;

// Place powerups in the game world
void powerup( int genpos, int occ, char symbol )
{
	if ((rand() % occ) == 0) {
		int ospot = rand() % 4;
		int i;
		for(i=0; i < 4; i++) {
			if(ospot == i) {
				rows[i][genpos] = symbol;
			}
		}
	}
}

// generate the game world
void generate( void )
{
	int i;
	for (i=3; i < length; i++) {
		if ((i % 2) == 0) {
			row0[i] = ' ';
			row1[i] = ' ';
			row2[i] = ' ';
			row3[i] = ' ';
			powerup(i,20,'S');
			powerup(i,5,'T');
		}
		else {
			int x = rand() & 0xf;
			if (x == 0xf) {
				x = 0;
			}
			int j;
			for(j=0; j < 4; j++) {
				if ((x>>j) & 1)
					rows[j][i] = 'X';
				else
					rows[j][i] = ' ';
			}
		}
	}
}

// replace character at spot with space
void eraser( int spot, int ind )
{
	int i;
	for(i=0; i < 4; i++) {
		if (spot == i) {
			rows[i][ind] = ' ';
			return;
		}
	}	
}

// interrupt service routine, called by timer2
void user_isr( void )
{
	// Timer
	if((IFS(0) & 0x100) == 0x100)
	{ 
		IFSCLR(0) = 0x00000100; // Clear the timer interrupt status flag

		if(timeoutcount < 10) {
			timeoutcount++;
			return;
		}
		timeoutcount = 0;
		if ((pos != (length-1)) && (!crashed)) {
			p0 = &p0[1];
			p1 = &p1[1];
			p2 = &p2[1];
			p3 = &p3[1];
			pos++;
			score++;
			if (!crashed)
				eraser(dspot,pos-1);
			if (PR2 > 10000)
				PR2 *= 0.98;
			if (star) {
				if (stardur < 10) {
					stardur++;
				}
				else {
					star = 0;
					fader = 0;
					player = 1;
					stardur = 0;
				}
			}
		}
	}
}

// game initialization, starts timer, buttons and interrupts
void gameinit( void )
{
	// Initialize timer-flag
	IFSCLR(0) = 0x00000100; // Clear the timer interrupt status flag
	IECSET(0) = 0x00000100; // Enable timer interrupts
	IPCSET(2) = 0x0000001C; // Set priority level = 7
	IPCSET(2) = 0x00000003; // Set subpriority level = 3

	// Initialize buttons
	TRISF |= 0x2;			// bit 1
//	TRISD |= 0x20;			// bit 5
	TRISD |= 0x40;			// bit 6
	TRISD |= 0x80;			// bit 7
	
	// Initialize timer
	TMR2 = 0x0;			// Clear the timer register
	PR2 = 31250;		// sets the roof the tmr2 counts towards before resetting.
	T2CON |= 0x8000;	// start the timer
	T2CON |= 0x70;		// sets the scaling to 256
	
	enable_interrupt();
}

int btn1flag = 0;
int btn3flag = 0;
int btn4flag = 0;

// main game function, called each frame
void gamerun( void )
{
	// button 1, resets the game
	if ((PORTF & 0x2) == 0x2) {
		if (!btn1flag) {
			// action
			if (!crashed)
				eraser(dspot,pos);
			crashed = 0;
			p0 = row0;
			p1 = row1;
			p2 = row2;
			p3 = row3;
			PR2 = 31250;
			pos = 0;
			score = 0;
			srand(TMR2);
			generate();
			start = 0;
			
			btn1flag = 1;
		}
	}
	else if (btn1flag) {
		btn1flag = 0;
	}

	// booleans for disabled movement control
	if ((pos != (length-1)) && (!crashed) && (!start)) {
		// button 3, moves player down
		if ((PORTD & 0x40) == 0x40) {
			if (!btn3flag) {
				// action
				if (dspot < 3) 
					dspot++;
					eraser(pspot,pos);				
				btn3flag = 1;
			}
		}
		else if (btn3flag) {
			btn3flag = 0;
		}
		// button 4, moves player down
		if ((PORTD & 0x80) == 0x80) {
			if (!btn4flag) {
				// action
				if (dspot > 0)
					dspot--;
					eraser(pspot,pos);
				btn4flag = 1;
			}
		}
		else if (btn4flag) {
			btn4flag = 0;
		}
	
		// flash when exiting starmode
		if (stardur > 7) {
			if (fader < (50*(11-stardur))) {
				fader++;
			} else {
				player ^= 1;
				fader = 0;
			}
		}

		// hit detection
		int i;
		for(i=0; i < 4; i++) {
			if(dspot == i) {
				switch (rows[i][pos]) {
				case 'X':
					if (!star)
						crashed = 1;
					break;
				case 'O':
						stardur = 0;
						star = 1;
						player = 0;
					break;
				case 'T':
						score += 10;
					break;
				}
				if (player) {
					rows[i][pos] = 'D';
				} else {
					rows[i][pos] = '*';
				}
				break;
			}
		}
		pspot = dspot;

		// display game screen	
		display_string(0,p0);
		display_string(1,p1);
		display_string(2,p2);
		display_string(3,p3);
	} else if (start) {
		// display start screen
		display_string(0,"Welcome!");
		display_string(1,"Button 4 = Up");
		display_string(2,"Button 3 = Down");
		display_string(3,"Button 1 = Start/Reset");
	} else {
		// format score
		char * end = itoaconv( score );
		char print[] = "   score:   ";
		print[9] = end[0];
		if(score > 10)
			print[10] = end[1];
		if(score > 100)
			print[11] = end[2];
		if (score == (length-1)) {
			//display congrats message
			display_string(1,"   CONGRATS!");

		} else {
			//display fail message
			display_string(1,"   GAME OVER");
		}
		// display remainder of end screens
		display_string(0,"");
		display_string(2,print);
		display_string(3,"");
	}
	display_update();
}
