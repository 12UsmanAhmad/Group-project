#include "stm32f407xx.h"

#define INF 1874949133

// Constants
static const uint32_t LOW_LIMIT = 425;
static const uint32_t HIGH_LIMIT = 575;

void SysTick_Handler(void);
void waitForATime(uint32_t howLongToWait);

int convertADCValue(uint16_t value);
double convertADCValue2(uint32_t value);

void AdcConfig(void);
void DacConfig(void);
void PinConfig(void);

void setup(void);

double getVoltage(int scale);
double getVoltage10(void);
