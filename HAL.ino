/*------------------Display Specific Funcions----------------------------*/

//init TouchScreen port pins
TouchScreen ts = TouchScreen(XP, YP, XM, YM);

unsigned long ul_CurrentMillis = 0UL;
unsigned long ul_EnterInitials = 0UL;
const unsigned long ul_InitialsTimer = 45000UL;

char intString[11]; // enough for 4294967295\0

char lifelost = 0;

//Get value with Serial.println(sizeof(pacmanData_t));
#define PACMANDATASIZE sizeof(pacmanData_t)
#define EEPROMADD 10
#define EEPROMADD1 (EEPROMADD + PACMANDATASIZE)

struct pacmanData_t
{
  byte Init1;
  unsigned char Ver1;
  uint32_t maxscore;
  byte initials[3];
  unsigned char Ver2;
  byte Init2;
}
PacManData;

void initDisplay(void) {

  Tft.TFTinit();  //init TFT library

  displayClear(BLACK);
}

uint8_t mylives = 0;

void displayClear(uint8_t color) {

  Tft.fillRectangle(4, 45, 225, 224, tftcolors[color]);

  //detect lives : 2,1,0
  if (lives != 0)
  {
    mylives = lives;
  }

  if ((mylives > 0) || (lives > 0))
  {
    if (lives == 0)
    {
      mylives--;
    }
    lifelost = 1;
  }
}

byte enterHiScore = 0;
byte initials[3] = {'A', 'A', 'A'};
byte previnit[3];
byte prevsecleft = 0;

void displayGameOver(void) {

  int x = 0;
  int y = 3;

  uint32_t prevmaxscore;

  prevmaxscore = PacManData.maxscore;

  if (score > PacManData.maxscore)
  {
    PacManData.maxscore = score;
    WriteEEPROM();
  }

  drawString("GAME", x, y, WHITE);
  drawString("OVER", x + 4 * 4 + 1, y, WHITE); y += 6;

  intToString(score);
  drawString("SCORE", getCenterX(5), y , WHITE); y += 6;
  drawString(intString, getCenterX(7), y, WHITE); y += 6;

  intToString(prevmaxscore);
  drawString("HI-SCORE", getCenterX(8), y, WHITE); y += 6;
  drawString(intString, getCenterX(7), y, WHITE);

  delay(1000);

  for (byte i = 0; i < 3; i++)
  {
    byte j;
    for (j = 0; j < 8; j++)
    {
      intString[j] = ' ';
    }
    intString[j] = 0;

    for (j = 3; j < 6; j++)
    {
      intString[j] = PacManData.initials[j - 3];
    }
    drawString(intString, x, y, WHITE);

    delay(1000);

    byte foundend = 0;

    intToString(prevmaxscore);
    for (j = 0; j < 8; j++)
    {
      if (intString[j] == 0)
      {
        foundend = 1;
      }

      if (foundend)
      {
        intString[j] = ' ';
      }
    }
    intString[j] = 0;

    drawString(intString, getCenterX(7), y, WHITE);

    delay(1000);
  }

  if (score > prevmaxscore)
  {
    x = 0;
    y = 3;

    uint8_t dir;
    uint8_t sel = 1;

    delay(2000);

    displayClear(BLACK);

    drawString("NEW", x, y, WHITE);
    drawString("SCORE!", x + 4 * 3 + 1, y, WHITE); y += 6;

    drawString("ENTER", getCenterX(5), y, WHITE); y += 6;
    drawString("INITIALS", getCenterX(8), y, WHITE); y += 6;

    enterHiScore = 1;

    ul_EnterInitials = millis();

    for (byte i = 0; i < 3; i++)
    {
      previnit[i] = 0;
    }

    do
    {
      //BUG:restore this
      //delay(10);

      dir = getControl();

      uint8_t oldpos = sel;
      uint8_t letter = initials[oldpos];

      if (dir == UP)
      {
        letter++;

        if (letter > 127)
        {
          letter = 32;
        }
      }

      if (dir == DOWN)
      {
        letter--;

        if (letter < 32)
        {
          letter = 127;
        }
      }

      if (dir == LEFT)
      {
        sel--;

        if (sel > 5)
        {
          sel = 2;
        }
      }

      if (dir == RIGHT)
      {
        sel++;

        if (sel > 2)
        {
          sel = 0;
        }
      }

      initials[oldpos] = letter;

      for (byte i = 0; i < 3; i++)
      {
        uint8_t color;

        if (i == sel)
        {
          color = BLUE;
        }
        else
        {
          color = WHITE;
        }

        if (previnit[i] != initials[i] + color)
        {
          previnit[i] = initials[i] + color;
          drawChar(initials[i], getCenterX(3) + i * 4 , y, color);
        }
      }

      ul_CurrentMillis = millis();

      byte secleft = 45 - (ul_CurrentMillis - ul_EnterInitials) / 1000;

      if (secleft != prevsecleft)
      {
        prevsecleft = secleft;

        intToString(secleft);
        if (intString[1] == 0)
        {
          intString[1] = ' ';
          intString[2] = 0;
        }
        drawString(intString, getCenterX(2), y + 6, WHITE);
      }

    } while (!(ul_CurrentMillis - ul_EnterInitials > ul_InitialsTimer));

    for (byte i = 0; i < 3; i++)
    {
      PacManData.initials[i] = initials[i];
    }

    WriteEEPROM();

    delay(1000);

    enterHiScore = 0;
  }

  displayClear(BLACK);

  x = 0;
  y = 3;

  y += 6;
  drawString("TOUCH", getCenterX(5), y, WHITE); y += 6;
  drawString("TO", getCenterX(2), y, WHITE); y += 6;
  drawString("RESTART", getCenterX(7), y, WHITE); y += 6;

  delay(1000);

}

