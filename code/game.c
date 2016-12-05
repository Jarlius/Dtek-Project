/* mipslabwork.c

   This file written 2015 by F Lundevall

   This file should be changed by YOU! So add something here:

   This file modified 2015-12-24 by Ture Teknolog 

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "headers.h"  /* Declatations for these labs */

int mytime = 0;
int timeoutcount = 0;
char textstring[] = "text, more text, and even more text!";

/* Interrupt Service Routine */
void user_isr( void )
{
	// Switch
	if((IFS(0) & 0x800) == 0x800) {
		mytime += 0x3;
		IFSCLR(0) = 0x800;
	}
	// Timer
	if((IFS(0) & 0x100) == 0x100)
	{ 
		IFSCLR(0) = 0x00000100; // Clear the timer interrupt status flag

		if(timeoutcount < 10) {
			timeoutcount++;
			return;
		}
		timeoutcount = 0;
		mytime++;
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
	
	// Initialize switch-flag
	TRISD |= 0x200;			// Set sw2 to input - testad
	IFSCLR(0) = 0x800; 		//11 biten	//flagga
	IECSET(0) = 0x800; 		//11 biten	//enable switch interrupts
	IPCSET(2) = 0x18000000;	//28:26	//mainprio 6
	IPCSET(2) = 0x2000000;	//25:24	//subprio 2

	// Initialize timer
	TMR2 = 0x0;			// Clear the timer register
	PR2 = 31250;		// sets the roof the tmr2 counts towards before resetting.
	T2CON |= 0x8000;	// start the timer
	T2CON |= 0x70;		// sets the scaling to 256
	
	enable_interrupt();
}

/* This function is called repetitively from the main program */
void gamerun( void )
{
	display_string(0,itoaconv(mytime));
	display_update();
}
