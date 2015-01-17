#version 430 core

layout( local_size_x = 1024 ) in;

layout( r32ui, binding = 0 ) uniform uimageBuffer gbuffer;
layout( rgba32ui, binding = 1 ) uniform uimageBuffer octree0;
layout( rgba32ui, binding = 2 ) uniform uimageBuffer octree1;
layout( rgba32ui, binding = 3 ) uniform uimageBuffer octree2;
layout( rgba32ui, binding = 4 ) uniform uimageBuffer octree3;
layout( rgba32ui, binding = 5 ) uniform uimageBuffer octree4;
layout( rgba32ui, binding = 6 ) uniform uimageBuffer octree5;

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
			   in float cubeRadius, in vec3 rorigin );

uint pack( vec3 ans );
vec3 unpack( uint v ); 
vec3 unpackNormal( uint v );
int loadChildSelector( int node );
int loadParent( int node );
vec3 loadColor( int node );
vec3 loadNormal( int node );
int loadChild( int node, int sel );
int loadNode( int addr );

const int octreeWidth = 16384;
const int octreeNodeSize = 12;
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
      vec3 col, norm;
      tval = raycastOctree( origin, ray, cubeCenter, cubeRadius, col, norm );
      vec3 rpos = ray * tval + origin;
      vec3 lray = normalize( rpos - light );
      float lval = raycastOctreeShadow( light, lray, cubeCenter, cubeRadius, origin );
      vec3 lpos = lray * lval + light;
      if( distance( lpos, rpos ) - 0.5 < tval / ( sqrt( screen.x * screen.y ) ) )
	ans = clamp( dot( -lray, norm ).xxx, 0.1, 1.0 ) * col;
      else
	ans = 0.1 * col;
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
  int node = 0;
  int sel = 0;
  float newRadius = 1.0;

  for( uint i = 0; i < maxCount; ++i ){

    int asel = int( dot( vec3( 1, 2, 4 ), 
			 clamp( sign( origin - cubeCenter * cubeRadius ), 0, 1 ))); 

    if( sel == 8 ){
      if( node == 0 ){
	tval = 0.0;
	break;
      }

      sel = loadChildSelector( node );
      node = loadParent( node );
      newRadius *= 2;
      cubeCenter -= cubeVecs[ sel ] * newRadius / 2;
      
      asel = int( dot( vec3( 1, 2, 4 ),
		       clamp( sign( origin - cubeCenter * cubeRadius ), 0, 1 ))); 
      sel = ( ( sel ^ asel ) + 1 );
      continue;

    } else{	    
      int sl = sel ^ asel;
      int addr = loadChild( node, sl );
      if( addr == unexplored )
	break;
      else if( addr <= valid ){
	  
	vec3 cc;
	float r = newRadius / 2;
	cc = cubeCenter + cubeVecs[ sl ] * r;
	float t = raycastCube( origin, ray, cc * cubeRadius, cubeRadius * r ); 
	if( t > 0.0 ){

	  node = addr;
	  cubeCenter = cc;
	  newRadius /= 2;
	  tval = t;
	  sel = 0;
	  if( newRadius * cubeRadius / tval < 1.0 / ( screen.z * sqrt( screen.x * screen.y ) ) )
	    break;
	  continue;
	}
      }
    }
    sel = sel + 1;
  }
  color = loadColor( node );
  normal = loadNormal( node );
  return tval;
}
float raycastOctreeShadow( in vec3 origin, in vec3 ray, in vec3 cubeCenter, 
		     in float cubeRadius, in vec3 rorigin ){
  float tval;
  int node = 0;
  int sel = 0;
  float newRadius = 1.0;

  for( uint i = 0; i < maxCount; ++i ){

    int asel = int( dot( vec3( 1, 2, 4 ), 
			 clamp( sign( origin - cubeCenter * cubeRadius ), 0, 1 ))); 

    if( sel == 8 ){
      if( node == 0 ){
	tval = 0.0;
	break;
      }
      sel = loadChildSelector( node );
      node = loadParent( node );
      newRadius *= 2;
      cubeCenter -= cubeVecs[ sel ] * newRadius / 2;
      
      asel = int( dot( vec3( 1, 2, 4 ),
		       clamp( sign( origin - cubeCenter * cubeRadius ), 0, 1 ))); 
      sel = ( ( sel ^ asel ) + 1 );
      continue;

    } else{	    
      int sl = sel ^ asel;
      int addr = loadChild( node, sl );
      if( addr == unexplored )
	break;
      else if( addr <= valid ){
	  
	vec3 cc;
	float r = newRadius / 2;
	cc = cubeCenter + cubeVecs[ sl ] * r;
	float t = raycastCube( origin, ray, cc * cubeRadius, cubeRadius * r ); 
	if( t > 0.0 ){
	  node = addr;
	  cubeCenter = cc;
	  newRadius /= 2;
	  tval = t;
	  sel = 0;
 	  if( newRadius * cubeRadius / distance( tval * ray + origin, rorigin ) < 
 	      1.0 / ( sqrt( screen.x * screen.y ) * screen.z ) )
	    break;
	  continue;
	}
      }
    }
    sel = sel + 1;
  }
  return tval;
}

