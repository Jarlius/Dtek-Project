/* mipslabwork.c

   This file written 2015 by F Lundevall

   This file should be changed by YOU! So add something here:

   This file modified 2015-12-24 by Ture Teknolog 

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "headers.h"  /* Declatations for these labs */
#include <stdlib.h>

int timeoutcount = 0;

int score = 0;
int crashed = 0;
int dspot = 1;
int pspot = 1;

#define length 200

char row0[length] = "     XXGET RDYXX";
char row1[length] = "  X             ";
char row2[length] = "  X             ";
char row3[length] = "     XXOMGZORZXX";

char * p0 = row0;
char * p1 = row1;
char * p2 = row2;
char * p3 = row3;

void generate( void )
{
	int i;
	for (i=16; i < length; i++) {
		if ((i % 2) == 0) {
			row0[i] = ' ';
			row1[i] = ' ';
			row2[i] = ' ';
			row3[i] = ' ';
		} 
		else {
			int j;
			int this= 0;
			if ((TMR2 % 2) == 0) {
				row0[i] = 'X';
				this++;
			} else {
				row0[i] = ' ';
			}
			for (j=0; j < this; j++) {
				int p = 3;
			}
			if ((TMR2 % 2) == 0) {
				row1[i] = 'X';
				this++;
			} else {
				row1[i] = ' ';
			}	
			for (j=0; j < this; j++) {
				int p = 3;
			}
			if ((TMR2 % 2) == 0) {
				row2[i] = 'X';
				this++;
			} else {
				row2[i] = ' ';
			}
			for (j=0; j < this; j++) {
				int p = 3;
			}
			if (((TMR2 % 2) == 0)&&(this<3)) {
				row3[i] = 'X';
			} else {
				row3[i] = ' ';
			
				
			}
			for (j=0;j<1000;j++) {
				int p = (TMR2)*j;
			}
		}
	}
}

void eraser( int spot, int i )
{
	switch (spot) {
	case 0:
		row0[i] = ' ';
		break;
	case 1:
		row1[i] = ' ';
		break;
	case 2:
		row2[i] = ' ';
		break;
	case 3:
		row3[i] = ' ';
		break;
	}	
}

/* Interrupt Service Routine */
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
		if ((score != (length-1)) && (!crashed)) {
			p0 = &p0[1];
			p1 = &p1[1];
			p2 = &p2[1];
			p3 = &p3[1];
			score++;
			if (!crashed)
				eraser(dspot,score-1);
/*			if (PR2 > 10000)
				PR2 *= 0.9;
*/		}
	}
}

/* Lab-specific initialization goes here */
void gameinit( void )
{
	// Initialize timer-flag
	IFSCLR(0) = 0x00000100; // Clear the timer interrupt status flag
	IECSET(0) = 0x00000100; // Enable timer interrupts
	IPCSET(2) = 0x0000001C; // Set priority level = 7
	IPCSET(2) = 0x00000003; // Set subpriority level = 3

	// Initialize buttons
	TRISF |= 0x2;			// bit 1
	TRISD |= 0x20;			// bit 5
	TRISD |= 0x40;			// bit 6
	TRISD |= 0x80;			// bit 7

	generate();
	
	// Initialize timer
	TMR2 = 0x0;			// Clear the timer register
	PR2 = 31250;		// sets the roof the tmr2 counts towards before resetting.
	T2CON |= 0x8000;	// start the timer
	T2CON |= 0x70;		// sets the scaling to 256
	
	enable_interrupt();
}

int btn1flag = 0;
int btn2flag = 0;
int btn3flag = 0;
int btn4flag = 0;

/* This function is called repetitively from the main program */
void gamerun( void )
{
	if ((PORTF & 0x2) == 0x2) {
		if (!btn1flag) {
			// action
			if (!crashed)
				eraser(dspot,score);
			score = 0; // TODO score
			crashed = 0;
			p0 = row0;
			p1 = row1;
			p2 = row2;
			p3 = row3;
			PR2 = 31250;
			score = 0;
			generate();
			
			btn1flag = 1;
		}
	}
	else if (btn1flag) {
		btn1flag = 0;
	}

/*	if ((PORTD & 0x20) == 0x20) {
		if (!btn2flag) {
			// action
			mytime++;
			display_string(1,"great success");

			btn2flag = 1;
		}
	}
	else if (btn2flag) {
		btn2flag = 0;
	}
*/
	
	if ((score != (length-1)) && (!crashed)) {
		if ((PORTD & 0x40) == 0x40) {
			if (!btn3flag) {
				// action
				if (dspot < 3) 
					dspot++;
					eraser(pspot,score);				
				btn3flag = 1;
			}
		}
		else if (btn3flag) {
			btn3flag = 0;
		}
	
		if ((PORTD & 0x80) == 0x80) {
			if (!btn4flag) {
				// action
				if (dspot > 0)
					dspot--;
					eraser(pspot,score);
				btn4flag = 1;
			}
		}
		else if (btn4flag) {
			btn4flag = 0;
		}
	
		// hit detection
	
		switch (dspot) {
		case 0:
			if (row0[score] == 'X')
				crashed = 1;
			else
				row0[score] = 'D';
			break;
		case 1:
			if (row1[score] == 'X')
				crashed = 1;
			else
				row1[score] = 'D';
			break;
		case 2:
			if (row2[score] == 'X')
				crashed = 1;
			else
				row2[score] = 'D';
			break;
		case 3:
			if (row3[score] == 'X')
				crashed = 1;
			else
				row3[score] = 'D';
			break;
		}

		pspot = dspot;

		display_string(0,p0);
		display_string(1,p1);
		display_string(2,p2);
		display_string(3,p3);
	} else {
		char * end = itoaconv( score );
		char print[] = "   score:   ";
		print[9] = end[0];
		if(score > 10)
			print[10] = end[1];
		if(score > 100)
			print[11] = end[2];
		if (score == (length-1)) {
			display_string(1,"   CONGRATS!");

		} else {
			display_string(1,"   GAME OVER");
		}
		display_string(0,"");
		display_string(2,print);
		display_string(3,"");
	}
	display_update();
}
