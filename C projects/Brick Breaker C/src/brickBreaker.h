#include "brickBreakerAssets.h"
//sizes
enum {paddleWidth = 76, paddleHeight = 20,
        ballWidth = 10, ballHeight = 10,
        brickWidth = 60, brickHeight = 20,
        level_maxBricks = 100};

//structs
struct point {
    int x, y;
};
struct line {
    struct point p1, p2;
};
struct paddle {
    struct point pos; //top left corner
    int width, height, velX; //vel can be any int value (variable speed, depends on gyroscope)
    int data[paddleHeight][paddleWidth];
};
struct ball {
    struct point pos; //top left corner
    int width, height, velX, velY; //vel can equal -1,0,1 (constant speed)
    int data[ballHeight][ballWidth]; //stores RGB values
};
struct brick {
    struct point pos; //top left corner
    int hits, width, height;
    int normal[brickHeight][brickWidth]; //stores RGB values of normal brick
    int cracked[brickHeight][brickWidth]; //stores RGB values of cracked brick
};
struct levelData {
    int brickArrSize;
    struct brick brickArr[level_maxBricks]; //max 100 for 640x240
    struct paddle pad;
    struct ball b; 
};

//object instantiation
//a stands for asset
//#define ball_20x20 {{0,0}, ballHeight, ballWidth, 0, 0, ball_20x20_rgb58}
#define ball_10x10 {{0, 0}, ballWidth, ballHeight, 0, 0, ball_10x10_rgb58}
#define paddle_50 {{0, 0}, paddleWidth, paddleHeight, 0, paddle_rgb50}

//used to instantiate level structs
#define blue_brick {{0, 0}, 0, brickWidth, brickHeight, blue_brick_rgb1, blue_brick_broken_rgb2}
#define green_brick {{0, 0}, 0, brickWidth, brickHeight, green_brick_rgb3, green_brick_broken_rgb4}
#define purple_brick {{0, 0}, 0, brickWidth, brickHeight, purple_brick_rgb5, purple_brick_broken_rgb6}
#define red_brick {{0, 0}, 0, brickWidth, brickHeight, red_brick_rgb7, red_brick_broken_rgb8}
#define orange_brick {{0, 0}, 0, brickWidth, brickHeight, orange_brick_rgb9, orange_brick_broken_rgb10}
#define light_blue_brick {{0, 0}, 0, brickWidth, brickHeight, light_blue_brick_rgb11, light_blue_brick_broken_rgb12}
#define yellow_brick {{0, 0}, 0, brickWidth, brickHeight, yellow_brick_rgb13, yellow_brick_broken_rgb14}
#define dark_green_brick {{0, 0}, 0, brickWidth, brickHeight, dark_green_brick_rgb15, dark_green_brick_broken_rgb16}
#define gray_brick {{0, 0}, 0, brickWidth, brickHeight, gray_brick_rgb17, gray_brick_broken_rgb18}
#define brown_brick {{0, 0}, 0, brickWidth, brickHeight, brown_brick_rgb19, brown_brick_broken_rgb20}

//used as default bricks
struct brick blue_brick_struct = blue_brick;
struct brick green_brick_struct = green_brick;
struct brick purple_brick_struct = purple_brick;
struct brick red_brick_struct = red_brick;
struct brick orange_brick_struct = orange_brick;
struct brick light_blue_brick_struct = light_blue_brick;
struct brick yellow_brick_struct = yellow_brick;
struct brick dark_green_brick_struct = dark_green_brick;
struct brick gray_brick_struct = gray_brick;
struct brick brown_brick_struct = brown_brick;

struct brick brickList[10] = {blue_brick, green_brick, purple_brick, red_brick, orange_brick, light_blue_brick, yellow_brick,
                                dark_green_brick, gray_brick, brown_brick};