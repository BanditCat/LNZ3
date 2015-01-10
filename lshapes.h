////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

// Shapes defined by an inside function over threespace. On return, pos is read 
// as a color value.

#ifndef LSHAPES_H
#define LSHAPES_H

typedef struct{
  lvec center;
  float radius;
} sphereParams;
int sphere( lvec pos, const void* params );

typedef struct{
  lvec center;
  float scale;
  u32 iterations;
} mandlebrotParams;
int mandelbrot( lvec pos, const void* params );

#endif // LSHAOES_H
