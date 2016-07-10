// Arduino + LCD version of matrixman
// based on original by Mike Szczys
// http://hackaday.com/2015/06/01/1-pixel-pacman/
// high score logic and porting to touchscreen LCD by @arduinoenigma

#include <SPI.h>
#include <SeeedTouchScreen.h>

// the following defines allow FFastTftILI9341.h to control a Seeed or an Adafruit screen.
#define TFTSeed
//#define TFTAdafruit

//to compare performance using Xark vs. Seed drivers, uncomment one block below

//uncomment the following three lines to run with Xark drivers
#define pixelsize 3
#include <FastTftILI9341.h>
PDQ_ILI9341 Tft;

//uncomment the following two lines to run on Seeed drivers
//#define pixelsize 4
//#include <TFTv2.h>

#include <EEPROM.h>
#include <avr/eeprom.h>

//change Player *pawn to -> struct PlayerTAG *pawn
//void flashEnemy(struct PlayerTAG *pawn, uint8_t color)

typedef struct PlayerTAG {
  uint8_t id;         //Index used to find stored values
  uint8_t x;          //Position on the game surface, 0 is left
  uint8_t y;          //Position on the game surface, 0 is top
  uint8_t tarX;       //Target X coord. for enemy
  uint8_t tarY;       //Target Y coord. for enemy
  int16_t speed;      //Countdown how freqeuntly to move
  uint8_t travelDir;  //Uses directional defines below
  uint8_t color;      //Uses color defines below
  uint8_t inPlay;     //On the hunt = TRUE, in reserve = FALSE
  uint8_t dotCount;   /*For player tracks level completion
                          For enemy decides when to go inPlay*/
  uint8_t dotLimit;   //How many dots before this enemy is inPlay
  uint8_t speedMode;  //Index used to look up player speed
}
Player;

//Color definitions
#define BLUE    0
#define YELLOW  1
#define RED     2
#define PINK    3
#define ORANGE  4
#define CYAN    5
#define BLACK   6
#define GREY    7
#define WHITE   8
#define LAVENDAR  9
#define GREEN   10

//Color values
static const uint8_t colors[][3] = {
  { 0, 0, 255 },      //Blue
  { 255, 255, 0 },    //Yellow
  { 255, 0, 0 },      //Red
  { 255, 153, 204 },  //Pink
  { 255, 102, 0 },    //Orange
  { 0, 255, 255 },    //Cyan
  { 0, 0, 0 },        //Black
  { 120, 120, 120 },     //Grey
  { 255, 255, 255 },  //White
  { 196, 64, 255},    //Lavendar
  { 0, 255, 0}        //Green
};

uint16_t tftcolors[11];

//Directions of travel
#define UP      0
#define DOWN    1
#define LEFT    2
#define RIGHT   3
//Miscellaneous
#define ESCAPE  4
#define NOINPUT 5
#define BUTTON  6

uint32_t const board[36] = {
  0b00000000000000000000000000000000,
  0b00000000000000000000000000000000,
  0b00000000000000000000000000000000,
  0b00111111111111111111111111111100,
  0b00100000000000011000000000000100,
  0b00101111011111011011111011110100,
  0b00101111011111011011111011110100,
  0b00101111011111011011111011110100,
  0b00100000000000000000000000000100,
  0b00101111011011111111011011110100,
  0b00101111011011111111011011110100,
  0b00100000011000011000011000000100,
  0b00111111011111011011111011111100,
  0b00000001011111011011111010000000,
  0b00000001011000000000011010000000,
  0b00000001011011111111011010000000,
  0b00111111011010000001011011111100,
  0b00000000000010000001000000000000,
  0b00111111011010000001011011111100,
  0b00000001011011111111011010000000,
  0b00000001011000000000011010000000,
  0b00000001011011111111011010000000,
  0b00111111011011111111011011111100,
  0b00100000000000011000000000000100,
  0b00101111011111011011111011110100,
  0b00101111011111011011111011110100,
  0b00100011000000000000000011000100,
  0b00111011011011111111011011011100,
  0b00111011011011111111011011011100,
  0b00100000011000011000011000000100,
  0b00101111111111011011111111110100,
  0b00101111111111011011111111110100,
  0b00100000000000000000000000000100,
  0b00111111111111111111111111111100,
  0b00000000000000000000000000000000,
  0b00000000000000000000000000000000
};

uint32_t const dots[36] = {
  0b00000000000000000000000000000000,
  0b00000000000000000000000000000000,
  0b00000000000000000000000000000000,
  0b00000000000000000000000000000000,
  0b00011111111111100111111111111000,
  0b00010000100000100100000100001000,
  0b00010000100000100100000100001000,
  0b00010000100000100100000100001000,
  0b00011111111111111111111111111000,
  0b00010000100100000000100100001000,
  0b00010000100100000000100100001000,
  0b00011111100111100111100111111000,
  0b00000000100000000000000100000000,
  0b00000000100000000000000100000000,
  0b00000000100000000000000100000000,
  0b00000000100000000000000100000000,
  0b00000000100000000000000100000000,
  0b00000000100000000000000100000000,
  0b00000000100000000000000100000000,
  0b00000000100000000000000100000000,
  0b00000000100000000000000100000000,
  0b00000000100000000000000100000000,
  0b00000000100000000000000100000000,
  0b00011111111111100111111111111000,
  0b00010000100000100100000100001000,
  0b00010000100000100100000100001000,
  0b00011100111111100111111100111000,
  0b00000100100100000000100100100000,
  0b00000100100100000000100100100000,
  0b00011111100111100111100111111000,
  0b00010000000000100100000000001000,
  0b00010000000000100100000000001000,
  0b00011111111111111111111111111000,
  0b00000000000000000000000000000000,
  0b00000000000000000000000000000000,
  0b00000000000000000000000000000000
};

