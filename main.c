#include <SDL/SDL.h>
#include "imageloader.h"
#include "testlevel.h"
#define SCR_H 240
#define SCR_W 320
#define BLOCKPERSCRNH SCR_H/16
#define BLOCKPERSCRNW SCR_W/16
int up,down,left,right,start;
int inputStateChanged;
int camx=0,camy=0;
unsigned int lastTime=0;
float delta;
typedef struct vector2{
	int x;
	int y;
}Vector2;
typedef struct vector2f{
	float x;
	float y;
}Vector2f;
typedef struct anim{
	Vector2 pos;
	Vector2 dim;
}Anim;
typedef struct collider{
	Vector2 offset;
	Vector2 dim;
}Collider;
typedef struct entity{
	Vector2f pos;
	Vector2f speed;
	int health;
	SDL_Surface *sprite;
	Anim anims[16];
	int numanim;
	int curanim;
	Collider col;
	int onground;
}Entity;
SDL_Surface *screen;
    SDL_Surface *playerspr;
	SDL_Surface *tiles;
	SDL_Surface *backg;
	Entity player;
void calculateRectFromBlockNum(int num,int mapw, int blockd,SDL_Rect* r){
		r->x=(num%mapw)*blockd;
		r->y=(num/mapw)*blockd;
		r->w=blockd;
		r->h=blockd;
}
void drawMapMainLayer(){
		SDL_Rect srcr,dstr;
		int startheight=camy/16;
		int startwidth=camx/16;
		int endheight=((camy+SCR_H)/16)+1<LEVEL_HEIGHT?((camy+SCR_H)/16)+1:LEVEL_HEIGHT;
		int endwidth=((camx+SCR_W)/16)+1<LEVEL_WIDTH?((camx+SCR_W)/16)+1:LEVEL_WIDTH;
		for(int i=startheight;i<endheight;i++){
			for(int j=startwidth;j<endwidth;j++){
				
				calculateRectFromBlockNum(level[i*LEVEL_WIDTH+j],8,16,&srcr);
				
				dstr.x=(j*16)-camx;
				dstr.y=(i*16)-camy;
				if(srcr.x!=0||srcr.y!=0){
					printf("Block coords in tilemap: x:%i y:%i\n",srcr.x,srcr.y);
					printf("Block coords in frame: x:%i y:%i\n",dstr.x,dstr.y);
				}
				SDL_BlitSurface( tiles, &srcr, screen, &dstr );
			}
		}
}
void initPlayer(){
	player.pos.x=0;
	player.pos.y=300;
	player.numanim=1;
	player.anims[0].pos.x=0;
	player.anims[0].pos.y=0;
	player.anims[0].dim.x=64;
	player.anims[0].dim.y=32;
	player.curanim=0;
	player.health=100;
	player.sprite=playerspr;
	player.col.offset.x=16;
	player.col.offset.y=0;
	player.col.dim.x=48;
	player.col.dim.y=32;
}
void drawEntity(Entity *e){
	SDL_Rect srcr,dstr;
	dstr.x=e->pos.x-camx;
	dstr.y=e->pos.y-camy;
	srcr.x=e->anims[e->curanim].pos.x;
	srcr.y=e->anims[e->curanim].pos.y;
	srcr.w=e->anims[e->curanim].dim.x;
	srcr.h=e->anims[e->curanim].dim.y;
	SDL_BlitSurface( playerspr, &srcr, screen,&dstr );
}
void centerPlayer(){
	camx=player.pos.x-(SCR_W/2);
	if(camx<0){
		camx=0;
	}else if(camx+SCR_W>(LEVEL_WIDTH*16)){
		camx=(LEVEL_WIDTH*16)-SCR_W;
	}
	camy=player.pos.y-(SCR_H/2);
	if(camy<0){
		camy=0;
	}else if(camy+SCR_H>(LEVEL_HEIGHT*16)){
		camy=(LEVEL_HEIGHT*16)-SCR_H;
	}
	
}
void drawBackground(){
	SDL_Rect dstr1,dstr2;
	dstr1.x=(int)(camx*-0.5f)%(backg->w);
	dstr1.y=(camy*-0.5f);
	dstr2.x=dstr1.x+backg->w;
	dstr2.y=dstr1.y;
	SDL_BlitSurface( backg, NULL, screen, &dstr1 );
	SDL_BlitSurface( backg, NULL, screen, &dstr2 );
}


