#include <TFT_eSPI.h>
#include <SPI.h>
#include <math.h>

// ============================================================
// J.A.R.V.I.S. HUD FOR LILYGO / TTGO T-DISPLAY
// ============================================================
//
// CHANGE ONLY THIS:
//
// false = HORIZONTAL 240 x 135
// true  = VERTICAL   135 x 240
//
// ============================================================

#define VERTICAL true

#if VERTICAL
  #define SCREEN_W 135
  #define SCREEN_H 240
  #define CENTER_X 67
  #define CENTER_Y 121
#else
  #define SCREEN_W 240
  #define SCREEN_H 135
  #define CENTER_X 120
  #define CENTER_Y 67
#endif

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

// ============================================================
// PARTICLES
// ============================================================

#define PARTICLE_COUNT 300

struct Particle {
  float angle;
  float radius;
  float speed;
  float z;
};

Particle particles[PARTICLE_COUNT];

// ============================================================
// ANIMATION VARIABLES
// ============================================================

float animTime = 0.0f;
float ringAngle = 0.0f;
float radarAngle = 0.0f;
float scanPosition = 0.0f;

// ============================================================
// COLOR
// ============================================================

uint16_t orange(uint8_t brightness) {

  uint8_t r = brightness;

  uint8_t g =
    (uint8_t)(brightness * 0.46f);

  uint8_t b =
    (uint8_t)(brightness * 0.025f);

  return spr.color565(r, g, b);
}

// ============================================================
// PARTICLE INITIALIZATION
// ============================================================

void createParticles() {

  randomSeed(analogRead(0));

  for (int i = 0; i < PARTICLE_COUNT; i++) {

    particles[i].angle =
      random(0, 6283) / 1000.0f;

    particles[i].radius =
      random(20, 58);

    particles[i].speed =
      random(3, 14) / 10000.0f;

    particles[i].z =
      random(-100, 100) / 100.0f;
  }
}

// ============================================================
// CENTER REACTOR
// ============================================================

void drawCore() {

  float pulse =
    1.0f +
    sinf(animTime * 4.0f) * 0.12f;

  // Glow rings
  for (int r = 22; r >= 5; r -= 3) {

    float fade =
      1.0f -
      (float)r / 23.0f;

    uint8_t brightness =
      20 + (uint8_t)(fade * 80);

    spr.drawCircle(
      CENTER_X,
      CENTER_Y,
      (int)(r * pulse),
      orange(brightness)
    );
  }

  // Energy rings
  spr.drawCircle(
    CENTER_X,
    CENTER_Y,
    (int)(13 * pulse),
    orange(190)
  );

  spr.drawCircle(
    CENTER_X,
    CENTER_Y,
    (int)(9 * pulse),
    orange(255)
  );

  // Core
  spr.fillCircle(
    CENTER_X,
    CENTER_Y,
    (int)(3 * pulse),
    orange(255)
  );
}

// ============================================================
// ORBITAL RING
// ============================================================

void drawOrbit(
  float radius,
  float flatten,
  float rotation,
  uint8_t brightness
) {

  const int SEGMENTS = 90;

  int lastX = 0;
  int lastY = 0;

  bool first = true;

  for (int i = 0; i <= SEGMENTS; i++) {

    float a =
      TWO_PI * i / SEGMENTS + rotation;

    float x =
      cosf(a) * radius;

    float y =
      sinf(a) * radius * flatten;

    int px =
      CENTER_X + (int)x;

    int py =
      CENTER_Y + (int)y;

    if (!first) {

      spr.drawLine(
        lastX,
        lastY,
        px,
        py,
        orange(brightness)
      );
    }

    lastX = px;
    lastY = py;

    first = false;
  }
}

// ============================================================
// BROKEN HUD RING
// ============================================================

