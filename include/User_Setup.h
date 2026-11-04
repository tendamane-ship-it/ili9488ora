// ============================================
// TFT_eSPI - ILI9488 configuration
// ESP32-S3 DevKitC-1
// ============================================

#define ILI9488_DRIVER

// --------------------------------------------
// Display resolution
// --------------------------------------------

#define TFT_WIDTH  320
#define TFT_HEIGHT 480

// --------------------------------------------
// SPI pins
// --------------------------------------------

#define TFT_MOSI 7
#define TFT_MISO 5
#define TFT_SCLK 4

// --------------------------------------------
// TFT control pins
// --------------------------------------------

#define TFT_CS   6
#define TFT_DC 2
#define TFT_RST  3

#define USE_FSPI_PORT

// --------------------------------------------
// SPI speed
// --------------------------------------------

#define SPI_FREQUENCY 27000000

// --------------------------------------------
// XPT2046 Touch
// --------------------------------------------

#define TOUCH_CS 17
#define TOUCH_IRQ 14
#define SPI_TOUCH_FREQUENCY 2500000

// --------------------------------------------
// Fonts
// --------------------------------------------

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF