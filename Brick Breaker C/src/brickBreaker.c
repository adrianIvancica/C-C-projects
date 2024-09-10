#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "brickBreaker.h"
#include "ADXL435.c"
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <sys/mman.h> 
#include "address_map_arm_brl4.h"
// resolution register
#define resOffset 0x00003028
#define statusOffset 0x0000302c
//TODO: double check width and height
#define W_WIDTH 640 //window width
#define W_HEIGHT 240 //window height

#define SWAP(X, Y) do {int temp = X; X = Y; Y = temp;} while (0)

// virtual to real address pointers
volatile unsigned int *key_reg_ptr = NULL;
volatile unsigned int *res_reg_ptr = NULL;
volatile unsigned int *stat_reg_ptr = NULL;
void *h2p_lw_virtual_base;

volatile unsigned int *vga_pixel_buf_ptr = NULL;
void *vga_pixel_virtual_base;

volatile unsigned int *vga_char_ptr = NULL;
void *vga_char_virtual_base;

int fd;

//colors
//rgb format (8 bits): R(3bits) G(3bits) B(2bits)
#define BLACK 0x0
#define WHITE 0xFF

//defining game states
enum state{menu, game, gameOver, levelCompleted, emptyState};
enum menuOptions{mainMenu, levelMenu, gameOverMenu, pauseMenu, levelCompletedMenu, emptyMenu};
enum level{one, two, three};

enum state STATE = menu;
enum menuOptions MENU_OPTIONS = mainMenu;
enum level LEVEL = one;

int paused = 0;

/* function prototypes */
void VGA_setpixel(int, int, int);
void VGA_text_line(int, int, char *);
void VGA_rectangle(int, int, int, int, int);
void VGA_solid_rectangle(int, int, int, int, int);
void VGA_line(int, int, int, int, int);
void VGA_circle(int, int, int, int);
void VGA_solid_circle(int, int, int, int);

void drawMenu(enum menuOptions, enum state, enum state, enum menuOptions, enum menuOptions);
void drawBall(struct ball);
void drawPaddle(struct paddle);
void drawBrick(struct brick);

int isButtonSelected(int);
void collisionDetection(struct levelData*);

void initLevel(struct levelData*);
void updatePaddle(struct paddle*);
void updateBall(struct ball*);
void updateLevel(struct levelData*);

void makeRandomLevel(struct levelData*, int);
int putBrick(struct levelData*, struct brick, int ,int);

//gyroscope vars
uint8_t devid;
int16_t mg_per_lsb = 4;
int16_t XYZ[3];

