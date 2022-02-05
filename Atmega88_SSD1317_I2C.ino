//Atmega88 SSD1317 OLED 4 pins I2C
//Buffer size 384Bytes 
//Four 7 segment numbers are displayed. No flicker.
//Decimal point and frame graphics are provided. 
//Without the character table, 30% of Flash and Ram are used only.
 
 #define SDA_PORT PORTC
 #define SDA_PIN 4 
 #define SCL_PORT PORTC
 #define SCL_PIN 5 
 #define I2C_FASTMODE 1
 
static byte bufr[384] = {};
unsigned long tim , start , wait, bufridx ;

#include <SoftI2CMaster.h>
#define oled 0x3c 

 const byte initcode[]= { 0xae,0xd5,0x80,0xa8,0x2f,0xd3,0x00,0x40,0x8d,0x14,
                          0x20,0x00,0xa1,0xc8,0xda,0x12,0x81,0xcf,0xd9,0xf1, //0x81 contrast
                          0xdb,0x40,0xa4,0xa6,0x2e,0xaf};
 
void setup(void) {
//Serial.begin(9600);
 i2c_init();
 //analogReference(INTERNAL);

 //int n= analogRead(1);

  for (byte j=0; j<sizeof(initcode); j++) { command(initcode[j]); }
  fillRoundRect(0,0,21,21,5,1);   displayon();delay(1000);

  displayon();
  drawCircle(5,35,10,1); displayon();delay(1000);wipe();
  drawLine(0,0,5,47,1);   drawLine(31,0,0,62,1);  
  displayon();delay(1000);wipe();
   Move (0,0); Print("Boda "); displayon();delay(1000);wipe();

}

void loop(void)
{  static unsigned int c;
  numout(c++); //delay(20);
 /*analogWrite(9,212);analogWrite(10,22);
 Move (0,0); Print("Boda ");  Move (0,2); Print("14x12");

 Move (0,7); Print("2018 "); displayon(0); wipe(); 
 int n= analogRead(0); Move(0,4); Hexout(n >>8); Hexout(n & 0xff);
 n= analogRead(1); Move(0,6); Hexout(n >>8); Hexout(n & 0xff); displayon(3); wipe(); 
// Serial.print(x); Serial.print(" ");

// Move (0,0); Bcdout(1009<<1);   displayon(3);
delay(2000); 
 clearDisplay();wipe();
 numout(5674);
 delay(2000);  clearDisplay();wipe();
*/
}

#define swap(a, b) { int16_t t = a; a = b; b = t; }

void drawPixel(int16_t x, int16_t y, uint16_t color) {  
//if (x>63 || y>47) return;    
bufr[x+((y/8)<<6)] |= (1 << (y&7));  
}

void command(byte c) {
  i2c_start(oled<<1); i2c_write(0x00); i2c_write(c);i2c_stop();
}

void clearDisplay(void) {  
  wipe();displayon();
}

void hLine(byte x, byte y, byte w, byte color) {
  
  byte *pBuf = bufr;
  pBuf += ((y/8)*64); pBuf += x;
  byte mask = 1 << (y&7);
  if (color) {while(w--) { *pBuf++ |= mask; };}
      else  {while(w--) { *pBuf++ &= ~mask; };}
    }

