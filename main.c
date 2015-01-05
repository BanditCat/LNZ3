////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

// Main entry point.
#include "lnz.h"
#include <stdio.h>
#include "math.h"

#define COMPUTE_GROUP_SIZE 1024

#define GBUFFER_WIDTH ( fullscreenDM.w )
#define GBUFFER_HEIGHT ( fullscreenDM.h )
#define GBUFFER_SIZE ( GBUFFER_HEIGHT * GBUFFER_WIDTH )

#define OCTREE_SIZE 20000000

#define WIREFRAME_SIZE 8192

#define FOV_MINIMUM ( pi * 0.01 )
#define FOV_MAXIMUM ( pi * 0.99 )

int movingWindow = 0;
int movingFov = 0;
float fov = pi / 2.0;

int wireframe = 0;
int fullscreen = 0;
int dwidth, dheight;
int pixelSize = 1;
// Amortized time.
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
  } else if( ev->key.state == SDL_PRESSED && ev->key.keysym.sym == SDLK_w ){
    if( wireframe )
      wireframe = 0;
    else
      wireframe = 1;
  } else if( ev->key.state == SDL_PRESSED && ev->key.keysym.sym == SDLK_m ){
    movingWindow = 1;
  } else if( ev->key.state == SDL_RELEASED && ev->key.keysym.sym == SDLK_m ){
    movingWindow = 0;
    int x, y;
    SDL_GetWindowSize( mainWindow, &x, &y );
    dwidth = x;
    dheight = y;
    glViewport( 0, 0, x, y );
  } else if( ev->key.state == SDL_PRESSED && ev->key.keysym.sym == SDLK_v ){
    movingFov = 1;
  } else if( ev->key.state == SDL_RELEASED && ev->key.keysym.sym == SDLK_v ){
    movingFov = 0;
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
      trns[ 2 ] += ( dist - odist ) * -100;
      trns[ 0 ] += ev->tfinger.dx * 100;
      trns[ 1 ] += ev->tfinger.dy * -100;
    } else if( SDL_GetNumTouchFingers( ev->tfinger.touchId ) == 3 ){
      SDL_Finger* f0 = SDL_GetTouchFinger( ev->tfinger.touchId, 0 );
      SDL_Finger* f1 = SDL_GetTouchFinger( ev->tfinger.touchId, 1 );
      SDL_Finger* f2 = SDL_GetTouchFinger( ev->tfinger.touchId, 2 );
      float dx0 = 0, dy0 = 0, dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
      if( ev->tfinger.fingerId == f0->id ){
	dx0 = ev->tfinger.dx; dy0 = ev->tfinger.dy;
      } else if( ev->tfinger.fingerId == f1->id ){
	dx1 = ev->tfinger.dx; dy1 = ev->tfinger.dy;
      } else if( ev->tfinger.fingerId == f2->id ){
	dx2 = ev->tfinger.dx; dy2 = ev->tfinger.dy;
      } 
      float dist = sqrt( pow( f0->x + dx0 - f1->x, 2.0 ) +
			 pow( f0->y + dy0 - f1->y, 2.0 ) );
      dist += sqrt( pow( f1->x + dx1 - f2->x, 2.0 ) +
		    pow( f1->y + dy1 - f2->y, 2.0 ) ); 
      dist += sqrt( pow( f2->x + dx2 - f0->x, 2.0 ) +
		    pow( f2->y + dy2 - f0->y, 2.0 ) ); 
      float odist = sqrt( pow( f0->x - f1->x, 2.0 ) + pow( f0->y - f1->y, 2.0 ) );
      odist += sqrt( pow( f1->x - f2->x, 2.0 ) + pow( f1->y - f2->y, 2.0 ) ); 
      odist += sqrt( pow( f2->x - f0->x, 2.0 ) + pow( f2->y - f0->y, 2.0 ) ); 
      fov += ( dist - odist ) * 4.0;
      if( fov < FOV_MINIMUM )
	fov = FOV_MINIMUM;
      if( fov > FOV_MAXIMUM )
	fov = FOV_MAXIMUM;
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
    } else if( movingFov ){
      fov += ev->motion.yrel / 512.0;
      if( fov < FOV_MINIMUM )
	fov = FOV_MINIMUM;
      if( fov > FOV_MAXIMUM )
	fov = FOV_MAXIMUM;
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
  u8* dt = LNZLoadResourceOrDie( "render.glsl", &sz );
  shd[ 0 ] = LNZCompileOrDie( (char*)dt, GL_COMPUTE_SHADER );
  GLuint prg = LNZLinkOrDie( 1, shd );
  free( dt );
  dt = LNZLoadResourceOrDie( "gbuffer.frag", &sz );
  shd[ 0 ] = LNZCompileOrDie( (char*)dt, GL_FRAGMENT_SHADER );
  free( dt );
  dt = LNZLoadResourceOrDie( "gbuffer.vert", &sz );
  shd[ 1 ] = LNZCompileOrDie( (char*)dt, GL_VERTEX_SHADER );
  GLuint bprg = LNZLinkOrDie( 2, shd );
  free( dt );
  dt = LNZLoadResourceOrDie( "wireframe.frag", &sz );
  shd[ 0 ] = LNZCompileOrDie( (char*)dt, GL_FRAGMENT_SHADER );
  free( dt );
  dt = LNZLoadResourceOrDie( "wireframe.vert", &sz );
  shd[ 1 ] = LNZCompileOrDie( (char*)dt, GL_VERTEX_SHADER );
  GLuint wprg = LNZLinkOrDie( 2, shd );
  free( dt );



  // 0 and 1 are gbuffers, 2 is the octree.
  GLuint buffers[ 3 ], texs[ 3 ];

  GLuint screenQuadBuffer;
  GLfloat screenQuad[ 8 ] = { -1, -1, 1, -1, 1, 1, -1, 1 };
  glGenBuffers( 1, &screenQuadBuffer );
  glBindBuffer( GL_ARRAY_BUFFER, screenQuadBuffer );
  glBufferData( GL_ARRAY_BUFFER, 4 * sizeof( GLfloat ) * 2,
		screenQuad, GL_STREAM_DRAW );
  
  

  glGenBuffers( 3, buffers );

  for( u32 i = 0; i < 2; ++i ){
    glBindBuffer( GL_ARRAY_BUFFER, buffers[ i ] );
    glBufferData( GL_ARRAY_BUFFER, GBUFFER_SIZE * sizeof( GLuint ),
		  NULL, GL_DYNAMIC_COPY );
  }
 
  
  GLuint wireframeBuffer;
  u32 actualWireframeSize = 1;
  {
    glBindBuffer( GL_ARRAY_BUFFER, buffers[ 2 ] );
    glBufferData( GL_ARRAY_BUFFER, OCTREE_SIZE * OCTREE_NODE_SIZE * sizeof( u32 ),
		  NULL, GL_DYNAMIC_COPY );
    GLuint* octree = glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
    
    sphereParams sp = { { 0.0, 0.2, 0.3 }, 0.5 };
    initOctree( sphere, octree, &sp );

    // Build wireframe
    GLfloat* wireframeData = lmalloc( WIREFRAME_SIZE * 6 * sizeof( GLfloat ) );
    u32* nodes = lmalloc( WIREFRAME_SIZE * sizeof( u32 ) );
    lvec* cubeCenters = lmalloc( WIREFRAME_SIZE * sizeof( lvec ) );
    float* cubeRadii = lmalloc( WIREFRAME_SIZE * sizeof( float ) );
    nodes[ 0 ] = 0;
    cubeCenters[ 0 ][ 0 ] = cubeCenters[ 0 ][ 1 ] = cubeCenters[ 0 ][ 2 ] = 0.0;
    cubeRadii[ 0 ] = 10.0;
    

    for( u32 i = 0; i < 8 && actualWireframeSize < WIREFRAME_SIZE; ++i ){
      if( octree[ 2 + i ] <= VALID_CHILD ){
	nodes[ actualWireframeSize ] = octree[ 2 + i ];
	lvec cc, ad;
	lvcopy( cc, cubeCenters[ 0 ] );
	lvcopy( ad, cubeVecs[ i ] );
	lvscale( ad, cubeRadii[ 0 ] / 2.0 );
	lvadd( cc, ad );
	lvcopy( cubeCenters[ actualWireframeSize ], cc );
	cubeRadii[ actualWireframeSize++ ] = cubeRadii[ 0 ] / 2.0;
      }	  
    }
    u32 oldsize = actualWireframeSize - 1;
    while( oldsize != actualWireframeSize &&
	   actualWireframeSize < WIREFRAME_SIZE ){ 
      oldsize = actualWireframeSize;
      for( u32 cur = actualWireframeSize - 1; 
	   cubeRadii[ cur ] == cubeRadii[ cur - 1 ] && 
	     actualWireframeSize < WIREFRAME_SIZE; 
	   --cur ){
	for( u32 i = 0; i < 8 && actualWireframeSize < WIREFRAME_SIZE; ++i ){
	  if( octree[ cur * OCTREE_NODE_SIZE + 2 + i ] <= VALID_CHILD ){
	    nodes[ actualWireframeSize ] = octree[ cur * OCTREE_NODE_SIZE +
						   2 + i ];
	    lvec cc, ad;
	    lvcopy( cc, cubeCenters[ cur ] );
	    lvcopy( ad, cubeVecs[ i ] );
	    lvscale( ad, cubeRadii[ cur ] / 2.0 );
	    lvadd( cc, ad );
	    lvcopy( cubeCenters[ actualWireframeSize ], cc );
	    cubeRadii[ actualWireframeSize++ ] = cubeRadii[ cur ] / 2.0;
	  }	  
	}
      }
    }
    
    for( u32 i = 1; i < actualWireframeSize; ++i ){
      lvec ne;
      lunpackNormal( octree[ nodes[ i ] * OCTREE_NODE_SIZE ], ne );
      lvnormalize( ne );
      lvscale( ne, cubeRadii[ i ] / 3.0 );
      lvadd( ne, cubeCenters[ i ] );
      lvcopy( wireframeData + 6 * i, cubeCenters[ i ] );
      lvcopy( wireframeData + 6 * i + 3, ne ); 
    }

    glUnmapBuffer( GL_ARRAY_BUFFER );
  
  
    glGenBuffers( 1, &wireframeBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, wireframeBuffer );
    glBufferData( GL_ARRAY_BUFFER, actualWireframeSize * 6 * sizeof( GLfloat ), 
		  wireframeData, GL_STREAM_DRAW );

    free( wireframeData );
    free( nodes );
    free( cubeCenters );
    free( cubeRadii );

  }
    
  

  glGenTextures( 3, texs );
  for( u32 i = 0; i < 3; ++i ){
    glBindTexture( GL_TEXTURE_BUFFER, texs[ i ] );
    glTexBuffer( GL_TEXTURE_BUFFER, GL_R32UI, buffers[ i ] );
  }
  

  GLuint screenQuadVao;
  glGenVertexArrays( 1, &screenQuadVao );
  glBindVertexArray( screenQuadVao );
  glBindBuffer( GL_ARRAY_BUFFER, screenQuadBuffer );
  glBindAttribLocation( bprg, 0, "vposition" );
  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
  glEnableVertexAttribArray( 0 );

  GLuint wireframeVao;
  glGenVertexArrays( 1, &wireframeVao );
  glBindVertexArray( wireframeVao );
  glBindBuffer( GL_ARRAY_BUFFER, wireframeBuffer );
  glBindAttribLocation( wprg, 0, "vposition" );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
  glEnableVertexAttribArray( 0 );

  
  double time = rand();
  u64 ntime = SDL_GetPerformanceCounter();
  GLuint mvploc = glGetUniformLocation( wprg, "mvp" );
  GLuint rmvloc = glGetUniformLocation( prg, "rmv" );
  GLuint bscreenloc = glGetUniformLocation( bprg, "screen" );
  GLuint screenloc = glGetUniformLocation( prg, "screen" );
  GLuint gcountloc = glGetUniformLocation( prg, "gcount" );
  GLuint fovloc = glGetUniformLocation( prg, "fov" );
  int bsel = 0, nbsel = 1;

  

  while( 1 ){
    LNZLoop();
    {
      GLuint er = glGetError();
      if( er != GL_NO_ERROR ){
	SDL_Log( "\n\nOpenGL Error Number %x!\n\n", er );
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
    

    rotx += udtime * drotx * 2;
    roty += udtime * droty * 2;
    
    // Model view projection, model view, inverse model view and projection matrices.
    lmat mvp, mv, rmv, proj;    
    float aspect = sqrt( dwidth / (double)dheight );
    {
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
      lmprojection( proj, fov, aspect, 0.0125, 1024.0 );
      lmcopy( mvp, mv );
      lmmult( mvp, proj );

    } 

    // Render to gbuffer.
    glUseProgram( prg );
    glBindImageTexture( 0, texs[ nbsel ], 0, GL_FALSE, 0,
			GL_WRITE_ONLY, GL_R32UI );
    //    glBindImageTexture( 1, texs[ 2 ], 0, GL_FALSE, 0,
    //			GL_READ_ONLY, GL_R32UI );
    glUniform1ui( gcountloc, ( dwidth / pixelSize ) * ( dheight / pixelSize ) );
      
    glUniform4f( screenloc, dwidth / pixelSize, dheight / pixelSize,
		 pixelSize, aspect );
    glUniform1f( fovloc, fov );
    glUniformMatrix4fv( rmvloc, 1, GL_FALSE, rmv );
    
    glDispatchCompute( ( ( dwidth / pixelSize ) * ( dheight / pixelSize ) ) / 
		       COMPUTE_GROUP_SIZE + 1, 1, 1 );
    
    // Render quad.
    glUseProgram( bprg );
    glUniform4f( bscreenloc, dwidth / pixelSize, dheight / pixelSize, pixelSize, aspect );
   
    glBindVertexArray( screenQuadVao );
    glBindImageTexture( 4, texs[ bsel ], 0, GL_FALSE, 0, 
    	GL_READ_WRITE, GL_R32UI );
     glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

 
    // Wireframe of normals
    if( wireframe ){
      glUseProgram( wprg );
      glUniformMatrix4fv( mvploc, 1, GL_FALSE, mvp );

      glBindVertexArray( wireframeVao );
      glDrawArrays( GL_LINES, 0, actualWireframeSize * 2 );
    }


    SDL_GL_SwapWindow( mainWindow );

    if( bsel ){
      bsel = 0;
      nbsel = 1;
    } else{
      bsel = 1;
      nbsel = 0;
    }
  }
  exit( EXIT_SUCCESS );
}