int main(void)
{
	// Declare volatile pointers to I/O registers 
    // (volatile means that IO load and store instructions will be used to access these pointer locations, instead of regular memory loads and stores)

	// === get FPGA addresses ==================
	// Open /dev/mem
	if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1)
	{
		printf("ERROR: could not open \"/dev/mem\"...\n");
		return (1);
	}
    
	// get virtual addr that maps to physical
    // 0 to HW_REGS_BASE - 1 = memory
    // HW_REGS_BASE = 0xff200000, start of peripherals
    // HW_REGS_SPAN = 0x00005000, not sure why its this size but its large enough to map all peripherals except HPS I2C0
	h2p_lw_virtual_base = mmap(NULL, HW_REGS_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, HW_REGS_BASE);
	if (h2p_lw_virtual_base == MAP_FAILED)
	{
		printf("ERROR: mmap1() failed...\n");
		close(fd);
		return (1);
	}

    // address to resolution register
    // relative to base 0xff200000, resOffset = 0x00000050
    key_reg_ptr = (unsigned int *)(h2p_lw_virtual_base + KEY_BASE);

	// address to resolution register
    // relative to base 0xff200000, resOffset = 0x00003028
	res_reg_ptr = (unsigned int *)(h2p_lw_virtual_base + resOffset);

	// address to vga status
    // relative to base 0xff200000, statusOffset = 0x0000302c
	stat_reg_ptr = (unsigned int *)(h2p_lw_virtual_base + statusOffset);

	// === get VGA char addr =====================
	// get virtual addr that maps to physical
    // FPGA_CHAR_BASE = 0xC9000000
    // FPGA_CHAR_SPAN = 0x00002000
	vga_char_virtual_base = mmap(NULL, FPGA_CHAR_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, FPGA_CHAR_BASE);
	if (vga_char_virtual_base == MAP_FAILED)
	{
		printf("ERROR: mmap2() failed...\n");
		close(fd);
		return (1);
	}

	// Get the address that maps to the FPGA On-chip Memory Character Buffer
    // starts at 0xC9000000
	vga_char_ptr = (unsigned int *)(vga_char_virtual_base);
	// === get VGA pixel addr ====================
	// get virtual addr that maps to physical
    // FPGA_ONCHIP_BASE = 0xC8000000
    // FPGA_ONCHIP_SPAN = 0x00080000
	vga_pixel_virtual_base = mmap(NULL, FPGA_ONCHIP_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, FPGA_ONCHIP_BASE);
	if (vga_pixel_virtual_base == MAP_FAILED)
	{
		printf("ERROR: mmap3() failed...\n");
		close(fd);
		return (1);
	}

	// Get the address that maps to the FPGA LED control
    // starts at 0xC8000000
	vga_pixel_buf_ptr = (unsigned int *)(vga_pixel_virtual_base);

	// === get gyroscope info ====================
    // Map physical addresses
    Map_Physical_Addrs();

    // Configure Pin Muxing
    Pinmux_Config();

    // Initialize I2C0 Controller
    I2C0_Init();

    // 0xE5 is read from DEVID(0x00) if I2C is functioning correctly
    ADXL345_REG_READ(0x00, &devid);

    // Correct Device ID
    if (devid == 0xE5){
        printf("Device ID Verified\n");
        // Initialize accelerometer chip
        ADXL345_Init();
        printf("ADXL345 Initialized\n");
    }
    else {
        printf("Incorrect device ID\n");
    }

	// get the y and x resolution
	// y in top 16, x in bottom 16 bits
	printf("%0x\n", (int)*res_reg_ptr);
	// m bits y is top8 -- n bits x is next8
	printf("%0x\n", (int)*stat_reg_ptr);

    int stat_width = *res_reg_ptr & 0x0000FFFF;
    int stat_height = (*res_reg_ptr & 0xFFFF0000) >> 16;

    struct levelData level1 = {0, {}, paddle_50, ball_10x10};
    makeRandomLevel(&level1, 10);
    struct levelData level2 = {0, {}, paddle_50, ball_10x10};
    makeRandomLevel(&level2, 20);
    struct levelData level3 = {0, {}, paddle_50, ball_10x10};
    makeRandomLevel(&level3, 30);

    enum state prevState = emptyState; //previous state
    enum state curState = menu; // current state
    enum menuOptions prevOption = emptyMenu; //previous menu option
    enum menuOptions curOption = mainMenu; //current menu option

    //TODO: 
    // when a level is completed, make the same level repeatable witin game loop
    // set designated levels as read only
    // make initLevel take a copy of a designated level and return a pointer to a new level
    // new level will be modified during game loop and destroyed once the player wins or loses
	while (1)
	{
		if (STATE == menu && MENU_OPTIONS == mainMenu){
            //printf("Main Menu\n");
            curState = menu;
            curOption = mainMenu;
            if (curState != prevState || curOption != prevOption){
                printf("Draw main Menu\n");
                drawMenu(mainMenu, curState, prevState, curOption, prevOption);
            }

            //play (KEY3)
            if(isButtonSelected(3)) { 
                printf("B3: Level menu option\n");
                MENU_OPTIONS = levelMenu;
            }
            //quit (KEY2)
            else if(isButtonSelected(2)) {
                printf("B2: quit\n");
                char erase[80] = "                                                                                \0";
                VGA_text_line(0,0, erase);
                VGA_text_line(0,1, erase);
                VGA_text_line(0,2, erase);
                VGA_text_line(0,3, erase);
                VGA_text_line(0,4, erase);
                break;
            }
        }
        else if (STATE == menu && MENU_OPTIONS == levelMenu){
            //printf("Level Menu\n");
            curState = menu;
            curOption = levelMenu;
            if (curOption != prevOption){
                printf("Draw level menu\n");
                drawMenu(levelMenu, curState, prevState, curOption, prevOption);
            }

            //Level 1 (KEY3)
            if(isButtonSelected(3)) { 
                printf("B3: Level 1\n");
                STATE = game;
                LEVEL = one;
            }
            //Level 2 (KEY2)
            else if((isButtonSelected(2))) { 
                printf("B2: Level 2\n");
                STATE = game;
                LEVEL = two;
            }
            //Level 3 (KEY1)
            else if(isButtonSelected(1)) { 
                printf("B1: Level 3\n");
                STATE = game;
                LEVEL = three;
            } 
            //Main Menu (KEY0)
            else if(isButtonSelected(0)) { 
                printf("B0: Main menu\n");
                MENU_OPTIONS = mainMenu;
            } 
        }
        else if (STATE == game){
            if (paused == 1){
                //printf("Pause menu\n");
                curOption = pauseMenu;
                if (curOption != prevOption){
                    printf("Draw pause menu\n");
                    drawMenu(pauseMenu, curState, prevState, curOption, prevOption);
                }
                    
                //resume (KEY3)
                if(isButtonSelected(3)) { 
                    printf("B3: resume\n");
                    paused = 0;
                }
                //main menu (KEY2)
                else if(isButtonSelected(2)) { 
                    printf("B2: main menu\n");
                    STATE = menu;
                    MENU_OPTIONS = mainMenu;
                    curOption = mainMenu;
                }
            }
            curState = game;
            while (!paused && !(STATE == gameOver) && !(STATE == levelCompleted)){
                //pause (KEY0)
                if (curState != prevState){
                    printf("Erase prev menu\n");
                    drawMenu(emptyMenu, curState, prevState, curOption, prevOption);
                }

                if(isButtonSelected(0)) {
                    printf("B0: pause\n");
                    paused = 1;
                } 

                if (LEVEL == one){
                    if (curState != prevState)
                        initLevel(&level1);
                    updateLevel(&level1);
                    usleep(10000);
                }
                else if (LEVEL == two){
                    if (curState != prevState)
                        initLevel(&level2);
                    updateLevel(&level2);
                    usleep(9000);
                }
                else if (LEVEL == three){
                    if (curState != prevState)
                        initLevel(&level3);
                    updateLevel(&level3);
                    usleep(8000);
                }
                prevState = curState;
                
            }            
        }
        else if (STATE == gameOver){
            //printf("Game Over\n");
            curState = gameOver;
            if (curState != prevState){
                printf("Draw game over menu\n");
                drawMenu(gameOverMenu, curState, prevState, curOption, prevOption);
            }
                
            //main menu (KEY3)
            else if(isButtonSelected(3)) { 
                printf("B3: Main menu\n");
                STATE = menu;
                MENU_OPTIONS = mainMenu;
            }
        }
        else if (STATE == levelCompleted){
            curState = levelCompleted;
            if(curState != prevState){
                printf("Draw level completed menu\n");
                drawMenu(levelCompletedMenu, curState, prevState, curOption, prevOption);
            }
                
            //main menu (KEY3)
            if(isButtonSelected(3)) { 
                printf("B3: Main Menu\n");
                STATE = menu;
                MENU_OPTIONS = mainMenu;
            }
        }
        prevState = curState;
        prevOption = curOption;
	}
    
   Close_Device();
}

