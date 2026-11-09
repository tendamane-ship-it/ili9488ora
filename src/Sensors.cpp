#include "Sensors.h"
#include "SerialLog.h"
#include "settings.h"

// ------------------------------------------------------------
// SENSOR OBJECTS
// ------------------------------------------------------------

static Bsec2 bsec;
static uint8_t bsecMemory[BSEC_INSTANCE_SIZE];
static const uint8_t bsecConfig[] = {
#include <config/bme680/bme680_iaq_33v_3s_28d/bsec_iaq.txt>
};
static uint8_t bmeI2cAddress = 0;
bool bme680OK = false;
bool bmeReadingOK = false;
unsigned long lastBmeReading = 0;

BH1750 lightMeter;
bool bh1750OK = false;
bool bh1750ReadingOK = false;
unsigned long lastLuxReading = 0;

// ------------------------------------------------------------
// SENSOR VALUES
// ------------------------------------------------------------

float bmeTemp = 0.0;
float bmeHumidity = 0.0;
float bmePressure = 0.0;
float bmeGas = 0.0;
float bh1750Lux = 0.0;

// ------------------------------------------------------------
// AIR QUALITY
// ------------------------------------------------------------

float bmeIaq = 0.0f;
bool bmeIaqReady = false;
uint8_t bmeIaqAccuracy = 0;
int8_t bmeIaqStabilizationStatus = -1;
int8_t bmeIaqRunInStatus = -1;

static constexpr unsigned long BSEC_STATE_SAVE_INTERVAL_MS =
    6UL * 60UL * 60UL * 1000UL;
static constexpr unsigned long BSEC_DEBUG_INTERVAL_MS = 5000UL;
static unsigned long lastBsecStateSave = 0;
static unsigned long lastBsecDebug = 0;
static unsigned long lastBsecErrorLog = 0;
static bool bsecStateSaved = false;

static uint32_t bsecVersionCode()
{
    return (static_cast<uint32_t>(bsec.version.major) << 24) |
        (static_cast<uint32_t>(bsec.version.minor) << 16) |
        (static_cast<uint32_t>(bsec.version.major_bugfix) << 8) |
        static_cast<uint32_t>(bsec.version.minor_bugfix);
}

static bool loadBsecState()
{
    if (preferences.getUInt("bsecVer", 0) != bsecVersionCode())
    {
        Serial.println("BSEC state missing or from a different version");
        return false;
    }

    if (preferences.getBytesLength("bsecState") != BSEC_MAX_STATE_BLOB_SIZE)
    {
        Serial.println("BSEC state missing or has an invalid size");
        return false;
    }

    uint8_t state[BSEC_MAX_STATE_BLOB_SIZE];
    if (preferences.getBytes("bsecState", state, sizeof(state)) != sizeof(state))
    {
        Serial.println("BSEC state read from NVS failed");
        return false;
    }

    if (!bsec.setState(state))
    {
        Serial.printf("BSEC state restore failed: %d\n", bsec.status);
        return false;
    }

    Serial.println("BSEC state restored from NVS");
    bsecStateSaved = true;
    return true;
}

static bool saveBsecState()
{
    uint8_t state[BSEC_MAX_STATE_BLOB_SIZE];
    if (!bsec.getState(state))
    {
        Serial.printf("BSEC state export failed: %d\n", bsec.status);
        return false;
    }

    if (preferences.putBytes("bsecState", state, sizeof(state)) != sizeof(state))
    {
        Serial.println("BSEC state write to NVS failed");
        return false;
    }

    if (preferences.putUInt("bsecVer", bsecVersionCode()) != sizeof(uint32_t))
    {
        Serial.println("BSEC version write to NVS failed");
        return false;
    }

    lastBsecStateSave = millis();
    bsecStateSaved = true;
    Serial.println("BSEC state saved to NVS");
    return true;
}

