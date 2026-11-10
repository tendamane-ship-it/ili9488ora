#include "ScreenSaver.h"
#include "SerialLog.h"
#include "RuntimePolicies.h"
#include <new>

static TFT_eSPI* tftDisplay = nullptr;

static TFT_eSprite* heartSprite = nullptr;
static bool heartSpriteReady = false;
static runtime_policies::RamProtection memoryProtection;
static bool spriteMemoryWarningLogged = false;

static unsigned long lastHeartFrame = 0;
static float heartPhase = 0.0;

static constexpr size_t HEART_SPRITE_BYTES = 300UL * 320UL;
static constexpr size_t HEART_SPRITE_RESERVE_BYTES =
    runtime_policies::RAM_PROTECTION_LOW_BYTES +
    HEART_SPRITE_BYTES + 8UL * 1024UL;

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

    heartSprite = new (std::nothrow) TFT_eSprite(tftDisplay);
    if (heartSprite == nullptr)
        Serial.println("SCREEN SAVER: heart sprite object allocation failed");
}

bool monitorScreenSaverMemory()
{
    const size_t freeHeap = ESP.getFreeHeap();
    const bool wasProtected = memoryProtection.active();
    const bool isProtected = memoryProtection.update(freeHeap);

    if (!wasProtected && isProtected)
    {
        Serial.printf(
            "RAM PROTECTION: ACTIVE; free heap %u KB is below 100 KB\n",
            static_cast<unsigned int>(freeHeap / 1024)
        );

        if (heartSprite != nullptr && heartSpriteReady)
        {
            heartSprite->deleteSprite();
            heartSpriteReady = false;
        }
    }
    else if (wasProtected && !isProtected)
    {
        Serial.printf(
            "RAM PROTECTION: INACTIVE; free heap recovered to %u KB\n",
            static_cast<unsigned int>(freeHeap / 1024)
        );
    }

    return isProtected;
}


// ============================================================
// UPDATE SCREEN SAVER
// ============================================================

bool updateScreenSaver(bool active)
{
    if (!active || tftDisplay == nullptr || heartSprite == nullptr ||
        memoryProtection.active())
        return false;


    // --------------------------------------------------------
    // INITIALIZE SPRITE
    // --------------------------------------------------------

    if (!heartSpriteReady)
    {
        const size_t freeHeap = ESP.getFreeHeap();
        if (freeHeap < HEART_SPRITE_RESERVE_BYTES)
        {
            if (!spriteMemoryWarningLogged)
            {
                Serial.printf(
                    "SCREEN SAVER: skipped; %u KB free, need at least %u KB "
                    "to keep 100 KB in reserve\n",
                    static_cast<unsigned int>(freeHeap / 1024),
                    static_cast<unsigned int>(
                        HEART_SPRITE_RESERVE_BYTES / 1024
                    )
                );
                spriteMemoryWarningLogged = true;
            }
            return false;
        }

        Serial.println("SCREEN SAVER: creating heart sprite...");
        heartSprite->setColorDepth(8);

        if (heartSprite->createSprite(300, 320))
        {
            heartSpriteReady = true;
            spriteMemoryWarningLogged = false;

            Serial.println("SCREEN SAVER: heart sprite CREATED");

            heartPhase = 0.0;
            lastHeartFrame = millis();

            if (monitorScreenSaverMemory())
                return false;
        }
        else
        {
            if (!spriteMemoryWarningLogged)
            {
                Serial.printf(
                    "SCREEN SAVER: heart sprite allocation failed; "
                    "free heap %u KB; continuing without screensaver\n",
                    static_cast<unsigned int>(ESP.getFreeHeap() / 1024)
                );
                spriteMemoryWarningLogged = true;
            }
            return false;
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
        return true;

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

    return true;
}