/****************************************************************************************
 * VGA graphics
 ****************************************************************************************/
void VGA_text_line(int x, int y, char *text_ptr) 
{   
    /*
    the character buffer provides a resolution of 80x60 characters where each character occupies an 8x8 block of pixels on the screen
    x range: 0 to 79, y range: 0 to 59, base = 0xC9000000
    top left located at 0,0. Bottom right at 79,59
    character at location 0,0: address = 0xC9000000
    character at location 1,0: address = base + 0b(000000 0000001) = 0xC9000001
    character at location 0,1: address = base + 0b(000001 0000000) = 0xC9000080
    character at location 79,59: address = base + 0b(111011 1001111) = 0xC9001DCF
    */

	int offset;
	volatile char *character_buffer = (char *)vga_char_ptr; // VGA character buffer

	offset = (y << 7) + x;
	while (*(text_ptr)) 
	{
        //stop writing if text string doesn't fit on the line
        if ((offset & 0x7F) > 79){
            break;
        }
		// write to the character buffer
		*(character_buffer + offset) = *(text_ptr);
		++text_ptr;
		++offset;
	}
}

void VGA_setpixel(int x, int y, int color){
    /*
    coordinate 0,0 = top left cornder. 639,479 = bottom right
    pixels are addressed in the pixel bufer by using the combination of a base and x,y offset
    base = 0xC8000000
    for 320x240: x = 9 bits, y= 8bits
    pixel at location 0,0: address = 0xC8000000
    pixel at location 1,0: address = base + (00000000 000000001) = 
    pixel at location 0,1: address = base + (00000001 000000000) = 
    pixel at location 319,239: address = base + (111011111 1001111111) = 
    */

    if (x >= 0 && x < W_WIDTH && y >= 0 && y < W_HEIGHT){
        int *pixel_ptr;
        //320x240
        pixel_ptr = (char *)vga_pixel_buf_ptr + (y << 10) + x;
        // set pixel color
        *(char *)pixel_ptr = color;
    }
}

//draw a line
//Bresenham's Line Drawing Algorithm
void VGA_line(int x0, int y0, int x1, int y1, int color)
{
  int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1; 
  int err = dx + dy, e2; /* error value e_xy */
 
  for (;;){  /* loop */
    VGA_setpixel(x0,y0,color);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
    if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
  }
}

//draw rectangle
//(x1,y1) = top left corner, (x2,y2) = bottom right corner
void VGA_rectangle(int x1, int y1, int x2, int y2, int color){
    VGA_line(x1, y1, x2, y1, color);
    VGA_line(x1, y2, x2, y2, color);
    VGA_line(x1, y1, x1, y2, color);
    VGA_line(x2, y1, x2, y2, color);
}

//draw solid rectangle
//(x1,y1) = top left corner, (x2,y2) = bottom right corner
void VGA_solid_rectangle(int x1, int y1, int x2, int y2, int color)
{
    int col;
    for (col = y1; col <= y2; ++col){
        VGA_line(x1, col, x2, col, color);
    }
}

//draw a raster circle
void VGA_circle(int x0, int y0, int radius, int color)
{
  int f = 1 - radius;
  int ddF_x = 1;
  int ddF_y = -2 * radius;
  int x = 0;
  int y = radius;
 
  VGA_setpixel(x0, y0 + radius, color);
  VGA_setpixel(x0, y0 - radius, color);
  VGA_setpixel(x0 + radius, y0, color);
  VGA_setpixel(x0 - radius, y0, color);
  while (x < y)
  {
    // ddF_x == 2 * x + 1;
    // ddF_y == -2 * y;
    // f == x*x + y*y - radius*radius + 2*x - y + 1;
    if (f >= 0) 
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;    
    VGA_setpixel(x0 + x, y0 + y, color);
    VGA_setpixel(x0 - x, y0 + y, color);
    VGA_setpixel(x0 + x, y0 - y, color);
    VGA_setpixel(x0 - x, y0 - y, color);
    VGA_setpixel(x0 + y, y0 + x, color);
    VGA_setpixel(x0 - y, y0 + x, color);
    VGA_setpixel(x0 + y, y0 - x, color);
    VGA_setpixel(x0 - y, y0 - x, color);
  }
}

//draw a solid raster circle
void VGA_solid_circle(int x0, int y0, int radius, int color)
{
  int f = 1 - radius;
  int ddF_x = 1;
  int ddF_y = -2 * radius;
  int x = 0;
  int y = radius;
 
  VGA_setpixel(x0, y0 + radius, color);
  VGA_setpixel(x0, y0 - radius, color);
  VGA_line(x0 + radius, y0, x0 - radius, y0, color);
  while (x < y)
  {
    // ddF_x == 2 * x + 1;
    // ddF_y == -2 * y;
    // f == x*x + y*y - radius*radius + 2*x - y + 1;
    if (f >= 0) 
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;    
    VGA_line(x0 + x, y0 + y, x0 - x, y0 + y, color);
    VGA_line(x0 + x, y0 - y, x0 - x, y0 - y, color);
    VGA_line(x0 + y, y0 + x, x0 - y, y0 + x, color);
    VGA_line(x0 + y, y0 - x, x0 - y, y0 - x, color);
  }
}

void drawBall(struct ball b){
    int i, j;
    for (i = 0; i < b.width; i++){
        for (j = 0; j < b.height; j++){
            VGA_setpixel(b.pos.x + i,b.pos.y + j,b.data[j][i]);
        }
    }
}

