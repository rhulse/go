//  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
//                          DV.H                                            *
//    This include file contains definitions and function declarations      *
//               for the DESQview module.                                   *
//                                                                          *
//  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
#if !defined(DV_H)
#define DV_H

int     DvInit( void );     // return DESQview version
void    DvSeize( void );
void    DvRelease( void );
void    DvPause( void );

#endif
