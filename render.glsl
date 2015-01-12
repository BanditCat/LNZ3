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
uniform vec3 light;


float raycastCube( in vec3 origin, in vec3 ray, in vec3 cubeCenter, 
		   in float cubeRadius );
float raycastOctree( in vec3 origin, in vec3 ray, in vec3 cubeCenter, 
		     in float cubeRadius, out vec3 color, out vec3 mormal );
float raycastOctreeShadow( in vec3 origin, in vec3 ray, in vec3 cubeCenter, 
			   in float cubeRadius, out vec3 color, out vec3 mormal );

uint pack( in vec3 ans );
vec3 unpack( in uint v );
vec3 unpackNormal( uint v );
float unpackFloat( uint v );
float unpackRadius( uint v );

const uint octreeNodeSize = 16;
const uint maxCount = 2048;
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
    
    vec3 oray = { x * screen.w, y / screen.w, 1 / tan( fov * 0.5 ) };
  
    vec3 cubeCenter = { 0.0, 0.0, 0.0 };
    float cubeRadius = 50.0;

    origin = ( vec4( origin, 1.0 ) * rmv ).xyz;
    vec3 ray = normalize( ( vec4( oray, 1.0 ) * rmv ).xyz - origin );

    {
      vec3 dm1, dm2;
      vec3 col, norm;
      tval = raycastOctree( origin, ray, cubeCenter, cubeRadius, col, norm );
      vec3 rpos = ray * tval + origin;
      vec3 lray = normalize( rpos - light );
      float lval = raycastOctreeShadow( light, lray, cubeCenter, cubeRadius, dm1, dm2 );
      vec3 lpos = lray * lval + light;
      if( distance( lpos, rpos ) < tval / ( 0.7 * sqrt( screen.x * screen.y ) ) )
	ans = clamp( dot( -lray, norm ).xxx, 0.3, 1.0 ) * col;
      else
	ans = 0.3 * col;
    }
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


float raycastOctree( in vec3 origin, in vec3 ray, in vec3 cubeCenter, 
		     in float cubeRadius, out vec3 color, out vec3 normal ){
  float tval;
  uint node = 0;
  uint sel = 0;
  float newRadius = cubeRadius;

  for( uint i = 0; i < maxCount; ++i ){

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
      newRadius = unpackRadius( imageLoad( octree, int( node * octreeNodeSize + 14)).x);
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
			     int( node * octreeNodeSize + 2 + ( sel ^ asel ) )).x;
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
	  newRadius = r * cubeRadius;
	  tval = t;
	  sel = 0;
	  if( newRadius / tval < 1.0 / sqrt( screen.x * screen.y ) )
	    break;
	  else
	    continue;
	}
      }
    }
    sel = sel + 1;
  }
  color = unpack( imageLoad( octree, int( node * octreeNodeSize + 1 ) ).x );
  normal = unpackNormal( imageLoad( octree, int( node * octreeNodeSize ) ).x );
  return tval;
}
float raycastOctreeShadow( in vec3 origin, in vec3 ray, in vec3 cubeCenter, 
		     in float cubeRadius, out vec3 color, out vec3 normal ){
  float tval;
  uint node = 0;
  uint sel = 0;
  float newRadius = cubeRadius;

  for( uint i = 0; i < maxCount; ++i ){

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
      newRadius = unpackRadius( imageLoad( octree, int( node * octreeNodeSize + 14)).x);
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
			     int( node * octreeNodeSize + 2 + ( sel ^ asel ) )).x;
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
	  newRadius = r * cubeRadius;
	  tval = t;
	  sel = 0;
	  continue;
	}
      }
    }
    sel = sel + 1;
  }
  color = unpack( imageLoad( octree, int( node * octreeNodeSize + 1 ) ).x );
  normal = unpackNormal( imageLoad( octree, int( node * octreeNodeSize ) ).x );
  return tval;
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
  float sign = 1.0;
  if( u >= uint( uint(1) << 31 ) ){
    sign = -1.0;
    u -= ( uint(1) << 31 );
  }
  uint nfn = u & ( ( uint(1) << 23 ) - 1 );
  nfn += ( 1 << 23 );
  int ne = int( u >> 23 );
  ne -= 126;
  float nnf = nfn / float( uint(1) << 24 );
  float nr = exp2( ne ) * nnf;
  return sign * nr;
}




float unpackRadius( uint u ){
  return exp2( 0.0 - float( u ) );
}