void drawPaddle(struct paddle pad){
    int i, j;
    for (i = 0; i < pad.width; i++){
        for (j = 0; j < pad.height; j++){
            VGA_setpixel(pad.pos.x + i,pad.pos.y + j,pad.data[j][i]);
        }
    }
}

void drawBrick(struct brick br){
    
    int i, j;
    for (i = 0; i < br.width; i++){
        for (j = 0; j < br.height; j++){
            if (br.hits == 0){
                VGA_setpixel(br.pos.x + i,br.pos.y + j, br.normal[j][i]);
            }
            if (br.hits == 1){
                VGA_setpixel(br.pos.x + i,br.pos.y + j, br.cracked[j][i]);
            }
            if (br.hits >= 2)   {
                VGA_setpixel(br.pos.x + i,br.pos.y + j, BLACK);
            }
        }
    }
}

void drawMenu(enum menuOptions menuType, enum state cs, enum state ps, enum menuOptions mcs, enum menuOptions mps){ // cs = current state,  ps = previous state, m = menu
	char title[80] = "BRICK BREAKER\0";
    char play[80] = "1. Play\0";
    char quit[80] = "2. Quit\0";
    char selectLevel[80] = "SELECT LEVEL\0";
    char level1[80] = "1. Level 1\0";
    char level2[80] = "2. Level 2\0";
    char level3[80] = "3. Level 3\0";
    char backToMainMenu1[80] = "1. Main Menu\0";
    char backToMainMenu2[80] = "2. Main Menu\0";
    char backToMainMenu4[80] = "4. Main Menu\0";
    char paused[80] = "PAUSED\0";
    char resume[80] = "1. Resume\0";
    char gameOver[80] = "GAME OVER\0";
    char youWin[80] = "YOU WIN!\0";
    char erase[80] = "                                                                                \0";

    if (cs != ps || mcs != mps){
        VGA_text_line(0,0, erase);
        VGA_text_line(0,1, erase);
        VGA_text_line(0,2, erase);
        VGA_text_line(0,3, erase);
        VGA_text_line(0,4, erase);
    }
    if (menuType == mainMenu){
        VGA_text_line(0, 0, title);
        VGA_text_line(0, 1, play);
        VGA_text_line(0, 2, quit);
    }
    else if (menuType == levelMenu){
        VGA_text_line(0, 0, selectLevel);
        VGA_text_line(0, 1, level1);
        VGA_text_line(0, 2, level2);
        VGA_text_line(0, 3, level3);
        VGA_text_line(0, 4, backToMainMenu4);
    }
    else if (menuType == gameOverMenu){
        VGA_solid_rectangle(0,0,W_WIDTH -1, W_HEIGHT -1, BLACK);
        VGA_text_line(0, 0, gameOver);
        VGA_text_line(0, 1, backToMainMenu1);
    }
    else if (menuType == pauseMenu){
        VGA_solid_rectangle(0,0,W_WIDTH -1, W_HEIGHT -1, BLACK);
        VGA_text_line(0, 0, paused);
        VGA_text_line(0, 1, resume);
        VGA_text_line(0, 2, backToMainMenu2);
    }
    else if (menuType == levelCompletedMenu){
        VGA_solid_rectangle(0,0,W_WIDTH -1, W_HEIGHT -1, BLACK);
        VGA_text_line(0, 0, youWin);
        VGA_text_line(0, 1, backToMainMenu1);
    }
}

/****************************************************************************************
 * brick breaker logic
 ****************************************************************************************/
int isButtonSelected(int keyNumber){
    if (keyNumber < 0 || keyNumber > 3){
        printf("Enter a button number between 0 and 3");
        return 0;
    }

    while((*key_reg_ptr & 0x8) && keyNumber == 3){
        //printf("Key 3: %d", *key_reg_ptr & 0x8);
        if (!(*key_reg_ptr & 0x8))
            return 1;
    }
    while((*key_reg_ptr & 0x4) && keyNumber == 2){
        //printf("Key 2: %d", *key_reg_ptr & 0x4);
        if (!(*key_reg_ptr & 0x4))
            return 1;
    }
    while((*key_reg_ptr & 0x2) && keyNumber == 1){
        //printf("Key 1: %d", *key_reg_ptr & 0x2);
        if (!(*key_reg_ptr & 0x2))
            return 1;
    }
    while((*key_reg_ptr & 0x1) && keyNumber == 0){
        //printf("Key 0: %d", *key_reg_ptr & 0x1);
        if (!(*key_reg_ptr & 0x1))
            return 1;
    }

    return 0;
}