static void bsecDataCallback(
    const bme68xData data,
    const bsecOutputs outputs,
    const Bsec2
)
{
    if (!(data.status & BME68X_GASM_VALID_MSK))
        return;

    bmeTemp = data.temperature;
    bmeHumidity = data.humidity;
    bmePressure = data.pressure;
    bmeGas = data.gas_resistance / 1000.0f;
    bmeReadingOK = true;
    lastBmeReading = millis();

    for (uint8_t i = 0; i < outputs.nOutputs; i++)
    {
        const bsecData& output = outputs.output[i];
        switch (output.sensor_id)
        {
            case BSEC_OUTPUT_IAQ:
                bmeIaq = output.signal;
                bmeIaqAccuracy = output.accuracy;
                bmeIaqReady = true;
                break;
            case BSEC_OUTPUT_STABILIZATION_STATUS:
                bmeIaqStabilizationStatus =
                    output.signal >= 0.5f ? 1 : 0;
                break;
            case BSEC_OUTPUT_RUN_IN_STATUS:
                bmeIaqRunInStatus = output.signal >= 0.5f ? 1 : 0;
                break;
            default:
                break;
        }
    }

    const unsigned long now = millis();
    if (bmeIaqReady &&
        ((!bsecStateSaved && bmeIaqAccuracy > 0) ||
         now - lastBsecStateSave >= BSEC_STATE_SAVE_INTERVAL_MS))
    {
        saveBsecState();
    }
}

// ------------------------------------------------------------
// SENSOR TIMING
// ------------------------------------------------------------

unsigned long lastSensorRead = 0;

const unsigned long SENSOR_READ_INTERVAL = 5000;

// ------------------------------------------------------------
// INITIALIZE SENSORS
// ------------------------------------------------------------

void initSensors()
{
    // --------------------------------------------------------
    // I2C
    // --------------------------------------------------------

    Wire.begin(8, 9);

    // --------------------------------------------------------
    // BME680 and BSEC2
    // --------------------------------------------------------

    bsec.allocateMemory(bsecMemory);
    bool bsecStarted = bsec.begin(0x76, Wire);
    if (bsecStarted)
        bmeI2cAddress = 0x76;
    if (!bsecStarted)
    {
        bsecStarted = bsec.begin(0x77, Wire);
        if (bsecStarted)
        {
            bmeI2cAddress = 0x77;
            Serial.println("BME680/BSEC2 found at 0x77");
        }
    }

    if (bsecStarted)
    {
        if (!bsec.setConfig(bsecConfig))
        {
            Serial.printf("BSEC configuration failed: %d\n", bsec.status);
        }
        else
        {
            loadBsecState();

            bsecSensor sensorList[] = {
                BSEC_OUTPUT_IAQ,
                BSEC_OUTPUT_RAW_TEMPERATURE,
                BSEC_OUTPUT_RAW_PRESSURE,
                BSEC_OUTPUT_RAW_HUMIDITY,
                BSEC_OUTPUT_RAW_GAS,
                BSEC_OUTPUT_STABILIZATION_STATUS,
                BSEC_OUTPUT_RUN_IN_STATUS
            };

            if (bsec.updateSubscription(
                    sensorList,
                    ARRAY_LEN(sensorList),
                    BSEC_SAMPLE_RATE_LP))
            {
                bsec.attachCallback(bsecDataCallback);
                bme680OK = true;
                Serial.printf(
                    "BME680/BSEC2 ready at 0x%02X; BSEC %u.%u.%u.%u\n",
                    bmeI2cAddress,
                    bsec.version.major,
                    bsec.version.minor,
                    bsec.version.major_bugfix,
                    bsec.version.minor_bugfix
                );
            }
            else
            {
                Serial.printf(
                    "BSEC output subscription failed: %d\n",
                    bsec.status
                );
            }
        }
    }
    else
    {
        Serial.printf(
            "BME680/BSEC2 initialization failed; BSEC=%d, BME68x=%d\n",
            bsec.status,
            bsec.sensor.status
        );
    }

    // --------------------------------------------------------
    // BH1750
    // --------------------------------------------------------

    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE))
    {
        bh1750OK = true;
        Serial.println("BH1750 found at 0x23");
    }
    else
    {
        Serial.println("BH1750 NOT FOUND");
    }

    // --------------------------------------------------------
    if (bh1750OK)
    {
        bh1750Lux = lightMeter.readLightLevel();
        bh1750ReadingOK = bh1750Lux >= 0.0f;
        if (bh1750ReadingOK)
        {
            lastLuxReading = millis();
            Serial.print("BH1750 Lux: ");
            Serial.print(bh1750Lux);
            Serial.println(" lx");
        }
        else
        {
            Serial.println("BH1750 reading FAILED");
        }
    }

    Serial.println("========================");
}

