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
float raycastOctree( in vec3 origin, in vec3 ray, uimageBuffer octree, 
		     in vec3 cubeCenter, in float cubeRadius );
uint pack( in vec3 ans );

void main( void ){ 
  uint index = uint( gl_GlobalInvocationID.x );
  
  if( index < gcount ){
    vec3 ans;
    
  
 
    vec3 origin = { 0, 0, 0 };
    float x = ( float( index % uint( screen.x ) ) / ( screen.x - 1.0 ) ) * 2.0 - 1.0;
    float y = ( float( index / uint( screen.x ) ) / ( screen.y - 1.0 ) ) * 2.0 - 1.0;
    
    vec3 ray = { x * screen.w, y / screen.w, 1 / tan( fov * 0.5 ) };
  
    vec3 cubeCenter = { 0.0, 0.0, 0.0 };
    float cubeRadius = 10;

    origin = ( vec4( origin, 1.0 ) * rmv ).xyz;
    ray = normalize( ( vec4( ray, 1.0 ) * rmv ).xyz - origin );
    vec3 col;
    uint i;
    for( i = 0;  i < 10; ++i ){
      float t = raycastOctree( origin, ray, octree, cubeCenter, cubeRadius );
      if( t > 0.0 )
	ans = vec3( 0.0, 1.0, 0.5 ) * vec3( t / 10 );
      else
	ans = vec3( 0.0 );
    }
    
    
    imageStore( gbuffer, int( index ), uvec4( pack( ans ) ) );

  }
}
uint pack( in vec3 ans ){
  uint val = uint( clamp( ans.r, 0.0, 1.0 ) * 2047.0 );
  val += uint( clamp( ans.g, 0.0, 1.0 ) * 2047.0 ) << 11;
  val += uint( clamp( ans.b, 0.0, 1.0 ) * 1023.0 ) << 22;
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

uniform uint octreeSize = 10;
uniform uint octreeNormal = 0;
uniform uint octreeColor = 1;
uniform uint octreeChildren = 2;
uniform uint maxCount = 16;

float raycastOctree( in vec3 origin, in vec3 ray, uimageBuffer octree, 
		     in vec3 cubeCenter, in float cubeRadius ){
  uint top = 0;
  uint nodes[ 128 ];
  vec3 centers[ 128 ];
  float radii[ 128 ];
  float tvalues[ 128 ];
  
  tvalues[ 0 ] = raycastCube( origin, ray, cubeCenter, cubeRadius );
  if( tvalues[ 0 ] <= 0.0 )
    return 0.0;
 
  return tvalues[ 0 ];
 
  nodes[ 0 ] = 0;
  centers[ 0 ] = cubeCenter;
  radii[ 0 ] = cubeRadius;

  for( uint c = 0; c < maxCount; ++c ){
    for( uint i = 0; i < 8; ++i ){
 
      vec3 newCenter = { ( ( i >> 0 ) & 1 ) * 2.0 - 1.0,
			 ( ( i >> 1 ) & 1 ) * 2.0 - 1.0,
			 ( ( i >> 2 ) & 1 ) * 2.0 - 1.0 };
      float newRadius = cubeRadius / 2.0;
      newCenter = newRadius * newCenter + cubeCenter;
	  
    }
  }
}