void collisionDetection(struct levelData *ld){

    struct point bottomLeftBall = {ld->b.pos.x, ld->b.pos.y + ld->b.height - 1};
    struct point bottomRightBall = {ld->b.pos.x + ld->b.width - 1, ld->b.pos.y + ld->b.height - 1};
    struct point topLeftBall = {ld->b.pos.x, ld->b.pos.y};
    struct point topRightBall = {ld->b.pos.x + ld->b.width - 1, ld->b.pos.y};

    struct point topLeftPaddle = {ld->pad.pos.x, ld->pad.pos.y};
    struct point firstThirdsPaddle = {ld->pad.pos.x + ld->pad.width / 3 - 1, ld->pad.pos.y};
    struct point secondThirdsPaddle = {ld->pad.pos.x + (ld->pad.width / 3) * 2 - 1, ld->pad.pos.y};
    struct point topRightPaddle = {ld->pad.pos.x + ld->pad.width - 1, ld->pad.pos.y};

    //Ball can never be larger than on chunk of the paddle
    //Left chunk of paddle
    if(((firstThirdsPaddle.x >= bottomLeftBall.x && bottomLeftBall.x >= topLeftPaddle.x) || (firstThirdsPaddle.x >= bottomRightBall.x && bottomRightBall.x >= topLeftPaddle.x)) && bottomLeftBall.y == topLeftPaddle.y) {
        ld->b.velX = -1;
        ld->b.velY *= -1;
    }
    //Right chunk of paddle
    else if(((topRightPaddle.x >= bottomLeftBall.x && bottomLeftBall.x >= secondThirdsPaddle.x) || (topRightPaddle.x >= bottomRightBall.x && bottomRightBall.x >= secondThirdsPaddle.x)) && bottomLeftBall.y == topLeftPaddle.y) {
        ld->b.velX = 1;
        ld->b.velY *= -1;
    }
    //Middle chunk of paddle
    else if(((secondThirdsPaddle.x >= bottomLeftBall.x && bottomLeftBall.x >= firstThirdsPaddle.x) || (secondThirdsPaddle.x >= bottomRightBall.x && bottomRightBall.x >= firstThirdsPaddle.x)) && bottomLeftBall.y == topLeftPaddle.y) {
        ld->b.velX = 0;
        ld->b.velY *= -1;
    }

    int i;
    for (i = 0; i < ld->brickArrSize; i++) {
        if (ld->brickArr[i].hits < 2) {
            struct point bottomLeftBrick = {ld->brickArr[i].pos.x, ld->brickArr[i].pos.y + ld->brickArr[i].height - 1};
            struct point bottomRightBrick = {ld->brickArr[i].pos.x + ld->brickArr[i].width - 1, ld->brickArr[i].pos.y + ld->brickArr[i].height - 1};
            struct point topLeftBrick = {ld->brickArr[i].pos.x, ld->brickArr[i].pos.y};
            struct point topRightBrick = {ld->brickArr[i].pos.x + ld->brickArr[i].width - 1, ld->brickArr[i].pos.y};

            //Left Side Brick Right Side Ball
            if (((topLeftBrick.y <= topRightBall.y && topRightBall.y <= bottomLeftBrick.y) || (topLeftBrick.y <= bottomRightBall.y && bottomRightBall.y <= bottomLeftBrick.y)) && (bottomLeftBrick.x == bottomRightBall.x)) {
                ld->brickArr[i].hits += 1;
                drawBrick(ld->brickArr[i]);
                ld->b.velX *= -1;
                break;
            }
            //Right Side Brick Left Side Ball
            else if(((topRightBrick.y <= topLeftBall.y && topLeftBall.y <= bottomRightBrick.y) || (topRightBrick.y <= bottomLeftBall.y && bottomLeftBall.y <= bottomRightBrick.y)) && (bottomRightBrick.x == bottomLeftBall.x)) {
                ld->brickArr[i].hits += 1;
                drawBrick(ld->brickArr[i]);
                ld->b.velX *= -1;
                break;
            }
            //Bottom Brick Top Ball
            else if (((bottomRightBrick.x >= topLeftBall.x && topLeftBall.x >= bottomLeftBrick.x) || (bottomRightBrick.x >= topRightBall.x && topRightBall.x >= bottomLeftBrick.x)) && (topLeftBall.y == bottomLeftBrick.y)) {
                ld->brickArr[i].hits += 1;
                drawBrick(ld->brickArr[i]);
                ld->b.velY *= -1;
                break;
            }
            //Top Brick Bottom Ball
            else if (((topRightBrick.x >= bottomLeftBall.x && bottomLeftBall.x >= topLeftBrick.x) || (topRightBrick.x >= bottomRightBall.x && bottomRightBall.x >= topLeftBrick.x)) && (bottomLeftBall.y == topLeftBrick.y)) {
                ld->brickArr[i].hits += 1;
                drawBrick(ld->brickArr[i]);
                ld->b.velY *= -1;
                break;
            }
        }
    }

    //Ball hits bottom of screen 
    if (ld->b.velY > 0 && ((ld->b.pos.y) + (ld->b.height-1) >= W_HEIGHT - 1)) {
        //ld->b.velY *= -1; // reverses y velocity
        STATE = gameOver;
    }
    //Ball hits top of screen and reverses y velocity
    if (ld->b.velY < 0 && ld->b.pos.y <= 0) {
        ld->b.velY *= -1;
    }
    //Ball hits left side of screen and reverses x velocity
    if (ld->b.velX < 0 && ld->b.pos.x <= 0) {
        ld->b.velX *= -1;
    }
    //Ball hits right side of screen and reverses x velocity
    if (ld->b.velX > 0 && ld->b.pos.x+ld->b.width-1 >= W_WIDTH  - 1) {
        ld->b.velX *= -1;
    }

    int completed = 1;
    for (i = 0; i < ld->brickArrSize; i++){
        if (ld->brickArr[i].hits != 2){
            completed = 0;
            break;
        }
    }
    if (completed)
        STATE = levelCompleted;
}

