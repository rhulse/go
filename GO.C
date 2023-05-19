//
//
//        This module contains the C routines of the GO utility.
//
//

// #define NO_EXT_KEYS    // extensions disabled
#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <stdlib.h>
#include <direct.h>
#include <string.h>
#include <easyc.h>             // defines EQ LT GT etc
#include "keyboard.h"
#include "window.h"
#include "screen.h"
#include "int24.h"
#include "mylib.h"

// global variables

int     file_search = TRUE;
int     dir_search = TRUE;
int     search_drive;
char   *search_string;
char   *dir_search_string;
int     dir_search_length;
char   *file_search_string;
int     matching_file_count = 0;
int     matching_dir_count = 0;
int     dir_offset = 0;
int     file_offset = 15;
int     reset_drive = FALSE;
int     find_first = FALSE;
int     string_found = FALSE;
int     final_message = FALSE;
int     dohelp = FALSE;
int     choice = FALSE;
int     search_all = FALSE;
char   *no_file = " -------- ";
int     original_drive;
extern int critical_error;
unsigned int new_drive;
WINDOW *file_display;

// structure for found files

struct directory
    {
        char *file_name;     // pointer for filename
        char *dir_name;      // pointer for full directory
        struct directory *prev;
        struct directory *next;
    } *first, *node, *new, *top_line, *bottom_line;


int     Args2 ( char * );
int     Args3 ( char * );
void    SetupStrings ( char * );
void    TestDir( void );
void    PrintDirEntry ( WINDOW *, struct directory *, int );
void    AddToDirList ( char *, char * );
int     RunFileWin ( void );
BOOL    FileHomeKey ( WINDOW * );
BOOL    FileEndKey ( WINDOW * );
BOOL    FilePgUpKey ( WINDOW * );
BOOL    FilePgDnKey ( WINDOW * );
BOOL    FileUpKey ( WINDOW * );
BOOL    FileDnKey ( WINDOW * );
BOOL    DoTopLine ( WINDOW * );
BOOL    DoBottomLine ( WINDOW * );
void    Init( void );
void    DoResets( void );
void    Help( void );
void    Bleep ( void );

struct key_s
{
    unsigned int        key;                    // the key that selects the
    BOOLEAN          ( *function)( void * );    // function with pointer to anything
}valid_keys[] =
{
    {   HOME   , ( *FileHomeKey) },
    {   END    , ( *FileEndKey)  },
    {   U_ARROW, ( *FileUpKey)   },
    {   D_ARROW, ( *FileDnKey)   },
    {   PG_UP  , ( *FilePgUpKey) },
    {   PG_DN  , ( *FilePgDnKey) }


};
#define num_keys  sizeof ( valid_keys ) / sizeof ( struct key_s )

void main( int argc, char **argv )
{
    char   *original_dir;

    Init();

    original_drive = DosGetDrive();     // save original drive and dir
    original_dir = getcwd ( NULL, 0 );

    switch ( argc )
    {
        case 3 :    if ( Args3( argv[2] ) )  // valid switches ?
                    {
                        dohelp = TRUE;         // no --- Help
                        exit( 0 );           // then exit
                    }
                    break;                   // yes --- continue

        case 2 :    if ( Args2( argv[1] ) )  // string found ?
                        exit( 0 );           // yes --- exit
                    break;                   // no --- continue

        default :   dohelp= TRUE;              // wrong # or args
                    exit( 0 );               // help then exit
    }

    SetupStrings ( argv[1] ); // create the search string

    if ( dir_search BOR file_search )
    {

        DosCD ( "\\" );                 // start at the root
        TestDir();                      // test it

        if ( matching_file_count EQ 0 ) // if no files
        {
            DosCD ( original_dir );     // restore original directory
            final_message = TRUE;       // not found message
        }
        else if ( matching_file_count EQ 1 OR
                ( matching_dir_count EQ 1 AND choice NE TRUE ) )
        {                   // only one found and no choice
            DosCD ( first -> dir_name );     // change to its directory
            string_found = TRUE;             // new directory message
            reset_drive = FALSE;
        }
        else                                 // more than one file
        {
            if ( RunFileWin () )             // display them
            {
                DosCD ( node -> dir_name );  // if enter pressed do a cd
                string_found = TRUE;         // new directory message
            }
            else                             // if ESC is pressed
                DosCD ( original_dir );      // restore original directory
        }
        free ( original_dir );
    }
    else
    {
        dohelp = TRUE;
        exit( 0 );
    }
}

