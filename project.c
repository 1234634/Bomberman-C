
///////////////////////////////////////////////////////////////////////////////
// Headers.

#include <stdint.h>
#include "system.h"
#include <stdio.h>
#include<time.h>
#include<stdlib.h>
#include "sprites_rgb333.h"
#include<pthread.h>
#include<unistd.h>
///////////////////////////////////////////////////////////////////////////////
// HW stuff.

#define WAIT_UNITL_0(x) while(x != 0){}
#define WAIT_UNITL_1(x) while(x != 1){}

#define SCREEN_W 160
#define SCREEN_H 120
#define SCREEN_RGB333_W 160
#define SCREEN_RGB333_H 120

#define gpu_p32 ((volatile uint32_t*)LPRS2_GPU_BASE)
#define palette_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0x1000))
#define unpack_idx4_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0x800000))
#define pack_idx4_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0xa00000))
#define joypad_p32 ((volatile uint32_t*)LPRS2_JOYPAD_BASE)
#define unpack_rgb333_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0xc00000))

typedef struct {
	unsigned a      : 1;
	unsigned b      : 1;
	unsigned z      : 1;
	unsigned start  : 1;
	unsigned up     : 1;
	unsigned down   : 1;
	unsigned left   : 1;
	unsigned right  : 1;
} bf_joypad;
#define joypad (*((volatile bf_joypad*)LPRS2_JOYPAD_BASE))

typedef struct {
	uint32_t m[SCREEN_H][SCREEN_W];
} bf_unpack_rgb333;
#define unpack_rgb333 (*((volatile bf_unpack_rgb333*)unpack_rgb333_p32))



///////////////////////////////////////////////////////////////////////////////
// Game config.

#define STEP 16

#define RECT_H 16
#define RECT_W 16
#define SQ_A 16
#define BLOCK_L 16
#define ON 1
#define OFF 0


///////////////////////////////////////////////////////////////////////////////
// Game data structures.
//draw_sprite_from_atlas(0,48,16,16,gs.bomb.x,gs.bomb.y);
typedef struct {
	uint16_t x;
	uint16_t y;
} point_t;

typedef struct{

	int spriteX;
	int spriteY;
	int spriteWidth;
	int spriteHeight;

}sprite_desc;

typedef struct{

	int state; //1 if ON , 0 if OFF
	sprite_desc sd;
	point_t cord;

}var_elem;


typedef enum {BOMBERMAN} player_t;

typedef struct {
	// Upper left corners.
	point_t bomberman;

   var_elem block[10];
   var_elem enemy[2];
	
   point_t goal;
	
   var_elem bomb;


	player_t active;

    bf_joypad joy_now;
    bf_joypad joy_last;
} game_state_t;

void draw_sprite_from_atlas(
	uint16_t src_x,
	uint16_t src_y,
	uint16_t w,
	uint16_t h,
	uint16_t dst_x,
	uint16_t dst_y
) {
	
	
	for(uint16_t y = 0; y < h; y++){
		for(uint16_t x = 0; x < w; x++){
			uint32_t src_idx = 
				(src_y+y)*BombermanGeneralSprites__w +
				(src_x+x);
			uint32_t dst_idx = 
				(dst_y+y)*SCREEN_RGB333_W +
				(dst_x+x);
			uint16_t pixel = BombermanGeneralSprites__p[src_idx];
        if(pixel !=0x001){
            unpack_rgb333_p32[dst_idx] = pixel;
		}
	}
}
	
	
}



///////////////////////////////////////////////////////////////////////////////
// Game code.
// Check Field limitations
int checkInRange(point_t , point_t ,int );
void setBlocks(game_state_t *);
void setEnemies(game_state_t *);
void setEnemyPos(game_state_t *);

void* enemyFunc(void* argGs)
{
	
	
	while(1)
	{	
		setEnemyPos((game_state_t*)argGs);		
		sleep(1);
	
	}

	return NULL;
}

