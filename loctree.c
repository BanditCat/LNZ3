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

void initOctree( int (*inside)( lvec pos, const void* p ), u32* octree,
		 const void* params ){
  octree[ OCTREE_NODE_SIZE * 0 ] = 1;  
  for( u32 i = 0; i < 8; ++i ){
    lvec cc;
    lvcopy( cc, cubeVecs[ i ] );
    lvscale( cc, 0.5 );
    octree[ OCTREE_NODE_SIZE * 0 + 2 + i ] =
      calculateNode( inside, cc, 0.5, octree, params );
  }
}
void growOctree( int (*inside)( lvec pos, const void* p ), u32* octree,
		 const void* params, u32 count ){
  u32 level = 0;
  u32 nodes[ MAX_OCTREE_DEPTH ];
  lvec cubeCenters[ MAX_OCTREE_DEPTH ];
  float cubeRadii[ MAX_OCTREE_DEPTH ];
  u32 sels[ MAX_OCTREE_DEPTH ];
  u32 calced = 0;
  nodes[ 0 ] = 0;
  cubeCenters[ 0 ][ 0 ] = cubeCenters[ 0 ][ 1 ] = cubeCenters[ 0 ][ 2 ] = 0.0;
  cubeRadii[ 0 ] = 1.0;
  sels[ 0 ] = 0;


 
  while( calced < count ){
    for( u32 i = 0; i < 8 && calced < count; ++i ){
      if( octree[ nodes[ level ] * OCTREE_NODE_SIZE + 2 + i ] == UNEXPLORED_CHILD ){
	lvec cc;
	lvcopy( cc, cubeVecs[ i ] );
	lvscale( cc, cubeRadii[ level ] * 0.5 );
	lvadd( cc, cubeCenters[ level ] );
	octree[ nodes[ level ] * OCTREE_NODE_SIZE + 2 + i ] = 
	  calculateNode( inside, cc, cubeRadii[ level ] * 0.5, octree, params );
	++calced;
      }
    }
    
    s32 ulvl = level;
    do{
      while( sels[ ulvl ] == 7 && ulvl >= 0 )
	sels[ ulvl-- ] = 0;
      if( ulvl < 0 ){
	ulvl = 0;
	++level;
      } else
	++sels[ ulvl ];
      while( ulvl < (s32)level ){
	u32 nn = octree[ nodes[ ulvl ] * OCTREE_NODE_SIZE + 2 + sels[ ulvl ] ];
	if( nn == UNEXPLORED_CHILD ){
	  level = ulvl;
	  break;
	}
	if( nn <= VALID_CHILD ){
	  lvec cc;
	  lvcopy( cc, cubeVecs[ sels[ ulvl ] ] );
	  lvscale( cc, cubeRadii[ ulvl ] * 0.5 );
	  lvadd( cc, cubeCenters[ ulvl ] );
	  lvcopy( cubeCenters[ ulvl + 1 ], cc );
	  cubeRadii[ ulvl + 1 ] = cubeRadii[ ulvl ] * 0.5;
	  nodes[ ++ulvl ] = nn;
	} else
	  break;
      }      
    } while( ulvl < (s32)level );
  }
}
u32 calculateNode( int (*inside)( lvec, const void* ), const lvec cubeCenter,
		   float cubeRadius, u32* octree, const void* params ){
  lvec col = { 0.0, 0.0, 0.0 };
  lvec normal = { 0.0, 0.0, 0.0 };
  lvec pos;
  u32 coverage = 0;
  for( pos[ 0 ] = cubeCenter[ 0 ] - cubeRadius; 
       pos[ 0 ] <= cubeCenter[ 0 ] + cubeRadius;
       pos[ 0 ] += ( 2.0 * cubeRadius ) / ( SUPERSAMPLE_DIM - 1 ) ){
    for( pos[ 1 ] = cubeCenter[ 1 ] - cubeRadius; 
	 pos[ 1 ] <= cubeCenter[ 1 ] + cubeRadius;
	 pos[ 1 ] += ( 2.0 * cubeRadius ) / ( SUPERSAMPLE_DIM - 1 ) ){
      for( pos[ 2 ] = cubeCenter[ 2 ] - cubeRadius; 
	   pos[ 2 ] <= cubeCenter[ 2 ] + cubeRadius;
	   pos[ 2 ] += ( 2.0 * cubeRadius ) / ( SUPERSAMPLE_DIM - 1 ) ){
	lvec t;
	lvcopy( t, pos );
	int inchk = inside( t, params );
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
  return octree[ 0 ]++;
}
