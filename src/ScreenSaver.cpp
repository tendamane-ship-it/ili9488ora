#include "ScreenSaver.h"
#include "SerialLog.h"

static TFT_eSPI* tftDisplay = nullptr;

static TFT_eSprite* heartSprite = nullptr;
static bool heartSpriteReady = false;

static unsigned long lastHeartFrame = 0;
static float heartPhase = 0.0;

enum HeartDrawStage
{
    HEART_STABLE,
    HEART_DRAWING_RED
};

static HeartDrawStage heartDrawStage = HEART_STABLE;
static float heartBaseScale = 4.0;
static float heartRedScale = 4.0;
static int heartDirection = 0;
static float heartTraceProgress = 0.0;
static constexpr float HEART_FULL_TURN = 6.2831853f;
static constexpr float HEART_REST_SCALE = 4.0f;
static constexpr float HEART_FIRST_PEAK_SCALE = 8.8f;
static constexpr float HEART_SECOND_PEAK_SCALE = 6.8f;
static constexpr float HEART_SCALE_STEP = 0.8f;
static constexpr float HEART_TRACE_STEP = 4.8f;

static void drawHeartArc(
    float scale,
    uint16_t color,
    float startAngle,
    float endAngle
)
{
    const int cx = 150;
    const int cy = 135;
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
    uint16_t color,
    float progress
)
{
    const float endAngle = HEART_FULL_TURN * progress;
    drawHeartArc(scale, color, 0.0f, endAngle);
    drawHeartArc(
        scale,
        color,
        HEART_FULL_TURN,
        HEART_FULL_TURN - endAngle
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
            heartDrawStage = HEART_STABLE;
            heartBaseScale = HEART_REST_SCALE;
            heartRedScale = heartBaseScale;
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

    float cycle =
        fmod(
            heartPhase,
            TWO_PI
        );

    float targetScale = HEART_REST_SCALE;

    if (cycle < 3.20)
    {
        float p =
            sin(
                (cycle / 3.20) * PI
            );

        targetScale =
            HEART_REST_SCALE
            + (HEART_FIRST_PEAK_SCALE - HEART_REST_SCALE) * p;
    }

    else if (
        cycle >= 3.80 &&
        cycle < 5.20
    )
    {
        float p =
            sin(
                ((cycle - 3.80) / 1.40) * PI
            );

        targetScale =
            HEART_REST_SCALE
            + (HEART_SECOND_PEAK_SCALE - HEART_REST_SCALE) * p;
    }


    if (heartDrawStage == HEART_STABLE)
    {
        float scaleDifference = targetScale - heartBaseScale;
        if (fabs(scaleDifference) > 0.05)
        {
            heartDirection = scaleDifference > 0 ? 1 : -1;
            float distance = fabs(scaleDifference);
            float redStep = min(HEART_SCALE_STEP, distance);
            heartRedScale =
                heartBaseScale + heartDirection * redStep;
            heartTraceProgress = 0.0;
            heartDrawStage = HEART_DRAWING_RED;
        }
    }

    if (heartDrawStage != HEART_STABLE)
    {
        heartTraceProgress = min(
            heartTraceProgress + HEART_TRACE_STEP,
            HEART_FULL_TURN
        );

        if (heartTraceProgress >= HEART_FULL_TURN)
        {
            heartTraceProgress = 0.0;
            heartBaseScale = heartRedScale;
            heartDrawStage = HEART_STABLE;
        }
    }

    heartSprite->fillSprite(TFT_BLACK);

    if (heartDrawStage == HEART_DRAWING_RED)
    {
        drawHeartOutline(heartBaseScale, TFT_RED, 1.0);
        drawHeartOutline(
            heartRedScale,
            TFT_RED,
            heartTraceProgress / HEART_FULL_TURN
        );
    }
    else
    {
        drawHeartOutline(heartBaseScale, TFT_RED, 1.0);
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
        305
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