#include "Sensors.h"

// ------------------------------------------------------------
// SENSOR OBJECTS
// ------------------------------------------------------------

Adafruit_BME680 bme;
bool bme680OK = false;

BH1750 lightMeter;
bool bh1750OK = false;

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
        if (bme.performReading())
        {
            bmeTemp = bme.temperature;
            bmeHumidity = bme.humidity;
            bmePressure = bme.pressure / 100.0;
            bmeGas = bme.gas_resistance / 1000.0;

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

        Serial.print("BH1750 Lux: ");
        Serial.print(bh1750Lux);
        Serial.println(" lx");
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
        if (bme.performReading())
        {
            bmeTemp = bme.temperature;
            bmeHumidity = bme.humidity;
            bmePressure = bme.pressure / 100.0;
            bmeGas = bme.gas_resistance / 1000.0;
        }
    }

    if (bh1750OK)
    {
        bh1750Lux = lightMeter.readLightLevel();
    }

    updateAirQuality();
}

// ------------------------------------------------------------
// AIR QUALITY
// ------------------------------------------------------------

void updateAirQuality()
{
    if (!bme680OK || bmeGas <= 0)
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