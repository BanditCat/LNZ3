////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

// Octree functions and macros.

#ifndef LOCTREE_H
#define LOCTREE_H

// 10 uints, normal, color, and 8 kids. The normal on node 0 is just the size.
#define OCTREE_NODE_SIZE 40

#define UNEXPLORED_CHILD ( (u32)-1 )
#define FULL_CHILD ( (u32)-2 )
#define EMPTY_CHILD ( (u32)-3 )

// Should be 2^n + 1. 
#define SUPERSAMPLE_DIM 5
#define SUPERSAMPLES 125

static const lvec cubeVecs[ 8 ] = 
  { { -1.0, -1.0, -1.0 },
    {  1.0, -1.0, -1.0 },
    { -1.0,  1.0, -1.0 },
    {  1.0,  1.0, -1.0 },
    { -1.0, -1.0,  1.0 },
    {  1.0, -1.0,  1.0 },
    { -1.0,  1.0,  1.0 },
    {  1.0,  1.0,  1.0 } };

u32 lpack( const lvec v );
void lunpack( u32 ans, lvec v );
u32 lpackNormal( const lvec v );
void lunpackNormal( u32 ans, lvec v );


// The inside function returns 1 if pos is inside, 0 otherwise. It must also set
// pos to a 0-1 rgb color triplet.
void initOctree( int (*inside)( lvec pos ), u32* octree );
void growOctree( int (*inside)( lvec pos ), u32* octree, u32 count );
// returns -1--3 or the index of the node created.
u32 calculateNode( int (*inside)( lvec pos ), const lvec cubeCenter, 
		    float cubeRadius, u32* octree ); 




#endif // LOCTREE_H
