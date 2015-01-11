////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#include "lnz.h"

int sphere( lvec pos, const void* params ){
  const sphereParams* p = params;
  lvec t;
  lvcopy( t, p->center );
  lvscale( t, -1.0 );
  lvadd( t, pos );

  lvscale( pos, 0.5 );
  lvec add = { 0.5, 0.5, 0.5 };
  lvadd( pos, add );

  if( sqrt( lvdot( t, t ) <= p->radius ) ){
    return 1;
  } else
    return 0;
}
int mandelbrot( lvec pos, const void* params ){
  const mandelbrotParams* p = params;
  lvec t;
  lvcopy( t, p->center );
  lvscale( t, -1.0 );
  lvadd( t, pos );

  lvec col;
  lvcopy( col, pos );
  lvscale( col, 0.5 );
  lvec add = { 0.5, 0.5, 0.5 };
  lvadd( col, add );

  if( sqrt( lvdot( pos, pos ) <= 1.0 ) ){
    float crl = pos[ 0 ] * p->scale + p->center[ 0 ];
    float cim = pos[ 1 ] * p->scale + p->center[ 1 ];
    float rl = crl + pos[ 2 ] * p->scale;
    float im = cim;
    u32 i;
    for( i = 0; i < p->iterations; ++i ){
      if( rl * rl + im * im > 4.0 )
	break;
      float t = rl * rl - im * im + crl;
      im = 2 * rl * im + cim;
      rl = t;
    }
    if( i < p->iterations )
      return 0;
    lvcopy( pos, col );
    return 1;
  } else{
    lvcopy( pos, col );
    return 0;
  }
}
