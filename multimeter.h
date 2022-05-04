#include "stm32f407xx.h"

#define INF 1874949133

// Constants
static const uint32_t LOW_LIMIT = 425;
static const uint32_t HIGH_LIMIT = 575;

void SysTick_Handler(void);
void waitForATime(uint32_t howLongToWait);

int convertADCValue(uint16_t value);
double convertADCValue2(uint32_t value);

// Configures ADC to read input voltage
void AdcConfig(void);
// Configures DAC to output voltage
void DacConfig(void);
// Configures pins to be set to logic high or low
void PinConfig(void);
// Configures input registers
void InpConfig(void);

// Main program method
void setup(void);

void updatePinState(int pin, int state);

int isButtonPressed();

double getVoltage(int scale);
double getVoltage10(void);
