#include <TFT_eSPI.h>
#include <SPI.h>
#include <math.h>

// ============================================================
// C.H.I.T.T.I. KNOWLEDGE CORE
// LILYGO / TTGO T-DISPLAY ESP32
//
// VERTICAL = false -> 240 x 135
// VERTICAL = true  -> 135 x 240
//
// The animation is deliberately asymmetric:
// - central rotating AI core
// - multiple independent orbital rings
// - 5 knowledge nodes revolving around the core
// - nodes connected by dynamic energy strings
// - dense particle/data field
// - brighter and dimmer regions
// - rotating scan/radar elements
// ============================================================

#define VERTICAL true

#if VERTICAL
  #define SCREEN_W 135
  #define SCREEN_H 240
  #define CX 67
  #define CY 120
#else
  #define SCREEN_W 240
  #define SCREEN_H 135
  #define CX 120
  #define CY 67
#endif

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

// ============================================================
// PERFORMANCE
// ============================================================

#define PARTICLES 420
#define FIELD_LINES 34
#define ORBIT_SEGMENTS 72
#define NODE_COUNT 5

// ============================================================
// PARTICLES
// ============================================================

struct Particle {
  float a;
  float r;
  float z;
  float speed;
  float drift;
  uint8_t brightness;
};

Particle particles[PARTICLES];

// ============================================================
// KNOWLEDGE NODES
// ============================================================

struct Node {
  float orbit;
  float angle;
  float speed;
  float tilt;
  float size;
  float phase;
  uint8_t brightness;
};

Node nodes[NODE_COUNT];

// ============================================================
// GLOBAL ANIMATION
// ============================================================

float tAnim = 0.0f;
float coreRotation = 0.0f;
float ringRotation = 0.0f;
float radarRotation = 0.0f;
float scan = 0.0f;

// ============================================================
// ORANGE / AMBER
// ============================================================

uint16_t amber(uint8_t b) {

  // Global brightness multiplier
  float brightness = 1.50f;

  int r = (int)(b * brightness);
  int g = (int)(b * 0.43f * brightness);
  int bl = (int)(b * 0.018f * brightness);

  if (r > 255) r = 255;
  if (g > 255) g = 255;
  if (bl > 255) bl = 255;

  return spr.color565(
    (uint8_t)r,
    (uint8_t)g,
    (uint8_t)bl
  );
}
// ============================================================
// SETUP PARTICLES
// ============================================================

void makeParticles() {

  randomSeed(analogRead(0));

  for (int i = 0; i < PARTICLES; i++) {

    particles[i].a =
      random(0, 6283) / 1000.0f;

    // Uneven radius creates the asymmetric cloud
    particles[i].r =
      random(17, 61);

    particles[i].z =
      random(-100, 101) / 100.0f;

    particles[i].speed =
      random(2, 15) / 10000.0f;

    particles[i].drift =
      random(-20, 21) / 1000.0f;

    particles[i].brightness =
      random(45, 210);
  }
}

// ============================================================
// SETUP KNOWLEDGE NODES
// ============================================================

void makeNodes() {

  // Each node has its own orbit and speed.
  // They intentionally do NOT form a perfect symmetric pattern.

  nodes[0] = {35.0f, 0.2f, 0.021f, 0.33f, 3.0f, 0.0f, 255};
  nodes[1] = {45.0f, 2.0f, -0.014f, 0.62f, 2.0f, 1.7f, 210};
  nodes[2] = {53.0f, 4.1f, 0.009f, 0.28f, 2.5f, 3.1f, 230};
  nodes[3] = {40.0f, 5.0f, -0.018f, 0.82f, 2.0f, 4.4f, 190};
  nodes[4] = {58.0f, 1.1f, 0.007f, 0.48f, 2.5f, 5.3f, 220};
}

// ============================================================
// 3D -> SCREEN
// ============================================================

void projectPoint(
  float x,
  float y,
  float z,
  int &sx,
  int &sy,
  float &depth
) {

  // Rotate around Y
  float cy = cosf(coreRotation);
  float syy = sinf(coreRotation);

  float x1 = x * cy - z * syy;
  float z1 = x * syy + z * cy;

  // Rotate around X
  float cx = cosf(0.42f);
  float sxr = sinf(0.42f);

  float y1 = y * cx - z1 * sxr;
  float z2 = y * sxr + z1 * cx;

  depth = z2;

  float perspective =
    1.0f / (1.0f - z2 * 0.22f);

  sx = CX + (int)(x1 * perspective);
  sy = CY + (int)(y1 * perspective);
}

