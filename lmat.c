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
void lmcopy( lmat dst, const lmat src ){
  for( u32 i = 0; i < 16; ++i )
    dst[ i ] = src[ i ];
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

void lmprojection( lmat mat, float fov, float aspect, float near, float far ){  
  lmat m;
  float depth = far - near;
  
  for( u32 i = 0; i < 16; ++i )
    m[ i ] = 0;
  m[ 1 + 4 * 1 ] = aspect / ( tan( 0.5f * fov ) );
  m[ 0 + 4 * 0 ] = m[ 1 * 4 + 1 ] / ( aspect * aspect );
  m[ 2 + 4 * 2 ] = far / depth;
  m[ 3 + 4 * 2 ] = ( -far * near ) / depth;
  m[ 2 + 4 * 3 ] = 1.0;
  m[ 3 + 4 * 3 ] = 1.0;
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
void lvmult( lvec va, const lmat m ){
  for( u32 i = 0; i < 4; ++i ){
    lvec v;
    lvcopy( v, va );
    float a = 0.0;
    for( u32 j = 0; j < 3; ++j )
      a += v[ j ] * m[ 4 * i + j ];
    a += 1.0 * m[ 4 * i + 3 ];
    va[ i ] = a;     
  }
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

void lmtranspose( lmat m ){
  lmat t;
  lmcopy( t, m );
  for( u32 i = 0; i < 4; ++i )
    for( u32 j = 0; j < 4; ++j )
      m[ i * 4 + j ] = t[ j * 4 + i ];
}
void lmlogMatrix( const lmat m ){
  for( u32 i = 0; i < 4; ++i ){
    if( i == 0 )
      SDL_Log( "{ %4.4f, %4.4f, %4.4f, %4.4f\n", m[ 0 ], m[ 1 ], m[ 2 ], m[ 3 ] );
    else if( i == 3 )
      SDL_Log( "  %4.4f, %4.4f, %4.4f, %4.4f }\n\n", m[ 0 + i * 4], m[ 1 + i * 4 ], m[ 2 + i * 4], m[ 3 + i * 4] );
    else 
      SDL_Log( "  %4.4f, %4.4f, %4.4f, %4.4f\n", m[ 0 + i * 4], m[ 1 + i * 4], m[ 2 + i * 4 ], m[ 3 + i * 4 ] );
  }
}
void lvcopy( lvec v, const lvec w ){
  for( u32 i = 0; i < 3; ++i )
    v[ i ] = w[ i ];
}

void lvadd( lvec v, const lvec w ){
  for( u32 i = 0; i < 3; ++i )
    v[ i ] += w[ i ];
}
void lvscale( lvec v, float s ){
  for( u32 i = 0; i < 3; ++i )
    v[ i ] *= s;
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


// Helper functions from http://www.cs.rochester.edu/~brown/Crypto/assts/projects/adj.html 
/*
   Recursive definition of determinate using expansion by minors.
*/
float Determinant(float **a,int n)
{
   int i,j,j1,j2;
   float det = 0;
   float **m = NULL;

   if (n < 1) { /* Error */

   } else if (n == 1) { /* Shouldn't get used */
      det = a[0][0];
   } else if (n == 2) {
      det = a[0][0] * a[1][1] - a[1][0] * a[0][1];
   } else {
      det = 0;
      for (j1=0;j1<n;j1++) {
         m = lmalloc((n-1)*sizeof(float *));
         for (i=0;i<n-1;i++)
            m[i] = lmalloc((n-1)*sizeof(float));
         for (i=1;i<n;i++) {
            j2 = 0;
            for (j=0;j<n;j++) {
               if (j == j1)
                  continue;
               m[i-1][j2] = a[i][j];
               j2++;
            }
         }
         det += pow(-1.0,j1+2.0) * a[0][j1] * Determinant(m,n-1);
         for (i=0;i<n-1;i++)
            free(m[i]);
         free(m);
      }
   }
   return(det);
}




/*
   Find the cofactor matrix of a square matrix
*/
void CoFactor(float **a,int n,float **b)
{
   int i,j,ii,jj,i1,j1;
   float det;
   float **c;

   c = lmalloc((n-1)*sizeof(float *));
   for (i=0;i<n-1;i++)
     c[i] = lmalloc((n-1)*sizeof(float));

   for (j=0;j<n;j++) {
      for (i=0;i<n;i++) {

         /* Form the adjoint a_ij */
         i1 = 0;
         for (ii=0;ii<n;ii++) {
            if (ii == i)
               continue;
            j1 = 0;
            for (jj=0;jj<n;jj++) {
               if (jj == j)
                  continue;
               c[i1][j1] = a[ii][jj];
               j1++;
            }
            i1++;
         }

         /* Calculate the determinate */
         det = Determinant(c,n-1);

         /* Fill in the elements of the cofactor */
         b[i][j] = pow(-1.0,i+j+2.0) * det;
      }
   }
   for (i=0;i<n-1;i++)
      free(c[i]);
   free(c);
}

void lminvert( lmat m ){
  float **tm = lmalloc( sizeof( float * ) * 4 );
  float **tn = lmalloc( sizeof( float * ) * 4 );
  for( u32 i = 0; i < 4; ++i ){
    tm[ i ] = lmalloc( sizeof( float ) * 4 );
    tn[ i ] = lmalloc( sizeof( float ) * 4 );
    for( u32 j = 0; j < 4; ++j )
      tm[ i ][ j ] = 0 - m[ i * 4 + j ];
  }
  CoFactor( tm, 4, tn );
  for( u32 i = 0; i < 4; ++i )
    for( u32 j = 0; j < 4; ++j )
      m[ i + j * 4 ] = tn[ i ][ j ];
  for( u32 i = 0; i < 4; ++i ){
    free( tn[ i ] );
    free( tm[ i ] );
  }
  free( tm );
  free( tn );
}
		      