uint8_t getCenterX(uint8_t chars)
{
  return (16 - (4 * chars) / 2);
}

void displayPixel(uint8_t x, uint8_t y, char color) {

  byte r = pixelsize ;

  if ((x > 31) || (y > 33))
  {
    return;
  }

  if (color == GREY)
  {
    Tft.drawCircle(x * 7 + 8, y * 7 + 35, r, tftcolors[BLACK]);
    r = pixelsize-1;
  }

  Tft.fillCircle(x * 7 + 8, y * 7 + 35, r, tftcolors[color]);

}

void displayClose(void) {

}

void displayLatch(void) {


}

/*---------------------Control Specific Funcitons ----------------------*/
#define JOYUP      24
#define JOYDOWN    25
#define JOYLEFT    26
#define JOYRIGHT   27
#define JOYBUT     28

void initControl(void) {

}

uint8_t getControl(void) {

  // a point object holds x y and z coordinates.
  Point p = ts.getPoint();

  //map the ADC value read to into pixel co-ordinates
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, 320);

  if (lifelost)
  {
    lifelost = 0;        // screen was cleared during gameplay (lives>0)
    delay(3000);
  }

  if ((p.z > __PRESURE) && (p.z < 1000))
  {
    if ((!gameRunning) && (!enterHiScore))
    {
      displayClear(BLACK);  // ugly hack to clear screen and restart game
      lifelost = 1;
      return BUTTON;
    }

    if (p.y < 50)
    {
      return UP;
    }

    if (p.y > 270)
    {
      return DOWN;
    }

    if ((p.y > 50) && (p.y < 270))
    {
      if (p.x < 100)
      {
        return LEFT;
      }

      if (p.x > 140)
      {
        return RIGHT;
      }
    }
  }

  return NOINPUT;
}

void controlDelayMs(uint16_t ms) {
  delayMicroseconds(100 * ms);
}

uint16_t colorPickerPct(uint8_t pctRed, uint8_t pctGreen, uint8_t pctBlue)
{
  uint16_t red = (pctRed / 100.0) * 31;
  uint16_t green = (pctGreen / 100.0) * 63;
  uint16_t blue = (pctBlue / 100.0) * 31;

  uint16_t c = red * 2048 + green * 32 + blue;

  return c;
}

