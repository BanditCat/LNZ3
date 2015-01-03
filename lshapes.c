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
  lvdot( t, t );
  if( sqrt( lvdot( t, t ) <= p->radius ) ){
    for( u32 i = 0; i < 3; ++i )
      pos[ i ] = 1.0;
    return 1;
  } else
    return 0;
}

      
