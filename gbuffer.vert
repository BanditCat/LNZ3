#version 430 core
in vec2 vposition; 

void main() { 
  vec4 t = vposition.xyyy;
  t.z = 0.5;
  t.w = 1;
  gl_Position = t;
}

