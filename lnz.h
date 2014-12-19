////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

// LNZ main header.

#ifndef LNZ_H
#define LNZ_H


#include "SDL.h"
#define GLEW_STATIC
#include "glew.h"
#include <limits.h>

// Types.
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;
typedef signed char s8;
typedef signed short int s16;
typedef signed int s32;
typedef signed long long int s64;
typedef float f32;
typedef double f64;

// Types sanity check.
#if CHAR_BIT != 8
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if INT_MAX != 2147483647 || INT_MIN != -2147483648
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if UINT_MAX != 4294967295 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if SHRT_MAX != 32767 || SHRT_MIN != -32768 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if USHRT_MAX != 65535 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if SCHAR_MAX != 127 || SCHAR_MIN != -128
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if UCHAR_MAX != 255 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if LLONG_MAX != 9223372036854775807 || LLONG_MIN != 9223372036854775808ull
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if ULLONG_MAX != 18446744073709551615ull
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 

#define pi 3.1415926535897932384626433832795028841971693993751058209749445923078

// Desktop display mode on display 0 is used as the basis for fullscreen mode.
SDL_DisplayMode fullscreenDM;
// The main window.
SDL_Window* mainWindow;
// The OpenGL context.
SDL_GLContext glContext;

// Initializes or dies.
void LNZInit( int fullscreen, const char* windowTitle, 
	      double windowedWidth, double windowedHeight );
// Reinitializes or dies.
void LNZReinit( int fullscreen, const char* windowTitle,
		double windowedWidth, double windowedHeight );
// OS dependent init.
void LNZOSReinit( void );

void LNZQuit( void );

// The main event loop.
void LNZLoop( void );

// Sets the function that gets called for SDL_KEYDOWN and UP events.
void LNZSetKeyHandler( void(*)( const SDL_Event* ) );
void LNZSetTouchHandler( void(*)( const SDL_Event* ) );
void LNZSetMouseHandler( void(*)( const SDL_Event* ) );
void LNZSetWindowHandler( void(*)( const SDL_Event* ) );

// These functions return malloc'd resources that need to be free'd.
// The size in bytes is stored in size. The second version exits on failure.
// All data has a complementary nul, so buffer size is actually size + 1.
u8* LNZLoadResource( const char* name, u64* size );
u8* LNZLoadResourceOrDie( const char* name, u64* size );

// NOTE! This function frees source when done.
GLuint LNZCompileOrDie( char* source, GLenum type );
// NOTE! This function destroys the shaders when done.
GLuint LNZLinkOrDie( u32 count, const GLuint* shaders );

#include "lmat.h"

#endif //LNZ_H 
