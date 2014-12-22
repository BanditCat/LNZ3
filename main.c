////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

// Main entry point.
#include "lnz.h"
#include <stdio.h>
#include "math.h"

#define PARTICLE_GROUPS 64
#define PARTICLE_COUNT ( 1024 * PARTICLE_GROUPS )

#define GBUFFER_WIDTH ( fullscreenDM.w )
#define GBUFFER_HEIGHT ( fullscreenDM.h )
#define GBUFFER_SIZE ( GBUFFER_HEIGHT * GBUFFER_WIDTH )


int movingWindow = 0;
int fullscreen = 0;
int dwidth, dheight;
int pixelSize = 1;
float sfps = 30.0;
float rotx = 0, roty = 0, drotx = 0, droty = 0;
int rel = 0;
int blowup = 0;
float scale = 1;
lvec trns = { 0, 0, 90 };
u64 disableMouseTime = 0;

void keys( const SDL_Event* ev ){ 
  if( ev->key.state == SDL_PRESSED && ev->key.keysym.sym == SDLK_ESCAPE )
    exit( EXIT_SUCCESS );
  else if( ev->key.state == SDL_PRESSED && ev->key.keysym.sym == SDLK_f )
    SDL_Log( "\nfps: %.3f", sfps );
  else if( ev->key.state == SDL_PRESSED && ev->key.keysym.sym == SDLK_r ){
    if( rel ){
      rel = 0;
      drotx = droty = 0;
    } else
      rel = 1;
  } else if( ev->key.state == SDL_PRESSED && ev->key.keysym.sym == SDLK_b ){
    if( blowup )
      blowup = 0;
    else
      blowup = 1;
  } else if( ev->key.state == SDL_PRESSED && ev->key.keysym.sym == SDLK_m ){
    movingWindow = 1;
  } else if( ev->key.state == SDL_RELEASED && ev->key.keysym.sym == SDLK_m ){
    movingWindow = 0;
    int x, y;
    SDL_GetWindowSize( mainWindow, &x, &y );
    dwidth = x;
    dheight = y;
    glViewport( 0, 0, x, y );
  } else if( ev->key.state == SDL_PRESSED && ev->key.keysym.sym == SDLK_SPACE ){
    if( scale )
      scale = 0;
    else
      scale = 1;
  } else if( ev->key.state == SDL_PRESSED && 
	     ev->key.keysym.sym == SDLK_LEFTBRACKET ){
    if( pixelSize > 1 )
      pixelSize--;
  } else if( ev->key.state == SDL_PRESSED && 
	     ev->key.keysym.sym == SDLK_RIGHTBRACKET ){
    if( pixelSize < 100 )
      pixelSize++;
  } else if( ev->key.state == SDL_PRESSED && ev->key.keysym.sym == SDLK_RETURN ){
    if( fullscreen ){
      SDL_SetWindowFullscreen( mainWindow, 0 );
      fullscreen = 0;
    } else{
      SDL_SetWindowFullscreen( mainWindow, SDL_WINDOW_FULLSCREEN_DESKTOP );
      fullscreen = 1;
    }
  }
}

