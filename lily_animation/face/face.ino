#include <TFT_eSPI.h>
#include "MatrixCodeNFI18.h"

// ============================================================
// LILYGO / TTGO POCKET SPHERE
//
// RGB animated 3D sphere made from hundreds of glowing dots.
// Inspired by the reference "pocket sphere" animation:
//
//  - dense circular dot grid
//  - real 3D rotation
//  - rainbow colour field
//  - spiral / vortex distortion
//  - moving highlight
//  - different visual modes
//
// The Matrix font is included because it is already part of
// your project, but this animation intentionally uses RGB dots
// rather than Matrix letters.
// ============================================================

TFT_eSPI tft = TFT_eSPI();

#define FRAME_MS          28

// Screen is 240 x 135 in landscape on the TTGO/LILYGO setup.
#define SPHERE_RADIUS     51

// Dot density
#define LAT_COUNT         25
#define LON_COUNT         42
#define TOTAL_POINTS      (LAT_COUNT * LON_COUNT)

// Dot size:
// 0 = 1 pixel
// 1 = 3x3-ish filled circle
#define DOT_RADIUS        1

// Automatically switch between different sphere looks.
#define AUTO_MODE         true
#define MODE_TIME         9000

// ------------------------------------------------------------
// 3D point
// ------------------------------------------------------------
struct SpherePoint {
  float x;
  float y;
  float z;
  float u;
  float v;
};

SpherePoint points[TOTAL_POINTS];

float rotY = 0.0;
float rotX = 0.18;

float timeAnim = 0.0;

unsigned long lastFrame = 0;
unsigned long modeStart = 0;

int mode = 0;

// ------------------------------------------------------------
// RGB colour helper
// HSV hue 0..255 -> RGB565
// ------------------------------------------------------------
uint16_t HSVto565(float hue, float sat, float val) {

  while (hue < 0)
    hue += 255.0;

  while (hue >= 255.0)
    hue -= 255.0;

  float h = hue / 255.0 * 6.0;

  int i = floor(h);
  float f = h - i;

  float p = val * (1.0 - sat);
  float q = val * (1.0 - sat * f);
  float t = val * (1.0 - sat * (1.0 - f));

  float r, g, b;

  switch (i % 6) {

    case 0:
      r = val;
      g = t;
      b = p;
      break;

    case 1:
      r = q;
      g = val;
      b = p;
      break;

    case 2:
      r = p;
      g = val;
      b = t;
      break;

    case 3:
      r = p;
      g = q;
      b = val;
      break;

    case 4:
      r = t;
      g = p;
      b = val;
      break;

    default:
      r = val;
      g = p;
      b = q;
      break;
  }

  uint8_t R = constrain((int)(r * 255.0), 0, 255);
  uint8_t G = constrain((int)(g * 255.0), 0, 255);
  uint8_t B = constrain((int)(b * 255.0), 0, 255);

  return tft.color565(R, G, B);
}

// ------------------------------------------------------------
// Create a clean spherical point grid.
//
// Using latitude rings gives the same dense "dot sphere"
// structure seen in the reference.
// ------------------------------------------------------------
void createSphere() {

  int index = 0;

  for (int lat = 0; lat < LAT_COUNT; lat++) {

    // Avoid making the poles collapse into one giant cluster.
    float v =
      -PI / 2.0 +
      PI * ((float)lat + 0.5) / LAT_COUNT;

    float cv = cos(v);
    float sv = sin(v);

    for (int lon = 0; lon < LON_COUNT; lon++) {

      float u =
        2.0 * PI *
        ((float)lon / LON_COUNT);

      points[index].x = cv * cos(u);
      points[index].y = sv;
      points[index].z = cv * sin(u);

      points[index].u = u;
      points[index].v = v;

      index++;
    }
  }
}

// ------------------------------------------------------------
// Project 3D point onto display
// ------------------------------------------------------------
void projectPoint(
  float x,
  float y,
  float z,
  int &sx,
  int &sy,
  float &depth
) {

  // Rotate Y
  float x1 =
    x * cos(rotY) -
    z * sin(rotY);

  float z1 =
    x * sin(rotY) +
    z * cos(rotY);

  // Rotate X
  float y2 =
    y * cos(rotX) -
    z1 * sin(rotX);

  float z2 =
    y * sin(rotX) +
    z1 * cos(rotX);

  // Perspective
  float camera = 2.6;

  float scale =
    camera /
    (camera - z2 * 0.72);

  int cx = tft.width() / 2;
  int cy = tft.height() / 2;

  sx =
    cx +
    (int)(
      x1 *
      SPHERE_RADIUS *
      scale
    );

  sy =
    cy +
    (int)(
      y2 *
      SPHERE_RADIUS *
      scale
    );

  depth = z2;
}

