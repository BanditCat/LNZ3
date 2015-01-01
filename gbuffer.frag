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



  color = vec4( unpack( v ), 1.0 );
}

vec3 unpack( in uint v ){
  vec3 ans;
  ans.r = float( ( v >> 0 ) & 2047 ) / 2047.0;
  ans.g = float( ( v >> 11 ) & 2047 ) / 2047.0;
  ans.b = float( ( v >> 22 ) & 1023 ) / 1023.0;
  return ans;
}
