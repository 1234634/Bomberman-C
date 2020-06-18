
///////////////////////////////////////////////////////////////////////////////
// Headers.

#include <stdint.h>
#include "system.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//#include "sprites_rgb333.h"

///////////////////////////////////////////////////////////////////////////////
// HW stuff.

#define WAIT_UNITL_0(x) while(x != 0){}
#define WAIT_UNITL_1(x) while(x != 1){}

#define SCREEN_W 320
#define SCREEN_H 240
#define SCREEN_RGB333_W 160
#define SCREEN_RGB333_H 120

#define SCREEN_IDX4_W8 (SCREEN_IDX4_W/8)
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
} bf_unpack_idx4;
#define unpack_idx4 (*((volatile bf_unpack_idx4*)unpack_idx4_p32))



///////////////////////////////////////////////////////////////////////////////
// Game config.

#define STEP 5

#define RECT_H 20
#define RECT_W 20
#define SQ_A 20

#define UNPACKED_0_PACKED_1 0



///////////////////////////////////////////////////////////////////////////////
// Game data structures.

typedef struct {
	uint16_t x;
	uint16_t y;
} point_t;

typedef enum {BOMBERMAN} player_t;

typedef struct {
	// Upper left corners.
	point_t bomberman;
    point_t enemy_1;
    point_t enemy_2;

    point_t block[10];
	
	point_t goal;


	player_t active;
} game_state_t;

/*void draw_sprite_from_atlas(
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
			unpack_rgb333_p32[dst_idx] = pixel;
		}
	}
	
	
}*/



///////////////////////////////////////////////////////////////////////////////
// Game code.

