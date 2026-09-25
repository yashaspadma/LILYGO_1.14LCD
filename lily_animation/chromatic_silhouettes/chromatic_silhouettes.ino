
#include <TFT_eSPI.h>
#include <SPI.h>
#include <math.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

#define W 240
#define H 135
#define CX 120
#define CY 67

uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return tft.color565(r, g, b);
}

uint16_t rainbow(float h, float v=1.0f) {
  h = fmodf(h, 360.0f);
  if (h < 0) h += 360.0f;
  
  float x = h / 60.0f;
  int s = (int)x;
  float f = x - s;
  float r,g,b;
  switch(s) {
    case 0: r=1; g=f; b=0; break;
    case 1: r=1-f; g=1; b=0; break;
    case 2: r=0; g=1; b=f; break;
    case 3: r=0; g=1-f; b=1; break;
    case 4: r=f; g=0; b=1; break;
    default:r=1; g=0; b=1-f; break;
  }
  return spr.color565((uint8_t)(r*255*v),(uint8_t)(g*255*v),(uint8_t)(b*255*v));
}

void setupDisplay() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  spr.setColorDepth(16);
  spr.createSprite(W,H);
}

float t=0;

void drawRing(float y,float radius,float phase,float hue) {
  const int pts=80;
  for(int i=0;i<pts;i++){
    float a=TWO_PI*i/pts;
    float wob=1.0f+0.12f*sinf(a*7.0f+phase*2.0f);
    int x=CX+(int)(cosf(a+phase)*radius*wob);
    int yy=CY+(int)y;
    if(x>=0&&x<W&&yy>=0&&yy<H)
      spr.fillCircle(x,yy,(i%4==0)?1:0,rainbow(hue+i*4+t*25));
  }
}

void drawSilhouette(){
  spr.fillSprite(TFT_BLACK);

  // Head / neck / shoulders / torso
  for(int i=0;i<42;i++){
    float yy=-40+i*2.0f;
    float n=(yy+40)/82.0f;
    float radius;
    if(n<0.25f) radius=9+12*n;
    else if(n<0.42f) radius=18+22*(n-0.25f);
    else radius=25+8*sinf(n*PI);
    float phase=t*1.8f+n*5.0f;

    int count=72;
    for(int j=0;j<count;j++){
      float a=TWO_PI*j/count;
      float r=radius*(1.0f+0.08f*sinf(a*5+phase));
      int x=CX+(int)(cosf(a+0.18f*sinf(n*8+t))*r);
      int y=CY+(int)yy;
      if(x>=0&&x<W&&y>=0&&y<H){
        float hue=j*5+n*280+t*45;
        spr.drawPixel(x,y,rainbow(hue,0.55f+0.45f*cosf(a)));
      }
    }
  }

  // Long spring-like vertical scanline
  for(int i=0;i<120;i++){
    float y=-58+i;
    float n=i/119.0f;
    float amp=8+30*sinf(n*PI);
    float x=CX+sinf(n*PI*18+t*4)*amp;
    int yy=CY+(int)y;
    if(yy>=0&&yy<H)
      spr.fillCircle((int)x,yy,1,rainbow(i*5+t*60));
  }

  spr.pushSprite(0,0);
}

void setup(){setupDisplay();}
void loop(){drawSilhouette();t+=0.045f;if(t>TWO_PI)t-=TWO_PI;yield();}