int Args2 ( char *string )
{
    int temp = FALSE;

    // if a drive is specified or we have to search all then return false
    if ( string[1] NE ':' ) 
    {
        if ( NOT DosCD ( string ) ) // see if off current directory
        {
            temp = TRUE;
        }
        else
        {
            DosCD ( ".." );             //  see if off the parent directory
            if ( NOT DosCD ( string ) )
            {
                temp = TRUE;
            }
            else
            {
                DosCD ( "\\" );             //  see if off root directory
                if ( NOT DosCD ( string ) )
                {
                temp = TRUE;
                }
            }
        }
    }
    return ( temp );
}

int Args3 ( char *arg_three )
{
    int temp = FALSE;

    if ( ( arg_three[0] EQ '-') OR ( arg_three[0] EQ '/') )
        switch ( toupper ( arg_three[1] ) )
        {
            case 'F':   dir_search = FALSE;
                        break;

            case 'O':   find_first = TRUE;
                        break;

            case 'C':   choice = TRUE;
                        break;

            case 'S':   search_all = TRUE;
                        break;

            default :   temp = TRUE;
                        break;
        }
        return ( temp );
}

void    SetupStrings ( char *arg_one )
{
    search_string = strdup ( strupr ( arg_one ) ); // copy the search string

    if ( search_string[1] EQ ':' )          // if a drive is specified
    {
        new_drive = search_string[0] - 'A'; // work out which one
        DosSetDrive ( new_drive );          // and change to it
        if ( critical_error )
            exit(1);
        reset_drive = TRUE;                 // reset drive if file not found
        search_string += 2;                 // move pointer past drive
    }
    else if ( search_string[0] EQ '\\' )
    {
        search_string ++;
        file_search = FALSE;
    }

    if ( strchr ( search_string, '?' ) OR ( strchr ( search_string, '*' ) ) )
        dir_search = FALSE;

    if ( dir_search )
    {
        dir_search_string = search_string;
        dir_search_length = strlen ( dir_search_string );
    }
    if ( file_search )
    {
        file_search_string = malloc ( strlen ( search_string ) + 3 );
        strcpy ( file_search_string,  search_string );
        if ( NOT strchr ( file_search_string, '.' ) )
            strcat ( file_search_string, ".*" );
    }
}

void TestDir( void )
{
    char *current_dir, *cptr;
    int i, n = FALSE;
    struct dta buffer;

    current_dir = getcwd ( NULL, 0 );

// checks to see if the current directory matches.

    i = strlen ( current_dir );

    if ( dir_search AND ( i GE dir_search_length ) )
    {
        cptr = current_dir + ( i - dir_search_length );
        if ( NOT strcmp ( cptr, search_string ) )
        {
            AddToDirList ( current_dir, NULL );
            n = TRUE;
        }
    }

    if ( file_search )
    {
        if ( NOT DosFindFirst ( file_search_string,
                        _A_NORMAL BOR _A_HIDDEN BOR _A_SYSTEM, &buffer ) )
        {
            if ( NOT find_first )
            {
                matching_dir_count ++;
                n = TRUE;
                do
                {
                    AddToDirList ( current_dir, buffer.name );
                }
                while ( NOT DosFindNext ( &buffer ) );
            }
            else        // batch file type mode
            {           // stops at the first matching file it comes to
                free ( current_dir );
                string_found = TRUE;
                exit ( 0 );
            }
        }
    }

// if there was a match in the current directory then update the file offset

    if ( n AND ( i GT file_offset ) )
        file_offset = i + 1;    // set the starting point of the files

    // This is the bit that recursively calls itself and moves through
    // a directory structure

    if ( NOT  DosFindFirst ( "*.*", _A_SUBDIR, &buffer ) )
    {
        do
        {
            if ( buffer.attrib EQ _A_SUBDIR AND buffer.name[0] NE '.' )
            {
                DosCD ( buffer.name );
                TestDir();
                DosCD ( current_dir );
            }
        }
        while ( NOT DosFindNext ( &buffer ) );
    }
    free ( current_dir );
}

