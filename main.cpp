#include<iostream>
#include<SDL2/SDL.h>


const int width =800,hight=600;

int main(int argc, char *argv[])
{
    SDL_Init( SDL_INIT_EVERYTHING );

    SDL_Window *window = SDL_CreateWindow( "Hello SDL WORLD", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, hight, SDL_WINDOW_ALLOW_HIGHDPI );

    if ( NULL == window )
    {
        std::cout << "Could not create window: " << SDL_GetError( ) << std::endl;
        return 1;
    }

    SDL_Event windowEvent;

    while ( true )
    {
        if ( SDL_PollEvent( &windowEvent ) )
        {
            if ( SDL_QUIT == windowEvent.type )
            { break; }
        }
    }

    SDL_DestroyWindow( window );
    SDL_Quit( );

    return EXIT_SUCCESS;
}
