// ============================================
// Konfigurimi i TFT_eSPI – ILI9488
// ESP32-S3 DevKitC-1
// ============================================

#define ILI9488_DRIVER

// --------------------------------------------
// Rezolucioni i ekranit
// --------------------------------------------

#define TFT_WIDTH  320
#define TFT_HEIGHT 480

// --------------------------------------------
// Kunjat e SPI-së
// --------------------------------------------

#define TFT_MOSI 7
#define TFT_MISO 5
#define TFT_SCLK 4

// --------------------------------------------
// Kunjat e kontrollit të TFT-së
// --------------------------------------------

#define TFT_CS   6
#define TFT_DC 2
#define TFT_RST  3

#define USE_FSPI_PORT

// --------------------------------------------
// Shpejtësia e SPI-së
// --------------------------------------------

#define SPI_FREQUENCY 27000000

// --------------------------------------------
// Prekja XPT2046
// --------------------------------------------

#define TOUCH_CS 17
#define TOUCH_IRQ 14
#define SPI_TOUCH_FREQUENCY 2500000

// --------------------------------------------
// Shkrimet
// --------------------------------------------

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF