//
//  CDX.C - Change Directory, Extended
//
//  By: Michael Holmes & Bob Flanders
//      Copyright (c) 1991, Ziff Communications Co.
//
//  This module has been tested to compile and work under
//      MSC 5.1 and 6.0
//      TurboC 2.0
//      Borland C++ 2.0
//
//  Use the following commands to compile, based on compiler used:
//
//  Microsoft C 6.0:
//      cl -AT -Gs -Os cdx.c /link /NOE
//
//  Microsoft C 5.1:
//      cl -AS -D_MSC_VER=510 -Gs -Os cdx.c /link /NOE
//
//  Turbo C 2.0:
//      tcc -mt -G -O -Z cdx.c
//
//  Borland C++ 2.0:
//      bcc -mt -lt -G -O -Z cdx.c
//
//  Modification log:
//  -----------------
//
//  13/15/91 - Initial version completed. (rvf & mdh)


//
//     Common Includes for all compilers
//
#include <dos.h>                // compiler header files
#include <stdlib.h>
#include <string.h>
#include <easyc.h>

#define  QLEN   68              // maximum len of question

//
//  Microsoft C dependancies
//

#if defined(_MSC_VER)           // MSC specific definitions

#pragma pack(1)                 // pack structs/byte boundry
#include <memory.h>             // MSC header file needed

void _far *    _fmalloc(size_t);    // far malloc function
typedef struct find_t FindParms;    // dos find block

//
//  MSC 6.0 Dependancies
//

#if    _MSC_VER GE 600          // MSC 6 or greater

#define _FASTCALL _fastcall     // MSC v6 quick calling

#else                           // end of MSC 6 definitions

//
//  MSC 5.1 Dependancies
//

#define _FASTCALL               // MSC 5 definitions

void _fstrcpy( char far *to , char far *from ) // _fstrcpy interface
{                               // use memmove to move data
    movedata(FP_SEG(from), FP_OFF(from),
	     FP_SEG(to),   FP_OFF(to),	 QLEN);
}

#endif                          // end of MSC 5 definitions

//
//  nullcheck -- replace MSC's null pointer assignment routine
//

int  _nullcheck()
{
    return(0);
}

//
//  setenvp -- replace MSC's set environment routine
//

void _setenvp()
{
    return;
}

//
//  Turbo C & Borland C Dependancies
//

#elif defined(__TURBOC__)       // Turbo C definitions

#include <mem.h>                // TC header files needed
#include <dir.h>
#include <alloc.h>

#define _FASTCALL _fastcall     // not supported: fastcall
#define _far far                // far definition

typedef struct ffblk FindParms; // find parameters

extern int directvideo = 0;     // use bios routines

#define _ffree       farfree    // free call
#define _fmalloc     farmalloc  // far allocate call
#define name         ff_name    // name field in parms
#define attrib       ff_attrib  // file attributes field

//
//  Turbo C 2.0 dependancies
//

#if (__TURBOC__ < 0x0297)

void _fstrcpy(char far *to, char far *from) // _fstrcpy interface
{                                           // use memmove to move data
    movedata(FP_SEG(from), FP_OFF(from),
	     FP_SEG(to),   FP_OFF(to),	 QLEN);
}

#endif

#endif                      // end of Borland def's

//
//  Common Definitions
//

#define CHR(x)  x + 'A' - 1 // nbr to drive letter

//
//  Formal routine declarations
//

void _FASTCALL srch_drv( int , char );     // formal declarations
void _FASTCALL cdxlook( char * , char * );
int  _FASTCALL cdxdir( char * , int * , FindParms * );
void	       cdxhelp();
void _FASTCALL cdxreq( char * );
void	       cdxask();
void _FASTCALL cdxdisp( char * );
void _FASTCALL cdxff( char * );
int  _FASTCALL drvrdy( char );

//
//  global work areas
//

int level,                  // current level
    batch,                  // /b switch encountered
    f_parm,                 // file parameter number
    ac;                     // argument count

enum	{
    NOFF,                   // no /F switch
    FFSET,                  // /F found, no * directory
    FFALL                   // /F found, * dir found
    } fsw;                  // /f switch found

char    **av,               // argument list
    *check_path,            // path to check & not scan
    *file_wa;               // work area for file name

FindParms ffb;              // file find block

//
//     request structure & pointers
//

struct  req
    {
    struct req _far *nxtreq;		    /* pointer to next request	*/
    char	    reqdisp;		    /* TRUE if already displayed*/
    char	    reqpath[QLEN];	    /* path for request w/drive */
    } _far *firstreq, _far *lastreq;        /* first & last pointers    */

//
//  main -- CDX  change directory, extended
//

void    main( int argc , char *argv[] )
{
    char    start_path[65],     // starting path
        drive_list[27],         // list of drives to search
        *q, *p;                 // work pointer
    int start_drive,            // starting drive
        max_drive,              // max number of drives
        start_parm,             // 1st arg to search for
        floppies = 0,           // search floppy drives
        i, j;                   // work counter
    static
    char    sn[3][3] = { "\\", ".", ".." }, // special directory names
            hdr[] = "CDX 1.0 -- Copyright (c) 1991 Ziff Communications Co.\r\n"
            "PC Magazine þ Michael Holmes and Bob Flanders\r\n";


    cputs( hdr );                   // put out pgm banner
    ac = argc;                      // make arg count global

    if ( ac EQ 1 )                  // q. need help?
        cdxhelp();                  // a. yes .. give it to 'em

    firstreq = ( void _far * ) 0;   // zero first request
    lastreq  = ( void _far * ) 0;   // ..and last request ptrs

    check_path = malloc(65);        // get memory for path
    file_wa = malloc(65);           // get memory for file work

//
//  rearrange paramters, /parms first
//

    av = ( char ** )malloc( sizeof( char * )*( ac+2 ) );    // allocate arg ptrs

    av[j = 0] = argv[0];              // setup base pointer

    for ( i = j = 1; i LE ac; i++ )   // setup slash-parameters
    {
        strupr( argv[i] );            // uppercase string

        if ( argv[i][0] EQ '/' )      // q. slash parameter?
        {                             // a. yes ..
            av[j++] = argv[i];        // .. move in the pointer

            if ( argv[i][1] EQ 'F')   // q. /F parameter?
                av[j++] = argv[++i];  // a. yes .. next too
        }
    }

    for ( i = 1; i LE ac; i++ )         // setup dir's, if any
    {
        if ( argv[i][0] NE '/')         // q. slash parameter?
            av[j++] = argv[i];          // a. no .. set dir parm

         else if ( argv[i][1] EQ 'F' )  // q. /F parmater?
            i++;                        // a. yes .. skip next parm
    }

    p = av[1];                          // get pointer to 1st arg

    _dos_getdrive( &start_drive );      // ..then current drive
    _dos_setdrive( 0 , &max_drive );    // get max nbr of drives

    drive_list[0] = CHR( start_drive ); // setup default drive list
    drive_list[1] = 0;

//
//  parse parameters
//

    for ( start_parm = 1; start_parm < ac; start_parm++ ) // for all arguments
    {
        if ( *av[start_parm] NE '/' )   // q. slash paramter?
            break;                      // a. no .. exit now

        p = av[start_parm];             // get pointer to string

        switch ( *(++p) )               // switch character
        {
            case 'B':                   // batch switch
            batch = 1;                  // accept first found
            break;                      // check next parameter

            case 'P':                   // prompt switch
            batch = 0;                  // accept first found
            break;                      // check next parameter

            case 'F':                   // find file parameter
            if ( ++start_parm GE ac )   // q. not enough parms?
                cdxhelp();              // a. yes .. get some help

            fsw = FFSET;            // show /f encountered
            f_parm = start_parm;    // save parameter number
            break;                  // check next parameter

            case '+':               // search floppies?
            floppies = 1;           // set the switch
            break;                  // check next parm

            default:                // error otherwise
            cdxhelp();              // give help & die
        }
    }

    if ( start_parm GE ac )           // q. directories not given?
        if ( fsw )                    // q. file switch given?
        {
            av[start_parm] = "*";   // a. yes .. do all drives
            ac++;                   // bump up argument count
        }
    else
        cdxhelp();                  // else .. just give help

//
//  Build drive list
//

    if ( fsw AND (*av[start_parm] EQ '*' ) )     // q. need to check root?
        fsw = FFALL;                            // a. yes .. set flag
                    // q. drive list supplied?
    if ( ( q = strchr ( p=av[start_parm], ':' ) ) NE 0 )
    {                                           // a. yes .. build list
        *q = 0;                                 // .. and terminate string

        if ( NOT strlen(av[start_parm] = ++q ) )    // q. anything after drives?
            if ( ++start_parm GE ac )               // q. any other arguments?
            {
                av[start_parm] = "*";           // a. no .. force any dir
                ac++;
            }

        strupr( q = p );                        // uppercase the list
                        // q. look at all drives or exclude some drives?
        if ( ( *p EQ '*') OR ( *p EQ '-') )
        {               // a. yes .. set up list
            for ( p = drive_list, i = 3; i LE max_drive; i++ )
                if ( NOT strchr ( q , CHR(i) ) )    // q. in exclude list?
                    *p++ = CHR(i);                  // a. no .. add to list

            for ( i = 1; ( i LT 3 ) AND floppies; i++) // check if A: & B: are out
                if ( NOT strchr ( q , CHR(i) ) )       // q. in exclude list?
                    *p++ = CHR(i);                     // a. no .. add to list

            *p = 0;                              // terminate drv search list
        }
        else
            strcpy( drive_list , p );            // else .. copy in drives
    }

//
//  Check for "special" parameters \, .. (parent) and . (current)
//

    for ( p = av[start_parm] , i = 3; i--; )    // check the special names
        if ( NOT strcmp( p , sn[i] ) AND        // q. is it a special arg..
            ( start_parm EQ ( ac - 1 ) ) )      // .. and is the only parm
        {
            _dos_setdrive(drive_list[0] -       // a. yes .. change drives
				('A' - 1), &i);
            chdir(p);                           // .. and directories ..
            exit(0);                            // .. then exit
        }

//
//  Start searching the requested drives
//

    cdxask();                       // tell 'em we're looking..

    for ( p = drive_list; *p; p++ )     // loop thru drive list
        srch_drv( start_parm , *p );    // ..checking for the path

    while ( firstreq )              // q. request active?
        cdxask();                   // a. yes .. check if ans'd

//
//  If we get here, the path was not found.
//

    batch = 0;                      // let this msg get out
    cdxdisp( "Requested path/file not found\n\r" );   // ..give error message
    exit(1);                        // ..then exit w/error
}

//
//  srch_drv -- check a drive for a particular directory
//

void _FASTCALL srch_drv( int start_parm ,   // 1st directory entry
                         char drive )       // drive to search
{
    int i;                              // work drive nbr
    char    cur_path[65];               // current path
    register
    char    *p, *q;                     // work pointers


    if ( drvrdy( drive ) )              // q. is drive ready?
        return;                         // a. no .. exit now

    drive -= ('A' - 1);                 // setup drive as 1=A:

    *check_path = 0;                    // clear check path
    _getdcwd( drive , cur_path , sizeof(cur_path) );    // get current path

    if ( *( p = av[start_parm] ) EQ '\\' )  // q. start from root?
    {
        cur_path[3] = 0;                // a. yes .. chg to root
        p++;                            // ..and start past "\"

        if ( strcmp( av[start_parm] , "\\" ) EQ 0 )     // q. 1st parm bkslash?
            p = av[++start_parm];       // a. yes.. reset start
    }

    if ( cur_path[strlen( cur_path ) - 1 ] NE '\\' )    // q. end in "\"?
        strcat( cur_path , "\\" );                      // a. no .. add one

//
//  Look for the directory on requested drive
//

    for(;;)                                 // loop thru looking..
    {
        level = start_parm;                 // setup which arg is 1st
        cdxlook( cur_path , p );            // look for a directory
        strcpy( check_path , cur_path );    // save old path

        cur_path[ strlen( cur_path ) - 1 ] = 0; // trim off trailing "\"

        if ( ( q = strrchr( cur_path , '\\' ) ) NE 0 ) // q. are we at the root?
            *(q + 1) = 0;               // a. no .. back up a subdir
        else
            break;                      // else .. not found here
    }
}

//
//  cdxlook -- look at a particular directory for target
//

void _FASTCALL cdxlook( char *wtl , // where to look
               char *wtlf )         // what to look for
{
    int    i;                       // flag variable
    register
    char   *wwtlf,                  // working what to look for
           *p;                      // string pointer
    FindParms *fb;                  // find block pointer


    fb = ( FindParms *)malloc( sizeof( FindParms ) );   // get a find block
    wwtlf = (char *)malloc(65);     // ..and working string

    if ( ( fsw EQ FFALL ) AND ( strlen( wtl ) EQ 3 ) )  // q. file switch set?
        cdxff( wtl );               // a. yes .. check here

    p = &wwtlf[strlen(wtl)];        // save end of string ptr
    strcpy( wwtlf , wtl);           // build what to look for..
    strcpy( p , wtlf );
    strcat( p , strchr( wtlf, '.' ) ? "*" : "*.*" ); // append appropriate wild

//
//  loop to reentrantly search for requested directory
//

    for ( i = 0; cdxdir( wwtlf , &i , fb ) ; ) // look for dir's to search
    {
        strcpy( p , fb->name );         // copy in directory name
        strcat( p , "\\" );             // ..and the trailing "\"

        if ( ++level EQ ac )            // q. at bottom level?
        {                               // a. yes .. test this dir
            if ( fsw NE NOFF )          // q. file switch set?
                cdxff( wwtlf );         // a. yes .. check for file
             else                       // else ..
                cdxreq( wwtlf );        // .. build req for this dir
        }
        else
            cdxlook( wwtlf , av[level] );   // else, look for next level

        level--;                        // on failure.. go back 1
    }

    strcpy( p , "*.*" );                // setup to look at any dirs

    for ( i = 0; cdxdir( wwtlf , &i , fb ); )   // look thru dir at this lvl
    {
        strcpy( p , fb->name );         // bld nxt search recoursion
        strcat( p , "\\" );             // ..ending with a backslash

        if ( strcmpi( check_path , wwtlf ) )    // q. already do this dir?
            cdxlook( wwtlf , wtlf );    // a. no .. look down path
    }

    free( wwtlf );                      // free gotten memory
    free( fb );                         // . . . . .

}

