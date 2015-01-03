////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#version 430 core
layout( location = 0 ) in vec3 vposition; 

uniform mat4 mvp;

void main() { 
  vec4 t = vposition.xyzz;
  t.w = 1;
  t = t * mvp;
  gl_Position = t;
}