void drawBrokenRing(
  float radius,
  float rotation
) {

  const int SEGMENTS = 72;

  for (int i = 0; i < SEGMENTS; i++) {

    // Create gaps
    if ((i / 4) % 2 == 0)
      continue;

    float a1 =
      TWO_PI * i / SEGMENTS + rotation;

    float a2 =
      TWO_PI * (i + 1) / SEGMENTS + rotation;

    int x1 =
      CENTER_X +
      (int)(cosf(a1) * radius);

    int y1 =
      CENTER_Y +
      (int)(sinf(a1) * radius * 0.55f);

    int x2 =
      CENTER_X +
      (int)(cosf(a2) * radius);

    int y2 =
      CENTER_Y +
      (int)(sinf(a2) * radius * 0.55f);

    spr.drawLine(
      x1,
      y1,
      x2,
      y2,
      orange(175)
    );
  }
}

// ============================================================
// PARTICLE CLOUD
// ============================================================

void drawParticles() {

  for (int i = 0; i < PARTICLE_COUNT; i++) {

    Particle &p = particles[i];

    p.angle += p.speed;

    float radius =
      p.radius +
      sinf(
        animTime * 1.5f +
        i * 0.13f
      ) * 2.0f;

    float x =
      cosf(p.angle) * radius;

    float y =
      sinf(p.angle) * radius;

    float perspective =
      1.0f /
      (1.0f - p.z * 0.20f);

    int px =
      CENTER_X +
      (int)(x * perspective);

    int py =
      CENTER_Y +
      (int)(
        y *
        0.58f *
        perspective
      );

    if (
      px < 0 ||
      px >= SCREEN_W ||
      py < 0 ||
      py >= SCREEN_H
    )
      continue;

    uint8_t brightness =
      55 +
      (uint8_t)((p.z + 1.0f) * 80.0f);

    if (brightness > 235)
      brightness = 235;

    if (p.z > 0.55f) {

      spr.fillCircle(
        px,
        py,
        1,
        orange(brightness)
      );

    } else {

      spr.drawPixel(
        px,
        py,
        orange(brightness)
      );
    }
  }
}

// ============================================================
// RADAR SWEEP
// ============================================================

void drawRadar() {

  float radius;

#if VERTICAL
  radius = 61;
#else
  radius = 61;
#endif

  // Outer radar rings
  spr.drawCircle(
    CENTER_X,
    CENTER_Y,
    radius,
    orange(75)
  );

  spr.drawCircle(
    CENTER_X,
    CENTER_Y,
    radius - 5,
    orange(40)
  );

  // Sweep
  float a = radarAngle;

  int x =
    CENTER_X +
    (int)(cosf(a) * radius);

  int y =
    CENTER_Y +
    (int)(sinf(a) * radius * 0.55f);

  spr.drawLine(
    CENTER_X,
    CENTER_Y,
    x,
    y,
    orange(180)
  );

  // Sweep trail
  for (int i = 1; i < 10; i++) {

    float aa =
      a - i * 0.035f;

    int xx =
      CENTER_X +
      (int)(cosf(aa) * radius);

    int yy =
      CENTER_Y +
      (int)(sinf(aa) * radius * 0.55f);

    spr.drawLine(
      CENTER_X,
      CENTER_Y,
      xx,
      yy,
      orange(150 - i * 12)
    );
  }
}

// ============================================================
// SCAN LINE
// ============================================================

void drawScanLine() {

#if VERTICAL

  scanPosition += 0.65f;

  if (scanPosition > SCREEN_W)
    scanPosition = 0;

  int x = (int)scanPosition;

  for (int y = 20; y < SCREEN_H - 10; y += 4) {

    uint8_t brightness =
      25 +
      (uint8_t)(
        45 *
        (1.0f -
        fabsf(y - CENTER_Y) /
        CENTER_Y)
      );

    spr.drawPixel(
      x,
      y,
      orange(brightness)
    );
  }

#else

  scanPosition += 0.7f;

  if (scanPosition > SCREEN_H)
    scanPosition = 0;

  int y = (int)scanPosition;

  for (int x = 10; x < SCREEN_W - 10; x += 4) {

    uint8_t brightness =
      25 +
      (uint8_t)(
        45 *
        (1.0f -
        fabsf(x - CENTER_X) /
        CENTER_X)
      );

    spr.drawPixel(
      x,
      y,
      orange(brightness)
    );
  }

#endif
}

