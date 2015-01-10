////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#version 430 core

layout( local_size_x = 1024 ) in;

layout( r32ui, binding = 0 ) uniform uimageBuffer gbuffer;
layout( r32ui, binding = 1 ) uniform uimageBuffer octree;

// Size of buffer that needs to be cleared.
uniform uint gcount;
uniform vec4 screen;
uniform mat4 rmv;
uniform float fov;


float raycastCube( in vec3 origin, in vec3 ray, in vec3 cubeCenter, 
		   in float cubeRadius );

uint pack( in vec3 ans );
vec3 unpack( in uint v );
vec3 unpackNormal( uint v );
float unpackFloat( uint v );
float unpackRadius( uint v );

const uint octreeNodeSize = 16;
const uint maxCount = 32;
const uint unexplored = 4294967295u;
const uint valid = 4294967292u;
const vec3 cubeVecs[ 8 ] = 
  { { -1.0, -1.0, -1.0 },
    {  1.0, -1.0, -1.0 },
    { -1.0,  1.0, -1.0 },
    {  1.0,  1.0, -1.0 },
    { -1.0, -1.0,  1.0 },
    {  1.0, -1.0,  1.0 },
    { -1.0,  1.0,  1.0 },
    {  1.0,  1.0,  1.0 } };

void main( void ){ 
  uint index = uint( gl_GlobalInvocationID.x );
  
  if( index < gcount ){
    vec3 ans;
    float tval = 0.0;

    vec3 origin = { 0, 0, 0 };
    float x = ( float( index % uint( screen.x ) ) / ( screen.x - 1.0 ) ) * 2.0 - 1.0;
    float y = ( float( index / uint( screen.x ) ) / ( screen.y - 1.0 ) ) * 2.0 - 1.0;
    
    vec3 ray = { x * screen.w, y / screen.w, 1 / tan( fov * 0.5 ) };
  
    vec3 cubeCenter = { 0.0, 0.0, 0.0 };
    float cubeRadius = 20.0;

    origin = ( vec4( origin, 1.0 ) * rmv ).xyz;
    ray = normalize( ( vec4( ray, 1.0 ) * rmv ).xyz - origin );


  
    {      
      uint node = 0;
      uint sel = 0;

      for( uint i = 0; i < 800; ++i ){

	uint asel = 0;
	if( origin.x - cubeCenter.x * cubeRadius > 0.0 )
	  asel = asel + 1;
	if( origin.y - cubeCenter.y * cubeRadius > 0.0 )
	  asel = asel + 2;
	if( origin.z - cubeCenter.z * cubeRadius > 0.0 )
	  asel = asel + 4;


	if( sel == 8 ){
	  if( node == 0 ){
	    tval = 0.0;
	    break;
	  }
	  sel = imageLoad( octree, int( node * octreeNodeSize + 15) ).x;
	  node = imageLoad( octree, int( node * octreeNodeSize + 14)).x;
	  cubeCenter.x = unpackFloat( imageLoad( octree, int( node * octreeNodeSize + 10)).x);
	  cubeCenter.y = unpackFloat( imageLoad( octree, int( node * octreeNodeSize + 11)).x);
	  cubeCenter.z = unpackFloat( imageLoad( octree, int( node * octreeNodeSize + 12)).x);
	  asel = 0;
	  if( origin.x - cubeCenter.x * cubeRadius > 0.0 )
	    asel = asel + 1;
	  if( origin.y - cubeCenter.y * cubeRadius > 0.0 )
	    asel = asel + 2;
	  if( origin.z - cubeCenter.z * cubeRadius > 0.0 )
	    asel = asel + 4;
	  
	  sel = ( ( sel ^ asel) + 1 );
	  continue;

	} else{	    

	  uint addr = imageLoad( octree, 
				 int( node * octreeNodeSize + 2 + ( sel ^ asel ) ) ).x;
	  if( addr == unexplored )
	    break;
	  else if( addr <= valid ){
	  
	    vec3 cc;
	    cc.x = unpackFloat( imageLoad( octree, int( addr * octreeNodeSize + 10)).x);
	    cc.y = unpackFloat( imageLoad( octree, int( addr * octreeNodeSize + 11)).x);
	    cc.z = unpackFloat( imageLoad( octree, int( addr * octreeNodeSize + 12)).x);
	    float r = unpackRadius( imageLoad( octree, 
					       int( addr * octreeNodeSize + 13 ) ).x );
	    float t = raycastCube( origin, ray, cc * cubeRadius, cubeRadius * r ); 
	    if( t > 0.0 ){
	      node = addr;
	      cubeCenter = cc;
	      tval = t;
	      sel = 0;
	      continue;
	    }
	  }
	}
	sel = sel + 1;
      }
    }

    ans = tval * vec3( 0.2, 1.0, 0.6 ) / 100.0;
    imageStore( gbuffer, int( index ), uvec4( pack( ans ) ) );
  }
}
  uint pack( in vec3 ans ){
  uint val = uint( trunc( clamp( ans.r, 0.0, 1.0 ) * 2047.0 ) );
  val += uint( trunc( clamp( ans.g, 0.0, 1.0 ) * 2047.0 ) ) << 11;
  val += uint( trunc( clamp( ans.b, 0.0, 1.0 ) * 1023.0 ) ) << 22;
  return val;
}
float raycastCube( in vec3 origin, in vec3 ray, in vec3 cubeCenter, 
		   in float cubeRadius ){
  
  vec3 axisOut = cubeCenter + sign( ray ) * vec3( cubeRadius );
  vec3 axisIn = cubeCenter - sign( ray ) * vec3( cubeRadius );
  vec3 t = ( axisOut - origin ) / ray;
  float tOut = min( min( t.x, t.y ), t.z );
  t = ( axisIn - origin ) / ray;
  float tIn = max( max( t.x, t.y ), t.z );
  
  if( tIn <= tOut )
    if( tIn > 0.0 )
      return tIn;
    else if( tOut > 0.0 )
      return tOut;
  return 0.0;
}



vec3 unpackNormal( uint ans ){
  vec3 v;
  v.x = ( ( ans >> 0 ) & 2047 ) / 2047.0;
  v.y = ( ( ans >> 11 ) & 2047 ) / 2047.0;
  v.z = ( ( ans >> 22 ) & 1023 ) / 1023.0;
  v = v * 2.0 - 1.0;
  return v;
}

vec3 unpack( uint ans ){
  vec3 v;
  v.x = ( ( ans >> 0 ) & 2047 ) / 2047.0;
  v.y = ( ( ans >> 11 ) & 2047 ) / 2047.0;
  v.z = ( ( ans >> 22 ) & 1023 ) / 1023.0;
  return v;
}

float unpackFloat( uint u ){
  float v = u;
  v /= 4294967296.0;
  v = v * 2.0 - 1.0;
  return v;
}

float unpackRadius( uint u ){
  return exp2( 0.0 - float( u ) );
}