void touches( const SDL_Event* ev ){
  disableMouseTime = SDL_GetPerformanceCounter();
  if( ev->tfinger.type == SDL_FINGERDOWN && ev->tfinger.x > 0.9 &&
      ev->tfinger.y < 0.2 &&
      SDL_GetNumTouchFingers( ev->tfinger.touchId ) == 1 ){
    if( ev->tfinger.y < 0.1 )
      exit( EXIT_SUCCESS );
    else if( fullscreen ){
      SDL_SetWindowFullscreen( mainWindow, 0 );
      fullscreen = 0;
    } else{
      SDL_SetWindowFullscreen( mainWindow, SDL_WINDOW_FULLSCREEN_DESKTOP );
      fullscreen = 1;
    }
  }
  else if( ev->tfinger.type == SDL_FINGERDOWN && ev->tfinger.x < 0.1 &&
	   ev->tfinger.y < 0.1 && 
	   SDL_GetNumTouchFingers( ev->tfinger.touchId ) == 1 ){
    if( rel ){
      rel = 0;
      drotx = droty = 0;
    } else
      rel = 1;
  } else if( ev->tfinger.type == SDL_FINGERDOWN && ev->tfinger.x > 0.9 &&
	   ev->tfinger.y > 0.9 && 
	   SDL_GetNumTouchFingers( ev->tfinger.touchId ) == 1 ){
    if( blowup )
      blowup = 0;
    else
      blowup = 1;
  } else if( ev->tfinger.type == SDL_FINGERDOWN && ev->tfinger.x < 0.1 &&
	   ev->tfinger.y > 0.9 && 
	   SDL_GetNumTouchFingers( ev->tfinger.touchId ) == 1 ){
    if( scale == 0.0 )
      scale = 1.0;
    else
      scale = 0.0;      
  } else{
    if( SDL_GetNumTouchFingers( ev->tfinger.touchId ) == 1 ){
      if( rel ){
	drotx += ev->tfinger.dx;
	droty += ev->tfinger.dy;
      } else{
	rotx += ev->tfinger.dx;
	roty += ev->tfinger.dy;
      }
    } else if( SDL_GetNumTouchFingers( ev->tfinger.touchId ) == 2 ){
      SDL_Finger* f0 = SDL_GetTouchFinger( ev->tfinger.touchId, 0 );
      SDL_Finger* f1 = SDL_GetTouchFinger( ev->tfinger.touchId, 1 );
      float odx, ody, dx, dy;
      if( ev->tfinger.fingerId == f0->id ){
	dx = f0->x - f1->x;
	dy = f0->y - f1->y;
      }	else{
	dx = f1->x - f0->x;
	dy = f1->y - f0->y;
      }
      odx = dx - ev->tfinger.dx;
      ody = dy - ev->tfinger.dy;
      float odist = sqrt( pow( odx, 2.0 ) + pow( ody, 2.0 ) );
      float dist = sqrt( pow( dx, 2.0 ) + pow( dy, 2.0 ) );
      trns[ 2 ] += ( dist - odist ) * -400;
      trns[ 0 ] += ev->tfinger.dx * 100;
      trns[ 1 ] += ev->tfinger.dy * -100;
    }
  }
}

void wms( const SDL_Event* ev ){
  if( ev->window.event == SDL_WINDOWEVENT_RESIZED ){
    dwidth = ev->window.data1;
    dheight = ev->window.data2;
    glViewport( 0, 0, dwidth, dheight );
  }
}
void mice( const SDL_Event* ev ){
  if( ev->type == SDL_MOUSEMOTION && 
      ( SDL_GetPerformanceCounter() - disableMouseTime ) /
      (double)SDL_GetPerformanceFrequency() > 0.1 &&
      // There is a bug where sdl gives a massive move event when moving the 
      // window.
      abs( ev->motion.xrel ) < 20 &&
      abs( ev->motion.yrel ) < 20 ){
    if( movingWindow ){
      int x, y;
      if( ev->motion.state & SDL_BUTTON_LMASK )
	SDL_GetWindowSize( mainWindow, &x, &y );
      else
	SDL_GetWindowPosition( mainWindow, &x, &y );
      x += ev->motion.xrel;
      y += ev->motion.yrel;
      if( ev->motion.state & SDL_BUTTON_LMASK ){
	if( x < 64 )
	  x = 64;
	if( x > fullscreenDM.w )
	  x = fullscreenDM.w;
	if( y < 64 )
	  y = 64;
	if( y > fullscreenDM.h )
	  y = fullscreenDM.h;
	SDL_SetWindowSize( mainWindow, x, y );
      } else{
	if( x < 0 )
	  x = 0;
	if( x + 64 > fullscreenDM.w )
	  x = fullscreenDM.w - 64;
	if( y < 0 )
	  y = 0;
	if( y + 64 > fullscreenDM.h )
	  y = fullscreenDM.h - 64;
	SDL_SetWindowPosition( mainWindow, x, y );
      }
    } else{
      if( ev->motion.state & SDL_BUTTON_LMASK ){
	trns[ 0 ] += ev->motion.xrel * 0.05;
	trns[ 1 ] += ev->motion.yrel * -0.05;
      } else if( ev->motion.state & SDL_BUTTON_RMASK ){
	trns[ 2 ] += ev->motion.yrel * 0.2;
      }else if( rel ){
	drotx += ev->motion.xrel * 0.0005;
	droty += ev->motion.yrel * 0.0005;
      } else{
	rotx += ev->motion.xrel * 0.0005;
	roty += ev->motion.yrel * 0.0005;
      }
    }
  }
}
    