//
//  cdxdir -- find the next directory entry
//

int _FASTCALL cdxdir( char *wwtlf ,     // working what to look for
             int *l ,                   // first/next flag
             FindParms *fb )            // find directory entry blk
{
//
//  look for directory entry using Find First & Find Next
//

    while( ( *l ? _dos_findnext( fb ) :     // q. find a file/directory?
             _dos_findfirst( wwtlf , _A_SUBDIR , fb ) ) EQ 0 )
    {
        *l = 1;                                 // a. yes .. set flag
        if ( ( fb->attrib BAND _A_SUBDIR ) AND  // q. a sub-directory?
            ( fb->name[0] NE '.' ) )            // ..and not "." or ".."
        return(1);                              // a. yes .. return a subdir

        else if ( kbhit() )                 // q. key hit?
            cdxask();                       // a. yes .. check it out
    }

    return( *l = 0 );                       // return to caller, w/error
}

//
//  cdxreq -- build request structure
//

void _FASTCALL cdxreq( char *msg )  // allocate/build req
{
    register
    struct req _far *wr;            // work req pointer

    int    i;                       // work variable


    while ( ( wr = _fmalloc( ( size_t )     // q. memory allocate ok?
           sizeof( struct req ) ) ) EQ 0 )
        cdxask();                           // a. no .. wait for answer

    wr->reqdisp = 0;                        // show not displayed
    _fstrcpy( wr->reqpath , (char far *) msg ); // .. copy in message
    wr->nxtreq = (void _far *) 0;   // zero next pointer

    if ( firstreq )                 // q. previous request?
        lastreq->nxtreq = wr;       // a. yes.. set prev's nxt
    else
        firstreq = wr;              // else .. set up first req

    lastreq = wr;                   // .. and set up new last

    cdxask();                       // let user see the prompt
}

//
//  cdxask -- display path & ask
//

void cdxask()                       // ask if this dir is it
{
    char    ques[75],               // question to ask
    c;                              // response character
    static
    int looking = 0;                // looking displayed
    register
    struct  req _far *wr;           // work req pointer
    int i;                          // work variable
//
//  If any requests are present ...
//
    if ( firstreq )                     // q. any request present?
    {                                   // a. yes.. process it
        if ( NOT firstreq->reqdisp )    // q. message displayed?
        {                               // a. no .. display it
            _fstrcpy( ( char far *) ques , // .. copy in message
            firstreq->reqpath );
            strcat( ques , " ?" );  // add a question mark

            cdxdisp( ques );        // .. display the question

            firstreq->reqdisp = 1;  // indicate req displayed
            looking = 0;            // show looking not disp'd
        }

        if ( ( i = kbhit() ) NE 0 ) // q. any char yet?
        c = getch();                // a. yes .. get it

        if ( i OR batch )           // q. char found or batch?
        {
            if ( ( toupper(c) EQ 'Y' ) OR batch )   // q. this the path?
            {                                       // a. yes.. go to the path
                _fstrcpy( ( char far * ) ques ,     // move in the path
                firstreq->reqpath );

                _dos_setdrive( *ques - ('A'-1) , &i );  // set the drive

                if ( strlen( ques ) GT 3 )           // q. root directory?
                    ques[ strlen( ques ) - 1 ] = 0; // a. no .. remove last '\'

                chdir( ques );          // .. go to the path
                exit(0);                // .. and then to dos
            }
            else                        // else.. if response is no
            {                           // free the request
                wr = firstreq;          // save pointer to current
                firstreq = wr->nxtreq;  // reset first pointer
                _ffree( wr );           // free the request

                cdxask();               // display another entry ..
            }
        }
    }
//
//  This is the first request ...
//
    else                    // no requests are on queue
    {
        while( kbhit() )    // clear other keys..
        getch();            // .. don't allow buffering

        if ( NOT looking )  // q. looking displayed?
        {                   // a. no .. display it
            cdxdisp( "Searching ..." ); // display "men at work"
            looking = 1;                // .. show we're displayed
        }
    }
}