void* bombHasBeenPlanted(void* argGs)
{
	game_state_t *gs = (game_state_t*)argGs;
	int movement=1;
	sleep(1);
	
    	gs->bomb.sd.spriteX = 16;
    	gs->bomb.sd.spriteY = 96;
    	gs->bomb.sd.spriteWidth = 48;
    	gs->bomb.sd.spriteHeight = 16;
	//movement &= checkFieldLimitations(gs->bomb.cord.x,gs->bomb.cord.y); 
	if(gs->bomb.cord.x==0) 
		movement &= 0;
	
	gs->bomb.cord.x -= movement*16;
	
	sleep(1);
    	gs->bomb.sd.spriteX = 0;
    	gs->bomb.sd.spriteY = 48;
    	gs->bomb.sd.spriteWidth = 16;
    	gs->bomb.sd.spriteHeight = 16;
    	gs->bomb.cord.x += movement*16;
    	gs->bomb.state = OFF;
	
	return NULL;
}

int bombExploding(game_state_t* gs )
{
   int ret =0;
   if( gs->bomb.sd.spriteWidth > 16)
	ret =1;

   return ret;


}

void resetPos(game_state_t*);
int checkFieldLimitations(int,int);
//Checking if argument is beetwen borders
int inRange(int , int , int );
//Check if bomberman hits the block 
int checkCollisionWithBlocks(int,int);
int main(void) {
	
	// Setup.
	gpu_p32[0] = 3; // 4b index mode.
	gpu_p32[1] = 0; // Unpacked
	

	// Game state.
	game_state_t gs;
    
	setBlocks(&gs);	
	
	pthread_t ptid1;
       	pthread_create(&ptid1, NULL, &enemyFunc, (void*)(&gs)); 
	pthread_t ptid2;
	
	gs.bomb.state =OFF;
    	gs.bomb.sd.spriteX = 0;
    	gs.bomb.sd.spriteY = 48;
    	gs.bomb.sd.spriteWidth = 16;
    	gs.bomb.sd.spriteHeight = 16;
	setEnemies(&gs);	

	gs.bomberman.x = 0;
	gs.bomberman.y = 0;


        gs.goal.x = 80;
        gs.goal.y = 64;


	
    gs.joy_last.down = joypad.down;
    gs.joy_last.up = joypad.up;
    gs.joy_last.left = joypad.left;
    gs.joy_last.right = joypad.right;
    gs.joy_last.z = joypad.z;

	
	while(1){
		
		
		/////////////////////////////////////
		// Poll controls.

        gs.joy_now.down = joypad.down;
        gs.joy_now.up = joypad.up;
        gs.joy_now.left = joypad.left;
        gs.joy_now.right = joypad.right;
        gs.joy_now.z= joypad.z;

		int mov_x = 0;
		int mov_y = 0;
		if(gs.joy_now.down == 1 && gs.joy_last.down == 0){
			mov_y = +1;
		}
		if(gs.joy_now.up == 1 && gs.joy_last.up == 0){
			mov_y = -1;
		}
		if(gs.joy_now.right == 1 && gs.joy_last.right == 0){
			mov_x = +1;
		}
		if(gs.joy_now.left == 1 && gs.joy_last.left == 0){
			mov_x = -1;
		}
		//TODO Have bug here. Hold right button and play with A button.
	int toggle_bomb = 0; 
	
	if(gs.joy_now.z ==1 && gs.joy_last.z == 0){
	
		 toggle_bomb = joypad.z;
         joypad.z = 0;	
	}
       // printf("T = %d\n",toggle_bomb);
	gs.joy_last = gs.joy_now;//rastuca ivica
		
		/////////////////////////////////////
		// Gameplay.
       
        int movement=1, x=0,y=0;
	point_t temp;
	temp.x = gs.bomberman.x+ mov_x*STEP;	
	temp.y = gs.bomberman.y+ mov_y*STEP;	
        
	movement &= checkFieldLimitations(temp.x,temp.y); 
        movement &= checkCollisionWithBlocks(temp.x,temp.y);
        for(int i=0; i <10; i++)
		    if(gs.block[i].state)
			    movement &= checkInRange(temp,gs.block[i].cord,0);   
            
        gs.bomberman.x += mov_x*STEP*movement;
        gs.bomberman.y += mov_y*STEP*movement;
        

	for(int i =0; i<2; i++)
	   if(gs.enemy[i].state)
		if(!checkInRange(temp,gs.enemy[i].cord,0))
			resetPos(&gs);

        if(toggle_bomb)
	{
		if(gs.bomb.state == OFF)
		{
			gs.bomb.state = ON;
			gs.bomb.cord.x = gs.bomberman.x;
			gs.bomb.cord.y = gs.bomberman.y;
       		pthread_create(&ptid2, NULL, &bombHasBeenPlanted, (void*)(&gs)); 
							
		}
				
			
	}

    // bomb destroying destroyable elements
	if(bombExploding(&gs))
	{
		for(int i =0; i<10; i++)
		   if(gs.block[i].state)
                if(!checkInRange(gs.block[i].cord,gs.bomb.cord,2))
			        gs.block[i].state =OFF;	
	
	    for(int i =0; i<2; i++)
	        if(gs.enemy[i].state)
		        if(!checkInRange(gs.enemy[i].cord,gs.bomb.cord,2))
    			    gs.enemy[i].state = OFF;    
   
   
		if(!checkInRange(temp,gs.bomb.cord,2))
			resetPos(&gs);
    }	
		
	if(!checkInRange(temp,gs.goal,0))
			resetPos(&gs);
		/////////////////////////////////////
		// Drawing.
		
		
		// Detecting rising edge of VSync.
		WAIT_UNITL_0(gpu_p32[2]);
		WAIT_UNITL_1(gpu_p32[2]);
		// Draw in buffer while it is in VSync.
		
		
		
		// Clear to green
		for(int r = 0; r < SCREEN_RGB333_H; r++){
			for(int c = 0; c < SCREEN_RGB333_W; c++){
				unpack_rgb333_p32[r*SCREEN_RGB333_W + c] = 0x911;
			}
		}

	
		

        draw_sprite_from_atlas(176,48,16,16,gs.goal.x,gs.goal.y); 

        for(int i=0;i<2;i++ )
		if(gs.enemy[i].state)
            draw_sprite_from_atlas(gs.enemy[i].sd.spriteX,gs.enemy[i].sd.spriteY,gs.enemy[i].sd.spriteWidth,gs.enemy[i].sd.spriteHeight,gs.enemy[i].cord.x,gs.enemy[i].cord.y);
 
                

        for(int i =0;i < 10;i++)
		if(gs.block[i].state)
            draw_sprite_from_atlas(gs.block[i].sd.spriteX,gs.block[i].sd.spriteY,gs.block[i].sd.spriteWidth,gs.block[i].sd.spriteHeight,gs.block[i].cord.x,gs.block[i].cord.y);

        if(gs.bomb.state == ON)
        {
            draw_sprite_from_atlas(gs.bomb.sd.spriteX,gs.bomb.sd.spriteY,gs.bomb.sd.spriteWidth,gs.bomb.sd.spriteHeight,gs.bomb.cord.x,gs.bomb.cord.y);
        }

        for(int j=1;j<6;j+=2)
            for(int i = 1;i<10;i+=2)
                   draw_sprite_from_atlas(48,48,16,16,i*BLOCK_L,j*BLOCK_L);
        
        draw_sprite_from_atlas(64,0,16,16,gs.bomberman.x,gs.bomberman.y);
		
		
		

	}

	pthread_join(ptid1, NULL); 
	pthread_join(ptid2, NULL); 
	return 0;
}