int main( int argc, char* argv[] ){
  (void)(argc);(void)(argv);

  srand( SDL_GetPerformanceCounter() );

  LNZInit( fullscreen, "LNZ3.1a", 0.9, 0.675 );
  SDL_GL_GetDrawableSize( mainWindow, &dwidth, &dheight );
  SDL_GL_SetSwapInterval( 0 );
  srand( 1337 );

  LNZSetKeyHandler( keys );
  LNZSetTouchHandler( touches );
  LNZSetMouseHandler( mice );
  LNZSetWindowHandler( wms );

		

  u64 sz;
  GLuint shd[ 2 ];
  u8* dt = LNZLoadResourceOrDie( "main.glsl", &sz );
  shd[ 0 ] = LNZCompileOrDie( (char*)dt, GL_COMPUTE_SHADER );
  GLuint prg = LNZLinkOrDie( 1, shd );
  dt = LNZLoadResourceOrDie( "gbuffer.frag", &sz );
  shd[ 0 ] = LNZCompileOrDie( (char*)dt, GL_FRAGMENT_SHADER );
  dt = LNZLoadResourceOrDie( "gbuffer.vert", &sz );
  shd[ 1 ] = LNZCompileOrDie( (char*)dt, GL_VERTEX_SHADER );
  GLuint bprg = LNZLinkOrDie( 2, shd );
  dt = LNZLoadResourceOrDie( "init.glsl", &sz );
  shd[ 0 ] = LNZCompileOrDie( (char*)dt, GL_COMPUTE_SHADER );
  GLuint iprg = LNZLinkOrDie( 1, shd );
  



  // 0 and 2 are position, 1 and 3 are velocity, 4 and 5 are gbuffers.
  GLuint buffers[ 6 ], texs[ 6 ];

  GLuint screenQuadBuffer;
  GLfloat screenQuad[ 8 ] = { -1, -1, 1, -1, 1, 1, -1, 1 };
  glGenBuffers( 1, &screenQuadBuffer );
  glBindBuffer( GL_ARRAY_BUFFER, screenQuadBuffer );
  glBufferData( GL_ARRAY_BUFFER, 4 * sizeof( GLfloat ) * 2,
		screenQuad, GL_STREAM_DRAW );
  

  glGenBuffers( 6, buffers );

  glBindBuffer(GL_ARRAY_BUFFER, buffers[ 0 ] );
  glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof( GLfloat ) * 4,
	       NULL, GL_DYNAMIC_COPY );

  // Randomize positions and velocities.
  GLfloat* positions = glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
  for( u32 i = 0; i < PARTICLE_COUNT; ++i ){
    for( u32 k = 0; k < 3; ++k )
      positions[ i * 4 + k ] = ( rand() / (double)RAND_MAX ) * 20.0 - 10.0;
    positions[ i * 4 + 3 ] = rand() / (double)RAND_MAX;
  }
  glUnmapBuffer( GL_ARRAY_BUFFER );
  glBindBuffer(GL_ARRAY_BUFFER, buffers[ 1 ] );
  glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof( GLfloat ) * 4,
	       NULL, GL_DYNAMIC_COPY );
  GLfloat* velocities = glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
  for( u32 i = 0; i < PARTICLE_COUNT; ++i ){
    for( u32 k = 0; k < 3; ++k )
      velocities[ i * 4 + k ] = ( rand() / (double)RAND_MAX ) * 0.2 - 0.1;
    velocities[ i * 4 + 3 ] = 0;
  }
  glUnmapBuffer( GL_ARRAY_BUFFER );

  for( u32 i = 2; i < 4; ++i ){
    glBindBuffer( GL_ARRAY_BUFFER, buffers[ i ] );
    glBufferData( GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof( GLfloat ) * 4,
		  NULL, GL_DYNAMIC_COPY );
  }
  for( u32 i = 4; i < 6; ++i ){
    glBindBuffer( GL_ARRAY_BUFFER, buffers[ i ] );
    glBufferData( GL_ARRAY_BUFFER, GBUFFER_SIZE * sizeof( GLuint ),
		  NULL, GL_DYNAMIC_COPY );
  }

  glGenTextures( 6, texs );
  for( u32 i = 0; i < 4; ++i ){
    glBindTexture( GL_TEXTURE_BUFFER, texs[ i ] );
    glTexBuffer( GL_TEXTURE_BUFFER, GL_RGBA32F, buffers[ i ] );
  }
  for( u32 i = 4; i < 6; ++i ){
    glBindTexture( GL_TEXTURE_BUFFER, texs[ i ] );
    glTexBuffer( GL_TEXTURE_BUFFER, GL_R32UI, buffers[ i ] );
  }

  GLuint ubuf;
  glGenBuffers( 1, &ubuf );
  glBindBuffer( GL_UNIFORM_BUFFER, ubuf );
  glBufferData( GL_UNIFORM_BUFFER, 64 * sizeof( GLfloat ) * 4, NULL,
		GL_DYNAMIC_COPY );
  

  GLuint screenQuadVao;
  glGenVertexArrays( 1, &screenQuadVao );
  glBindVertexArray( screenQuadVao );
  glBindBuffer( GL_ARRAY_BUFFER, screenQuadBuffer );
  glBindAttribLocation( bprg, 0, "vPosition" );
  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
  glEnableVertexAttribArray( 0 );

  
  double time = rand();
  u64 ntime = SDL_GetPerformanceCounter();
  GLuint dtloc = glGetUniformLocation( prg, "dt" );
  GLuint smvploc = glGetUniformLocation( prg, "mvp" );
  GLuint rmvloc = glGetUniformLocation( bprg, "rmv" );
  GLuint bscreenloc = glGetUniformLocation( bprg, "screen" );
  GLuint screenloc = glGetUniformLocation( prg, "screen" );
  GLuint gcountloc = glGetUniformLocation( iprg, "gcount" );
  GLfloat amasses[ 64 ];
  int bsel = 0, nbsel = 2;

  for( u32 i = 0; i < 64; ++i )
    amasses[ i ] = rand() / (double)RAND_MAX + 1;
  while( 1 ){
    LNZLoop();
    {
      GLuint er = glGetError();
      if( er != GL_NO_ERROR ){
	printf( "\n\nOpenGL Error Number %x!\n\n", er );
	exit( 42 );
      }
    }
    u64 ttime = ntime;
    ntime = SDL_GetPerformanceCounter();
    double dtime = ( ntime - ttime ) / (double)SDL_GetPerformanceFrequency();
    double udtime = dtime;
    dtime *= (double)scale;
    time += 0.001 * dtime;
    if( dtime )
      sfps = sfps * 0.95 + 0.05 / dtime;
    
    glBindBuffer( GL_UNIFORM_BUFFER, ubuf );
    GLfloat* attractors =
      (GLfloat*)glMapBuffer( GL_UNIFORM_BUFFER, GL_WRITE_ONLY );

    for( int i = 0; i < 64; i++ ){
      attractors[ i * 4 + 0 ] =  sin(time * (double)(i + 4) * 7.5 * 0.2)
      	* 50.0f;
      attractors[ i * 4 + 1 ] = cos(time * (double)(i + 7) * 3.9 * 0.2)
      	* 50.0f;
      attractors[ i * 4 + 2 ] = sin(time * (double)(i + 3) * 5.3 * 0.2)
      	* cos(time * (double)(i + 5) * 0.91f) * 100.0f;
      attractors[ i * 4 + 3 ] = amasses[ i ] * 15.0;
    }
    if( blowup ){
      attractors[ 0 ] = attractors[ 1 ] = attractors[ 2 ] = 0.0;
      attractors[ 1 ] = 10;
      attractors[ 3 ] = 16.15;
    }
    glUnmapBuffer( GL_UNIFORM_BUFFER );
    glBindBufferBase( GL_UNIFORM_BUFFER, 0, ubuf );

    rotx += udtime * drotx * 2;
    roty += udtime * droty * 2;
    
    lmat mvp, mv, rmv, proj;    
    {
      float aspect = sqrt( dwidth / (double)dheight );
      lvec sc = { 0.013 / aspect, 0.013 * aspect, 0.13 };
      lvec up = { cosf( rotx * 5 + pi / 2 ) * sinf( roty * 5 ), 
		  cosf( roty * 5 ), 
		  sinf( rotx * 5 + pi / 2 ) * sinf( roty * 5 ) };
      lvec right = { cosf( rotx * 5 ), 0.0, sinf( rotx * 5 ) };

      lmidentity( mv );
      lmbasis( mv, up, right );
      lmtranslate( mv, trns );
      lmcopy( rmv, mv );
      lminvert( rmv );
      lmidentity( proj );
      lmprojection( proj, 0.0125 );
      lmscale( proj, sc );
      lmcopy( mvp, mv );
      lmmult( mvp, proj );

    } 

    // Clear gbuffer. glClearBufferData is unreliable.
    glUseProgram( iprg );
    glBindImageTexture( 0, texs[ 4 + bsel / 2 ], 0, GL_FALSE, 0,
			GL_WRITE_ONLY, GL_R32UI );
    glUniform1ui( gcountloc, ( dwidth / pixelSize ) * ( dheight / pixelSize ) );
    glDispatchCompute( ( ( dwidth / pixelSize ) * ( dheight / pixelSize ) ) / 
		       1024 + 1, 1, 1 );

     
    glUseProgram( prg );
    glBindImageTexture( 0, texs[ 1 + bsel ], 0, GL_FALSE, 0,
			GL_READ_ONLY, GL_RGBA32F );
    glBindImageTexture( 1, texs[ 0 + bsel ], 0, GL_FALSE, 0, 
			GL_READ_ONLY, GL_RGBA32F );
    glBindImageTexture( 2, texs[ 1 + nbsel ], 0, GL_FALSE, 0, 
			GL_WRITE_ONLY, GL_RGBA32F );
    glBindImageTexture( 3, texs[ 0 + nbsel ], 0, GL_FALSE, 0, 
			GL_WRITE_ONLY, GL_RGBA32F );
    
    glBindImageTexture( 4, texs[ 4 + bsel / 2 ], 0, GL_FALSE, 0, 
			GL_READ_WRITE, GL_R32UI );
    
    glUniform1f( dtloc, dtime * 20 );
    glUniform4f( screenloc, dwidth / pixelSize, dheight / pixelSize, 
		 10.0 * ( 1024.0 * 1024.0 ) / ( PARTICLE_COUNT * pixelSize ), 
		 0 );
    glUniformMatrix4fv( smvploc, 1, GL_FALSE, mvp );
    
    glDispatchCompute( PARTICLE_GROUPS, 1, 1 );

    
      
    // Render quad.
    glUseProgram( bprg );
    glUniform4f( bscreenloc, dwidth / pixelSize, dheight / pixelSize, pixelSize, 0);
    glUniformMatrix4fv( rmvloc, 1, GL_FALSE, rmv );

    glBindVertexArray( screenQuadVao );
    glBindImageTexture( 4, texs[ 4 + bsel / 2 ], 0, GL_FALSE, 0, 
			GL_READ_WRITE, GL_R32UI );
    glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

    SDL_GL_SwapWindow( mainWindow );

    if( bsel ){
      bsel = 0;
      nbsel = 2;
    } else{
      bsel = 2;
      nbsel = 0;
    }
  }
  exit( EXIT_SUCCESS );
}