// ------------------------------------------------------------
// Draw one dot
// ------------------------------------------------------------
void drawSphereDot(
  int x,
  int y,
  uint16_t color,
  int radius
) {

  if (x < -radius ||
      y < -radius ||
      x >= tft.width() + radius ||
      y >= tft.height() + radius)
    return;

  if (radius == 0) {
    tft.drawPixel(x, y, color);
  }
  else {
    tft.fillCircle(x, y, radius, color);
  }
}

// ------------------------------------------------------------
// Different colour fields
// ------------------------------------------------------------
float getHue(
  float u,
  float v,
  float depth,
  int pointIndex
) {

  // MODE 0
  // Classic moving rainbow.
  if (mode == 0) {

    return
      u * 40.0 +
      v * 75.0 +
      timeAnim * 24.0;
  }

  // MODE 1
  // Strong diagonal rainbow bands like the reference.
  if (mode == 1) {

    return
      (u * 85.0) +
      (v * 120.0) +
      timeAnim * 18.0;
  }

  // MODE 2
  // Spiral colour field.
  if (mode == 2) {

    float spiral =
      u +
      v * 3.5;

    return
      spiral * 95.0 +
      timeAnim * 35.0;
  }

  // MODE 3
  // Bright rotating pole / vortex.
  if (mode == 3) {

    return
      atan2(
        sin(u) * cos(v),
        cos(u) * cos(v)
      ) * 80.0 +
      timeAnim * 32.0;
  }

  // MODE 4
  // Smooth travelling wave.
  return
    u * 55.0 +
    sin(v * 5.0 + timeAnim) * 65.0 +
    timeAnim * 15.0;
}

