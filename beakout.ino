/*
 *  Breakout game for Arduino and the Adafruit 1.8" TFT screen with joystick 
 *  Developed by Clement Baumann - July 5th, 2013
 *  Copy is prohibited unless explicitely authorized by author
 *  For any question please contact clement.baumann@epitech.eu
 */

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SD.h>
#include <SPI.h>

#define SD_CS    4
#define TFT_CS  10
#define TFT_DC   8
#define TFT_RST  -1

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#define BUTTON_NONE 0
#define BUTTON_DOWN 1
#define BUTTON_RIGHT 2
#define BUTTON_SELECT 3
#define BUTTON_UP 4
#define BUTTON_LEFT 5

#define CURSOR_WIDTH 20 // cursor width in pixels
int x = 0; // cursor position
int posx = random(12.8)*10; //initial ball position 
int posy = 70; // ball y coordinate
boolean descending = true; // whether ball is descending or not
boolean right = true; //ball going right
boolean playing = true; // whether the game is playing or not (false when game ended)
int nbDestroyedBricks = 0; //number of destroyed bricks 

class Brick;

// wall bricks
#define WALL_WIDTH 6 // number of bricks horizontally
#define WALL_HEIGHT 5 // number of bricks vertically
#define WALL_SIZE (WALL_WIDTH * WALL_HEIGHT)
#define BRICK_WIDTH 22 // pixels
#define BRICK_HEIGHT 12 // pixels
Brick* bricks[WALL_SIZE];

class Brick {
  public :
      int posxb;
      int posyb;
      boolean destroyed;
      Brick(int x, int y);
      void draw();
      void suppr();
      void IntersectsAscending();
      void IntersectsLeft();
      void IntersectsRight();
      void IntersectsDescending();
      void Intersects();
      
  private :
      int height;
      int width;     
};

Brick::Brick (int x, int y) {
  posxb = x;
  posyb = y;
  destroyed = false;
  height = BRICK_HEIGHT-3;
  width = BRICK_WIDTH-3;
  draw();
}

// draw the brick on screen
void Brick::draw () {
  tft.fillRect (posxb, posyb, width, height, ST7735_BLACK);
}

// erase the brick
void Brick::suppr( ) {
  destroyed = true;
  tft.fillRect(posxb ,posyb, 20,10,ST7735_BLUE);
  nbDestroyedBricks++;
}

// Manage intersection with ball
void Brick::Intersects () {
  if(!descending) {
    IntersectsAscending();
  }
  else {
    IntersectsDescending();
  }
  if(right) {
    IntersectsRight();
  }
  else {
    IntersectsLeft();
  }
}

void Brick::IntersectsAscending() {
  if((posx >= posxb && posx <= posxb+20) && (posy == posyb+10 || posy == posyb+9) && !destroyed) {
    descending = true;
    suppr();
  }
}

void Brick::IntersectsDescending() {
  if((posx >= posxb-3 && posx <= posxb+20) && (posy == posyb || posy == posyb-1) && !destroyed) {
    descending = false;
    suppr();
  }
}

void Brick::IntersectsLeft() {
  if((posy >= posyb && posy <= posyb+10) && (posx == posxb+19 || posx == posxb+20)  && !destroyed) {
    right = true;
    suppr();
  }
}

void Brick::IntersectsRight() {
  if((posy >= posyb+3 && posy <= posyb+10) && (posx == posxb || posx == posxb-1) && !destroyed) {
    right = false;
    suppr();
  }
}

void setup(void) {
  Serial.begin(9600);  
  tft.initR(INITR_REDTAB); // screen initialization
  Serial.println("OK!");
  tft.fillScreen(ST7735_BLUE); // set a blue background
  
  tft.fillRect(x, tft.height() - 5, 20, 3, ST7735_BLACK); // draw cursor

  for (int j = 0; j < WALL_HEIGHT; j++) {  
    for (int i = 0; i< WALL_WIDTH; i++) {
      bricks[i + j * WALL_WIDTH]  = new Brick(BRICK_WIDTH * i, BRICK_HEIGHT * j);
    }
  }
}

uint8_t buttonhistory = 0;

void loop() {
  while (playing) { // while game is not ended
    uint8_t b = readButton(); // check joystick state
    if (b == BUTTON_LEFT) { // joystick hit on the left
      if (x >= 0) {
        x -= 4;
      }
      tft.fillRect(x+20, tft.height()-5, CURSOR_WIDTH, 5, ST7735_BLUE); // reset cursor background
    }
    if (b == BUTTON_RIGHT) { // joystick hit on the right
      if(x < tft.width() - CURSOR_WIDTH) {
        x += 4;
      }
      tft.fillRect(x-20, tft.height()-5, CURSOR_WIDTH, 5, ST7735_BLUE); // reset cursor background
    }
    //If the ball hits the top or the cursor
    if(((x <= posx + 4 && x + 19 >= posx) && posy == tft.height() - 10 )||(posy == 0)) {
      descending = !descending;
    }
    // If the ball hits a vertical edge
    if((posx == 0) || (posx >= tft.width()-3)) {
      right = !right;
    }
    // Moves
    if(right) {
      posx += 2;
    }
    else {
      posx -= 2;
    }
    if(descending) { 
      posy += 2;    
    }
    else {
      posy -= 2;
    }
    if (right && descending) {
      tft.fillRect (posx - 2,posy - 2, 3, 3,ST7735_BLUE); // reset ball background
    }
    else if(!right && descending) {
      tft.fillRect (posx + 2,posy - 2, 3, 3,ST7735_BLUE); // reset ball background
    }
    else if(right && !descending) {
      tft.fillRect (posx - 2,posy + 2, 3, 3,ST7735_BLUE); // reset ball background
    }
    else {
      tft.fillRect (posx+2,posy+2,3, 3,ST7735_BLUE); // reset ball background
    }
    //ball
    tft.fillRect (posx, posy, 3, 3, ST7735_BLACK);
    //cursor
    tft.fillRect (x, tft.height() - 5, CURSOR_WIDTH, 5, ST7735_BLACK);
    //destroy bricks
    for (int i = 0 ; i < WALL_SIZE ; i++) {
      bricks[i]->Intersects();
    }
    
    //Game Over   
    if (nbDestroyedBricks == WALL_SIZE) {
      // draw the end text
      tft.setTextWrap(false);
      tft.setCursor(10, 60);
      tft.setTextColor(ST7735_RED);
      tft.setTextSize(1.5);
      tft.fillScreen(ST7735_BLACK);
      tft.println("CONGRATULATIONS !");
      //stop the game to prevent ball from moving
      playing = false;
    }
    if(posy > tft.height()) { // reset ball y coordinate
      posy = 70;
    }
  }
}

// Joystick state read function
uint8_t readButton(void) {
  float a = analogRead(3);
  
  a *= 5.0;
  a /= 1024.0;
  
  Serial.print("Button read analog = ");
  Serial.println(a);
  if (a < 0.2) return BUTTON_DOWN;
  if (a < 1.0) return BUTTON_RIGHT;
  if (a < 1.5) return BUTTON_SELECT;
  if (a < 2.0) return BUTTON_UP;
  if (a < 3.2) return BUTTON_LEFT;
  else return BUTTON_NONE;
}
