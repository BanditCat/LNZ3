////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#include "lnz.h"

void lmidentity( lmat m ){
  for( u32 i = 0; i < 4; ++i )
    for( u32 j = 0; j < 4; ++j )
      if( i == j )
	m[ i * 4 + j ] = 1;
      else
	m[ i * 4 + j ] = 0;
}


void lmscale( lmat mat, const lvec v ){
  GLfloat m[ 16 ];
  for( u32 i = 0; i < 4; ++i )
    for( u32 j = 0; j < 4; ++j )
      if( i == j )
	if( i == 3 )
	  m[ i * 4 + j ] = 1;
	else 
	  m[ i * 4 + j ] = v[ i ];
      else
	m[ i * 4 + j ] = 0;
  lmmult( mat, m );
}
void lmbasis( lmat mat, const lvec up, const lvec right ){
  lvec front = { up[ 0 ], up[ 1 ], up[ 2 ] };
  lvcross( front, right ); lvnormalize( front );
  lvec rright = { front[ 0 ], front[ 1 ], front[ 2 ] };
  lvcross( rright, up ); lvnormalize( rright );
  lvec rup = { up[ 0 ], up[ 1 ], up[ 2 ] };
  lvnormalize( rup );
  lmat m = { 0 };
  m[ 15 ] = 1.0;
  for( u32 i = 0; i < 3; ++i ){
    m[ i ] = rright[ i ];
    m[ 4 + i ] = rup[ i ];
    m[ 8 + i ] = front[ i ];
  }
  lmmult( mat, m );
}  
void lmprojection( lmat mat, float s ){  
  lmat m;
  for( u32 i = 0; i < 4; ++i )
    for( u32 j = 0; j < 4; ++j )
      if( i == j && i < 2 )
	  m[ i * 4 + j ] = 1;
      else if( i == 3 && j == 2 )
	m[ i * 4 + j ] = s;
      else if( i == 2 && j == 3 )
	m[ i * 4 + j ] = 1;
      else
	m[ i * 4 + j ] = 0;
  lmmult( mat, m );
}

void lmtranslate( lmat mat, lvec v ){
  lmat m;
  for( u32 i = 0; i < 4; ++i )
    for( u32 j = 0; j < 4; ++j )
      if( i == j )
	m[ i * 4 + j ] = 1;
      else if( j == 3 && i < 3 )
	m[ i * 4 + j ] = v[ i ];
      else
	m[ i * 4 + j ] = 0;
  lmmult( mat, m );
}

void lmmult( lmat m, const lmat n ){
  lmat tm;
  for( u32 i = 0; i < 16; ++i )
    tm[ i ] = m[ i ];
  for( u32 i = 0; i < 4; ++i )
    for( u32 j = 0; j < 4; ++j ){
      float a = 0;
      for( u32 k = 0; k < 4; ++k )
	a += n[ i * 4 + k ] * tm[ k * 4 + j ];
      m[ i * 4 + j ] = a;
    }
}

void lvcross( lvec a, const lvec b ){
  lvec t = { a[ 0 ], a[ 1 ], a[ 2 ] };
  a[ 0 ] = t[ 1 ] * b[ 2 ] - t[ 2 ] * b[ 1 ];
  a[ 1 ] = t[ 2 ] * b[ 0 ] - t[ 0 ] * b[ 2 ];
  a[ 2 ] = t[ 0 ] * b[ 1 ] - t[ 1 ] * b[ 0 ];
}
void lvnormalize( lvec v ){
  float s = sqrt( lvdot( v, v ) );
  for( u32 i = 0; i < 3; ++i )
    v[ i ] /= s;
}
float lvdot( const lvec u, const lvec v ){
  return u[ 0 ] * v[ 0 ] +
    u[ 1 ] * v[ 1 ] +
    u[ 2 ] * v[ 2 ];
}