// ------------------------------------------------------------
// Draw the sphere
// ------------------------------------------------------------
void drawSphere() {

  int cx = tft.width() / 2;
  int cy = tft.height() / 2;

  // Temporary projected data
  struct RenderPoint {

    int x;
    int y;

    float depth;
    float hue;
    float brightness;

    uint16_t color;
  };

  static RenderPoint rp[TOTAL_POINTS];

  // ----------------------------------------------------------
  // Transform every point
  // ----------------------------------------------------------
  for (int i = 0; i < TOTAL_POINTS; i++) {

    SpherePoint &p = points[i];

    float x = p.x;
    float y = p.y;
    float z = p.z;

    // --------------------------------------------------------
    // Surface distortion.
    //
    // This is what makes it look alive instead of being just
    // a mathematically perfect static sphere.
    // --------------------------------------------------------

    float angle = p.u;

    float distortion;

    if (mode == 2) {

      distortion =
        1.0 +
        0.075 *
        sin(
          angle * 5.0 -
          p.v * 7.0 +
          timeAnim * 3.0
        );

    }
    else {

      distortion =
        1.0 +
        0.025 *
        sin(
          angle * 4.0 +
          p.v * 5.0 +
          timeAnim * 2.0
        );
    }

    x *= distortion;
    y *= distortion;
    z *= distortion;

    float depth;

    projectPoint(
      x,
      y,
      z,
      rp[i].x,
      rp[i].y,
      depth
    );

    rp[i].depth = depth;

    // --------------------------------------------------------
    // Colour
    // --------------------------------------------------------
    rp[i].hue =
      getHue(
        p.u,
        p.v,
        depth,
        i
      );

    // --------------------------------------------------------
    // Lighting.
    //
    // Front of sphere = bright.
    // Back = dark.
    // --------------------------------------------------------

    float front =
      (depth + 1.0) * 0.5;

    float light =
      0.18 +
      pow(front, 1.65) * 0.82;

    // Moving specular highlight.
    float highlight =
      sin(
        p.u -
        timeAnim * 1.8
      );

    highlight =
      max(0.0f, highlight);

    light +=
      highlight *
      0.16;

    // Extra edge glow.
    float edge =
      1.0 -
      abs(depth);

    light +=
      max(0.0f, edge) *
      0.05;

    if (light > 1.0)
      light = 1.0;

    rp[i].brightness = light;

    // Slightly desaturated shadows.
    float saturation =
      0.95 -
      (1.0 - light) * 0.35;

    rp[i].color =
      HSVto565(
        rp[i].hue,
        saturation,
        light
      );
  }

  // ----------------------------------------------------------
  // Depth sort.
  //
  // Back dots first, front dots last.
  // ----------------------------------------------------------
  for (int i = 0; i < TOTAL_POINTS - 1; i++) {

    for (int j = i + 1; j < TOTAL_POINTS; j++) {

      if (rp[i].depth > rp[j].depth) {

        RenderPoint temp =
          rp[i];

        rp[i] =
          rp[j];

        rp[j] =
          temp;
      }
    }
  }

  // ----------------------------------------------------------
  // Draw dots
  // ----------------------------------------------------------
  for (int i = 0; i < TOTAL_POINTS; i++) {

    // Make the rear hemisphere noticeably darker.
    if (rp[i].depth < -0.45) {

      if (random(100) > 60)
        continue;
    }

    int radius = DOT_RADIUS;

    // Occasionally make front dots brighter/larger.
    if (rp[i].depth > 0.72 &&
        random(100) < 7) {

      radius = 2;
    }

    drawSphereDot(
      rp[i].x,
      rp[i].y,
      rp[i].color,
      radius
    );
  }

  // ----------------------------------------------------------
  // Bright rotating spiral / vortex.
  //
  // This gives the "hole / spiral" look visible in some of
  // your reference frames.
  // ----------------------------------------------------------

  if (mode == 2 || mode == 3) {

    for (int i = 0; i < 18; i++) {

      float a =
        timeAnim * 2.0 +
        i * 0.38;

      float r =
        5.0 +
        i * 2.15;

      float px =
        cx +
        cos(a) * r;

      float py =
        cy +
        sin(a) * r * 0.72;

      uint16_t c =
        HSVto565(
          timeAnim * 30.0 + i * 14.0,
          1.0,
          0.75 + i * 0.012
        );

      drawSphereDot(
        px,
        py,
        c,
        1
      );
    }
  }
}

// ------------------------------------------------------------
// Tiny orbiting particles
// ------------------------------------------------------------
void drawOrbitParticles() {

  int cx = tft.width() / 2;
  int cy = tft.height() / 2;

  for (int i = 0; i < 10; i++) {

    float a =
      timeAnim * 0.8 +
      i * 0.628;

    float rx =
      SPHERE_RADIUS +
      4 +
      sin(timeAnim * 2.0 + i) * 4;

    float ry =
      rx * 0.28;

    int x =
      cx +
      cos(a) * rx;

    int y =
      cy +
      sin(a) * ry;

    uint16_t c =
      HSVto565(
        timeAnim * 30.0 + i * 22,
        0.95,
        0.8
      );

    drawSphereDot(
      x,
      y,
      c,
      0
    );
  }
}

// ------------------------------------------------------------
// Setup
// ------------------------------------------------------------
void setup() {

  randomSeed(
    analogRead(0) ^
    micros()
  );

  tft.init();

  // Landscape: 240 x 135
  tft.setRotation(1);

  // Keep your custom font available for the project.
  tft.loadFont(MatrixCodeNFI18);

  tft.fillScreen(TFT_BLACK);

  createSphere();

  modeStart = millis();
}

// ------------------------------------------------------------
// Main loop
// ------------------------------------------------------------
void loop() {

  unsigned long now = millis();

  if (now - lastFrame < FRAME_MS)
    return;

  lastFrame = now;

  timeAnim += 0.045;

  // Smooth 3D rotation
  rotY += 0.032;

  rotX =
    0.16 +
    sin(timeAnim * 0.45) * 0.18;

  // ----------------------------------------------------------
  // Automatic visual mode switching
  // ----------------------------------------------------------
  if (AUTO_MODE &&
      now - modeStart > MODE_TIME) {

    mode++;

    if (mode > 4)
      mode = 0;

    modeStart = now;
  }

  // ----------------------------------------------------------
  // Draw
  // ----------------------------------------------------------
  tft.fillScreen(TFT_BLACK);

  drawSphere();

  drawOrbitParticles();
}