void PrintDirEntry ( WINDOW *win, struct directory *line, int n )
{
    WnPrintxy ( win, dir_offset, n, line -> dir_name );
    WnPrintxy ( win, file_offset, n,
    (  ( line -> file_name[0] EQ NULL ) ? no_file : line -> file_name ) );
}


void AddToDirList ( char *dir, char *file )
{
    if ( ( new = malloc ( sizeof ( struct directory ) ) ) EQ NULL )
    {
        AllocError();
    }
    if ( file NE NULL )
    {
        if ( ( new -> file_name = malloc ( strlen ( file ) + 1 ) ) EQ NULL )
        {
            AllocError();
        }
        strcpy ( new -> file_name, file ); // store file name
    }
    else
        new -> file_name = NULL;

    if ( ( new -> dir_name = malloc ( strlen ( dir ) + 1 ) ) EQ NULL )
    {
        AllocError();
    }
    strcpy ( new -> dir_name, dir ); // store directory name

    new -> prev = node; // node points to last in chain
    new -> next = NULL; // next points to a NULL


    if ( first EQ NULL )
        first = new;
    else
        node -> next = new;

    node = new;
    matching_file_count ++;
}

   //-------------------------------------------------------------------
   //  Runs the window with the file names in it.
   //-------------------------------------------------------------------

int RunFileWin ( void )
{
    WINDOW *file_win;
    int n, entry, wid, hgt, loop = TRUE, cd = TRUE;

    wid = ( ( wid = file_offset + 15 ) GT 79 ) ? 79 : wid;
    hgt = ( matching_file_count GT 20 ) ? 20 : matching_file_count;

    file_win = WnMake( wid, hgt, 1, 1, V_MAGENTA, V_BLACK );
    file_win -> copt = 0;

    bottom_line = first;
    top_line = first;

     // prints the files in the window

    for ( n = 0 ; n LT hgt ; n++ )
    {
        if ( bottom_line NE NULL )
        {
            PrintDirEntry ( file_win, bottom_line, n );
        }
        if ( n NE hgt - 1 )
            bottom_line = bottom_line -> next;
    }

    node = first;           // node points to top line too

    WnOpen ( file_win );

    do        // Main loop starts here
    {               
        WnChgAttr ( file_win, 0, file_win -> copt,  // hilites current option
                        V_LTWHITE, V_MAGENTA, file_win -> wid );
        VsDisp();

         // Wait for a key to be pressed and switch on it.
        entry = GetKey();

        WnChgAttr ( file_win, 0, file_win -> copt,  // dims current option
                    file_win -> fgc, file_win -> bgc, file_win -> wid );

        if ( entry EQ ENTER )
        {
            loop = FALSE; reset_drive = FALSE;  break;
        }
        else if ( entry EQ ESC )
        {
            loop = FALSE; cd = FALSE;  break;
        }
        else
        {
            n = num_keys;

            while ( n-- )
            {
                if ( entry EQ valid_keys[n].key )
                {
                    loop = ( valid_keys[n].function )( file_win );
                }
            }
        }
    }
    while( loop );

    return ( cd );        // do we change directory ??
}

   //-------------------------------------------------------------------
   //    Processes the HOME key
   //-------------------------------------------------------------------

BOOL FileHomeKey ( WINDOW *win )
{
    while ( DoTopLine ( win ) )

    node = top_line;
    win -> copt = 0;
    return ( TRUE );
}

   //-------------------------------------------------------------------
   //    Processes the END key
   //-------------------------------------------------------------------

BOOL FileEndKey ( WINDOW *win )
{
    while ( DoBottomLine ( win ) )

    node = bottom_line;
    win -> copt = win -> hgt - 1;
    return ( TRUE );
}

   //-------------------------------------------------------------------
   //   Processes the page up key
   //-------------------------------------------------------------------