//
//  cdxdisp -- display the requested value
//
void    _FASTCALL cdxdisp( char *ques ) // display the request
{
    register
    int i;              // work variable
    char    line[76];   // work area

    if ( batch )        // q. in batch mode?
        return;         // a. yes .. don't display

    line[0] = '\r';                     // start with return
    for( i = 73; i--; line[ i+1 ] = ' ' );  // blank out line
        line[74] = '\r';                // .. kill the string
    line[75] = 0;                       // .. end the string
    cputs( line );                      // .. clear question line

    cputs( ques );                      // display the question
}

//
//  cdxhelp -- give help message and die
//
void	cdxhelp()
{
    cputs("\nformat  CDX  [/B] [/+] [/F file] [d:]p1 p2 .. pn\n\r\n"
          "  where:\r\n"
          "    /B       change to first qualifying directory\r\n"
          "    /+       include floppies in * or - search\r\n"
          "    /F file  find directory containing file\r\n"
          "             file may contain wildcard ? or * characters\r\n"
          "    d        are the drives to search\r\n"
          "               *: searches all drives\r\n"
          "               -ddd: searches all drives except ddd\r\n"
          "               ddd:  searches only drives ddd\r\n"
          "    p1..pn   are the names of directories to search\r\n\n");

    exit(1);        // give help and rtn to DOS
}

//
//  cdxff -- check if /f file in this directory
//
void    _FASTCALL cdxff( char *s )
{
    char    file_wa[65];            // file name work area

    strcpy( file_wa , s );          // a. yes .. copy in dir
    strcat( file_wa , av[f_parm] ); // .. and file parameter

    if ( _dos_findfirst ( file_wa , // q. file found?
         _A_NORMAL , &ffb ) EQ 0 )
    cdxreq( s );                    // a. yes .. build request
}

//
//  drvrdy -- check if drive is ready
//
int _FASTCALL drvrdy( char d )  // returns true if drive Not rdy
{
    struct  dos4_i25            // dos 4.0 int 25 block
	{
        long sector;            // sector to read
        int  num_secs;          // number of sectors to read
        char far *read_addr;    // address of input area
    } d4_i25, far *d4_i25p;     // area and pointer

    union   REGS r;             // work registers
    struct  SREGS s;            // ..and work segment regs

    char    _far *bootrec;      // place to read boot sector

    d -= 'A';                   // d = 0 based drive number

//
//  DOS 3 or above, check for remote drive
//
    if ( _osmajor GE 3 )            // q. at least DOS 3.00?
    {                               // a. yes ..
        r.x.ax=0x4409;              // ax = check remote status
        r.h.bl = d + 1 ;            // bl = number of drive
        int86x(0x21, &r, &r, &s);   // ask dos about drive

        if ( r.x.dx BAND 0x1000 )   // q. is drive remote?
            return(0);              // a. yes .. assume ready
    }
    bootrec = _fmalloc( ( size_t ) 4096 );  // get space for a sector

//
//  DOS 3.31 and above INT 25h (read sector) interface
//
// dos version 4 interface?
    if ( _osmajor GT 3 OR ( ( _osmajor EQ 3 ) AND ( _osminor GT 30 ) ) )
    {                               // a. yes.. use long read
        r.x.cx = -1;                // cx = 0xffff
        d4_i25.sector = 0L;         // read sector 0
        d4_i25.num_secs = 1;        // .. for 1 sector
        d4_i25.read_addr = bootrec; // .. into boot record
        d4_i25p = &d4_i25;          // set up pointer
        r.x.bx = FP_OFF( d4_i25p ); // bx = offset of parm block
        s.ds   = FP_SEG( d4_i25p ); // ds = segment of block
    }
//
//  pre-DOS 3.31 INT 25h (read sector) interface
//
    else
    {
        r.x.cx = 1;                 // cx = number of sectors
        r.x.dx = 0;                 // dx = starting sector
        r.x.bx = FP_OFF( bootrec ); // bx = offset of buffer
        s.ds   = FP_SEG( bootrec ); // ds = segment of buffer
    }

    r.h.al = d;                     // al = drive number
    int86x( 0x25 , &r , &r , &s );  // read boot sector

    _ffree( bootrec );              // free the boot record

    return( r.x.cflag );            // return true if not ready
}
