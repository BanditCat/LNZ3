////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#version 430 core

layout( std140, binding = 0 ) uniform attractor_block{ 
  vec4 attractor[64]; // w is mass 
};

layout( local_size_x = 1024 ) in;

layout( rgba32f, binding = 0 ) uniform imageBuffer vbuffer; 
layout( rgba32f, binding = 1 ) uniform imageBuffer pbuffer;
layout( rgba32f, binding = 2 ) uniform imageBuffer vbuffer_out; 
layout( rgba32f, binding = 3 ) uniform imageBuffer pbuffer_out;
layout( r32ui, binding = 4 ) uniform uimageBuffer gbuffer;


// Time change per frame in seconds.
uniform float dt;
// Viewport size.
uniform vec4 screen;
// Modelview Projection Matrix.
uniform mat4 mvp;

void main( void ){ 
  vec4 vel = imageLoad( vbuffer, int( gl_GlobalInvocationID.x ) ); 
  vec4 pos = imageLoad( pbuffer, int( gl_GlobalInvocationID.x ) );
  int i,c;

  pos.xyz += vel.xyz * dt; 

  pos.w -= 0.004 * dt;

  // Loop for hi res
  float m = 1;
  for( c = 0; c < int( m ); ++c ){
    for( i = 0; i < 64; i++ ){ 
      vec3 dist = ( attractor[ i ].xyz - pos.xyz ) * 0.7; 
      vec3 accl = dt * ( 1.0 / m ) * 0.06 * attractor[ i ].w * 
    	normalize(dist) / dot( dist, dist );

      vel.xyz += accl;
    }
  }

  if( pos.w <= 0.0 ){ 
    pos.w += 1.0f; 
    vel.xyz *= 0.01;
    pos.xyz = -pos.xyz * 0.01; 
  }

  imageStore( pbuffer_out, int( gl_GlobalInvocationID.x ), pos );
  imageStore( vbuffer_out, int( gl_GlobalInvocationID.x ), vel );

  
  {    
    vec4 spos = pos;
    spos.w = 1;
    spos = ( spos * mvp );
    spos /= spos.w;
    spos.xy *= 0.5;
    spos.xy += 0.5;
    
    
    
    if( spos.z > 0.0 && spos.z < 1 &&
	spos.x >= 0 && spos.x < 1 &&
	spos.y >= 0 && spos.y < 1 ){
      float amp = spos.z * spos.z * screen.z;
      float rad = sqrt( 3.0 * amp / 3.141592 );
      int xs = int( spos.x * screen.x - rad );
      if( xs < 0 )
	xs = 0;
      int xe = int( spos.x * screen.x + rad );
      if( xe >= screen.x )
	xe = int( screen.x ) - 1;
      int ys = int( spos.y * screen.y - rad );
      if( ys < 0 )
	ys = 0;
      int ye = int( spos.y * screen.y + rad );
      if( ye >= screen.y )
	ye = int( screen.y ) - 1;
      float tot = 0;
      for( int x = xs; x <= xe; ++x ){
	for( int y = ys; y <= ye; ++y ){
	  vec2 dst = { x, y };
	  dst /= screen.xy;
	  dst -= spos.xy;
	  dst *= screen.xy;
	  tot += clamp( 1 - sqrt( dot( dst.xy, dst.xy ) ) / rad, 0, 1 );
	}
      }
      for( int x = xs; x <= xe; ++x ){
	for( int y = ys; y <= ye; ++y ){
	  vec2 dst = { x, y };
	  dst /= screen.xy;
	  dst -= spos.xy;
	  dst *= screen.xy;
	  float b = amp
	    * ( clamp( 1 - sqrt( dot( dst.xy, dst.xy ) ) / rad, 0, 1 ) /
		tot );	 

	  bool wrote = false;
	  while( !wrote ){
	    uint v = imageLoad( gbuffer, x + y * int( screen.x ) ).x;
	    uint ov = v;
	    uint hcount = v >> 24;
	    uint hue = ( v >> 12 ) & 4095;
	    uint intensity = v & 4095;	
	    
	    uint dif = int( round( b * 4095.0 * clamp( pos.w * 10, 0, 1 ) ) );
	    if( intensity + dif < 4095 )
	      intensity += dif;
	    else
	      intensity = 4095;
	    
	    
	    if( hcount < 63 ){
	      hcount += 1;
	      hue += int( clamp( pos.w * 64, 0, 63 ) );
	    }
	    

	    v = ( hcount << 24 ) + ( hue << 12 ) + intensity;
	    
	    uint w = imageAtomicCompSwap( gbuffer, 
					  x + y * int( screen.x ), ov, v );
	    if( w == ov )
	      wrote = true;
  
	  }
	}
      }
    }      
  }
}