int inRange(int argNum, int argLowBorder, int argHighBorder)
{
    int ret=1;
    
    if( argNum < argLowBorder || argNum > argHighBorder)
        ret = 0;


    return ret;
}
int checkInRange(point_t argInput, point_t argLimit,int argCoef)
{
	
		
   int ret = 1,tempY ,tempX;
   //inRange(argX, 1+ Coord.x -BLOCK_L, 1+ Coord.y+BLOCK_L-STEP);
 
   tempY = (!inRange(argInput.y,1+argLimit.y-BLOCK_L,1+argLimit.y + (1)*BLOCK_L-STEP));
   tempX = (!inRange(argInput.x,1+argLimit.x-BLOCK_L,1+argLimit.x+(1+argCoef)*BLOCK_L-STEP));
   ret = tempY || tempX;
                
    return ret;         
               

}	
int checkCollisionWithBlocks(int argX ,int argY)
{
   
   int ret = 1,tempY ,tempX;

        for(int j=0;j<6;j+=2)
            for(int i = 0;i<10;i+=2)
            {
                tempY = (!inRange(argY,1+j*BLOCK_L,1+(j+2)*BLOCK_L-STEP));
                tempX = (!inRange(argX,1+i*BLOCK_L,1+(i+2)*BLOCK_L-STEP));
                ret = tempY || tempX;
                
                if(ret == 0)
                    return ret;         
            
            }
               
    return ret;
}


