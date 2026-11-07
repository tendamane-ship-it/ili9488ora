#include "ScreenSaver.h"
#include "SerialLog.h"

static TFT_eSPI* tftDisplay = nullptr;

static TFT_eSprite* heartSprite = nullptr;
static bool heartSpriteReady = false;

static unsigned long lastHeartFrame = 0;
static float heartPhase = 0.0;

static constexpr float HEART_FULL_TURN = 6.2831853f;
static constexpr float HEART_MIN_SCALE = 4.0f;
static constexpr float HEART_MAX_SCALE = 9.0f;

static void drawHeartArc(
    float scale,
    uint16_t color,
    float startAngle,
    float endAngle
)
{
    const int cx = 150;
    const int cy = 160;
    const float angleStep = startAngle <= endAngle ? 0.015f : -0.015f;

    bool firstPoint = true;
    int oldX = 0;
    int oldY = 0;

    for (float t = startAngle;
         angleStep > 0 ? t <= endAngle : t >= endAngle;
         t += angleStep)
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

        int px = cx + static_cast<int>(x * scale);
        int py = cy - static_cast<int>(y * scale);

        if (!firstPoint)
        {
            heartSprite->drawLine(oldX, oldY, px, py, color);
            heartSprite->drawLine(oldX + 1, oldY, px + 1, py, color);
            heartSprite->drawLine(oldX - 1, oldY, px - 1, py, color);
            heartSprite->drawLine(oldX, oldY + 1, px, py + 1, color);
            heartSprite->drawLine(oldX, oldY - 1, px, py - 1, color);
            heartSprite->drawLine(oldX + 2, oldY + 1, px + 2, py + 1, color);
            heartSprite->drawLine(oldX - 2, oldY - 1, px - 2, py - 1, color);
        }

        oldX = px;
        oldY = py;
        firstPoint = false;
    }
}

static void drawHeartOutline(
    float scale,
    uint16_t color
)
{
    const float halfTurn = HEART_FULL_TURN / 2.0f;
    drawHeartArc(scale, color, 0.0f, halfTurn);
    drawHeartArc(
        scale,
        color,
        HEART_FULL_TURN,
        halfTurn
    );
}


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

        if (heartSprite->createSprite(300, 320))
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

    heartPhase += 0.60;

    float scaleProgress =
        (1.0f - cos(heartPhase)) / 2.0f;
    float heartScale =
        HEART_MIN_SCALE
        + (HEART_MAX_SCALE - HEART_MIN_SCALE) * scaleProgress;

    heartSprite->fillSprite(TFT_BLACK);
    drawHeartOutline(heartScale, TFT_RED);


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
        160
    );

    heartSprite->setTextDatum(
        TL_DATUM
    );


    // --------------------------------------------------------
    // PUSH SPRITE TO TFT
    // --------------------------------------------------------

    heartSprite->pushSprite(
        90,
        0
    );
}