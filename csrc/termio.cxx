#include "ptoc.h"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*+ TERMINAL IO DRIVERS SOURCE PROGRAM - UNMODIFIED   +*/
/*+ YOU MUST MODIFY THIS PROGRAM FOR YOUR APPLICATION +*/
/*+ AND RENAME THE MODIFIED PROGRAM "TERMIO.LIB"      +*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*+ CONSTANT DEFINITIONS +*/
const integer alphalen = 10;

/*+ TYPE DEFINITIONS +*/
typedef unsigned char byte1;
const int min_byte1 = 0;
const int max_byte1 = 255;
typedef array<0,alphalen,byte1> alpha;

/*+ VARIABLE DEFINITIONS +*/
/* first 5 bytes for some misc terminal values */
byte1 DELMIS,		/* delay after other functions */
      DELCUS,		     /* delay after moving cursor */
      X_OFF,		     /* offset to add to column */
      Y_OFF,		     /* offset to add to row */
      XY;	             /* flag for column/row or row/column */
/* string sequences */
alpha CLRSCR,		/* CLEAR SCREEN */
      CUR, 		     /* CURSOR ADDRESSING LEADIN STRING */
      eraeos,		     /* CLEAR TO END OF SCREEN */
      eraeol,		     /* CLEAR TO END OF LINE */
      HOME,		     /* HOME UP CURSOR */
      LockKbd,		/* LOCK KEYBOARD */
      UnlockKbd,		/* UNLOCK KEYBOARD */
      LINDEL,	        /* delete screen line containing cursor */
      LININS,	        /* insert a blank line on screen */
      INVON,	        /* turn on highlighting - inverse video */
      INVOFF,	        /* turn off highlighting */
      CRSON,	        /* SET CURSOR ON AND BLINKING */
      CRSOFF;            /* SET CURSOR DISPLAY OFF */


void writes( alpha strng )
/* writes writes a string of type alpha to the console
  device. */
{
    byte1 ix;

    for( ix=1; ix <= strng[0]; ix ++)
        output <<  chr(strng[ix]);
}  /* of writes */


void Clear_Screen() {
    byte1 ix;

    writes( CLRSCR );
    for( ix=0; ix <= DELMIS; ix ++)/**/;
}


void gotoxy(byte1 x_coord, byte1 y_coord) {
    byte1 ix;
    byte1 x_pos, y_pos;

    x_pos = x_coord + X_OFF;
    y_pos = y_coord + Y_OFF;
    if (( XY==2 ) && ( x_pos<31 ))
        x_pos = x_pos + 96;
    writes( CUR );
    if (( XY==1 ) || ( XY==2 ))
        output <<  chr(x_pos) << chr(y_pos) ;
    else
        output <<  chr(y_pos) << chr(x_pos);
    for( ix=0; ix <= DELCUS; ix ++)/**/;
}


boolean INITTERM();

/* RETURNS TRUE IF TERMINAL DATA FILE FOUND  */
/*	  FALSE IF DATA FILE NOT FOUND!     */
typedef file<byte1> BFILE;



static void gets1( BFILE& fb, alpha& strng )
/* gets a string of type alpha from the
     specified file of type BFILE. */
{
    byte1 ix;

    fb >> strng[0];	        /* first byte is always length */
    for( ix=1; ix <= strng[0]; ix ++)
        fb >> strng[ix] ;
}  /* of gets */

boolean INITTERM() {
    byte1 bx;
    BFILE termio;


    /* OPEN file TERMIO.FIL for READ assign TERMIO */
    boolean INITTERM_result;
    reset(termio, "TERMIO.FIL");
    if (eof(termio))    /* file does not exist */
        INITTERM_result = false;
    else {
        INITTERM_result = true;
        /* first 5 bytes in this sequence */
        /* strings must be read back in same sequence as were written */
        termio >>
               bx >>	 /* length byte */
               DELMIS >> DELCUS >> X_OFF >> Y_OFF >> XY;
        gets1( termio, CLRSCR );
        gets1( termio, CUR );
        gets1( termio, eraeos );
        gets1( termio, eraeol );
        gets1( termio, HOME );
        gets1( termio, LockKbd );
        gets1( termio, UnlockKbd );
        gets1( termio, LINDEL );
        gets1( termio, LININS );
        gets1( termio, INVON );
        gets1( termio, INVOFF );
        gets1( termio, CRSON );
        gets1( termio, CRSOFF );
    }  /*else*/
    return INITTERM_result;
}  /* of INITTERM *//* CLOSE(termio); */