void vLine(byte x, byte y, byte h, byte color) {
  byte *pBuf = bufr;
  pBuf += ((y/8) * 64); pBuf += x;
  byte mod = y & 7;
  if (mod) { mod = 8 - mod;
    static byte premask[8] = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
    if( h < mod) premask[mod] &= (0XFF >> (mod-h));
    if (color) *pBuf |=  premask[mod]; else *pBuf &= ~premask[mod];
     if(h<mod) return;  h -= mod; pBuf += 64;
      }
  
  if(h >= 8) {
      byte val =0 ; if (color) val = 255; 
      do  {*pBuf = val; pBuf += 64; h -= 8;} while(h >= 8);
      }
      
  if(h) { mod = h & 7;
   static byte postmask[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
   if (color) *pBuf |= postmask[mod]; else *pBuf &= ~postmask[mod]; 
  }
}

void wipe(void) {
  //for (int h=0; h<=255; h++ ) bufr[h]=0;
  memset(bufr, 0, 384);
  }
  
  void numout(unsigned int n){
  if (n>9999) n=9999; //wipe();
  hLine(0,0,64,1);hLine(0,47,64,1); vLine(0,1,47,1);vLine(63,1,47,1);
  segment(n / 1000,0); n = n % 1000; segment(n / 100,1);  n = n % 100; 
  segment(n /10,2);  n = n % 10; segment(n,3); displayon();  
   }
  
void segment(byte c, byte pos) {
  byte p15,n15; p15=pos*15;
  if (c>9) c = 9;
  const byte tab[]= {0x7e,0x30,0x6d,0x79,0x33,0x5b,0x5f,0x70,0x7f,0x7b};
  const byte msk[]= {0x40,0x01,0x08,0x02,0x04,0x20,0x10};
  byte color,js=3,xpos[]={3,18,33,48};

  for (byte n=0; n<3 ; n++) { color = (msk[n] & tab[c]); 
  { for (byte j=8; j<11; j++) hLine(3+pos*15,j+n*15,13,color); }
  }  fillRect(xpos[pos],8,2,35,0);fillRect(xpos[pos]+11,8,2,35,0);
    for (byte n=0; n<2 ; n++) { 
    n15=n*15; if (msk[n+3] & tab[c]){
    for (byte j=js; j<js+2; j++) { vLine(j+p15,8+n15,18,1); }}   //seg f,e
    if (msk[n+5] & tab[c]){
    for (byte j=js; j<js+2; j++) { vLine(j+11+p15,8+n15,18,1);}} //seg b,c
  }
  }
void Move(byte x, byte y){
  bufridx=y*64 +x;
  }
    
void Print(const char *txt){
while (*txt) Cout(*txt++);
} 

void Bcdout(unsigned int bcd)
{ unsigned int t5;
if (bcd>9999) bcd=9999;
for (t5=1000;t5>0;t5/=10) {Cout(0x30+bcd/t5); bcd%=t5;}
} 

void Sgnout( int bcd)
{ unsigned int t5;
if (bcd<0) {Cout('-'); bcd=abs(bcd);} else Cout(32);
for (t5=1000;t5>0;t5/=10) {Cout(0x30+bcd/t5); bcd%=t5;}
} 

void Bcd2( char bcd) {
if (bcd>9) Cout ((bcd/10) + 0x30); else Cout(32);
Cout ((bcd%10)+0x30);
}

void Hexout (unsigned char key){
unsigned char n;
n=(key/0x10)+0x30; if (n>0x39) n+=7; Cout(n); //convert to hex char
n=(key%0x10)+0x30; if (n>0x39) n+=7; Cout(n);
} 


 const PROGMEM byte Chartable[] = 
{ 0x00 , 0x00 , 0x00 , 0x00 , 0x00, // Sp
0x00 , 0x00 , 0x2F , 0x00 , 0x00, // !
0x00 , 0x07 , 0x00 , 0x07 , 0x00, // ""
0x14 , 0x7F , 0x14 , 0x7F , 0x14, // #
0x24 , 0x2A , 0x7F , 0x2A , 0x12, // $
//0x7f , 0x7f , 0x7F , 0x7f , 0x7f, // $:full square
0x23 , 0x13 , 0x08 , 0x64 , 0x62, // % 
0x36 , 0x49 , 0x55 , 0x22 , 0x50, // &
0x00 , 0x05 , 0x03 , 0x00 , 0x00, // '
0x00 , 0x1C , 0x22 , 0x41 , 0x00, // (
0x00 , 0x41 , 0x22 , 0x1C , 0x00, // )
0x14 , 0x08 , 0x3E , 0x08 , 0x14, // *
0x08 , 0x08 , 0x3E , 0x08 , 0x08, // +
0x00 , 0x00 , 0x50 , 0x30 , 0x00, // ,
0x10 , 0x10 , 0x10 , 0x10 , 0x10, // -
0x00 , 0x60 , 0x60 , 0x00 , 0x00, // .
0x20 , 0x10 , 0x08 , 0x04 , 0x02, // /
0x3E , 0x51 , 0x49 , 0x45 , 0x3E, // 0
0x00 , 0x42 , 0x7F , 0x40 , 0x00, // 1
0x42 , 0x61 , 0x51 , 0x49 , 0x46, // 2
0x21 , 0x41 , 0x45 , 0x4B , 0x31, // 3
0x18 , 0x14 , 0x12 , 0x7F , 0x10, // 4
0x27 , 0x45 , 0x45 , 0x45 , 0x39, // 5
0x3C , 0x4A , 0x49 , 0x49 , 0x30, // 6
0x01 , 0x71 , 0x09 , 0x05 , 0x03, // 7
0x36 , 0x49 , 0x49 , 0x49 , 0x36, // 8
0x06 , 0x49 , 0x49 , 0x29 , 0x1E, // 9
0x00 , 0x36 , 0x36 , 0x00 , 0x00, // :
0x00 , 0x56 , 0x36 , 0x00 , 0x00, // ;
0x08 , 0x14 , 0x22 , 0x41 , 0x00, // <
0x14 , 0x14 , 0x14 , 0x14 , 0x14, // =
0x00 , 0x41 , 0x22 , 0x14 , 0x08, // >
0x02 , 0x01 , 0x51 , 0x09 , 0x06, // ???
0x32 , 0x49 , 0x59 , 0x51 , 0x3E, // @
0x7E , 0x11 , 0x11 , 0x11 , 0x7E, // A UpperCase
0x7F , 0x49 , 0x49 , 0x49 , 0x36, // B
0x3E , 0x41 , 0x41 , 0x41 , 0x22, // C
0x7F , 0x41 , 0x41 , 0x22 , 0x1C, // D
0x7F , 0x49 , 0x49 , 0x49 , 0x41, // E
0x7F , 0x09 , 0x09 , 0x09 , 0x01, // F
0x3E , 0x41 , 0x49 , 0x49 , 0x7A, // G
0x7F , 0x08 , 0x08 , 0x08 , 0x7F, // H
0x00 , 0x41 , 0x7F , 0x41 , 0x00, // I
0x20 , 0x40 , 0x41 , 0x3F , 0x01, // J
0x7F , 0x08 , 0x14 , 0x22 , 0x41, // K
0x7F , 0x40 , 0x40 , 0x40 , 0x40, // L
0x7F , 0x02 , 0x0C , 0x02 , 0x7F, // M
0x7F , 0x04 , 0x08 , 0x10 , 0x7F, // N
0x3E , 0x41 , 0x41 , 0x41 , 0x3E, // O
0x7F , 0x09 , 0x09 , 0x09 , 0x06, // P
0x3E , 0x41 , 0x51 , 0x21 , 0x5E, // Q
0x7F , 0x09 , 0x19 , 0x29 , 0x46, // R
0x46 , 0x49 , 0x49 , 0x49 , 0x31, // S
0x01 , 0x01 , 0x7F , 0x01 , 0x01, // T
0x3F , 0x40 , 0x40 , 0x40 , 0x3F, // U
0x1F , 0x20 , 0x40 , 0x20 , 0x1F, // V
0x3F , 0x40 , 0x38 , 0x40 , 0x3F, // W
0x63 , 0x14 , 0x08 , 0x14 , 0x63, // X
0x07 , 0x08 , 0x70 , 0x08 , 0x07, // Y
0x61 , 0x51 , 0x49 , 0x45 , 0x43, // Z
0x00 , 0x7F , 0x41 , 0x41 , 0x00, // [
0x02 , 0x04 , 0x08 , 0x10 , 0x20, // Bksp
0x00 , 0x41 , 0x41 , 0x7F , 0x00, // ]
0x11 , 0x39 , 0x55 , 0x11 , 0x1F, // Lf
0x40 , 0x40 , 0x40 , 0x40 , 0x40, // _
0x10 , 0x38 , 0x54 , 0x10 , 0x1F, // Cr
0x20 , 0x54 , 0x54 , 0x54 , 0x78, // A LowerCase
0x7F , 0x48 , 0x44 , 0x44 , 0x38, // B
0x38 , 0x44 , 0x44 , 0x44 , 0x20, // C
0x38 , 0x44 , 0x44 , 0x48 , 0x7F, // D
0x38 , 0x54 , 0x54 , 0x54 , 0x18, // E
0x08 , 0x7E , 0x09 , 0x01 , 0x02, // F
0x0C , 0x52 , 0x52 , 0x52 , 0x3E, // G
0x7F , 0x08 , 0x04 , 0x04 , 0x78, // H
0x00 , 0x44 , 0x7D , 0x40 , 0x00, // I
0x20 , 0x40 , 0x44 , 0x3D , 0x00, // J
0x7F , 0x10 , 0x28 , 0x44 , 0x00, // K
0x00 , 0x41 , 0x7F , 0x40 , 0x00, // L
0x7C , 0x04 , 0x18 , 0x04 , 0x78, // M
0x7C , 0x08 , 0x04 , 0x04 , 0x78, // N
0x38 , 0x44 , 0x44 , 0x44 , 0x38, // O
0x7C , 0x14 , 0x14 , 0x14 , 0x08, // P
0x08 , 0x14 , 0x14 , 0x18 , 0x7C, // Q
0x7C , 0x08 , 0x04 , 0x04 , 0x08, // R
0x48 , 0x54 , 0x54 , 0x54 , 0x20, // S
0x04 , 0x3F , 0x44 , 0x40 , 0x20, // T
0x3C , 0x40 , 0x40 , 0x20 , 0x7C, // U
0x1C , 0x20 , 0x40 , 0x20 , 0x1C, // V
0x3C , 0x40 , 0x30 , 0x40 , 0x3C, // W
0x44 , 0x28 , 0x10 , 0x28 , 0x44, // X
0x0C , 0x50 , 0x50 , 0x50 , 0x3C, // Y
0x44 , 0x64 , 0x54 , 0x4C , 0x44, // Z
0x11 , 0x39 , 0x55 , 0x11 , 0x1F, // Lf
0x10 , 0x38 , 0x54 , 0x10 , 0x1F, // Cr
0x20 , 0x54 , 0x57 , 0x54 , 0x78, // A'
0x38 , 0x54 , 0x57 , 0x54 , 0x18, // E'
0x00 , 0x48 , 0x7B , 0x40 , 0x00, // I'
0x38 , 0x44 , 0x47 , 0x44 , 0x38, // O'
0x38 , 0x47 , 0x44 , 0x47 , 0x38, // O''
0x38 , 0x45 , 0x44 , 0x45 , 0x38, // O:
0x3C , 0x40 , 0x47 , 0x20 , 0x7C, // U'
0x3C , 0x47 , 0x40 , 0x27 , 0x7C, // U''
0x3C , 0x41 , 0x40 , 0x21 , 0x7C }; // U:

void Cout(byte Byte)
{ unsigned int t5;
for (byte t4 = 0; t4<= 4; t4++) {
t5 = Byte-32 ; t5 *= 5 ; t5 += t4; 
bufr[bufridx++]= pgm_read_byte_near(Chartable + t5);
}
bufr[bufridx++]=0; 
}


//================= Graphic stuff, mostly of Adafruit origin ====================

// Draw a circle outline
void drawCircle(int x0, int y0, int r, unsigned int color) {

  int f = 1 - r, ddF_x = 1, ddF_y = -2 * r, x = 0, y = r;
  drawPixel(x0  , y0+r, color);
  drawPixel(x0  , y0-r, color);
  drawPixel(x0+r, y0  , color);
  drawPixel(x0-r, y0  , color);

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);
  }
}

