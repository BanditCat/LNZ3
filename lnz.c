////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

// main LNZ soure file.
#include "lnz.h"

// Desktop display mode on display 0 is used as the basis for fullscreen mode.
SDL_DisplayMode fullscreenDM = { 0 };
// The main window.
SDL_Window* mainWindow = NULL;
// The OpenGL context.
SDL_GLContext glContext = NULL;

// Handlers.
void (*keyHandler)( const SDL_Event* ) = NULL;
void (*touchHandler)( const SDL_Event* ) = NULL;
void (*mouseHandler)( const SDL_Event* ) = NULL;
void (*windowHandler)( const SDL_Event* ) = NULL;


void LNZQuit( void ){
  if( glContext != NULL )
    SDL_GL_DeleteContext( glContext );
  if( mainWindow != NULL )
    SDL_DestroyWindow( mainWindow );
  SDL_Quit();
}

void LNZInit( int fullscreen, const char* windowTitle,
	      double windowedWidth, double windowedHeight ){
  // Initialize SDL.
  if( SDL_Init( SDL_INIT_EVERYTHING ) ){
    fprintf( stderr, "\nSDL Could not initialize: %s\n", SDL_GetError() );
    exit( EXIT_FAILURE );
  }
  atexit( LNZQuit );

  // Get video mode.
  if( SDL_GetDesktopDisplayMode( 0, &fullscreenDM ) ){
    SDL_LogError( SDL_LOG_CATEGORY_APPLICATION,
		  "\nFailed to get desktop display mode: %s\n",
		  SDL_GetError() );
    exit( EXIT_FAILURE );
  }
  SDL_Log( "\nFullscreen mode is %dx%d@%dhz with format %s\n",  fullscreenDM.w, 
	   fullscreenDM.h, fullscreenDM.refresh_rate, 
	   SDL_GetPixelFormatName( fullscreenDM.format ) );


  LNZReinit( fullscreen, windowTitle, windowedWidth, windowedHeight );
  {
    GLint max;
    glGetIntegerv( GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &max );
    SDL_Log( "Initialized with GL_MAX_COMPUTE_SHARED_MEMORY_SIZE = %i", max );
  }
}


void LNZReinit( int fullscreen, const char* windowTitle,
		double windowedWidth, double windowedHeight ){
  if( glContext != NULL )
    SDL_GL_DeleteContext( glContext );
  if( mainWindow != NULL )
    SDL_DestroyWindow( mainWindow );

  // Create window.
  if( fullscreen ){
    mainWindow = SDL_CreateWindow( windowTitle, SDL_WINDOWPOS_UNDEFINED, 
				   SDL_WINDOWPOS_UNDEFINED,
				   fullscreenDM.w, fullscreenDM.h,
				   SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
				   SDL_WINDOW_INPUT_GRABBED | 
				   SDL_WINDOW_FULLSCREEN |
				   SDL_WINDOW_ALLOW_HIGHDPI |	   
				   SDL_WINDOW_INPUT_FOCUS |
				   SDL_WINDOW_MOUSE_FOCUS );			
    SDL_ShowCursor( 0 );
    SDL_SetRelativeMouseMode( 1 );
  }else{
    float dim = ( fullscreenDM.w > fullscreenDM.h ) ? 
      fullscreenDM.h : fullscreenDM.w;
    mainWindow = SDL_CreateWindow( windowTitle, SDL_WINDOWPOS_UNDEFINED, 
				   SDL_WINDOWPOS_UNDEFINED,
				   dim * windowedWidth, 
				   dim * windowedHeight,
				   SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
				   SDL_WINDOW_INPUT_FOCUS |
				   SDL_WINDOW_MOUSE_FOCUS |
				   SDL_WINDOW_INPUT_GRABBED | 
				   SDL_WINDOW_ALLOW_HIGHDPI );
  }
  if( mainWindow == NULL ){
    SDL_LogError( SDL_LOG_CATEGORY_APPLICATION,
		  "\nFailed to create window: %s\n",
		  SDL_GetError() );
    exit( EXIT_FAILURE );
  }

  // Create OpenGL context.
  SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 0 );
  SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  if( ( glContext = SDL_GL_CreateContext( mainWindow ) ) == NULL ){
    SDL_LogError( SDL_LOG_CATEGORY_APPLICATION,
		  "\nFailed to get OpenGL context: %s\n",
		  SDL_GetError() );
    exit( EXIT_FAILURE );
  }

  // Init GLEW.
  {
    glewExperimental = GL_TRUE;
    GLenum status = glewInit();
    if( status != GLEW_OK ){
      SDL_LogError( SDL_LOG_CATEGORY_APPLICATION,
		    "\nFailed to initialize GLEW: %s\n",
		    glewGetErrorString( status ) );
      exit( EXIT_FAILURE );
    }
  }
  // Clear init errors.
  while( glGetError() != GL_NO_ERROR )
    ;
  LNZOSReinit();
}

