////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#include "lnz.h"
#include <math.h>

u32 lpackColor( const lvec v ){
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

u32 lpackFloat( float v ){
  if( v == 0.0 )
    return 0;
  u32 sgn = 0;
  if( v < 0.0 ){
    v = -v;
    sgn = ( 1 << 31 );
  }
  
  int e;
  float f = frexpf( v, &e );
  e += 126;
  u32 ans = f * (float)( 1 << 24 );

  ans = ans - (1<<23);
  ans += ( (u32)e << 23 );
  return sgn + ans;
}


float lunpackFloat( u32 u ){
  if( u == 0 )
    return 0.0;
  float sign = 1.0;
  if( u >= (u32)( 1 << 31 ) ){
    sign = -1.0;
    u -= ( 1 << 31 );
  }
  u32 nfn = u & ( ( 1 << 23 ) - 1 );
  nfn += ( 1 << 23 );
  int ne = u >> 23;
  ne -= 126;
  float nnf = nfn / (float)( 1 << 24 );
  float nr = pow( 2.0, ne ) * nnf;
  return sign * nr;
}
u32 lpackRadius( float v ){
  return 0 - log2( v );
}
float lunpackRadius( u32 u ){
  return pow( 2.0, 0.0 - (float)u );
}

void initOctree( int (*inside)( lvec pos, const void* p ), void* ot,
		 const void* params ){
  u32* octree = (u32*)ot;
  setOctreeSize( octree, 1 );  
  {
    lvec col = { 0.0, 0.0, 0.0 };
    storeOctree( octree, 1, lpackColor( col ) );
  }
  storeOctree( octree, 10, lpackFloat( 0.0 ) );
  storeOctree( octree, 11, lpackFloat( 0.0 ) );
  storeOctree( octree, 12, lpackFloat( 0.0 ) );
  storeOctree( octree, 13, lpackRadius( 1.0 ) );
  storeOctree( octree, 14, 0 );
  storeOctree( octree, 15, 0 );

  
  for( u32 i = 0; i < 8; ++i ){
    lvec cc;
    lvcopy( cc, cubeVecs[ i ] );
    lvscale( cc, 0.5 );
    storeOctree( octree, OCTREE_NODE_SIZE * 0 + 2 + i,
		 calculateNode( inside, cc, 0.5, octree, params, 0, i ) );
  }
}
void growOctree( int (*inside)( lvec pos, const void* p ), void* ot,
		 const void* params, u32 count ){
  u32* octree = (u32*)ot;
  u32 level = 0;
  u32 nodes[ MAX_OCTREE_DEPTH ] = { 0 };
  lvec cubeCenters[ MAX_OCTREE_DEPTH ] = { { 0, 0, 0 } };
  float cubeRadii[ MAX_OCTREE_DEPTH ] = { 0 };
  u32 sels[ MAX_OCTREE_DEPTH ] = { 0 };
  u32 calced = 0;
  nodes[ 0 ] = 0;
  cubeCenters[ 0 ][ 0 ] = cubeCenters[ 0 ][ 1 ] = cubeCenters[ 0 ][ 2 ] = 0.0;
  cubeRadii[ 0 ] = 1.0;


 
  while( calced < count ){
    for( u32 i = 0; i < 8 && calced < count; ++i ){
      if( loadOctree( octree, nodes[ level ] * OCTREE_NODE_SIZE + 2 + i ) == UNEXPLORED_CHILD ){
	lvec cc;
	lvcopy( cc, cubeVecs[ i ] );
	lvscale( cc, cubeRadii[ level ] * 0.5 );
	lvadd( cc, cubeCenters[ level ] );
	u32 nn = calculateNode( inside, cc, cubeRadii[ level ] * 0.5, octree, params,
			 nodes[ level ], i );
	storeOctree( octree, nodes[ level ] * OCTREE_NODE_SIZE + 2 + i, nn );
	if( nn <= VALID_CHILD )
	  ++calced;
      }
    }
    
    s32 ulvl = level;
    do{
      while( ulvl >= 0 && sels[ ulvl ] >= 7 )
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
		   float cubeRadius, void* ot, const void* params, u32 parent, 
		   u32 child ){
  u32* octree = (u32*)ot;
  lvec col = { 0.0, 0.0, 0.0 };
  lvec normal = { 0.0, 0.0, 0.0 };
  lvec pos = { 0.0, 0.0, 0.0 };
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
  
  storeOctree( octree, OCTREE_NODE_SIZE * getOctreeSize( octree ) + 0, lpackNormal( normal ) );
  storeOctree( octree, OCTREE_NODE_SIZE * getOctreeSize( octree ) + 1, lpackColor( col ) );
  for( u32 i = 0; i < 8; ++i )
    storeOctree( octree, OCTREE_NODE_SIZE * getOctreeSize( octree ) + 2 + i, UNEXPLORED_CHILD );
  for( u32 i = 0; i < 3; ++i )
    storeOctree( octree, OCTREE_NODE_SIZE * getOctreeSize( octree ) + 10 + i, 
		 lpackFloat( cubeCenter[ i ] ) );
  storeOctree( octree, OCTREE_NODE_SIZE * getOctreeSize( octree ) + 13, 
	       lpackRadius( cubeRadius ) ); 
  storeOctree( octree, OCTREE_NODE_SIZE * getOctreeSize( octree ) + 14, parent );
  storeOctree( octree, OCTREE_NODE_SIZE * getOctreeSize( octree ) + 15, child );

  u32 nsz = getOctreeSize( octree );
  setOctreeSize( octree, nsz + 1 );
  return nsz;
}
u32 loadOctree( void* octree, u32 addr ){
  u32* ot = (u32*)octree;
  return ot[ addr ];
}
void storeOctree( void* octree, u32 addr, u32 value ){
  u32* ot = (u32*)octree;
  ot[ addr ] = value;
}
u32 getOctreeSize( void* octree ){
  return ( (u32*)octree )[ 0 ];
}
void setOctreeSize( void* octree, u32 sz ){
  ( (u32*)octree )[ 0 ] = sz;
} 
