
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

struct P { float x,y,z,h; };
#define N 2400
P p[N];
float ry=0, rx=0.28f, rz=0;

void makePoints() {
  int k=0;
  const int rings=48, around=50;
  for(int a=0;a<rings;a++) {
    float lat=-PI/2.0f + PI*(a+0.5f)/rings;
    float cl=cosf(lat), sl=sinf(lat);
    for(int b=0;b<around;b++) {
      float q=TWO_PI*b/around;
      float twist=q + 2.0f*sl + 0.22f*sinf(7*lat+q*0.4f);
      p[k].x=cl*cosf(twist);
      p[k].y=sl;
      p[k].z=cl*sinf(twist);
      p[k].h=360.0f*b/around + 55.0f*a;
      k++;
    }
  }
}

void drawSphere() {
  spr.fillSprite(TFT_BLACK);
  float sy=sinf(ry), cy=cosf(ry), sx=sinf(rx), cx=cosf(rx);
  for(int i=0;i<N;i++) {
    float x=p[i].x, y=p[i].y, z=p[i].z;
    float x1=x*cy-z*sy, z1=x*sy+z*cy;
    float y2=y*cx-z1*sx, z2=y*sx+z1*cx;
    if(z2 < -0.30f) continue;
    float pr=1.0f/(1.0f-z2*0.28f);
    int x2=CX+(int)(x1*50*pr);
    int y2s=CY+(int)(y2*50*pr);
    if(x2<1||x2>=W-1||y2s<1||y2s>=H-1) continue;
    float br=0.50f+0.50f*(z2+0.3f)/1.3f;
    if(br>1) br=1;
    uint16_t c=rainbow(p[i].h+ry*35,br);
    if(z2>0.60f) spr.fillCircle(x2,y2s,1,c);
    else spr.drawPixel(x2,y2s,c);
  }
  spr.pushSprite(0,0);
}

void setup(){ setupDisplay(); makePoints(); }
void loop(){
  drawSphere();
  ry += 0.025f;
  rx += 0.0025f;
  if(ry>TWO_PI) ry-=TWO_PI;
  if(rx>TWO_PI) rx-=TWO_PI;
  yield();
}
