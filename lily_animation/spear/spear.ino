#include <TFT_eSPI.h>
#include <SPI.h>
#include <math.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

// ----------------------------------------------------
// DISPLAY
// ----------------------------------------------------

#define SCREEN_W 240
#define SCREEN_H 135

// Sphere
#define CX 120
#define CY 67

#define SPHERE_RADIUS 48

// Number of points
#define LATITUDE_LINES 40
#define LONGITUDE_POINTS 64

// ----------------------------------------------------
// 3D POINT
// ----------------------------------------------------

struct Point3D {
  float x;
  float y;
  float z;
};

Point3D points[LATITUDE_LINES * LONGITUDE_POINTS];

// ----------------------------------------------------
// ROTATION
// ----------------------------------------------------

float rotationY = 0.0;
float rotationX = 0.25;
float rotationZ = 0.0;

// Rotation speed
float speedY = 0.035;
float speedX = 0.008;
float speedZ = 0.003;

// ----------------------------------------------------
// SETUP
// ----------------------------------------------------

void setup() {

  Serial.begin(115200);

  tft.init();

  // Landscape
  tft.setRotation(1);

  tft.fillScreen(TFT_BLACK);

  // Sprite = entire screen buffer
  sprite.setColorDepth(16);
  sprite.createSprite(SCREEN_W, SCREEN_H);

  // Disable flicker
  sprite.fillSprite(TFT_BLACK);

  generateSphere();
}

// ----------------------------------------------------
// CREATE SPHERE POINTS
// ----------------------------------------------------

void generateSphere() {

  int index = 0;

  for (int lat = 0; lat < LATITUDE_LINES; lat++) {

    // Latitude from -90 to +90
    float theta =
      -PI / 2.0 +
      PI * ((float)lat / (LATITUDE_LINES - 1));

    float y = sin(theta);
    float ringRadius = cos(theta);

    for (int lon = 0; lon < LONGITUDE_POINTS; lon++) {

      float phi =
        2.0 * PI *
        ((float)lon / LONGITUDE_POINTS);

      points[index].x =
        ringRadius * cos(phi);

      points[index].y = y;

      points[index].z =
        ringRadius * sin(phi);

      index++;
    }
  }
}

// ----------------------------------------------------
// RGB COLOR
// ----------------------------------------------------

uint16_t makeColor(float hue) {

  hue = fmod(hue, 360.0);

  if (hue < 0)
    hue += 360.0;

  float r, g, b;

  float h = hue / 60.0;
  int i = floor(h);
  float f = h - i;

  float q = 1.0 - f;
  float t = f;

  switch (i) {

    case 0:
      r = 1;
      g = t;
      b = 0;
      break;

    case 1:
      r = q;
      g = 1;
      b = 0;
      break;

    case 2:
      r = 0;
      g = 1;
      b = t;
      break;

    case 3:
      r = 0;
      g = q;
      b = 1;
      break;

    case 4:
      r = t;
      g = 0;
      b = 1;
      break;

    default:
      r = 1;
      g = 0;
      b = q;
      break;
  }

  uint8_t R = r * 255;
  uint8_t G = g * 255;
  uint8_t B = b * 255;

  return sprite.color565(R, G, B);
}

// ----------------------------------------------------
// ROTATE POINT
// ----------------------------------------------------

Point3D rotatePoint(Point3D p) {

  Point3D r;

  // --------------------------------
  // Rotate Y
  // --------------------------------

  float cosY = cos(rotationY);
  float sinY = sin(rotationY);

  float x1 =
    p.x * cosY -
    p.z * sinY;

  float z1 =
    p.x * sinY +
    p.z * cosY;

  // --------------------------------
  // Rotate X
  // --------------------------------

  float cosX = cos(rotationX);
  float sinX = sin(rotationX);

  float y2 =
    p.y * cosX -
    z1 * sinX;

  float z2 =
    p.y * sinX +
    z1 * cosX;

  // --------------------------------
  // Rotate Z
  // --------------------------------

  float cosZ = cos(rotationZ);
  float sinZ = sin(rotationZ);

  float x3 =
    x1 * cosZ -
    y2 * sinZ;

  float y3 =
    x1 * sinZ +
    y2 * cosZ;

  r.x = x3;
  r.y = y3;
  r.z = z2;

  return r;
}