//borrowed from https://code.google.com/p/glcd-arduino/issues/detail?id=43
const unsigned char threebyfive[][3] PROGMEM =
{
  {0x00, 0x00, 0x00},  // (space)
  {0x17, 0x00, 0x00},  // !
  {0x03, 0x00, 0x03},  // "
  {0x0A, 0x1F, 0x0A},  // #
  {0x16, 0x13, 0x1A},  // 0x
  {0x09, 0x04, 0x0A},  // %
  {0x0A, 0x15, 0x1A},  // &
  {0x03, 0x00, 0x00},  // '
  {0x00, 0x0E, 0x11},  // (
  {0x11, 0x0E, 0x00},  // )
  {0x06, 0x06, 0x00},  // *
  {0x04, 0x0E, 0x04},  // +
  {0x0C, 0x1C, 0x00},  // ,
  {0x04, 0x04, 0x04},  // -
  {0x10, 0x00, 0x00},  // .
  {0x18, 0x04, 0x03},  // /
  {0x1F, 0x11, 0x1F},  // 0
  {0x02, 0x1F, 0x00},  // 1
  {0x1D, 0x15, 0x17},  // 2
  {0x15, 0x15, 0x1F},  // 3
  {0x0F, 0x08, 0x1E},  // 4
  {0x17, 0x15, 0x1D},  // 5
  {0x1F, 0x15, 0x1D},  // 6
  {0x01, 0x01, 0x1F},  // 7
  {0x1F, 0x15, 0x1F},  // 8
  {0x17, 0x15, 0x1F},  // 9
  {0x00, 0x0A, 0x00},  // :
  {0x00, 0x1A, 0x00},  // ;
  {0x04, 0x0A, 0x11},  // <
  {0x0A, 0x0A, 0x0A},  // =
  {0x11, 0x0A, 0x04},  // >
  {0x00, 0x15, 0x07},  // ?
  {0x1F, 0x15, 0x17},  // @
  {0x1F, 0x05, 0x1F},  // A
  {0x1F, 0x15, 0x1B},  // B
  {0x1F, 0x11, 0x11},  // C
  {0x1F, 0x11, 0x0E},  // D
  {0x1F, 0x15, 0x15},  // E
  {0x1F, 0x05, 0x01},  // F
  {0x1F, 0x11, 0x1D},  // G
  {0x1F, 0x04, 0x1F},  // H
  {0x11, 0x1F, 0x11},  // I
  {0x08, 0x10, 0x0F},  // J
  {0x1F, 0x04, 0x1B},  // K
  {0x1F, 0x10, 0x10},  // L
  {0x1F, 0x06, 0x1F},  // M
  {0x1C, 0x04, 0x1C},  // N
  {0x1F, 0x11, 0x1F},  // O
  {0x1F, 0x05, 0x07},  // P
  {0x0E, 0x19, 0x1E},  // Q
  {0x1F, 0x05, 0x1B},  // R
  {0x17, 0x15, 0x1D},  // S
  {0x01, 0x1F, 0x01},  // T
  {0x1F, 0x10, 0x1F},  // U
  {0x0F, 0x10, 0x0F},  // V
  {0x1F, 0x0C, 0x1F},  // W
  {0x1B, 0x04, 0x1B},  // X
  {0x17, 0x14, 0x1F},  // Y
  {0x19, 0x15, 0x13},  // Z
  {0x00, 0x1F, 0x11},  // [
  {0x03, 0x04, 0x18},  // BackSlash
  {0x11, 0x1F, 0x00},  // ]
  {0x06, 0x01, 0x06},  // ^
  {0x10, 0x10, 0x10},  // _
  {0x01, 0x01, 0x02},  // `
  {0x18, 0x14, 0x1C},  // a
  {0x1F, 0x14, 0x1C},  // b
  {0x1C, 0x14, 0x14},  // c
  {0x1C, 0x14, 0x1F},  // d
  {0x0C, 0x1A, 0x14},  // e
  {0x04, 0x1E, 0x05},  // f
  {0x17, 0x15, 0x1E},  // g
  {0x1F, 0x04, 0x1C},  // h
  {0x00, 0x1D, 0x00},  // i
  {0x08, 0x10, 0x0D},  // j
  {0x1F, 0x0C, 0x1A},  // k
  {0x00, 0x1F, 0x00},  // l
  {0x18, 0x0C, 0x18},  // m
  {0x18, 0x04, 0x18},  // n
  {0x1E, 0x12, 0x1E},  // o
  {0x1F, 0x05, 0x07},  // p
  {0x07, 0x05, 0x1F},  // q
  {0x1E, 0x04, 0x04},  // r
  {0x12, 0x15, 0x09},  // s
  {0x02, 0x1F, 0x02},  // t
  {0x1C, 0x10, 0x1C},  // u
  {0x0C, 0x10, 0x0C},  // v
  {0x0C, 0x18, 0x0C},  // w
  {0x14, 0x08, 0x14},  // x
  {0x16, 0x18, 0x06},  // y
  {0x04, 0x1C, 0x10},  // z
  {0x04, 0x0E, 0x11},  // {
  {0x00, 0x1F, 0x00},  // |
  {0x11, 0x0E, 0x04},  // }
  {0x02, 0x04, 0x02},  // ~
  {0x1F, 0x1F, 0x1F}   // 
};

void setup() {
  // put your setup code here, to run once:
  initHAL();
}

void loop() {
  //Everything is handled in the matrixman.c loop
  playMatrixman();
}