// ============================================================
// CENTRAL ENERGY CORE
// ============================================================

void drawCore() {

  float pulse =
    1.0f +
    0.12f * sinf(tAnim * 4.2f);

  // Irregular glow
  for (int r = 18; r >= 3; r -= 3) {

    float fade =
      1.0f - r / 20.0f;

    uint8_t b =
      25 + (uint8_t)(fade * 90);

    spr.drawCircle(
      CX,
      CY,
      (int)(r * pulse),
      amber(b)
    );
  }

  // Rotating inner rings
  for (int k = 0; k < 3; k++) {

    float rr =
      6.0f + k * 4.0f;

    int px1 =
      CX + (int)(cosf(coreRotation + k) * rr);

    int py1 =
      CY + (int)(sinf(coreRotation + k) * rr * 0.55f);

    int px2 =
      CX + (int)(cosf(coreRotation + k + PI) * rr);

    int py2 =
      CY + (int)(sinf(coreRotation + k + PI) * rr * 0.55f);

    spr.drawLine(
      px1, py1,
      px2, py2,
      amber(150 + k * 30)
    );
  }

  // Bright core
  spr.fillCircle(
    CX,
    CY,
    (int)(2.5f * pulse),
    amber(255)
  );

  // Core spark
  spr.drawPixel(
    CX + (int)(sinf(tAnim * 7.0f) * 3),
    CY + (int)(cosf(tAnim * 5.0f) * 2),
    amber(255)
  );
}

// ============================================================
// ASYMMETRIC ORBITAL RINGS
// ============================================================

void drawOrbit(
  float radius,
  float flatten,
  float rotation,
  uint8_t brightness
) {

  int lastX = 0;
  int lastY = 0;
  bool first = true;

  for (int i = 0; i <= ORBIT_SEGMENTS; i++) {

    float a =
      TWO_PI * i / ORBIT_SEGMENTS +
      rotation;

    // Uneven ring
    float wobble =
      1.0f +
      0.06f * sinf(a * 5.0f + tAnim) +
      0.025f * sinf(a * 11.0f - tAnim * 2.0f);

    float x =
      cosf(a) *
      radius *
      wobble;

    float y =
      sinf(a) *
      radius *
      flatten *
      wobble;

    int px = CX + (int)x;
    int py = CY + (int)y;

    if (!first) {

      // Break random sections of the ring
      if ((i + (int)(tAnim * 4)) % 13 != 0) {

        spr.drawLine(
          lastX,
          lastY,
          px,
          py,
          amber(brightness)
        );
      }
    }

    lastX = px;
    lastY = py;
    first = false;
  }
}

// ============================================================
// DATA ARC
// ============================================================

void drawDataArc(
  float radius,
  float start,
  float length,
  float flatten,
  uint8_t brightness
) {

  int steps = 26;

  for (int i = 0; i < steps; i++) {

    float f1 = (float)i / steps;
    float f2 = (float)(i + 1) / steps;

    float a1 =
      start + length * f1;

    float a2 =
      start + length * f2;

    int x1 =
      CX + (int)(cosf(a1) * radius);

    int y1 =
      CY + (int)(sinf(a1) * radius * flatten);

    int x2 =
      CX + (int)(cosf(a2) * radius);

    int y2 =
      CY + (int)(sinf(a2) * radius * flatten);

    spr.drawLine(
      x1, y1,
      x2, y2,
      amber(brightness)
    );
  }
}

// ============================================================
// KNOWLEDGE PARTICLE FIELD
// ============================================================

void drawKnowledgeField() {

  for (int i = 0; i < PARTICLES; i++) {

    Particle &p = particles[i];

    p.a += p.speed;

    // Some particles drift in and out
    float rr =
      p.r +
      2.5f * sinf(
        tAnim * 1.2f +
        p.a * 2.0f +
        i * 0.017f
      );

    // Asymmetric deformation
    float x =
      cosf(p.a) * rr;

    float y =
      sinf(p.a) * rr;

    x *=
      1.0f +
      0.12f * sinf(p.a * 3.0f);

    y *=
      0.70f +
      0.08f * sinf(p.a * 5.0f);

    float z =
      p.z +
      0.12f * sinf(
        tAnim +
        i * 0.03f
      );

    int px, py;
    float depth;

    projectPoint(
      x,
      y,
      z * 20.0f,
      px,
      py,
      depth
    );

    if (
      px < 1 ||
      px >= SCREEN_W - 1 ||
      py < 1 ||
      py >= SCREEN_H - 1
    )
      continue;

    // Bright and dim areas
    float localLight =
      0.35f +
      0.65f *
      (0.5f + 0.5f *
      sinf(
        p.a * 3.0f +
        tAnim * 2.0f
      ));

    int b =
      (int)(
        p.brightness *
        localLight
      );

    if (b < 25) b = 25;
    if (b > 240) b = 240;

    if (depth > 0.45f) {

      spr.fillCircle(
        px,
        py,
        1,
        amber(b)
      );

    } else {

      spr.drawPixel(
        px,
        py,
        amber(b)
      );
    }
  }
}

