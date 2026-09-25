#include <TFT_eSPI.h>
#include <SPI.h>
#include <math.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

// ============================================================
// DISPLAY
// ============================================================

#define SCREEN_W 240
#define SCREEN_H 135

#define CX 120
#define CY 67

// ============================================================
// SPHERE SETTINGS
// ============================================================

// Number of curved spears
#define SPEARS 64

// Dots per spear
#define DOTS_PER_SPEAR 64

// Total = 4096 points
#define TOTAL_POINTS (SPEARS * DOTS_PER_SPEAR)

#define SPHERE_RADIUS 48.0f

// ============================================================
// ROTATION
// ============================================================

float rotY = 0.0f;
float rotX = 0.22f;
float rotZ = 0.0f;

// Change these for speed
float speedY = 0.018f;
float speedX = 0.004f;
float speedZ = 0.002f;

// ============================================================
// POINT STRUCTURE
// ============================================================

struct Point3D {
  float x;
  float y;
  float z;

  // Color position
  float hue;
};

Point3D points[TOTAL_POINTS];

// ============================================================
// FAST SIN/COS VALUES
// ============================================================

float sinY;
float cosY;
float sinX;
float cosX;
float sinZ;
float cosZ;


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  tft.init();

  // TTGO T-Display
  tft.setRotation(1);

  tft.fillScreen(TFT_BLACK);

  // Create full screen framebuffer
  sprite.setColorDepth(16);
  sprite.createSprite(SCREEN_W, SCREEN_H);

  generateSpears();
}


// ============================================================
// GENERATE SPHERICAL SPEARS
// ============================================================

void generateSpears() {

  int index = 0;

  for (int s = 0; s < SPEARS; s++) {

    // Starting angle of this spear
    float spearPhase =
      TWO_PI * ((float)s / SPEARS);

    for (int d = 0; d < DOTS_PER_SPEAR; d++) {

      // -PI/2 to +PI/2
      float t =
        -PI / 2.0f +
        PI * ((float)d / (DOTS_PER_SPEAR - 1));

      // ------------------------------------------------------
      // Latitude
      // ------------------------------------------------------

      float latitude = t;

      float cosLat = cosf(latitude);
      float sinLat = sinf(latitude);

      // ------------------------------------------------------
      // Spiral twist
      //
      // This is what makes the lines curve around the sphere
      // instead of looking like a simple latitude/longitude grid.
      // ------------------------------------------------------

      float twist =
        spearPhase
        + 1.45f * sinLat
        + 0.35f * sinf(latitude * 3.0f);

      // ------------------------------------------------------
      // Small additional wave
      // ------------------------------------------------------

      twist +=
        0.12f *
        sinf(latitude * 6.0f + spearPhase);

      // ------------------------------------------------------
      // Convert spherical -> Cartesian
      // ------------------------------------------------------

      float x =
        cosLat * cosf(twist);

      float y =
        sinLat;

      float z =
        cosLat * sinf(twist);

      points[index].x = x;
      points[index].y = y;
      points[index].z = z;

      // Rainbow position
      points[index].hue =
        ((float)d / DOTS_PER_SPEAR) * 300.0f
        + s * 4.5f;

      index++;
    }
  }
}


// ============================================================
// HSV -> RGB565
// ============================================================

uint16_t rainbow(float hue, float brightness) {

  hue = fmodf(hue, 360.0f);

  if (hue < 0)
    hue += 360.0f;

  float h = hue / 60.0f;

  int sector = (int)h;

  float fraction = h - sector;

  float r, g, b;

  switch (sector) {

    case 0:
      r = 1.0f;
      g = fraction;
      b = 0.0f;
      break;

    case 1:
      r = 1.0f - fraction;
      g = 1.0f;
      b = 0.0f;
      break;

    case 2:
      r = 0.0f;
      g = 1.0f;
      b = fraction;
      break;

    case 3:
      r = 0.0f;
      g = 1.0f - fraction;
      b = 1.0f;
      break;

    case 4:
      r = fraction;
      g = 0.0f;
      b = 1.0f;
      break;

    default:
      r = 1.0f;
      g = 0.0f;
      b = 1.0f - fraction;
      break;
  }

  r *= brightness;
  g *= brightness;
  b *= brightness;

  uint8_t R = (uint8_t)(r * 255.0f);
  uint8_t G = (uint8_t)(g * 255.0f);
  uint8_t B = (uint8_t)(b * 255.0f);

  return sprite.color565(R, G, B);
}


