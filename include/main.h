
#define PI 3.141592653589793

#define DISPLAY_GPIO GPIOA
#define MOSI GPIO7
#define MISO GPIO6
#define SCK GPIO5
#define CS GPIO0
#define DC GPIO1
#define RS GPIO2

#define DISPLAY_WIDTH 135
#define DISPLAY_HEIGHT 240

//#define DISPLAY_WIDTH 128
//#define DISPLAY_HEIGHT 160
// Umfang des Displayspeichers
#define DISPLAY_MEMORY_WIDTH 240
#define DISPLAY_MEMORY_HEIGHT 320

//#define DISPLAY_MEMORY_WIDTH 128
//#define DISPLAY_MEMORY_HEIGHT 160

#define DISPLAY_Y_OFFSET 40
#define DISPLAY_X_OFFSET 52

//#define DISPLAY_Y_OFFSET 0
//#define DISPLAY_X_OFFSET 0

#define DISPLAY_COLOR_DEPTH 18

#define DISPLAY_PANEL_RGB_BGR 1 // 0 = RGB, 1 = BGR
#define DISPLAY_INVERSION 1

#define DISPLAY_ROTATION 1 // 1 = Normal, 2 = 90° im Uhrzeigersinn, 3 = 180°, 4 = 90° gegen den Uhrzeigersinn

/*
#if DISPLAY_ROTATION == 1
    #define DISPLAY_Y_OFFSET DISPLAY_MEMORY_HEIGHT - DISPLAY_HEIGHT
    #define DISPLAY_X_OFFSET 0
#elif DISPLAY_ROTATION == 2
    #define DISPLAY_Y_OFFSET DISPLAY_MEMORY_WIDTH - DISPLAY_WIDTH
    #define DISPLAY_X_OFFSET DISPLAY_MEMORY_HEIGHT - DISPLAY_HEIGHT
#elif DISPLAY_ROTATION == 3
    #define DISPLAY_Y_OFFSET 0
    #define DISPLAY_X_OFFSET DISPLAY_MEMORY_WIDTH - DISPLAY_WIDTH
#elif DISPLAY_ROTATION == 4
    #define DISPLAY_Y_OFFSET 0
    #define DISPLAY_X_OFFSET 0
#endif
*/


#define TIMER TIM3

void delayMs(uint16_t ms);
void beginTransmissionSpi(void);
void endTransmissionSpi(void);
void sendCommand(uint8_t command);
void sendData(uint8_t data);
void sendData16(uint16_t dataWord);
void setRotation(uint8_t rotation);
void setMemoryWriteWindow(uint16_t xStart, uint16_t xEnd, uint16_t yStart, uint16_t yEnd, uint8_t xOffset, uint8_t yOffset);
void writeMemoryStart(void);
void writePixel(uint8_t redByte, uint8_t greenByte, uint8_t blueByte);
void fillMemory(uint16_t xEnd, uint16_t yEnd, uint8_t redByte, uint8_t greenByte, uint8_t blueByte);
void setPixel(uint16_t xWord,  uint16_t yWord, uint8_t redByte, uint8_t greenByte, uint8_t blueByte);
void setVerticalScrolling(uint16_t topFixedArea, uint16_t bottomFixedArea);
void setVericalScrollingStartAddress(uint16_t startAddress);
void drawEllipse(int xm, int ym, int a, int b, uint8_t redByte, uint8_t greenByte, uint8_t blueByte);
void clearScreen(void);
void demoDisplay(void);
//void sys_tick_handler(void);