void LNZLoop( void ){
  SDL_Event ev;
  while( SDL_PollEvent( &ev ) ){
    if( ev.type == SDL_QUIT )
      exit( EXIT_SUCCESS );
    if( ( ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP ) && 
	( keyHandler != NULL ) )
      keyHandler( &ev );
    if( ( ev.type == SDL_FINGERDOWN || ev.type == SDL_FINGERUP || 
	  ev.type == SDL_FINGERMOTION ) && ( touchHandler != NULL ) )
      touchHandler( &ev );
    if( ( ( ev.type == SDL_MOUSEWHEEL && 
	    ev.wheel.which != SDL_TOUCH_MOUSEID ) ||
	  ( ev.type == SDL_MOUSEMOTION && 
	    ev.motion.which != SDL_TOUCH_MOUSEID ) ||
	  ( ( ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_MOUSEBUTTONUP ) && 
	    ev.button.which != SDL_TOUCH_MOUSEID ) ) &&
	( mouseHandler != NULL ) )
      mouseHandler( &ev );
    if( ( ev.type == SDL_WINDOWEVENT ) && ( windowHandler != NULL ) )
      windowHandler( &ev );
  }
}

void LNZSetKeyHandler( void (*kh)( const SDL_Event* ev ) ){
  keyHandler = kh;
}
void LNZSetTouchHandler( void (*th)( const SDL_Event* ev ) ){
  touchHandler = th;
}
void LNZSetMouseHandler( void (*mh)( const SDL_Event* ev ) ){
  mouseHandler = mh;
}
void LNZSetWindowHandler( void (*wh)( const SDL_Event* ev ) ){
  windowHandler = wh;
}  

GLuint LNZCompileOrDie( char* source, GLenum type ){
  GLuint obj = glCreateShader( type );
  if( !obj ){
    SDL_LogError( SDL_LOG_CATEGORY_APPLICATION,
		  "\nFailed to create shader object.\n" );
    free( source );
    exit( EXIT_FAILURE );
  }
  glShaderSource( obj, 1, (const GLchar**)&source, NULL );
  glCompileShader( obj );
  GLint suc;
  glGetShaderiv( obj, GL_COMPILE_STATUS, &suc );
  if( suc != GL_TRUE ){
    free( source );
    GLint sz;
    glGetShaderiv( obj, GL_INFO_LOG_LENGTH, &sz );
    char* msg = malloc( sz + 1 );
    glGetShaderInfoLog( obj, sz, NULL, msg );
    msg[ sz ] = 0;
    SDL_LogError( SDL_LOG_CATEGORY_APPLICATION,
		  "\nFailed to compile shader:\n%s\n", msg );
    free( msg );
    glDeleteShader( obj );
    exit( EXIT_FAILURE );
  }
  free( source );
  return obj;
}

GLuint LNZLinkOrDie( u32 count, const GLuint* shaders ){
  GLuint obj = glCreateProgram();
  if( !obj ){
    SDL_LogError( SDL_LOG_CATEGORY_APPLICATION,
		  "\nFailed to create program object.\n" );
    for( u32 i = 0; i < count; ++i )
      glDeleteShader( shaders[ i ] );
    exit( EXIT_FAILURE );
  }
  for( u32 i = 0; i < count; ++i )
    glAttachShader( obj, shaders[ i ] );
  glLinkProgram( obj );
  GLint suc;
  glGetProgramiv( obj, GL_LINK_STATUS, &suc );
  if( suc != GL_TRUE ){
    GLint sz;
    glGetProgramiv( obj, GL_INFO_LOG_LENGTH, &sz );
    char* msg = malloc( sz + 1 );
    glGetProgramInfoLog( obj, sz, NULL, msg );
    msg[ sz ] = 0;
    SDL_LogError( SDL_LOG_CATEGORY_APPLICATION,
		  "\nFailed to link program:\n%s\n", msg );
    free( msg );
    for( u32 i = 0; i < count; ++i ){
      glDetachShader( obj, shaders[ i ] );
      glDeleteShader( shaders[ i ] );
    }
    glDeleteProgram( obj );
    exit( EXIT_FAILURE );
  }
  for( u32 i = 0; i < count; ++i ){
    glDetachShader( obj, shaders[ i ] );
    glDeleteShader( shaders[ i ] );
  }
  return obj;
}

