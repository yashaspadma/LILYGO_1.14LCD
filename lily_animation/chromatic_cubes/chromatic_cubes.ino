
#include <TFT_eSPI.h>
#include <SPI.h>
#include <math.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

#define W 240
#define H 135
#define CX 120
#define CY 67

// ============================================================
// CUBE SIZE
// Increase this number for a larger cube.
// 36 = original
// 48 = large
// 55 = very large
// ============================================================
#define CUBE_SIZE 55.0f

uint16_t rainbow(float h, float v=1.0f) {
  h = fmodf(h, 360.0f);
  if (h < 0) h += 360.0f;

  float x = h / 60.0f;
  int s = (int)x;
  float f = x - s;

  float r, g, b;

  switch (s) {
    case 0: r=1;   g=f;   b=0;   break;
    case 1: r=1-f; g=1;   b=0;   break;
    case 2: r=0;   g=1;   b=f;   break;
    case 3: r=0;   g=1-f; b=1;   break;
    case 4: r=f;   g=0;   b=1;   break;
    default:r=1; g=0;   b=1-f; break;
  }

  return spr.color565(
    (uint8_t)(r * 255 * v),
    (uint8_t)(g * 255 * v),
    (uint8_t)(b * 255 * v)
  );
}

// ============================================================
// CUBE VERTICES
// ============================================================

struct V {
  float x;
  float y;
  float z;
};

V v[8] = {
  {-1,-1,-1},
  { 1,-1,-1},
  { 1, 1,-1},
  {-1, 1,-1},

  {-1,-1, 1},
  { 1,-1, 1},
  { 1, 1, 1},
  {-1, 1, 1}
};

// ============================================================
// CUBE EDGES
// ============================================================

int edges[12][2] = {
  {0,1},
  {1,2},
  {2,3},
  {3,0},

  {4,5},
  {5,6},
  {6,7},
  {7,4},

  {0,4},
  {1,5},
  {2,6},
  {3,7}
};

float angle = 0.0f;

// ============================================================
// DISPLAY SETUP
// ============================================================

void setup() {

  tft.init();

  tft.setRotation(1);

  tft.fillScreen(TFT_BLACK);

  spr.setColorDepth(16);
  spr.createSprite(W, H);
}

// ============================================================
// DRAW CUBE
// ============================================================

void drawCube() {

  spr.fillSprite(TFT_BLACK);

  int sx[8];
  int sy[8];

  // ----------------------------------------------------------
  // Calculate projected vertices
  // ----------------------------------------------------------

  for (int i = 0; i < 8; i++) {

    float x = v[i].x;
    float y = v[i].y;
    float z = v[i].z;

    // Y rotation
    float c = cosf(angle);
    float s = sinf(angle);

    float x1 = x * c - z * s;
    float z1 = x * s + z * c;

    // X rotation
    float cx = cosf(angle * 0.63f);
    float sxr = sinf(angle * 0.63f);

    float y1 = y * cx - z1 * sxr;
    float z2 = y * sxr + z1 * cx;

    // Perspective
    float perspective =
      1.0f / (2.8f - z2 * 0.25f);

    sx[i] =
      CX + (int)(x1 * CUBE_SIZE * perspective);

    sy[i] =
      CY + (int)(y1 * CUBE_SIZE * perspective);
  }

  // ----------------------------------------------------------
  // Draw cube edges
  // ----------------------------------------------------------

  for (int i = 0; i < 12; i++) {

    int a = edges[i][0];
    int b = edges[i][1];

    float hue =
      i * 30.0f +
      angle * 30.0f;

    spr.drawLine(
      sx[a],
      sy[a],
      sx[b],
      sy[b],
      rainbow(hue)
    );
  }

  // ----------------------------------------------------------
  // Dense dotted cube surfaces
  // ----------------------------------------------------------

  for (int face = 0; face < 6; face++) {

    for (int a = 0; a < 12; a++) {

      for (int b = 0; b < 12; b++) {

        float u =
          -1.0f + 2.0f * a / 11.0f;

        float w =
          -1.0f + 2.0f * b / 11.0f;

        V q;

        switch (face) {

          case 0:
            q = { 1, u, w };
            break;

          case 1:
            q = {-1, u, w };
            break;

          case 2:
            q = { u, 1, w };
            break;

          case 3:
            q = { u,-1, w };
            break;

          case 4:
            q = { u, w, 1 };
            break;

          default:
            q = { u, w,-1 };
            break;
        }

        // Y rotation
        float c = cosf(angle);
        float s = sinf(angle);

        float x = q.x * c - q.z * s;
        float z = q.x * s + q.z * c;

        // X rotation
        float c2 = cosf(angle * 0.63f);
        float s2 = sinf(angle * 0.63f);

        float y = q.y * c2 - z * s2;
        float zz = q.y * s2 + z * c2;

        // Perspective
        float perspective =
          1.0f / (2.8f - zz * 0.25f);

        int px =
          CX + (int)(x * CUBE_SIZE * perspective);

        int py =
          CY + (int)(y * CUBE_SIZE * perspective);

        if (px > 0 && px < W &&
            py > 0 && py < H) {

          float hue =
            150.0f * zz +
            face * 55.0f +
            angle * 30.0f;

          spr.fillCircle(
            px,
            py,
            1,
            rainbow(hue)
          );
        }
      }
    }
  }

  // ----------------------------------------------------------
  // Send frame to display
  // ----------------------------------------------------------

  spr.pushSprite(0, 0);
}

// ============================================================
// LOOP
// ============================================================

void loop() {

  drawCube();

  angle += 0.025f;

  if (angle > TWO_PI)
    angle -= TWO_PI;

  yield();
}
