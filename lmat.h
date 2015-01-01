////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

// Matrix functions.

#ifndef LMAT_H
#define LMAT_H

typedef GLfloat lmat[ 16 ];
typedef GLfloat lvec[ 3 ];

void lmidentity( lmat );
void lmcopy( lmat dst, const lmat src );
void lmscale( lmat, const lvec );
void lmbasis( lmat, const lvec up, const lvec right );
void lmmult( lmat, const lmat );
void lmprojection( lmat, float );
void lmtranslate( lmat, lvec );
void lmtranspose( lmat );
void lminvert( lmat );
void lmlogMatrix( const lmat m );

void lvcopy( lvec, const lvec );
void lvcross( lvec, const lvec );
void lvscale( lvec, float );
void lvadd( lvec v, const lvec w );
float lvdot( const lvec, const lvec );
void lvnormalize( lvec );

#endif // LMAT_H
