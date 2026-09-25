#include <TFT_eSPI.h>
#include "MatrixCodeNFI18.h"

TFT_eSPI tft = TFT_eSPI();

unsigned long lastFrame;
unsigned long lastCharacterChange;

float walkPhase;

const int CELL_W = 10;
const int CELL_H = 10;

int personX;
int personY;

char characters[13][8];
uint8_t brightness[13][8];


// ============================================================
// MATRIX CHARACTER SET
// ============================================================

const char glyphs[64] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "@#$%&*+=<>[]{}"
    "/\\|";


// ============================================================
// RANDOM MATRIX CHARACTER
// ============================================================

char randomGlyph() {

    return glyphs[
        random(0, sizeof(glyphs) - 1)
    ];
}


// ============================================================
// CREATE CHARACTER FIELD
// ============================================================

void generateCharacters() {

    for (int y = 0; y < 13; y++) {

        for (int x = 0; x < 8; x++) {

            characters[y][x] =
                randomGlyph();

            brightness[y][x] =
                random(80, 255);
        }
    }
}


// ============================================================
// CHANGE RANDOM CHARACTERS
// ============================================================

void randomizeSomeCharacters() {

    for (int i = 0; i < 12; i++) {

        int x =
            random(0, 8);

        int y =
            random(0, 13);

        characters[y][x] =
            randomGlyph();

        brightness[y][x] =
            random(70, 255);
    }
}


// ============================================================
// DRAW MATRIX BACKGROUND
// ============================================================

void drawBackgroundCode() {

    for (int i = 0; i < 18; i++) {

        int x =
            random(
                0,
                tft.width()
            );

        int y =
            random(
                0,
                tft.height()
            );

        char c =
            randomGlyph();

        uint16_t color =
            tft.color565(
                0,
                random(10, 45),
                0
            );

        tft.drawChar(
            x,
            y,
            c,
            color,
            TFT_BLACK,
            1
        );
    }
}


// ============================================================
// DRAW ONE BODY CHARACTER
// ============================================================

void drawBodyCharacter(
    int x,
    int y,
    int row,
    int col
) {

    if (
        x < 0 ||
        x >= tft.width() ||
        y < 0 ||
        y >= tft.height()
    ) {

        return;
    }


    char c =
        characters[
            row % 13
        ][
            col % 8
        ];


    if (random(0, 100) < 18) {

        c =
            randomGlyph();
    }


    int b =
        random(100, 256);


    if (random(0, 100) < 8) {

        b = 255;
    }


    uint16_t color;


    if (b > 220) {

        color =
            tft.color565(
                100,
                255,
                100
            );

    }

    else if (b > 160) {

        color =
            tft.color565(
                20,
                220,
                20
            );

    }

    else {

        color =
            tft.color565(
                0,
                b,
                0
            );
    }


    tft.drawChar(
        x,
        y,
        c,
        color,
        TFT_BLACK,
        1
    );
}


// ============================================================
// DRAW PERSON
// ============================================================

