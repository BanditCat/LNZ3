////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

// Matrix functions.

#ifndef LMAT_H
#define LMAT_H

typedef GLfloat lmat[ 16 ];
typedef GLfloat lvec[ 3 ];

void lmidentity( lmat );
void lmscale( lmat, const lvec );
void lmbasis( lmat, const lvec up, const lvec right );
void lmmult( lmat, const lmat );
void lmprojection( lmat, float );
void lmtranslate( lmat, lvec );

void lvcross( lvec, const lvec );
float lvdot( const lvec, const lvec );
void lvnormalize( lvec );

#endif // LMAT_H