vec3 unpackNormal( uint ans ){
  vec3 v;
  v.x = float( ( ans >> 0 ) & 2047 ) / 2047.0;
  v.y = float( ( ans >> 11 ) & 2047 ) / 2047.0;
  v.z = float( ( ans >> 22 ) & 1023 ) / 1023.0;
  v = v * 2.0 - 1.0;
  return v;
}

vec3 unpack( uint ans ){
  vec3 v;
  v.x = float( ( ans >> 0 ) & 2047 ) / 2047.0;
  v.y = float( ( ans >> 11 ) & 2047 ) / 2047.0;
  v.z = float( ( ans >> 22 ) & 1023 ) / 1023.0;
  return v;
}

// float unpackFloat( uint u ){
//   if( u == 0 )
//     return 0.0;
//   float sign = 1.0;
//   if( u >= uint( uint(1) << 31 ) ){
//     sign = -1.0;
//     u -= ( uint(1) << 31 );
//   }
//   uint nfn = u & ( ( uint(1) << 23 ) - 1 );
//   nfn += ( 1 << 23 );
//   int ne = int( u >> 23 );
//   ne -= 126;
//   float nnf = nfn / float( uint(1) << 24 );
//   float nr = exp2( ne ) * nnf;
//   return sign * nr;
// }





int loadChildSelector( int node ){
  return loadNode( node * octreeNodeSize + 11 );
}
int loadParent( int node ){
  int addr = node * octreeNodeSize + 10;
  return loadNode( addr );
}
vec3 loadColor( int node ){  
  return unpack( loadNode( node * octreeNodeSize + 1 ) );
}
vec3 loadNormal( int node ){
  return unpackNormal( loadNode( node * octreeNodeSize + 0 ) );
}
int loadChild( int node, int sel ){
  return loadNode( node * octreeNodeSize + sel + 2 );
}
int loadNode( int addr ){  
  int sel = addr % 6;
  int raddr = addr / 6;
  switch( sel ){
  case 0: return ivec4( imageLoad( octree0, raddr / 4 ) )[ raddr % 4 ];
  case 1: return ivec4( imageLoad( octree1, raddr / 4 ) )[ raddr % 4 ];
  case 2: return ivec4( imageLoad( octree2, raddr / 4 ) )[ raddr % 4 ];
  case 3: return ivec4( imageLoad( octree3, raddr / 4 ) )[ raddr % 4 ];
  case 4: return ivec4( imageLoad( octree4, raddr / 4 ) )[ raddr % 4 ];
  case 5: return ivec4( imageLoad( octree5, raddr / 4 ) )[ raddr % 4 ];
  }
 
}

