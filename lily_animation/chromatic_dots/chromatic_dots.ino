
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

#define DOTS 1800
float tt=0;

void drawDots(){
  spr.fillSprite(TFT_BLACK);
  for(int i=0;i<DOTS;i++){
    float a=(float)i*2.39996323f;
    float z=1.0f-2.0f*((i+0.5f)/DOTS);
    float r=sqrtf(1-z*z);

    // Blob / pear deformation
    float y=z;
    float width=1.0f+0.30f*(1.0f-y);
    float x=r*cosf(a)*width;
    float zz=r*sinf(a);

    // animate surface wave
    float wave=1.0f+0.07f*sinf(a*5+y*10-tt*3);
    x*=wave; zz*=wave;

    float cy=cosf(tt), sy=sinf(tt);
    float xr=x*cy-zz*sy;
    float zr=x*sy+zz*cy;

    float cxr=cosf(0.35f), sxr=sinf(0.35f);
    float yr=y*cxr-zr*sxr;
    float zr2=y*sxr+zr*cxr;

    if(zr2 < -0.25f) continue;

    float pr=1.0f/(1.0f-zr2*.32f);
    int px=CX+(int)(xr*47*pr);
    int py=CY+(int)(yr*47*pr);

    if(px<1||px>=W-1||py<1||py>=H-1) continue;

    float br=0.45f+0.55f*(zr2+0.25f)/1.25f;
    uint16_t c=rainbow(a*57.3f+tt*80,br);

    if(zr2>0.45f) spr.fillCircle(px,py,1,c);
    else spr.drawPixel(px,py,c);
  }
  spr.pushSprite(0,0);
}

void setup(){setupDisplay();}
void loop(){drawDots();tt+=0.025f;if(tt>TWO_PI)tt-=TWO_PI;yield();}