void initLevel(struct levelData * ld){
    VGA_solid_rectangle(0,0,W_WIDTH-1,W_HEIGHT-1, BLACK);
    int i;
    for(i = 0; i < ld->brickArrSize; i++) {
        drawBrick(ld->brickArr[i]);
    }

    ld->b.pos.x = 319 + (ld->pad.width / 2 - 1) - (ld->b.width / 2 - 1);
    ld->b.pos.y = 204;
    ld->b.velX = 0;
    ld->b.velY = -1;
    ld->pad.pos.x = 319;
    ld->pad.pos.y = 219;
    drawBall(ld->b);
    drawPaddle(ld->pad);
}

/*NOTE: impelement game over detection when ball hits bottom of screen*/

void updateBall(struct ball *b){

    int prevX = b->pos.x;
    int prevY = b->pos.y;

    b->pos.x += b->velX;
    b->pos.y += b->velY;

    if (b -> velX == 0 && b -> velY == 0){

    }
    else if (b -> velX > 0 && b -> velY > 0) { 
        VGA_line(prevX, prevY, prevX + b->width - 1, prevY, BLACK); //black out top
        VGA_line(prevX, prevY, prevX, prevY + b->height - 1, BLACK); //black out left
    } 
    else if (b -> velX > 0 && b -> velY == 0) {
        VGA_line(prevX, prevY, prevX, prevY + b->height - 1, BLACK); //black out left
    } 
    else if (b -> velX > 0 && b -> velY < 0) {
        VGA_line(prevX, prevY, prevX, prevY + b->height - 1, BLACK); //black out left
        VGA_line(prevX, prevY + b->height -1, prevX + b->width - 1, prevY + b->height -1, BLACK); //black out bottom
    } 
    else if (b -> velX == 0 && b -> velY > 0) {
        VGA_line(prevX, prevY, prevX + b->width - 1, prevY, BLACK); //black out top
    } 
    else if (b -> velX == 0 && b -> velY < 0) {
        VGA_line(prevX, prevY + b->height -1, prevX + b->width - 1, prevY + b->height -1, BLACK); //black out bottom
    }
    else if (b -> velX < 0 && b -> velY > 0) {
        VGA_line(prevX, prevY, prevX + b->width - 1, prevY, BLACK); //black out top
        VGA_line(prevX + b->width -1, prevY, prevX + b->width - 1, prevY + b->height -1, BLACK); //black out right
    } 
    else if (b -> velX < 0 && b -> velY == 0) {
        VGA_line(prevX + b->width -1, prevY, prevX + b->width - 1, prevY + b->height -1, BLACK); //black out right
    } 
    else if (b -> velX < 0 && b -> velY < 0) {
        VGA_line(prevX, prevY + b->height -1, prevX + b->width - 1, prevY + b->height -1, BLACK); //black out bottom
        VGA_line(prevX + b->width -1, prevY, prevX + b->width - 1, prevY + b->height -1, BLACK); //black out right
    }
    
    drawBall(*b);
}