int checkFieldLimitations(int argX,int argY)
{
    int ret =1;    
    ret &= inRange(argX,0,SCREEN_W-BLOCK_L);
    ret &= inRange(argY,0,SCREEN_H-BLOCK_L-8);
    
    return ret;

}

void resetPos(game_state_t* gs)
{
   gs->bomberman.x =0;
   gs->bomberman.y =0;
   setEnemies(gs);
   setBlocks(gs);

}
void setEnemyPos(game_state_t *gs)
{
    	int direction;
	srand(time(NULL));
	
	for(int i =0; i<2;i++)
    {
	int movX=0,movY=0;
	 direction = rand()%4;
	
	switch(direction)
	{
		case 0:
			movX =1; 
			break;
		case 1:
			movY =-1; 
			break;
		case 2:
			movX =-1; 
			break;
		default:       //argDirection ==3
			movY =1; 
			break;	
	
       }
	
        int movement=1 ;
	point_t temp;
	temp.x = gs->enemy[i].cord.x+ movX*STEP;	
	temp.y = gs->enemy[i].cord.y+ movY*STEP;	
        
	movement &= checkFieldLimitations(temp.x,temp.y); 
        movement &= checkCollisionWithBlocks(temp.x,temp.y);
        for(int i=0; i <10; i++)
		movement &= checkInRange(temp,gs->block[i].cord,0);   
            
        gs->enemy[i].cord.x += movX*STEP*movement;
        gs->enemy[i].cord.y += movY*STEP*movement;

     }
}

void setEnemies(game_state_t *gs)
{
	
    gs->enemy[0].cord.x = 80;
    gs->enemy[0].cord.y = 32;
   
    gs->enemy[1].cord.x = 112;
    gs->enemy[1].cord.y = 64;


    for(int i=0;i<2;i++)
    {
    	gs->enemy[i].state = 1;
    	gs->enemy[i].sd.spriteX = 0;
    	gs->enemy[i].sd.spriteY = 272;
    	gs->enemy[i].sd.spriteWidth = 16;
    	gs->enemy[i].sd.spriteHeight = 16;

    }

}

void setBlocks(game_state_t *gs)
{
	
    gs->block[0].cord.x = 64;
    gs->block[0].cord.y = 64;
   
    gs->block[1].cord.x = 48;
    gs->block[1].cord.y = 64;

    gs->block[2].cord.x = 64;
    gs->block[2].cord.y = 80;

    gs->block[3].cord.x = 96;
    gs->block[3].cord.y = 48;

    gs->block[4].cord.x = 128;
    gs->block[4].cord.y = 32;

    gs->block[5].cord.x = 96;
    gs->block[5].cord.y = 0;

    gs->block[6].cord.x = 96;
    gs->block[6].cord.y = 64;

    gs->block[7].cord.x = 128;
    gs->block[7].cord.y = 16;

    gs->block[8].cord.x = 96;
    gs->block[8].cord.y = 16;

    gs->block[9].cord.x = 32;
    gs->block[9].cord.y = 32;

    for(int i=0;i<10;i++)
    {
    	gs->block[i].state = 1;
    	gs->block[i].sd.spriteX = 64;
    	gs->block[i].sd.spriteY = 48;
    	gs->block[i].sd.spriteWidth = 16;
    	gs->block[i].sd.spriteHeight = 16;

    }

}
///////////////////////////////////////////////////////////////////////////////