void drawCircleHelper( int x0, int y0, int r, char cornername, unsigned int color) {
  int f     = 1 - r;
  int ddF_x = 1;
  int ddF_y = -2 * r;
  int x     = 0;
  int y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    if (cornername & 0x4) {
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 + y, y0 + x, color);
    } 
    if (cornername & 0x2) {
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      drawPixel(x0 - y, y0 - x, color);
      drawPixel(x0 - x, y0 - y, color);
    }
  }
}

void fillCircle(int x0, int y0, int r,
            unsigned int color) {
  vLine(x0, y0-r, 2*r+1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}

// Used to do circles and roundrects
void fillCircleHelper(int x0, int y0, int r,
    char cornername, int delta, unsigned int color) {

  int f     = 1 - r;
  int ddF_x = 1;
  int ddF_y = -2 * r;
  int x     = 0;
  int y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1) {
      vLine(x0+x, y0-y, 2*y+1+delta, color);
      vLine(x0+y, y0-x, 2*x+1+delta, color);
    }
    if (cornername & 0x2) {
      vLine(x0-x, y0-y, 2*y+1+delta, color);
      vLine(x0-y, y0-x, 2*x+1+delta, color);
    }
  }
}

// Bresenham's algorithm - thx wikpedia
void drawLine(int x0, int y0,
          int x1, int y1,
          unsigned int color) {
        int dx, dy , err,  ystep;
  int steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {swap(x0, y0);swap(x1, y1);}
 if (x0 > x1) {swap(x0, x1);swap(y0, y1);}
  dx = x1 - x0;dy = abs(y1 - y0); err = dx / 2;
  if (y0 < y1) {ystep = 1;} 
  else {ystep = -1;}

  for (; x0<=x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {y0 += ystep;err += dx;}
  }
}

