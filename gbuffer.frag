////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#version 430 core
layout( location = 0 ) out vec4 color;

layout( r32ui, binding = 4 ) coherent uniform uimageBuffer gbuffer;

uniform vec4 screen;
uniform mat4 rmv;



bool raycastCube( inout vec4 origin, in vec4 ray, in vec3 cubeCenter, 
		  in float cubeRadius, out vec3 ans );


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

  vec4 origin = { 0, 0, 0, 1 };
  vec4 ray = { ( ( gl_FragCoord.x / ( screen.x * screen.z ) ) * 2.0 - 1.0 ) * screen.w,  
	       ( ( gl_FragCoord.y / ( screen.y * screen.z ) ) * 2.0 - 1.0 ) / screen.w,  
	       1.0,
	       1.0 };
  
  vec3 cubeCenter = { 50, -50, 10 };
  float cubeRadius = 16.0;

  origin = origin * rmv;
  ray = ray * rmv;
  vec3 col;
  if( raycastCube( origin, ray, cubeCenter, cubeRadius, col ) )
    color.xyz = col;
  else
    color.xyz = ans;

}

bool raycastCube( inout vec4 origin, in vec4 ray, in vec3 cubeCenter, 
		  in float cubeRadius, out vec3 ans ){
  
  vec3 axis = { 0, 0, 0 };
  float t;
  //(x2 - x1)t + x1 = c
  //t = (c - x1) / (x2 - x1);
  ans = vec3( 0 );
  if( origin.x < cubeCenter.x - cubeRadius )
    axis.x = cubeCenter.x - cubeRadius;
  else if( origin.x > cubeCenter.x + cubeRadius )
    axis.x = cubeCenter.x + cubeRadius;
  if( axis.x != 0.0 ){
    t = ( axis.x - origin.x ) / ( ray.x - origin.x );
    vec3 intersection = { axis.x, 
			  ( ray.y - origin.y ) * t + origin.y,
			  ( ray.z - origin.z ) * t + origin.z };  
    
    if( abs( intersection.z - cubeCenter.z ) < cubeRadius && 
	abs( intersection.y - cubeCenter.y ) < cubeRadius ){
      ans.r = 0;
      ans.g = 1;
      ans.b = 1;
    }
  }
  if( origin.y < cubeCenter.y - cubeRadius )
    axis.y = cubeCenter.y - cubeRadius;
  else if( origin.y > cubeCenter.y + cubeRadius )
    axis.y = cubeCenter.y + cubeRadius;
  if( axis.y != 0.0 ){
    t = ( axis.y - origin.y ) / ( ray.y - origin.y );
    vec3 intersection = { ( ray.x - origin.x ) * t + origin.x,
			  axis.y, 
			  ( ray.z - origin.z ) * t + origin.z };  
    
    if( abs( intersection.z - cubeCenter.z ) < cubeRadius && 
	abs( intersection.x - cubeCenter.x ) < cubeRadius ){
      ans.r = 1;
      ans.g = 0;
      ans.b = 1;
    }
  }
  if( origin.z < cubeCenter.z - cubeRadius )
    axis.z = cubeCenter.z - cubeRadius;
  else if( origin.z > cubeCenter.z + cubeRadius )
    axis.z = cubeCenter.z + cubeRadius;
  if( axis.z != 0.0 ){
    t = ( axis.z - origin.z ) / ( ray.z - origin.z );
    vec3 intersection = { ( ray.x - origin.x ) * t + origin.x,
			  ( ray.y - origin.y ) * t + origin.y,
			  axis.z };  
    
    if( abs( intersection.y - cubeCenter.y ) < cubeRadius && 
	abs( intersection.x - cubeCenter.x ) < cubeRadius ){
      ans.r = 1;
      ans.g = 1;
      ans.b = 0;
    }
  }
  return ( dot( ans, ans ) != 0 );
}