BOOL FilePgUpKey ( WINDOW *win )
{
    int n = win -> hgt - 1;

    while ( n-- NE 0 )
        DoTopLine( win );
        
    return ( TRUE );
}
   //-------------------------------------------------------------------
   //   Processes the page down key
   //-------------------------------------------------------------------

BOOL FilePgDnKey ( WINDOW *win )
{
    int n = win -> hgt - 3;

    while ( n-- NE 0 )
        DoBottomLine( win );
        
    return ( TRUE );
}

   //-------------------------------------------------------------------
   //   Processes the up arrow
   //-------------------------------------------------------------------

BOOL FileUpKey ( WINDOW *win )
{
    if ( win -> copt GT 0 )
    {
        win -> copt--;
        node = node -> prev;
    }
    else
        DoTopLine( win );

    return ( TRUE );
}

   //-------------------------------------------------------------------
   // Processes the down arrow
   //-------------------------------------------------------------------

BOOL FileDnKey ( WINDOW *win )
{
    if ( win -> copt LT win -> hgt - 1 )
    {
        win -> copt++;
        node = node -> next;
    }
    else
        DoBottomLine( win );

    return ( TRUE );
}

   //-------------------------------------------------------------------
   // Erases the top line and replaces it.
   //-------------------------------------------------------------------

BOOL DoTopLine ( WINDOW *win )
{
    int     temp = FALSE;

    if ( top_line -> prev NE NULL )
    {
        top_line = top_line -> prev;
        bottom_line = bottom_line -> prev;
        node = node -> prev;
        WnScroll ( win, DOWN, 0, win -> hgt - 1 );
        WnCls ( win, 0, 1 );
        PrintDirEntry ( win, top_line, 0 );
        temp++;
    }
    return ( temp );
}

   //-------------------------------------------------------------------
   // Erases the bottom line and replaces it.
   //-------------------------------------------------------------------

BOOL DoBottomLine ( WINDOW *win )
{
    int hgt = win -> hgt - 1 , temp = FALSE;

    if ( bottom_line -> next NE NULL )
    {
        bottom_line = bottom_line -> next;
        top_line = top_line -> next;
        node = node -> next;
        WnScroll ( win, UP, 1, hgt );
        WnCls ( win, hgt, 1 );
        PrintDirEntry ( win, bottom_line, hgt );
        temp++;
    }
    return ( temp );
}

void Init( void )
{
    extern int video_method;         // use direct screen writes
    extern int background;  // pop up over old screen
    extern int mode;        // screen mode in use
    extern int shadow_attr; // attribute shadowing.
    extern int mode_change; // use original text mode
    extern int snow_free;   // snow free if CGA found
    extern  int hide_cursor;

    atexit( DoResets );     //  Reset screen and turn Cursor on

    mode_change = FALSE;  // dont change mode if it is any 80 x 25 text mode
    background = TRUE;    // pop up over the old screen
    snow_free = TRUE;
//    hide_cursor = FALSE;

    cputs ( "Searching .....\r" );

    WnInit();
    GoStealInt24();

    // initialises the first structure for any directory entries

    first = NULL;
    node = NULL;
}

void DoResets( void )
{
    if ( reset_drive )
        DosSetDrive ( original_drive );

    WnExit();
    GoRestoreInt24();

    if ( critical_error )
    {
        cputs ( "\n    System error...no change\n" );
        DosSetDrive ( original_drive );
    }
    else if ( string_found )
    {
        cputs ( "\n    New directory selected...\n" );
    }
    else if ( final_message )
    {
        cputs ( "\n    The requested name was not found.\n" );
    }
    else if ( dohelp )
    {
        Help();
    }
}

void Help( void )
{
    cputs ("GO Version 2.30 (C) 22 Aug 1992 by Richard Hulse.\r\n"
           " GO  moves quickly from one directory to another\r\n\n"
           "     Syntax : GO [drive:][\\] name [-F]\r\n"
           " where :\r\n"
           "   DRIVE is the drive to search.\r\n"
           "          The current drive is the default.\r\n"
           "  NAME is the file or directory to GO to.\r\n\n"
           " Using a \\ before the name will search only directories.\r\n"
           " Using a DOS wildcard ( * or ? ) or the -F switch will search\r\n"
           " only files.\r\n" );
}