// ------------------------------------------------------------
// UPDATE LOCAL SENSORS
// ------------------------------------------------------------

void updateLocalSensors()
{
    if (bh1750OK)
    {
        bh1750Lux = lightMeter.readLightLevel();
        bh1750ReadingOK = bh1750Lux >= 0.0f;
        if (bh1750ReadingOK)
            lastLuxReading = millis();
    }

}

// ------------------------------------------------------------
// BSEC2 PROCESSING
// ------------------------------------------------------------

void runBsec()
{
    if (!bme680OK)
        return;

    if (!bsec.run())
    {
        bmeReadingOK = false;
        const unsigned long now = millis();
        if (now - lastBsecErrorLog >= BSEC_DEBUG_INTERVAL_MS)
        {
            lastBsecErrorLog = now;
            Serial.printf(
                "BSEC run failed: BSEC=%d, BME68x=%d\n",
                bsec.status,
                bsec.sensor.status
            );
        }
    }

    const unsigned long now = millis();
    if (now - lastBsecDebug >= BSEC_DEBUG_INTERVAL_MS)
    {
        lastBsecDebug = now;
        Serial.print("Cilesia e ajrit (IAQ): ");
        if (bmeIaqReady)
        {
            Serial.print(bmeIaq, 1);
            Serial.print(", saktesia: ");
            Serial.print(bmeIaqAccuracy);
            Serial.print(" (");
            Serial.print(bmeIaqAccuracyLabel());
            Serial.print("), kategoria: ");
            Serial.print(airQualityCategory());
        }
        else
        {
            Serial.print("po kalibrohet");
        }
        Serial.print(", gazi: ");
        if (bmeReadingOK)
            Serial.print(bmeGas, 2);
        else
            Serial.print("N/A");
        Serial.println(" kOhm");
    }
}

const char* airQualityCategory()
{
    if (!bmeIaqReady || bmeIaqAccuracy == 0)
        return "Po kalibrohet";
    if (bmeIaq <= 50.0f)
        return "Ajri i pastër";
    if (bmeIaq <= 100.0f)
        return "Ajri mesatar";
    if (bmeIaq <= 150.0f)
        return "Pak i ndotur";
    if (bmeIaq <= 200.0f)
        return "I ndotur";
    if (bmeIaq <= 300.0f)
        return "Shume i ndotur";
    return "Teper i ndotur";
}

const char* bmeIaqAccuracyLabel()
{
    switch (bmeIaqAccuracy)
    {
        case 1: return "E ulët";
        case 2: return "Mesatare";
        case 3: return "E lartë";
        default: return "E pasigurt";
    }
}

// ------------------------------------------------------------
// PIR
// ------------------------------------------------------------

bool pirLastState = LOW;

unsigned long pirIgnoreUntil = 0;

int pirStableState = LOW;
int pirCandidateState = LOW;

unsigned long pirStateChangeTime = 0;

const unsigned long PIR_DEBOUNCE_TIME = 150;

// ------------------------------------------------------------
// INITIALIZE PIR
// ------------------------------------------------------------

void initPIR()
{
    pinMode(PIR_PIN, INPUT);

    Serial.println("PIR initialized on GPIO 42");
}

// ------------------------------------------------------------
// READ PIR
// ------------------------------------------------------------

int readPIR()
{
    int pirState = digitalRead(PIR_PIN);

    if (pirState != pirCandidateState)
    {
        pirCandidateState = pirState;
        pirStateChangeTime = millis();
    }

    if (pirCandidateState != pirStableState &&
        millis() - pirStateChangeTime >= PIR_DEBOUNCE_TIME)
    {
        pirStableState = pirCandidateState;
    }

    return pirStableState;
}