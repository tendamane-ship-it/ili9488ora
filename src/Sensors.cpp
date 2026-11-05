#include "Sensors.h"
#include "SerialLog.h"

// ------------------------------------------------------------
// SENSOR OBJECTS
// ------------------------------------------------------------

Adafruit_BME680 bme;
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

float airBaseline = 0.0;
bool airBaselineReady = false;

float airBaselineSum = 0.0;
int airBaselineCount = 0;

const int AIR_BASELINE_SAMPLES = 30;

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
    // BME680
    // --------------------------------------------------------

    if (bme.begin(0x76))
    {
        bme680OK = true;
        Serial.println("BME680 found at 0x76");
    }
    else if (bme.begin(0x77))
    {
        bme680OK = true;
        Serial.println("BME680 found at 0x77");
    }
    else
    {
        Serial.println("BME680 NOT FOUND");
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
    // INITIAL SENSOR READING
    // --------------------------------------------------------

    if (bme680OK)
    {
        bmeReadingOK = bme.performReading();
        if (bmeReadingOK)
        {
            bmeTemp = bme.temperature;
            bmeHumidity = bme.humidity;
            bmePressure = bme.pressure / 100.0;
            bmeGas = bme.gas_resistance / 1000.0;
            lastBmeReading = millis();

            Serial.print("BME680 Temperature: ");
            Serial.print(bmeTemp);
            Serial.println(" C");

            Serial.print("BME680 Humidity: ");
            Serial.print(bmeHumidity);
            Serial.println(" %");

            Serial.print("BME680 Pressure: ");
            Serial.print(bmePressure);
            Serial.println(" hPa");

            Serial.print("BME680 Gas: ");
            Serial.print(bmeGas);
            Serial.println(" kOhm");
        }
        else
        {
            Serial.println("BME680 reading FAILED");
        }
    }

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
    if (bme680OK)
    {
        bmeReadingOK = bme.performReading();
        if (bmeReadingOK)
        {
            bmeTemp = bme.temperature;
            bmeHumidity = bme.humidity;
            bmePressure = bme.pressure / 100.0;
            bmeGas = bme.gas_resistance / 1000.0;
            lastBmeReading = millis();
        }
    }

    if (bh1750OK)
    {
        bh1750Lux = lightMeter.readLightLevel();
        bh1750ReadingOK = bh1750Lux >= 0.0f;
        if (bh1750ReadingOK)
            lastLuxReading = millis();
    }

    updateAirQuality();
}

// ------------------------------------------------------------
// AIR QUALITY
// ------------------------------------------------------------

void updateAirQuality()
{
    if (!bmeReadingOK || bmeGas <= 0)
        return;

    if (!airBaselineReady)
    {
        airBaselineSum += bmeGas;
        airBaselineCount++;

        if (airBaselineCount >= AIR_BASELINE_SAMPLES)
        {
            airBaseline =
                airBaselineSum / AIR_BASELINE_SAMPLES;

            airBaselineReady = true;

            Serial.print("Air baseline: ");
            Serial.print(airBaseline, 2);
            Serial.println(" kOhm");
        }
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