//TODO: write gyro measurements to terminal and calibrate paddle sensitivity
void updatePaddle(struct paddle *pad){
    int xmg;
    int prevX = pad->pos.x;
    
    if (ADXL345_IsDataReady()){
        ADXL345_XYZ_Read(XYZ);
    }
    // Limit to +/- 1000mg, which means +/-250 in XYZ
    //x = (XYZ[0] + 250)/25;
    
    //pad sensitivity
    xmg = XYZ[0] * mg_per_lsb;
    if (xmg >= -50 && xmg <= 50){
        pad->velX = 0;
    }
    else if (xmg > 50 && xmg <= 250){
        pad->velX = 1;
    }
    else if (xmg > 250 && xmg <= 500){
        pad->velX = 2;
    }
    else if (xmg > 500 && xmg <= 750){
        pad->velX = 3;
    }
    else if ((xmg > 750 && xmg <= 1000) || xmg > 1000){
        pad->velX = 4;
    }
    else if (xmg < -50 && xmg >= -250){
        pad->velX = -1;
    }
    else if (xmg < -250 && xmg >= -500){
        pad->velX = -2;
    }
    else if (xmg < -500 && xmg >= -750){
        pad->velX = -3;
    }
    else if ((xmg < -750 && xmg >= -1000) || xmg < -1000){
        pad->velX = -4;
    }

    //stop at left edge of screen
    if (pad->velX < 0 && pad->pos.x <= 0) {
        pad->velX = 0;
    }
    //stop at right edge of screen
    else if (pad-> velX > 0 && pad->pos.x+pad->width-1 >= W_WIDTH - 1) {
        pad->velX = 0;
    }

    pad->pos.x += pad->velX;

    //erase previous pad if moved right
    if (pad->velX > 0){
        VGA_solid_rectangle(prevX, pad->pos.y, prevX + pad->velX - 1, pad->pos.y + pad->height - 1, BLACK); //black out left
    } 
    //erase previous pad if moved left
    if (pad->velX < 0){
        VGA_solid_rectangle(prevX + pad->width + pad->velX, pad->pos.y, prevX + pad->width - 1, pad->pos.y + pad->height - 1, BLACK); //black out right
    }
    
    drawPaddle(*pad);
}

void updateLevel(struct levelData* ld){
    collisionDetection(ld);
    updateBall(&ld->b);
    updatePaddle(&ld->pad);
}

int putBrick(struct levelData* ld, struct brick brick, int x, int y){
    if(ld->brickArrSize > 100) {
        printf("Cannot add more than 100 bricks\n");
        return -1;
    }
    int i;
    for (i = 0; i < ld->brickArrSize; i++) {
        if(ld->brickArr[i].pos.x == x*60+20 && ld->brickArr[i].pos.y == y*20) {
            //printf("Brick already exists at this location (%d,%d)\n", x, y);
            return -2;
        }
    }

    ld->brickArrSize += 1;
    brick.pos.x = x * 60 + 20;
    brick.pos.y = y * 20;
    ld->brickArr[ld->brickArrSize - 1] = brick;
    //printf("brick %d (%d,%d)\n", ld->brickArrSize -1, x, y);
    return 1;
}

void makeRandomLevel(struct levelData* ld, int size){
    int i;
    if (size > 100){
        printf("Size cannot exceed 100\n");
        return;
    }
    for (i=0; i < size; i++){
        int x = rand() % 10;
        int y = rand() % 3 + 1;

        struct brick br = brickList[rand() % 10];
        int n = putBrick(ld, br, x, y);
        if (n == -2)
            i--;
    }
}