void checkEntityGroundCol(Entity *e){
	int i, x1, x2, y1, y2;
	Vector2f entpos;
	entpos.x=(e->pos.x)+(e->col.offset.x);
	entpos.y=(e->pos.y)+(e->col.offset.y);
	e->onground = 0;
	i = e->col.dim.y > 16 ? 16 : e->col.dim.y;
	for (;;)
    {
		x1 = (entpos.x + (e->speed.x*delta)) / 16;
        x2 = (entpos.x + (e->speed.x*delta) + e->col.dim.x - 1) / 16;
		y1 = (entpos.y) / 16;
        y2 = (entpos.y + i - 1) / 16;
		
		if (x1 >= 0 && x2 < LEVEL_WIDTH && y1 >= 0 && y2 < LEVEL_HEIGHT)
        {
            if (e->speed.x > 0)
            {
                /* Trying to move right */

                if ((level[y1*LEVEL_WIDTH+x2] != 0) || (level[y2*LEVEL_WIDTH+x2]!=0))
                {
                    /* Place the player as close to the solid tile as possible */

                    e->pos.x = (x2 * 16)-e->col.offset.x;

                    e->pos.x -= e->col.dim.x + 1;

                    e->speed.x = 0;
                }
            }

            else if (e->speed.x < 0)
            {
                /* Trying to move left */

                if ((level[y1*LEVEL_WIDTH+x1] != 0) || (level[y2*LEVEL_WIDTH+x1]!=0))
                {
                    /* Place the player as close to the solid tile as possible */

                    e->pos.x = ((x1 + 1) * 16)-e->col.offset.x;

                    e->speed.x = 0;
                }
            }
        }

        if (i == e->col.dim.y)
        {
            break;
        }

        i += 16;

        if (i > e->col.dim.y)
        {
            i = e->col.dim.y;
        }
    }

	i = e->col.dim.x > 16 ? 16 : e->col.dim.x;
	for (;;)
    {
		x1 = (entpos.x ) / 16;
        x2 = (entpos.x + i) / 16;
		y1 = (entpos.y + (e->speed.y*delta)) / 16;
        y2 = (entpos.y + (e->speed.y*delta) + e->col.dim.y) / 16;
		
		if (x1 >= 0 && x2 < LEVEL_WIDTH && y1 >= 0 && y2 < LEVEL_HEIGHT)
        {
            if (e->speed.y > 0)
            {
                /* Trying to move right */

                if ((level[y2*LEVEL_WIDTH+x1] != 0) || (level[y2*LEVEL_WIDTH+x2]!=0))
                {
                    /* Place the player as close to the solid tile as possible */

                    e->pos.y = (y2 * 16)-e->col.offset.y;

                    e->pos.y -= e->col.dim.y;

                    e->speed.y = 0;
					e->onground = 1;
                }
            }

            else if (e->speed.y < 0)
            {
                /* Trying to move left */

                if ((level[y1*LEVEL_WIDTH+x1] != 0) || (level[y1*LEVEL_WIDTH+x2]!=0))
                {
                    /* Place the player as close to the solid tile as possible */

                    e->pos.y = ((y1 + 1) * 16)-e->col.offset.y;

                    e->speed.y = 0;
                }
            }
        }

        if (i == e->col.dim.y)
        {
            break;
        }

        i += 16;

        if (i > e->col.dim.y)
        {
            i = e->col.dim.y;
        }
    }
	e->pos.x+=(e->speed.x*delta);
	e->pos.y+=(e->speed.y*delta);
	
	if(e->pos.x<0){
		e->pos.x=0;
		
	}else if((e->pos.x+e->col.offset.x+e->col.dim.x)>=LEVEL_WIDTH*16){
		e->pos.x=(LEVEL_WIDTH*16)-e->col.dim.x-e->col.offset.x-1;
	}
	
	
}


void doPlayer(){
	player.speed.x=0;
	player.speed.y+=700*delta;
	if(player.speed.y>=200){
		player.speed.y=200;
	}
	if(right){
		player.speed.x=150;
	}
	if(left){
		player.speed.x=-150;
	}
	if(up){
		if(player.onground){
			player.speed.y=-300;
		}
	}
	checkEntityGroundCol(&player);
	/*if(down){
		player.pos.y+=1;
	}*/
	
	
}
void calcDelta(){
	unsigned int newtime=SDL_GetTicks();
	delta=(float)(newtime-lastTime)/1000;
	lastTime=newtime;
}
int main(int argc , char **argv)
{
    //SDL_Window *window;
    
    SDL_Event event;
    
    if(SDL_Init(SDL_INIT_EVERYTHING)) {
        fprintf(stderr,"SDL error %s\n", SDL_GetError());
        return 2;
    }
    
    playerspr=loadQoi("PLAYER.qoi");
	tiles=loadQoi("tiles.qoi");
	backg=loadJpeg("JUNGLE.jpg");
    //window = SDL_CreateWindow("SSFN normal renderer bitmap font test",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,800,600,0);
    screen = SDL_SetVideoMode(SCR_W,SCR_H,32,SDL_SWSURFACE);
    //memset(screen->pixels, 0xF8, screen->pitch*screen->h);
	initPlayer();
	SDL_Rect srcr,dstr;
	while(1){
		while (SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_KEYDOWN:
					inputStateChanged=1;
					switch(event.key.keysym.sym){
						case SDLK_RIGHT:
							right=1;
							break;
						case SDLK_LEFT:
							left=1;
							break;
						case SDLK_UP:
							up=1;
							break;
						case SDLK_DOWN:
							down=1;
							break;
						case SDLK_SPACE:
							start=1;
							break;
						
					}
					break;
				case SDL_KEYUP:
					inputStateChanged=1;
                    switch(event.key.keysym.sym){
						case SDLK_RIGHT:
							right=0;
							break;
						case SDLK_LEFT:
							left=0;
							break;
						case SDLK_UP:
							up=0;
							break;
						case SDLK_DOWN:
							down=0;
							break;
						case SDLK_SPACE:
							start=0;
							break;
						
					}
					break;    
                       

                    
                case SDL_QUIT:
                    SDL_Quit();
                    break;
				 
				 
				 
				 
			}
			
			 
			 
			 
			 
		}
		calcDelta();
		doPlayer();
		centerPlayer();
		drawBackground();
		drawMapMainLayer();
		drawEntity(&player);
		SDL_Flip( screen );
		//camx++;
	}
    //Update Screen
    //SDL_Flip( screen );
    //do_test(screen, argv[1]);
    
    //do{ /*SDL_UpdateWindowSurface(window);*/SDL_Flip(screen); SDL_Delay(10); } while(SDL_WaitEvent(&event) && event.type != SDL_QUIT &&
    //   event.type != SDL_MOUSEBUTTONDOWN && event.type != SDL_KEYDOWN);

    //SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}