void generateColors()
{
  for (uint8_t i = 0; i < 11; i++)
  {
    tftcolors[i] = colorPickerPct(colors[i][0] / 2.55, colors[i][1] / 2.55, colors[i][2] / 2.55);
  }
}

void intToString(uint32_t pV)
{
  uint8_t digits = 0;
  uint32_t V = pV;

  do
  {
    if (V > 0)
    {
      digits++;
      V /= 10;
    }
  }
  while (V > 0);

  V = pV;
  for (uint8_t i = 0; i < digits; i++)
  {
    intString[digits - (i + 1)] = V % 10 + '0';
    V /= 10;
  }
  intString[digits] = 0;
}

//borrowed from Seeed Technology Inc. TFT library
void drawChar(INT8U ascii, INT16U poX, INT16U poY, char fgcolor)
{
  if ((ascii < 32) || (ascii > 127))
  {
    ascii = '?';
  }

  for (int i = 0; i < 3; i++ ) {
    INT8U temp = pgm_read_byte(&threebyfive[ascii - 0x20][i]);
    for (INT8U f = 0; f < 6; f++)
    {
      char color;
      if ((temp >> f) & 0x01)
      {
        color = fgcolor;
      }
      else
      {
        color = BLACK;
      }

      displayPixel(poX + i, poY + f, color);
      displayPixel(poX + i + 1, poY + f, BLACK);
    }
  }
}

//borrowed from Seeed Technology Inc. TFT library
void drawString(char *string, INT16U poX, INT16U poY, char fgcolor)
{
  while (*string)
  {
    drawChar(*string, poX, poY, fgcolor);
    *string++;

    if (poX < 33)
    {
      poX += 4;
    }
  }
}

void ReadEEPROM()
{
  pacmanData_t PacManData1;

  // read eeprom data
  eeprom_read_block((void*)&PacManData, (void*)EEPROMADD, PACMANDATASIZE);               // load first block onto working structure
  eeprom_read_block((void*)&PacManData1, (void*)EEPROMADD1, PACMANDATASIZE);             // and second block onto temporary structure

  if ((unsigned char)(PacManData.Ver2 - PacManData.Ver1) != (unsigned char)1)            // if first block is corrupted
  {
    eeprom_read_block((void*)&PacManData, (void*)EEPROMADD1, PACMANDATASIZE);            // load second block onto working structure
    eeprom_write_block((const void*)&PacManData, (void*)EEPROMADD, PACMANDATASIZE);      // and save working structure into first block
  }

  if ((unsigned char)(PacManData1.Ver2 - PacManData1.Ver1) != (unsigned char)1)          // if second block is corrupted
  {
    eeprom_write_block((const void*)&PacManData, (void*)EEPROMADD1, PACMANDATASIZE);     // save working structure into second block
  }

  // initialize to default values if data seems corrupted
  if ((PacManData.Init1 != 3) || (PacManData.Init2 != 7))
  {
    PacManData.Init1 = 3;
    PacManData.Init2 = 7;

    PacManData.Ver1 = 0;
    PacManData.Ver2 = 1;

    PacManData.maxscore = 0;

    PacManData.initials[0] = 'A';
    PacManData.initials[1] = 'A';
    PacManData.initials[2] = 'A';

    WriteEEPROM();
  }
}

void WriteEEPROM()
{
  PacManData.Ver1 = PacManData.Ver1 + 1;
  PacManData.Ver2 = PacManData.Ver1 + 1;

  eeprom_write_block((const void*)&PacManData, (void*)EEPROMADD, PACMANDATASIZE);
  eeprom_write_block((const void*)&PacManData, (void*)EEPROMADD1, PACMANDATASIZE);
}

void initHAL()
{
  //Serial.begin(9600);
  ReadEEPROM();
  generateColors();
}

