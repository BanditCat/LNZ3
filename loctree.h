////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

// Octree functions and macros.

#ifndef LOCTREE_H
#define LOCTREE_H

// 16 uints, normal, color, and 8 kids, a cube center and radius, the parent node, and a 0-8 child slector. The normal on node 0 is just the size. 


#define OCTREE_WIDTH 16384
#define OCTREE_NODE_SIZE ( 16u )
// Maximum depth of an octree;
#define MAX_OCTREE_DEPTH ( 256u )

#define INVALID ( 4294967295u )
#define UNEXPLORED_CHILD ( 4294967295u )
#define FULL_CHILD ( 4294967294u )
#define EMPTY_CHILD ( 4294967293u )
#define VALID_CHILD ( 4294967292u )

// Should be 2^n + 1. 
#define SUPERSAMPLE_DIM 5
#define SUPERSAMPLES ( SUPERSAMPLE_DIM * SUPERSAMPLE_DIM * SUPERSAMPLE_DIM )

static const lvec cubeVecs[ 8 ] = 
  { { -1.0, -1.0, -1.0 },
    {  1.0, -1.0, -1.0 },
    { -1.0,  1.0, -1.0 },
    {  1.0,  1.0, -1.0 },
    { -1.0, -1.0,  1.0 },
    {  1.0, -1.0,  1.0 },
    { -1.0,  1.0,  1.0 },
    {  1.0,  1.0,  1.0 } };

u32 lpackColor( const lvec v );
void lunpack( u32 ans, lvec v );
u32 lpackNormal( const lvec v );
void lunpackNormal( u32 ans, lvec v );
// packs a -1.0 to 1.0 float into a u32.
u32 lpackFloat( float v );
float lunpackFloat( u32 v );
u32 lpackRadius( float v );
float lunpackRadius( u32 v );

// The inside function returns 1 if pos is inside, 0 otherwise. It must also set
// pos to a 0-1 rgb color triplet. The void * is per-shape parameters.
void initOctree( int (*inside)( lvec pos, const void* p ), void* octree, 
		 const void* params );
void growOctree( int (*inside)( lvec pos, const void* p ), void* octree,
		 const void* params, u32 count );
// returns -1--3 or the index of the node created.
u32 calculateNode( int (*inside)( lvec pos, const void* p ), const lvec cubeCenter, 
		   float cubeRadius, void* octree, const void* params,
		   u32 parent, u32 child ); 
u32 loadOctree( void* octree, u32 add );
void storeOctree( void* octree, u32 addr, u32 value );
u32 getOctreeSize( void* octree );
void setOctreeSize( void* octree, u32 val );




#endif // LOCTREE_H