// ============================================================
// MOVING DATA STREAMS
// ============================================================

void drawDataStreams() {

  for (int line = 0; line < FIELD_LINES; line++) {

    float base =
      TWO_PI *
      line /
      FIELD_LINES;

    float phase =
      base +
      tAnim *
      (0.25f + (line % 4) * 0.06f);

    float radius =
      20 +
      (line % 5) * 8;

    int previousX = 0;
    int previousY = 0;
    bool first = true;

    for (int p = 0; p < 22; p++) {

      float f =
        (float)p / 21.0f;

      float a =
        phase +
        f * (1.8f + (line % 3) * 0.4f);

      float r =
        radius +
        f * 18.0f +
        3.0f * sinf(
          f * 9.0f +
          tAnim * 3.0f +
          line
        );

      float x =
        cosf(a) * r;

      float y =
        sinf(a) * r * 0.52f;

      int sx = CX + (int)x;
      int sy = CY + (int)y;

      if (!first) {

        // Uneven intensity
        uint8_t b =
          35 +
          (uint8_t)(
            100.0f *
            (0.5f +
            0.5f *
            sinf(
              f * 12.0f -
              tAnim * 5.0f
            ))
          );

        if (line % 7 == 0)
          b += 60;

        if (b > 240)
          b = 240;

        spr.drawLine(
          previousX,
          previousY,
          sx,
          sy,
          amber(b)
        );
      }

      previousX = sx;
      previousY = sy;
      first = false;
    }
  }
}

// ============================================================
// KNOWLEDGE NODES
// ============================================================

void getNodePosition(
  int i,
  float &x,
  float &y,
  float &z
) {

  Node &n = nodes[i];

  n.angle += n.speed;

  float a =
    n.angle;

  float wobble =
    1.0f +
    0.10f *
    sinf(
      tAnim * 1.7f +
      n.phase
    );

  x =
    cosf(a) *
    n.orbit *
    wobble;

  y =
    sinf(a) *
    n.orbit *
    n.tilt;

  // Each node has its own depth motion
  z =
    sinf(
      a * 1.7f +
      n.phase
    ) *
    20.0f;
}

// ============================================================
// CONNECTION STRING BETWEEN NODES
// ============================================================

void drawEnergyString(
  float x1,
  float y1,
  float x2,
  float y2,
  float phase,
  uint8_t brightness
) {

  const int segments = 20;

  int lastX = (int)x1;
  int lastY = (int)y1;

  for (int i = 1; i <= segments; i++) {

    float f =
      (float)i / segments;

    // Curved connection
    float curve =
      sinf(
        f * PI
      ) *
      (8.0f +
       3.0f *
       sinf(tAnim * 3.0f + phase));

    float x =
      x1 +
      (x2 - x1) * f;

    float y =
      y1 +
      (y2 - y1) * f;

    // Perpendicular curve
    float dx = x2 - x1;
    float dy = y2 - y1;

    float length =
      sqrtf(dx * dx + dy * dy);

    if (length > 0.1f) {

      x +=
        (-dy / length) *
        curve;

      y +=
        (dx / length) *
        curve;
    }

    int px = (int)x;
    int py = (int)y;

    // Animated pulse moving along the string
    float pulse =
      0.45f +
      0.55f *
      sinf(
        f * 12.0f -
        tAnim * 6.0f +
        phase
      );

    uint8_t b =
      (uint8_t)(
        brightness *
        pulse
      );

    if (b < 20)
      b = 20;

    spr.drawLine(
      lastX,
      lastY,
      px,
      py,
      amber(b)
    );

    // Energy packets
    if (
      i % 5 == 0 &&
      pulse > 0.82f
    ) {

      spr.fillCircle(
        px,
        py,
        1,
        amber(255)
      );
    }

    lastX = px;
    lastY = py;
  }
}

// ============================================================
// DRAW NODES + CONNECTIONS
// ============================================================

