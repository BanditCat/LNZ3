////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

// OS dependant code goes in this file.
#define WIN32_LEAN_AND_MEAN

#include "lnz.h"

#include <windows.h>
#include <SDL_syswm.h>


// BUGBUG wish I could hide the soft keyboard here!
void LNZOSReinit( void ){
  SDL_SetRelativeMouseMode( 1 );
}
u8* LNZLoadResource( const char* name, u64* size ){
  HRSRC rsc;
  HGLOBAL hnd;
  u8* dt,* ans;
  if( ( rsc = FindResource( NULL, name, RT_RCDATA ) ) == NULL )
    return NULL;
  if( ( hnd = LoadResource( NULL, rsc ) ) == NULL )
    return NULL;
  if( ( dt = LockResource( hnd ) ) == NULL )
    return NULL;
  *size = SizeofResource( NULL, rsc );
  ans = malloc( *size + 1 );
  memcpy( ans, dt, *size );
  ans[ *size ] = '\0';
  return ans;
}

u8* LNZLoadResourceOrDie( const char* name, u64* size ){
  u8* ans = LNZLoadResource( name, size );
  if( ans == NULL ){
    SDL_LogError( SDL_LOG_CATEGORY_APPLICATION,
		  "\nFailed to load resource %s.\n",
		  name );
    exit( EXIT_FAILURE );
  }
  return ans;
}  
