////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#version 430 core
layout( location = 0 ) out vec4 color;

layout( r32ui, binding = 4 ) uniform uimageBuffer gbuffer;

uniform vec4 screen;
uniform mat4 rmv;



float raycastCube( in vec3 origin, in vec3 ray, in vec3 cubeCenter, 
		   in float cubeRadius );
float raycastOctree( in vec3 origin, in vec3 ray, uimageBuffer octree, 
		     in vec3 cubeCenter, in float cubeRadius );


void main(void) { 
  vec3 ans;

  uint v = imageLoad( gbuffer, int( gl_FragCoord.x / screen.z ) + 
		      int( gl_FragCoord.y / screen.z ) * int( screen.x ) ).x;
  uint hcount = v >> 24;
  uint hue = int( clamp( ( float( ( v >> 12 ) & 4095 ) / float( hcount ) ) * ( 4095.0 / 63.0 ), 0, 4095 ) );
  uint intensity = v & 4095; 

  float val = clamp( intensity * 0.00025, 0, 1 );
  float sat = float( 63 - hcount ) / 63.0;

  float h = ( float( hue ) * 6.0 / 4096.0 );
  int hs = int( floor( h ) );
  h -= hs;
  float p = val * ( 1 - sat );
  float q = val * ( 1 - sat * h );
  float t = val * ( 1 - sat * ( 1 - h ) );
  switch( hs ){
  case 0:
    ans.r = val;
    ans.g = t;
    ans.b = p;
    break;
  case 1:
    ans.r = q;
    ans.g = val;
    ans.b = p;
    break;
  case 2:
    ans.r = p;
    ans.g = val;
    ans.b = t;
    break;
  case 3:
    ans.r = p;
    ans.g = q;
    ans.b = val;
    break;
  case 4:
    ans.r = t;
    ans.g = p;
    ans.b = val;
    break;
  case 5:
    ans.r = val;
    ans.g = p;
    ans.b = q;
    break;
  default:
    ans.r = ans.g = ans.b = 1;
  }

  vec3 origin = { 0, 0, 0 };
  vec3 ray = { ( ( gl_FragCoord.x / ( screen.x * screen.z ) ) * 2.0 - 1.0 ) * screen.w,  
	       ( ( gl_FragCoord.y / ( screen.y * screen.z ) ) * 2.0 - 1.0 ) / screen.w,  
	       1.125 };
  
  vec3 cubeCenter = { 0.0, 0.0, 0.0 };
  float cubeRadius = 10;

  origin = ( vec4( origin, 1 ) * rmv ).xyz;
  ray = normalize( ( vec4( ray, 1 ) * rmv ).xyz - origin );
  vec3 col;
  uint i;
  for( i = 0;  i < 16; ++i ){
    float t = raycastOctree( origin, ray, gbuffer, cubeCenter, cubeRadius );
    if( t != 0.0 )
      color.xyz = vec3( 0.0, 1.0, 0.5 ) * vec3( t / 10 );
    else
      color.xyz = ans;
  }
  

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

  for( uint c = 0; c < 16; ++c ){
    for( uint i = 0; i < 8; ++i ){
 
    vec3 newCenter = { ( ( i >> 0 ) & 1 ) * 2.0 - 1.0,
		  ( ( i >> 1 ) & 1 ) * 2.0 - 1.0,
		  ( ( i >> 2 ) & 1 ) * 2.0 - 1.0 };
    float newRadius = cubeRadius / 2.0;
    newCenter = newRadius * newCenter + cubeCenter;
	  
    }
  }
}
