#include "ScreenSaver.h"

static TFT_eSPI* tftDisplay = nullptr;

static TFT_eSprite* heartSprite = nullptr;
static bool heartSpriteReady = false;

static unsigned long lastHeartFrame = 0;
static float heartPhase = 0.0;


// ============================================================
// INITIALIZE SCREEN SAVER
// ============================================================

void initScreenSaver(TFT_eSPI* display)
{
    tftDisplay = display;

    heartSprite = new TFT_eSprite(tftDisplay);
}


// ============================================================
// UPDATE SCREEN SAVER
// ============================================================

void updateScreenSaver(bool active)
{
    if (!active || tftDisplay == nullptr || heartSprite == nullptr)
        return;


    // --------------------------------------------------------
    // INITIALIZE SPRITE
    // --------------------------------------------------------

    if (!heartSpriteReady)
    {
        Serial.println(
            "SCREEN SAVER: creating heart sprite..."
        );

        heartSprite->setColorDepth(8);

        if (heartSprite->createSprite(300, 200))
        {
            heartSpriteReady = true;

            Serial.println(
                "SCREEN SAVER: heart sprite CREATED"
            );

            heartPhase = 0.0;
            lastHeartFrame = millis();
        }
        else
        {
            Serial.println(
                "SCREEN SAVER: heart sprite FAILED"
            );

            return;
        }
    }


    // --------------------------------------------------------
    // SCREEN SAVER FRAME RATE
    // ~12.5 FPS
    //
    // 80 ms between frames.
    // This reduces SPI traffic and leaves more time
    // for the WebServer.
    // --------------------------------------------------------

    if (millis() - lastHeartFrame < 80)
        return;

    lastHeartFrame = millis();


    // --------------------------------------------------------
    // HEARTBEAT
    // --------------------------------------------------------

    heartPhase += 0.12;

    float cycle =
        fmod(
            heartPhase,
            TWO_PI
        );

    float pulse = 1.0;


    // RRAHJA E PARE
    if (cycle < 0.55)
    {
        float p =
            sin(
                (cycle / 0.55) * PI
            );

        pulse =
            1.0 + 0.14 * p;
    }


    // RRAHJA E DYTE
    else if (
        cycle >= 0.75 &&
        cycle < 1.15
    )
    {
        float p =
            sin(
                ((cycle - 0.75) / 0.40) * PI
            );

        pulse =
            1.0 + 0.075 * p;
    }


    // --------------------------------------------------------
    // CLEAR ONLY THE SPRITE
    // --------------------------------------------------------

    heartSprite->fillSprite(
        TFT_BLACK
    );


    // --------------------------------------------------------
    // PARAMETRIC HEART
    // --------------------------------------------------------

    const int cx = 150;
    const int cy = 92;

    float scale =
        5.8 * pulse;

    bool firstPoint = true;

    int oldX = 0;
    int oldY = 0;


    for (
        float t = 0;
        t <= TWO_PI;
        t += 0.015
    )
    {
        float x =
            16.0 *
            sin(t) *
            sin(t) *
            sin(t);

        float y =
            13.0 * cos(t)
            - 5.0 * cos(2.0 * t)
            - 2.0 * cos(3.0 * t)
            - cos(4.0 * t);


        int px =
            cx +
            (int)(x * scale);

        int py =
            cy -
            (int)(y * scale);


        if (!firstPoint)
        {
            // ------------------------------------------------
            // HEART - 7 PX THICK
            // ------------------------------------------------

            heartSprite->drawLine(
                oldX,
                oldY,
                px,
                py,
                TFT_CYAN
            );

            heartSprite->drawLine(
                oldX + 1,
                oldY,
                px + 1,
                py,
                TFT_CYAN
            );

            heartSprite->drawLine(
                oldX - 1,
                oldY,
                px - 1,
                py,
                TFT_CYAN
            );

            heartSprite->drawLine(
                oldX,
                oldY + 1,
                px,
                py + 1,
                TFT_CYAN
            );

            heartSprite->drawLine(
                oldX,
                oldY - 1,
                px,
                py - 1,
                TFT_CYAN
            );

            heartSprite->drawLine(
                oldX + 2,
                oldY + 1,
                px + 2,
                py + 1,
                TFT_CYAN
            );

            heartSprite->drawLine(
                oldX - 2,
                oldY - 1,
                px - 2,
                py - 1,
                TFT_CYAN
            );
        }


        oldX = px;
        oldY = py;

        firstPoint = false;
    }


    // --------------------------------------------------------
    // MANE
    // --------------------------------------------------------

    heartSprite->setTextDatum(
        MC_DATUM
    );

    heartSprite->setTextFont(
        4
    );

    heartSprite->setTextColor(
        TFT_WHITE,
        TFT_BLACK
    );

    heartSprite->drawString(
        "Mane",
        150,
        178
    );

    heartSprite->setTextDatum(
        TL_DATUM
    );


    // --------------------------------------------------------
    // PUSH SPRITE TO TFT
    // --------------------------------------------------------

    heartSprite->pushSprite(
        90,
        60
    );
}