// ============================================================
// CORNER BRACKETS
// ============================================================

void drawCorners() {

  uint16_t c = orange(150);

#if VERTICAL

  // Top left
  spr.drawLine(5, 5, 25, 5, c);
  spr.drawLine(5, 5, 5, 18, c);

  // Top right
  spr.drawLine(SCREEN_W - 25, 5,
               SCREEN_W - 5, 5, c);
  spr.drawLine(SCREEN_W - 5, 5,
               SCREEN_W - 5, 18, c);

  // Bottom left
  spr.drawLine(5, SCREEN_H - 5,
               25, SCREEN_H - 5, c);
  spr.drawLine(5, SCREEN_H - 18,
               5, SCREEN_H - 5, c);

  // Bottom right
  spr.drawLine(SCREEN_W - 25,
               SCREEN_H - 5,
               SCREEN_W - 5,
               SCREEN_H - 5, c);

  spr.drawLine(SCREEN_W - 5,
               SCREEN_H - 18,
               SCREEN_W - 5,
               SCREEN_H - 5, c);

#else

  // Top left
  spr.drawLine(5, 5, 25, 5, c);
  spr.drawLine(5, 5, 5, 17, c);

  // Top right
  spr.drawLine(215, 5, 235, 5, c);
  spr.drawLine(235, 5, 235, 17, c);

  // Bottom left
  spr.drawLine(5, 130, 25, 130, c);
  spr.drawLine(5, 118, 5, 130, c);

  // Bottom right
  spr.drawLine(215, 130, 235, 130, c);
  spr.drawLine(235, 118, 235, 130, c);

#endif
}

// ============================================================
// HORIZONTAL HEADER
// ============================================================

void drawHorizontalHeader() {

  spr.setTextSize(1);

  spr.setTextColor(
    orange(255)
  );

  spr.setCursor(
    9,
    8
  );

  spr.print(
    "J.A.R.V.I.S."
  );

  spr.setTextColor(
    orange(175)
  );

  spr.setCursor(
    158,
    8
  );

  spr.print(
    "ONLINE"
  );

  spr.setCursor(
    174,
    17
  );

  spr.print(
    "V1.0"
  );
}

// ============================================================
// HORIZONTAL LEFT PANEL
// ============================================================

void drawHorizontalLeftPanel() {

  spr.setTextSize(1);
  spr.setTextColor(orange(165));

  spr.setCursor(8, 30);
  spr.print("SYSTEM");

  spr.setCursor(8, 40);
  spr.print("AI CORE [OK]");

  spr.setCursor(8, 50);
  spr.print("VISION  [OK]");

  spr.setCursor(8, 60);
  spr.print("AUDIO   [OK]");

  spr.setCursor(8, 70);
  spr.print("I/O     [OK]");
}

// ============================================================
// HORIZONTAL RIGHT PANEL
// ============================================================

void drawHorizontalRightPanel() {

  spr.setTextSize(1);
  spr.setTextColor(orange(170));

  spr.setCursor(177, 43);
  spr.print("SYNC");

  spr.setCursor(177, 53);
  spr.print("100%");

  spr.setCursor(177, 66);
  spr.print("CORE");

  spr.setCursor(177, 76);
  spr.print("36C");

  spr.setCursor(177, 92);
  spr.print("POWER");

  for (int i = 0; i < 5; i++) {

    spr.fillRect(
      177 + i * 7,
      104,
      5,
      3,
      orange(120 + i * 25)
    );
  }
}

// ============================================================
// HORIZONTAL BOTTOM
// ============================================================

void drawHorizontalBottom() {

  spr.setTextSize(1);
  spr.setTextColor(orange(130));

  spr.setCursor(
    42,
    123
  );

  spr.print(
    "STARK // AI SYSTEM"
  );
}

// ============================================================
// VERTICAL HEADER
// ============================================================

void drawVerticalHeader() {

  spr.setTextSize(1);

  spr.setTextColor(
    orange(255)
  );

  spr.setCursor(
    8,
    10
  );

  spr.print(
    "J.A.R.V.I.S."
  );

  spr.setTextColor(
    orange(175)
  );

  spr.setCursor(
    43,
    21
  );

  spr.print(
    "ONLINE"
  );

  spr.setCursor(
    48,
    31
  );

  spr.print(
    "V1.0"
  );
}