// ----------------------------------------------------
// DRAW SPHERE
// ----------------------------------------------------

void drawSphere() {

  sprite.fillSprite(TFT_BLACK);

  // --------------------------------
  // Slight glow behind sphere
  // --------------------------------

  for (int r = SPHERE_RADIUS + 3; r > SPHERE_RADIUS - 8; r--) {

    float alpha =
      (float)(SPHERE_RADIUS + 3 - r) / 11.0;

    uint8_t brightness =
      5 + alpha * 10;

    uint16_t glow =
      sprite.color565(
        brightness,
        brightness,
        brightness
      );

    sprite.drawCircle(
      CX,
      CY,
      r,
      glow
    );
  }

  // --------------------------------
  // Draw points
  // --------------------------------

  for (int i = 0;
       i < LATITUDE_LINES * LONGITUDE_POINTS;
       i++) {

    Point3D p =
      rotatePoint(points[i]);

    // --------------------------------
    // Perspective
    // --------------------------------

    float perspective =
      1.0 / (1.0 - p.z * 0.35);

    int screenX =
      CX +
      p.x *
      SPHERE_RADIUS *
      perspective;

    int screenY =
      CY +
      p.y *
      SPHERE_RADIUS *
      perspective;

    // --------------------------------
    // Don't draw points behind sphere
    // --------------------------------

    if (p.z < -0.15)
      continue;

    if (screenX < 0 ||
        screenX >= SCREEN_W ||
        screenY < 0 ||
        screenY >= SCREEN_H)
      continue;

    // --------------------------------
    // Dot size
    // --------------------------------

    int dotSize;

    if (p.z > 0.75)
      dotSize = 3;

    else if (p.z > 0.35)
      dotSize = 2;

    else
      dotSize = 1;

    // --------------------------------
    // COLOR
    // --------------------------------

    // Vertical rainbow gradient
    float hue =
      ((p.y + 1.0) * 180.0)
      +
      rotationY * 45.0;

    // Add horizontal color variation
    hue += p.x * 90.0;

    uint16_t color =
      makeColor(hue);

    // --------------------------------
    // Draw dot
    // --------------------------------

    if (dotSize == 1) {

      sprite.drawPixel(
        screenX,
        screenY,
        color
      );
    }

    else if (dotSize == 2) {

      sprite.fillCircle(
        screenX,
        screenY,
        1,
        color
      );
    }

    else {

      sprite.fillCircle(
        screenX,
        screenY,
        1,
        color
      );
    }
  }
}

// ----------------------------------------------------
// LOOP
// ----------------------------------------------------

void loop() {

  unsigned long startTime =
    millis();

  // --------------------------------
  // Draw current frame
  // --------------------------------

  drawSphere();

  // Push entire frame to display
  sprite.pushSprite(0, 0);

  // --------------------------------
  // Smooth rotation
  // --------------------------------

  rotationY += speedY;
  rotationX += speedX;
  rotationZ += speedZ;

  // Prevent floating-point values
  // from getting unnecessarily huge
  if (rotationY > TWO_PI)
    rotationY -= TWO_PI;

  if (rotationX > TWO_PI)
    rotationX -= TWO_PI;

  if (rotationZ > TWO_PI)
    rotationZ -= TWO_PI;

  // --------------------------------
  // Frame limiter
  // --------------------------------

  unsigned long frameTime =
    millis() - startTime;

  if (frameTime < 16) {
    delay(16 - frameTime);
  }
}
