////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#include "lnz.h"
#include <math.h>

u32 lpack( const lvec v ){
  u32 val;
  lvec ans;
  for( u32 i = 0; i < 3; ++i ){
    if( v[ i ] > 1.0 )
      ans[ i ] = 1.0;
    else if( v[ i ] < 0.0 )
      ans[ i ] = 0.0;
    else
      ans[ i ] = v[ i ];
  }

  val = lround( ans[ 0 ] * 2047.0 );
  val += lround( ans[ 1 ] * 2047.0 ) << 11;
  val += lround( ans[ 2 ] * 1023.0 ) << 22;
  return val;
}

void lunpack( u32 ans, lvec v ){
  v[ 0 ] = ( ( ans >> 0 ) & 2047 ) / 2047.0;
  v[ 1 ] = ( ( ans >> 11 ) & 2047 ) / 2047.0;
  v[ 2 ] = ( ( ans >> 22 ) & 1023 ) / 1023.0;
}

u32 lpackNormal( const lvec v ){
  u32 val;
  lvec ans;
  for( u32 i = 0; i < 3; ++i ){
    if( v[ i ] > 1.0 )
      ans[ i ] = 1.0;
    else if( v[ i ] < -1.0 )
      ans[ i ] = 0.0;
    else
      ans[ i ] = v[ i ] / 2.0 + 0.5;
  }

  val = lround( ans[ 0 ] * 2047.0 );
  val += lround( ans[ 1 ] * 2047.0 ) << 11;
  val += lround( ans[ 2 ] * 1023.0 ) << 22;
  return val;
}

void lunpackNormal( u32 ans, lvec v ){
  v[ 0 ] = ( ( ans >> 0 ) & 2047 ) / 2047.0;
  v[ 1 ] = ( ( ans >> 11 ) & 2047 ) / 2047.0;
  v[ 2 ] = ( ( ans >> 22 ) & 1023 ) / 1023.0;
  for( u32 i = 0; i < 3; ++i )
    v[ i ] = v[ i ] * 2.0 - 1.0;
}

void initOctree( int (*inside)( lvec pos ), u32* octree ){
  octree[ OCTREE_NODE_SIZE * 0 ] = 1;
  for( u32 i = 0; i < 8; ++i ){
    lvec cc;
    lvcopy( cc, cubeVecs[ i ] );
    lvscale( cc, 0.5 );
    octree[ OCTREE_NODE_SIZE * 0 + 2 + ( octree[ 0 ] - 1 ) ] =
      calculateNode( inside, cc, 0.5, octree );
    if( octree[ OCTREE_NODE_SIZE * 0 + 2 + ( octree[ 0 ] - 1 ) ] < (u32)-3 )
      ++octree[ 0 ];

  }
}
u32 calculateNode( int (*inside)( lvec pos ), const lvec cubeCenter,
		   float cubeRadius, u32* octree ){
  lvec col = { 0.0, 0.0, 0.0 };
  lvec normal = { 0.0, 0.0, 0.0 };
  lvec pos;
  u32 coverage = 0;
  for( pos[ 0 ] = cubeCenter[ 0 ] - cubeRadius; 
       pos[ 0 ] <= cubeCenter[ 0 ] + cubeRadius;
       pos[ 0 ] += ( 2 * cubeRadius ) / ( SUPERSAMPLE_DIM - 1 ) ){
    for( pos[ 1 ] = cubeCenter[ 1 ] - cubeRadius; 
	 pos[ 1 ] <= cubeCenter[ 1 ] + cubeRadius;
	 pos[ 1 ] += ( 2 * cubeRadius ) / ( SUPERSAMPLE_DIM - 1 ) ){
      for( pos[ 2 ] = cubeCenter[ 2 ] - cubeRadius; 
	   pos[ 2 ] <= cubeCenter[ 2 ] + cubeRadius;
	   pos[ 2 ] += ( 2 * cubeRadius ) / ( SUPERSAMPLE_DIM - 1 ) ){
	lvec t;
	lvcopy( t, pos );
	int inchk = inside( t );
	if( inchk ){
	  ++coverage;
	  lvadd( col, t );
	  lvcopy( t, pos );
	  lvscale( t, -1.0 );
	  lvadd( t, cubeCenter );
	  lvadd( normal, t );
	}
      }
    }
  }
  if( coverage == SUPERSAMPLES )
    return FULL_CHILD;
  if( coverage == 0 )
    return EMPTY_CHILD;
 
  lvnormalize( normal );
  lvscale( col, 1.0 / ( (float)coverage ) );
  
  octree[ OCTREE_NODE_SIZE * octree[ 0 ] + 0 ] = lpackNormal( normal );
  octree[ OCTREE_NODE_SIZE * octree[ 0 ] + 1 ] = lpack( col );
  for( u32 i = 0; i < 8; ++i )
    octree[ OCTREE_NODE_SIZE * octree[ 0 ] + 2 + i ] = UNEXPLORED_CHILD;
  return octree[ 0 ];
}