// ============================================================
// UPDATE ROTATION
// ============================================================

void updateRotation() {

  sinY = sinf(rotY);
  cosY = cosf(rotY);

  sinX = sinf(rotX);
  cosX = cosf(rotX);

  sinZ = sinf(rotZ);
  cosZ = cosf(rotZ);
}


// ============================================================
// DRAW SPHERE
// ============================================================

void drawSphere() {

  sprite.fillSprite(TFT_BLACK);

  updateRotation();

  for (int i = 0; i < TOTAL_POINTS; i++) {

    float x = points[i].x;
    float y = points[i].y;
    float z = points[i].z;

    // ========================================================
    // ROTATE Y
    // ========================================================

    float x1 =
      x * cosY -
      z * sinY;

    float z1 =
      x * sinY +
      z * cosY;

    // ========================================================
    // ROTATE X
    // ========================================================

    float y2 =
      y * cosX -
      z1 * sinX;

    float z2 =
      y * sinX +
      z1 * cosX;

    // ========================================================
    // ROTATE Z
    // ========================================================

    float x3 =
      x1 * cosZ -
      y2 * sinZ;

    float y3 =
      x1 * sinZ +
      y2 * cosZ;

    // ========================================================
    // BACKFACE / DEPTH
    // ========================================================

    if (z2 < -0.18f)
      continue;

    // ========================================================
    // PERSPECTIVE
    // ========================================================

    float perspective =
      1.0f / (1.0f - z2 * 0.30f);

    int sx =
      (int)(CX +
      x3 *
      SPHERE_RADIUS *
      perspective);

    int sy =
      (int)(CY +
      y3 *
      SPHERE_RADIUS *
      perspective);

    if (sx < 0 ||
        sx >= SCREEN_W ||
        sy < 0 ||
        sy >= SCREEN_H)
      continue;

    // ========================================================
    // DEPTH BRIGHTNESS
    // ========================================================

    float brightness =
      0.55f +
      (z2 + 1.0f) * 0.35f;

    if (brightness > 1.0f)
      brightness = 1.0f;

    // ========================================================
    // COLOR
    // ========================================================

    float hue =
      points[i].hue
      + rotY * 40.0f;

    uint16_t color =
      rainbow(hue, brightness);

    // ========================================================
    // DOT SIZE
    // ========================================================

    if (z2 > 0.65f) {

      // Foreground dots
      sprite.fillCircle(
        sx,
        sy,
        1,
        color
      );

    } else {

      // Background dots
      sprite.drawPixel(
        sx,
        sy,
        color
      );
    }
  }
}


// ============================================================
// MAIN LOOP
// ============================================================

void loop() {

  unsigned long start =
    micros();

  drawSphere();

  // Push complete framebuffer
  sprite.pushSprite(0, 0);

  // ==========================================================
  // ROTATION
  // ==========================================================

  rotY += speedY;
  rotX += speedX;
  rotZ += speedZ;

  if (rotY > TWO_PI)
    rotY -= TWO_PI;

  if (rotX > TWO_PI)
    rotX -= TWO_PI;

  if (rotZ > TWO_PI)
    rotZ -= TWO_PI;

  // ==========================================================
  // DEBUG FPS
  // ==========================================================

  static uint32_t lastFPS = 0;
  static int frames = 0;

  frames++;

  if (millis() - lastFPS > 1000) {

    Serial.print("FPS: ");
    Serial.println(frames);

    frames = 0;
    lastFPS = millis();
  }

  // Small yield so ESP32 remains responsive
  yield();
}