void drawPerson() {

    float legMotion =
        sin(walkPhase);

    float armMotion =
        sin(walkPhase);


    int cx =
        personX;

    int cy =
        personY;


    // --------------------------------------------------------
    // HEAD
    // --------------------------------------------------------

    drawBodyCharacter(
        cx,
        cy,
        0,
        3
    );

    drawBodyCharacter(
        cx - CELL_W,
        cy + CELL_H,
        1,
        2
    );

    drawBodyCharacter(
        cx,
        cy + CELL_H,
        1,
        3
    );

    drawBodyCharacter(
        cx + CELL_W,
        cy + CELL_H,
        1,
        4
    );


    // --------------------------------------------------------
    // NECK
    // --------------------------------------------------------

    drawBodyCharacter(
        cx,
        cy + CELL_H * 2,
        2,
        3
    );


    // --------------------------------------------------------
    // SHOULDERS
    // --------------------------------------------------------

    drawBodyCharacter(
        cx - CELL_W,
        cy + CELL_H * 3,
        3,
        2
    );

    drawBodyCharacter(
        cx,
        cy + CELL_H * 3,
        3,
        3
    );

    drawBodyCharacter(
        cx + CELL_W,
        cy + CELL_H * 3,
        3,
        4
    );


    // --------------------------------------------------------
    // TORSO
    // --------------------------------------------------------

    for (int row = 4; row <= 6; row++) {

        drawBodyCharacter(
            cx - CELL_W,
            cy + CELL_H * row,
            row,
            2
        );

        drawBodyCharacter(
            cx,
            cy + CELL_H * row,
            row,
            3
        );

        drawBodyCharacter(
            cx + CELL_W,
            cy + CELL_H * row,
            row,
            4
        );
    }


    // --------------------------------------------------------
    // LEFT ARM
    // --------------------------------------------------------

    int leftArmX =
        cx - CELL_W * 2;

    int leftArmY =
        cy + CELL_H * 4
        + armMotion * 10;


    drawBodyCharacter(
        leftArmX,
        leftArmY,
        7,
        1
    );

    drawBodyCharacter(
        leftArmX - CELL_W,
        leftArmY + CELL_H,
        8,
        1
    );

    drawBodyCharacter(
        leftArmX - CELL_W * 2,
        leftArmY + CELL_H * 2,
        9,
        0
    );


    // --------------------------------------------------------
    // RIGHT ARM
    // --------------------------------------------------------

    int rightArmX =
        cx + CELL_W * 2;

    int rightArmY =
        cy + CELL_H * 4
        - armMotion * 10;


    drawBodyCharacter(
        rightArmX,
        rightArmY,
        7,
        5
    );

    drawBodyCharacter(
        rightArmX + CELL_W,
        rightArmY + CELL_H,
        8,
        6
    );

    drawBodyCharacter(
        rightArmX + CELL_W * 2,
        rightArmY + CELL_H * 2,
        9,
        7
    );


    // --------------------------------------------------------
    // HIPS
    // --------------------------------------------------------

    drawBodyCharacter(
        cx - CELL_W,
        cy + CELL_H * 7,
        7,
        2
    );

    drawBodyCharacter(
        cx,
        cy + CELL_H * 7,
        7,
        3
    );

    drawBodyCharacter(
        cx + CELL_W,
        cy + CELL_H * 7,
        7,
        4
    );


    // --------------------------------------------------------
    // LEFT LEG
    // --------------------------------------------------------

    int leftLegX =
        cx - CELL_W;

    int leftLegY =
        cy + CELL_H * 8;

    int leftOffset =
        legMotion * 12;


    drawBodyCharacter(
        leftLegX,
        leftLegY,
        8,
        2
    );

    drawBodyCharacter(
        leftLegX - leftOffset * 0.3,
        leftLegY + CELL_H,
        9,
        1
    );

    drawBodyCharacter(
        leftLegX - leftOffset * 0.7,
        leftLegY + CELL_H * 2,
        10,
        0
    );


    // --------------------------------------------------------
    // RIGHT LEG
    // --------------------------------------------------------

    int rightLegX =
        cx + CELL_W;

    int rightLegY =
        cy + CELL_H * 8;

    int rightOffset =
        -legMotion * 12;


    drawBodyCharacter(
        rightLegX,
        rightLegY,
        8,
        4
    );

    drawBodyCharacter(
        rightLegX - rightOffset * 0.3,
        rightLegY + CELL_H,
        9,
        5
    );

    drawBodyCharacter(
        rightLegX - rightOffset * 0.7,
        rightLegY + CELL_H * 2,
        10,
        6
    );


    // --------------------------------------------------------
    // RANDOM DIGITAL PARTICLES
    // --------------------------------------------------------

    for (int i = 0; i < 8; i++) {

        int x =
            cx + random(-55, 56);

        int y =
            cy + random(-10, 125);

        char c =
            randomGlyph();

        uint16_t color =
            tft.color565(
                0,
                random(30, 120),
                0
            );

        tft.drawChar(
            x,
            y,
            c,
            color,
            TFT_BLACK,
            1
        );
    }
}


// ============================================================
// SETUP
// ============================================================

void setup() {

    Serial.begin(115200);

    tft.begin();

    tft.setRotation(0);

    tft.loadFont(MatrixCodeNFI18);

    tft.fillScreen(TFT_BLACK);

    personX =
        tft.width() / 2;

    personY =
        tft.height() / 2 - 60;

    walkPhase = 0;

    lastFrame = millis();

    lastCharacterChange = millis();

    generateCharacters();
}


// ============================================================
// LOOP
// ============================================================

void loop() {

    // Exact original 55 ms frame timing

    if (millis() - lastFrame < 55)
        return;

    lastFrame = millis();


    // Exact original character timing

    if (
        millis() - lastCharacterChange
        > 70
    ) {

        randomizeSomeCharacters();

        lastCharacterChange =
            millis();
    }


    // Exact original walking speed

    walkPhase += 0.22;


    if (walkPhase > TWO_PI) {

        walkPhase -= TWO_PI;
    }


    // Exact original draw order

    drawBackgroundCode();

    drawPerson();
}