// ============================================================
// VERTICAL TOP STATUS
// ============================================================

void drawVerticalStatus() {

  spr.setTextSize(1);
  spr.setTextColor(orange(155));

  spr.setCursor(7, 48);
  spr.print("AI CORE [OK]");

  spr.setCursor(7, 58);
  spr.print("VISION  [OK]");

  spr.setCursor(7, 68);
  spr.print("AUDIO   [OK]");

  spr.setCursor(7, 78);
  spr.print("I/O     [OK]");
}

// ============================================================
// VERTICAL BOTTOM STATUS
// ============================================================

void drawVerticalBottom() {

  spr.setTextSize(1);
  spr.setTextColor(orange(170));

  spr.setCursor(7, 190);
  spr.print("SYNC 100%");

  spr.setCursor(7, 201);
  spr.print("CORE 36C");

  spr.setCursor(7, 212);
  spr.print("POWER");

  // Power bar
  for (int i = 0; i < 5; i++) {

    spr.fillRect(
      48 + i * 7,
      212,
      5,
      3,
      orange(120 + i * 25)
    );
  }

  spr.setTextColor(orange(125));

  spr.setCursor(
    23,
    228
  );

  spr.print(
    "STARK // AI"
  );
}

// ============================================================
// HORIZONTAL HUD
// ============================================================

void drawHorizontalHUD() {

  drawCorners();

  drawHorizontalHeader();

  drawHorizontalLeftPanel();

  drawHorizontalRightPanel();

  drawHorizontalBottom();
}

// ============================================================
// VERTICAL HUD
// ============================================================

void drawVerticalHUD() {

  drawCorners();

  drawVerticalHeader();

  drawVerticalStatus();

  drawVerticalBottom();
}

// ============================================================
// MAIN FRAME
// ============================================================

void drawHUD() {

  spr.fillSprite(
    TFT_BLACK
  );

  // Large holographic structure

#if VERTICAL

  // Vertical display gets a tighter hologram
  drawOrbit(
    51,
    0.42f,
    ringAngle,
    115
  );

  drawOrbit(
    46,
    0.75f,
    -ringAngle * 0.7f,
    100
  );

  drawOrbit(
    40,
    1.20f,
    ringAngle * 1.4f,
    145
  );

  drawBrokenRing(
    55,
    -ringAngle * 0.8f
  );

#else

  drawOrbit(
    55,
    0.35f,
    ringAngle,
    110
  );

  drawOrbit(
    50,
    0.72f,
    -ringAngle * 0.7f,
    100
  );

  drawOrbit(
    43,
    1.20f,
    ringAngle * 1.4f,
    150
  );

  drawBrokenRing(
    58,
    -ringAngle * 0.8f
  );

#endif

  // Particle cloud
  drawParticles();

  // Radar
  drawRadar();

  // Core
  drawCore();

  // HUD text
#if VERTICAL
  drawVerticalHUD();
#else
  drawHorizontalHUD();
#endif

  // Moving scan
  drawScanLine();

  // Send frame
  spr.pushSprite(
    0,
    0
  );
}

// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

#if VERTICAL
  tft.init();
  tft.setRotation(0);
#else
  tft.init();
  tft.setRotation(1);
#endif

  tft.fillScreen(
    TFT_BLACK
  );

  spr.setColorDepth(16);

  spr.createSprite(
    SCREEN_W,
    SCREEN_H
  );

  createParticles();
}

// ============================================================
// LOOP
// ============================================================

void loop() {

  drawHUD();

  // Main orbital rotation
  ringAngle += 0.025f;

  // Radar
  radarAngle += 0.045f;

  // Pulse / animation
  animTime += 0.035f;

  if (ringAngle > TWO_PI)
    ringAngle -= TWO_PI;

  if (radarAngle > TWO_PI)
    radarAngle -= TWO_PI;

  if (animTime > TWO_PI)
    animTime -= TWO_PI;

  yield();
}
