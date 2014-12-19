////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#version 430 core
layout( location = 0 ) out vec4 color;

layout( r32ui, binding = 4 ) coherent uniform uimageBuffer gbuffer;

uniform vec4 screen;

void main(void) { 
  vec4 ans;

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
  ans.a = 1;

  color = ans;
}