int main(void) {
	
	// Setup.
	gpu_p32[0] = 2; // 1b index mode.
	gpu_p32[1] = UNPACKED_0_PACKED_1;
	palette_p32[0] = 0x0000ff00; // Green for background.
	palette_p32[1] = 0x000000ff; // Red for players.
	gpu_p32[0x800] = 0x00ff0000; // Blue for HUD.
    palette_p32[2] = 0xdad5d4;
    palette_p32[3] = 0xff52ba;
    palette_p32[4] = 0x52f6ff;
    palette_p32[5] = 0x5b52ff;


	// Game state.
	game_state_t gs;
	gs.bomberman.x = 0;
	gs.bomberman.y = 0;

    gs.enemy_1.x = 120;
    gs.enemy_1.y = 100;

    gs.enemy_2.x = 160;
    gs.enemy_2.y = 100;

    gs.goal.x = 140;
    gs.goal.y = 120;

    gs.block[0].x = 40;
    gs.block[0].y = 20;

    gs.block[0].x = 40;
    gs.block[0].y = 40;

    gs.block[1].x = 40;
    gs.block[1].y = 60;

    gs.block[2].x = 80;
    gs.block[2].y = 120;

    gs.block[3].x = 80;
    gs.block[3].y = 160;

    gs.block[4].x = 80;
    gs.block[4].y = 20;

    gs.block[5].x = 160;
    gs.block[5].y = 60;

    gs.block[6].x = 140;
    gs.block[6].y = 80;

    gs.block[7].x = 160;
    gs.block[7].y = 40;

    gs.block[8].x = 160;
    gs.block[8].y = 80;

    gs.block[9].x = 180;
    gs.block[9].y = 80;

	
	gs.active = BOMBERMAN;

	
	
	while(1){
		
		
		/////////////////////////////////////
		// Poll controls.
		int mov_x = 0;
		int mov_y = 0;
		if(joypad.down){
			mov_y = +1;
		}
		if(joypad.up){
			mov_y = -1;
		}
		if(joypad.right){
			mov_x = +1;
		}
		if(joypad.left){
			mov_x = -1;
		}
		//TODO Have bug here. Hold right button and play with A button.
		int toggle_active = joypad.a;
		
		
		
		
		
		
		/////////////////////////////////////
		// Gameplay.
        int widthBorder=1,heighthBorder = 1,widthBorderR=1,widthBorderL=1, heighthBorderD = 1,heighthBorderU = 1;
		
		switch(gs.active){
		case BOMBERMAN:

			widthBorderR = (gs.bomberman.x + mov_x*STEP + RECT_W) < SCREEN_W + 5;
            widthBorderL = gs.bomberman.x + mov_x*STEP >-5;
            heighthBorderD = (gs.bomberman.y+ mov_y*STEP + RECT_H) < SCREEN_H + 5;
            heighthBorderU = gs.bomberman.y + mov_y*STEP > -5;

            widthBorder = widthBorderR && widthBorderL;
            heighthBorder = heighthBorderU && heighthBorderD;
            gs.bomberman.x += mov_x*STEP*widthBorder;
            gs.bomberman.y += mov_y*STEP*heighthBorder;
            

            //printf("COORDS: %d, %d\n", gs.bomberman.x, gs.bomberman.y);
			break;
			
		}
		
		
		
		/////////////////////////////////////
		// Drawing.
		
		
		// Detecting rising edge of VSync.
		WAIT_UNITL_0(gpu_p32[2]);
		WAIT_UNITL_1(gpu_p32[2]);
		// Draw in buffer while it is in VSync
		
		
		
		
		
		
#if !UNPACKED_0_PACKED_1
		// Unpacked.
		
		// Clear to green
		for(int r = 0; r < SCREEN_H; r++){
			for(int c = 0; c < SCREEN_W; c++){
				unpack_idx4_p32[r*SCREEN_W + c] = 0;
			}
		}

        for(int j=1;j<12;j+=2)
            for(int i = 1;i<16;i+=2)
                for(int r=j*SQ_A;r<j*SQ_A+RECT_H;r++){
                    for(int c=i*SQ_A;c<i*SQ_A+RECT_W;c++)
                    {
                        unpack_idx4_p32[r*SCREEN_W+c]=2;
                    }
                } 

    


		for(int r = gs.goal.y; r < gs.goal.y+RECT_H; r++){
			for(int c = gs.goal.x; c < gs.goal.x+RECT_W; c++){
				unpack_idx4_p32[r*SCREEN_W + c] = 4;
			}
		}
        for(int i =0;i < 10;i++){
            for(int r = gs.block[i].y; r < gs.block[i].y+RECT_H; r++){
			    for(int c = gs.block[i].x; c < gs.block[i].x+RECT_W; c++){
				    unpack_idx4_p32[r*SCREEN_W + c] = 5;
			    }
            }
        }
		
		// Red rectangle.
		// Use array with 2D indexing.

        //draw_sprite_from_atlas(64,0,16,16,gs.bomberman.x,gs.bomberman.y);
		for(int r = gs.bomberman.y; r < gs.bomberman.y+RECT_H; r++){
			for(int c = gs.bomberman.x; c < gs.bomberman.x+RECT_W; c++){
				unpack_idx4_p32[r*SCREEN_W + c] = 1;
			}
		}

        for(int r = gs.enemy_1.y; r < gs.enemy_1.y+SQ_A; r++){
			    for(int c = gs.enemy_1.x; c < gs.enemy_1.x+SQ_A; c++){
				    unpack_idx4_p32[r*SCREEN_W + c] = 3;
			    }
		}

        for(int r = gs.enemy_2.y; r < gs.enemy_2.y+SQ_A; r++){
			    for(int c = gs.enemy_2.x; c < gs.enemy_2.x+SQ_A; c++){
				    unpack_idx4_p32[r*SCREEN_W + c] = 3;
			    }
		}
		
		
		
		
#else
		// Packed.


		//TODO This is just test. Implement same as for unpacked.
		for(int r = 0; r < 32; r++){
			for(int c = 0; c < 1; c++){
				pack_idx4_p32[r*(SCREEN_W/32) + c] = 0xffffffff;
			}
		}
		
		
#endif
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
