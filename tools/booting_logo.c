#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
 
int main(int argc, char** argv)
{
    SDL_Surface* screen=NULL;

    if(argc != 2){
        printf("usage:\n\tbooting_logo xxx.jpg\n");
        return -1;
    }
 
    SDL_Init(SDL_INIT_VIDEO);
    screen = SDL_SetVideoMode(320, 240, 16, SDL_SWSURFACE);

    SDL_Surface* png = IMG_Load(argv[1]);
    SDL_BlitSurface(png, NULL, screen, NULL);
    SDL_Flip(screen);
    SDL_Delay(3000);

    int x=0, y=0;
    uint16_t *p=screen->pixels;
    printf("uint16_t booting_logo[]={\n");
    for(y=0; y<240; y++){
        for(x=0; x<320; x++){
            printf("0x%x, ", *p++);
        }
    }
    printf("};");

    SDL_FreeSurface(png);
    SDL_Quit();
    return 0;
}