// Draw a rectangle
void drawRect(int x, int y,
          int w, int h,
          unsigned int color) {
  hLine(x, y, w, color);
  hLine(x, y+h-1, w, color);
  vLine(x, y, h, color);
  vLine(x+w-1, y, h, color);
}


void fillRect(int x, int y, int w, int h,
          unsigned int color) {
 int i;
  for ( i=x; i<x+w; i++) {
    vLine(i, y, h, color);
  }
}

// Draw a rounded rectangle
void drawRoundRect(int x, int y, int w,
  int h, int r, unsigned int color) {
  // smarter version
  hLine(x+r  , y    , w-2*r, color); // Top
  hLine(x+r  , y+h-1, w-2*r, color); // Bottom
  vLine(x    , y+r  , h-2*r, color); // Left
  vLine(x+w-1, y+r  , h-2*r, color); // Right
  // draw four corners
  drawCircleHelper(x+r    , y+r    , r, 1, color);
  drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
  drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
  drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
}
// Fill a rounded rectangle
void fillRoundRect(int x, int y, int w,
         int h, int r, unsigned int color) {
  // smarter version
  fillRect(x+r, y, w-2*r, h, color);
// draw four corners
  fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
  fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
}

// Draw a triangle
void drawTriangle(int x0, int y0,
        int x1, int y1,
        int x2, int y2, unsigned int color) {
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}

