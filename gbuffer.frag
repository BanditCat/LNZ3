////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#version 430 core
layout( location = 0 ) out vec4 color;

layout( r32ui, binding = 4 ) uniform uimageBuffer gbuffer;

vec3 unpack( in uint v );

uniform vec4 screen;

void main(void) { 
  vec3 ans;

  uint v = imageLoad( gbuffer, int( gl_FragCoord.x / screen.z ) + 
		      int( gl_FragCoord.y / screen.z ) * int( screen.x ) ).x;


  ans = unpack( v );
  vec3 ians = trunc( uvec3( ans * 256.0 ) );
  vec3 rans = ans * 256.0 - ians;
  ians /= 256.0;
 
  int rval = int( gl_FragCoord.x ) % 4 + ( int( gl_FragCoord.y ) % 2 * 4 );
  
  if( rans.r * 8.0 >= rval )
    ians.r += 0.00390625;
  if( rans.g * 8.0 >= rval )
    ians.g += 0.00390625;
  if( rans.b * 8.0 >= rval )
    ians.b += 0.00390625;


  color = vec4( ians, 1.0 );
}

vec3 unpack( in uint v ){
  vec3 ans;
  ans.r = float( ( v >> 0 ) & 2047 ) / 2047.0;
  ans.g = float( ( v >> 11 ) & 2047 ) / 2047.0;
  ans.b = float( ( v >> 22 ) & 1023 ) / 1027.0;

  return ans;
}
