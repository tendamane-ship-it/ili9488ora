#include <Arduino.h>

namespace
{
constexpr uint8_t BACKLIGHT_PIN = 15;
constexpr uint8_t MAX_BRIGHTNESS = 255;
constexpr unsigned long DEMO_STEP_INTERVAL_MS = 700;
constexpr uint8_t DEMO_STEP = 32;

uint8_t brightness = 128;
uint8_t demoBrightness = 0;
int8_t demoDirection = 1;
bool demoRunning = false;
unsigned long lastDemoStep = 0;

void setBrightness(uint8_t value)
{
    brightness = value;
    analogWrite(BACKLIGHT_PIN, brightness);
    Serial.printf("Ndricimi: %u/255\n", brightness);
}

void printHelp()
{
    Serial.println("Komandat:");
    Serial.println("  0-255 : vendos ndricimin (0=fikur, 255=max)");
    Serial.println("  a     : nis/ndal ciklin automatik");
    Serial.println("  s     : ndal ciklin dhe mban ndricimin aktual");
    Serial.println("  ?     : shfaq kete ndihme");
}

void processCommand(char* command)
{
    while (*command == ' ' || *command == '\t')
        ++command;

    char* end = command + strlen(command);
    while (end > command && (end[-1] == ' ' || end[-1] == '\t'))
        *--end = '\0';

    if (*command == '\0')
        return;

    if (strcmp(command, "?") == 0)
    {
        printHelp();
        return;
    }

    if (strcmp(command, "a") == 0 || strcmp(command, "A") == 0)
    {
        demoRunning = !demoRunning;
        demoBrightness = 0;
        demoDirection = 1;
        lastDemoStep = millis();
        Serial.println(demoRunning ? "Cikli automatik u nis." : "Cikli automatik u ndal.");
        return;
    }

    if (strcmp(command, "s") == 0 || strcmp(command, "S") == 0)
    {
        demoRunning = false;
        Serial.println("Cikli automatik u ndal.");
        return;
    }

    char* numberEnd = nullptr;
    const long value = strtol(command, &numberEnd, 10);
    if (numberEnd == command || *numberEnd != '\0' || value < 0 || value > MAX_BRIGHTNESS)
    {
        Serial.println("Komande e pavlefshme. Shkruaj ? per ndihme.");
        return;
    }

    demoRunning = false;
    setBrightness(static_cast<uint8_t>(value));
}

void readSerialCommands()
{
    static char command[16];
    static size_t commandLength = 0;

    while (Serial.available() > 0)
    {
        const char input = static_cast<char>(Serial.read());
        if (input == '\r' || input == '\n')
        {
            if (commandLength > 0)
            {
                command[commandLength] = '\0';
                processCommand(command);
                commandLength = 0;
            }
        }
        else if (commandLength < sizeof(command) - 1)
        {
            command[commandLength++] = input;
        }
        else
        {
            commandLength = 0;
            Serial.println("Komanda eshte shume e gjate.");
        }
    }
}

void updateDemo()
{
    const unsigned long now = millis();
    if (!demoRunning || now - lastDemoStep < DEMO_STEP_INTERVAL_MS)
        return;

    lastDemoStep = now;
    const int next = static_cast<int>(demoBrightness) + demoDirection * DEMO_STEP;
    if (next >= MAX_BRIGHTNESS)
    {
        demoBrightness = MAX_BRIGHTNESS;
        demoDirection = -1;
    }
    else if (next <= 0)
    {
        demoBrightness = 0;
        demoDirection = 1;
    }
    else
    {
        demoBrightness = static_cast<uint8_t>(next);
    }

    setBrightness(demoBrightness);
}
}

void setup()
{
    Serial.begin(115200);
    pinMode(BACKLIGHT_PIN, OUTPUT);
    analogWrite(BACKLIGHT_PIN, brightness);
    delay(300);

    Serial.println("\nTest i ndricimit te pasem ne GPIO 15");
    printHelp();
    setBrightness(brightness);
}

void loop()
{
    readSerialCommands();
    updateDemo();
}