// Fill a triangle
void fillTriangle ( int x0, int y0,
                    int x1, int y1,
                    int x2, int y2, unsigned int color) {

  int a, b, y, last;
  int dx01,dy01,dx02,dy02,dx12,dy12; 
  long sa,sb;
  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {swap(y0, y1); swap(x0, x1);}
  if (y1 > y2) {swap(y2, y1); swap(x2, x1);}
  if (y0 > y1) {swap(y0, y1); swap(x0, x1);}

  if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)      a = x1;
    else if(x1 > b) b = x1;
    if(x2 < a)      a = x2;
    else if(x2 > b) b = x2;
    hLine(a, y0, b-a+1, color);
    return;
  }
 dx01 = x1 - x0,dy01 = y1 - y0,dx02 = x2 - x0,dy02 = y2 - y0,
 dx12 = x2 - x1,dy12 = y2 - y1;sa   = 0, sb   = 0;
 // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2) last = y1;   // Include y1 scanline
  else         last = y1-1; // Skip it

  for(y=y0; y<=last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    // longhand: a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    //b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
  if(a > b) swap(a,b);
    hLine(a, y, b-a+1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    // longhand: a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    //  b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    if(a > b) swap(a,b);
    hLine(a, y, b-a+1, color);
  }
}

void displayon(void) {
  command(0x21);command(0x20);command(0x20 + 64 - 1);
  command(0x22);command(0); // Page start address 
  command((48 / 8) - 1); // Page end address
 {
    for (uint16_t i=0; i<(384); i++) {
      i2c_start(oled<<1);i2c_write(0x40);
      for (uint8_t x=0; x<16; x++) {i2c_write(bufr[i]);
      i++;}  i--;i2c_stop();
    }
  }
}