void drawKnowledgeNodes() {

  float nx[NODE_COUNT];
  float ny[NODE_COUNT];
  float nz[NODE_COUNT];

  // Calculate positions
  for (int i = 0; i < NODE_COUNT; i++) {

    getNodePosition(
      i,
      nx[i],
      ny[i],
      nz[i]
    );

    int sx, sy;
    float depth;

    projectPoint(
      nx[i],
      ny[i],
      nz[i],
      sx,
      sy,
      depth
    );

    nx[i] = sx;
    ny[i] = sy;
  }

  // Two / three linked knowledge pathways
  drawEnergyString(
    CX,
    CY,
    nx[0],
    ny[0],
    0.2f,
    190
  );

  drawEnergyString(
    CX,
    CY,
    nx[2],
    ny[2],
    2.1f,
    170
  );

  drawEnergyString(
    nx[0],
    ny[0],
    nx[3],
    ny[3],
    3.8f,
    135
  );

  drawEnergyString(
    nx[1],
    ny[1],
    nx[4],
    ny[4],
    5.1f,
    115
  );

  // Nodes
  for (int i = 0; i < NODE_COUNT; i++) {

    int x = (int)nx[i];
    int y = (int)ny[i];

    if (
      x < 0 ||
      x >= SCREEN_W ||
      y < 0 ||
      y >= SCREEN_H
    )
      continue;

    float pulse =
      1.0f +
      0.30f *
      sinf(
        tAnim * 4.0f +
        nodes[i].phase
      );

    int r =
      (int)(
        nodes[i].size *
        pulse
      );

    // Node halo
    spr.drawCircle(
      x,
      y,
      r + 4,
      amber(55)
    );

    spr.drawCircle(
      x,
      y,
      r + 2,
      amber(120)
    );

    // Node ring
    spr.drawCircle(
      x,
      y,
      r,
      amber(220)
    );

    // Node center
    spr.fillCircle(
      x,
      y,
      1,
      amber(255)
    );

    // Small rotating satellite around node
    float a =
      tAnim * 3.0f +
      nodes[i].phase;

    int sx =
      x +
      (int)(cosf(a) * (r + 3));

    int sy =
      y +
      (int)(sinf(a) * (r + 3));

    spr.drawPixel(
      sx,
      sy,
      amber(255)
    );
  }
}

// ============================================================
// ASYMMETRIC OUTER ARCS
// ============================================================

void drawOuterStructures() {

  // Large uneven arcs
  drawDataArc(
    61,
    ringRotation * 0.4f,
    1.7f,
    0.48f,
    105
  );

  drawDataArc(
    56,
    -ringRotation * 0.8f + 2.3f,
    2.4f,
    0.72f,
    80
  );

  drawDataArc(
    63,
    ringRotation * 1.2f + 4.1f,
    0.9f,
    0.36f,
    160
  );

  // Small floating brackets / data blocks
  for (int i = 0; i < 11; i++) {

    float a =
      i * 0.91f +
      ringRotation * (i % 2 ? -0.5f : 0.7f);

    float r =
      51 +
      (i % 4) * 3;

    int x =
      CX +
      (int)(cosf(a) * r);

    int y =
      CY +
      (int)(sinf(a) * r * 0.55f);

    int s =
      2 +
      (i % 3);

    spr.drawRect(
      x - s,
      y - s,
      s * 2,
      s * 2,
      amber(
        80 +
        (i % 4) * 35
      )
    );
  }
}

// ============================================================
// RADAR / SCANNER
// ============================================================

void drawRadar() {

  float r = 62;

  spr.drawCircle(
    CX,
    CY,
    r,
    amber(45)
  );

  spr.drawCircle(
    CX,
    CY,
    r - 3,
    amber(25)
  );

  float a =
    radarRotation;

  int x =
    CX +
    (int)(cosf(a) * r);

  int y =
    CY +
    (int)(sinf(a) * r * 0.55f);

  spr.drawLine(
    CX,
    CY,
    x,
    y,
    amber(130)
  );

  // Short sweep trail
  for (int i = 1; i < 12; i++) {

    float aa =
      a - i * 0.035f;

    int xx =
      CX +
      (int)(cosf(aa) * r);

    int yy =
      CY +
      (int)(sinf(aa) * r * 0.55f);

    spr.drawLine(
      CX,
      CY,
      xx,
      yy,
      amber(120 - i * 8)
    );
  }
}

// ============================================================
// MOVING SCAN
// ============================================================

