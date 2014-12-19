////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#version 430 core

layout( local_size_x = 1024 ) in;

layout( r32ui, binding = 0 ) coherent uniform uimageBuffer gbuffer;


// Size of buffer that needs to be cleared.
uniform uint gcount;

void main( void ){ 
  int i = int( gl_GlobalInvocationID.x );
  if( i < gcount )
    imageStore( gbuffer, i, uvec4( 0 ) );
}