void drawScan() {

#if VERTICAL

  scan += 0.65f;

  if (scan > SCREEN_W)
    scan = 0;

  int x = (int)scan;

  for (int y = 10; y < SCREEN_H - 10; y += 5) {

    spr.drawPixel(
      x,
      y,
      amber(35)
    );
  }

#else

  scan += 0.75f;

  if (scan > SCREEN_H)
    scan = 0;

  int y = (int)scan;

  for (int x = 10; x < SCREEN_W - 10; x += 5) {

    spr.drawPixel(
      x,
      y,
      amber(35)
    );
  }

#endif
}

// ============================================================
// HUD TEXT - HORIZONTAL
// ============================================================

void drawHorizontalHUD() {

  spr.setTextSize(1);

  spr.setTextColor(
    amber(235)
  );

  spr.setCursor(7, 7);
  spr.print("C.H.I.T.T.I.");

  spr.setTextColor(
    amber(125)
  );

  spr.setCursor(8, 18);
  spr.print("Speed 1 teraHertz");

  spr.setCursor(8, 28);
  spr.print("Memory 1 zettabyte");

  spr.setCursor(8, 38);
  spr.print("AI CORE   [OK]");

  spr.setCursor(8, 48);
  spr.print("VISION    [blind]");

  spr.setCursor(8, 58);
  spr.print("LANGUAGE  [kannada]");

  spr.setTextColor(
    amber(190)
  );

  spr.setCursor(176, 7);
  spr.print("STARK");

  spr.setCursor(176, 18);
  spr.print("ONLINE");

  spr.setCursor(176, 30);
  spr.print("SYNC 100%");

  spr.setCursor(176, 42);
  spr.print("NODES  05");

  spr.setCursor(176, 54);
  spr.print("CORE  36C");

  spr.setCursor(38, 124);
  spr.print("INTELLIGENCE IN MOTION");
}

// ============================================================
// HUD TEXT - VERTICAL
// ============================================================

void drawVerticalHUD() {

  spr.setTextSize(1);

  spr.setTextColor(
    amber(235)
  );

  spr.setCursor(7, 7);
  spr.print("C.H.I.T.T.I.");

  spr.setTextColor(
    amber(125)
  );

  spr.setCursor(7, 18);
  spr.print("Speed 1 teraHertz");

  spr.setCursor(7, 28);
  spr.print("Memory 1 zettabyte");

  spr.setCursor(7, 39);
  spr.print("AI [dull]");

  spr.setCursor(7, 49);
  spr.print("VISION [blind]");

  spr.setCursor(7, 59);
  spr.print("LANG []");

  spr.setCursor(7, 190);
  spr.print("SYNC 100%");

  spr.setCursor(7, 200);
  spr.print("NODES 05");

  spr.setCursor(7, 210);
  spr.print("CORE 36C");

  spr.setTextColor(
    amber(170)
  );

  spr.setCursor(14, 229);
  spr.print("boom Boom Robo Da");
}

// ============================================================
// MAIN FRAME
// ============================================================

void drawFrame() {

  spr.fillSprite(
    TFT_BLACK
  );

  // Large asymmetric architecture
  drawOrbit(
    56,
    0.34f,
    ringRotation,
    95
  );

  drawOrbit(
    48,
    0.72f,
    -ringRotation * 0.72f,
    75
  );

  drawOrbit(
    41,
    1.10f,
    ringRotation * 1.25f,
    125
  );

  drawOuterStructures();

  // Dense neural/data environment
  drawKnowledgeField();

  drawDataStreams();

  // Revolving knowledge satellites
  drawKnowledgeNodes();

  // Central AI core
  drawCore();

  // Radar
  drawRadar();

  // Scan
  drawScan();

  // Interface text
#if VERTICAL
  drawVerticalHUD();
#else
  drawHorizontalHUD();
#endif

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

  tft.init();

#if VERTICAL
  tft.setRotation(0);
#else
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

  makeParticles();
  makeNodes();
}

// ============================================================
// LOOP
// ============================================================

void loop() {

  drawFrame();

  ringRotation += 0.021f;
  radarRotation += 0.040f;
  coreRotation += 0.012f;
  tAnim += 0.035f;

  if (ringRotation > TWO_PI)
    ringRotation -= TWO_PI;

  if (radarRotation > TWO_PI)
    radarRotation -= TWO_PI;

  if (coreRotation > TWO_PI)
    coreRotation -= TWO_PI;

  if (tAnim > TWO_PI)
    tAnim -= TWO_PI;

  yield();
}
