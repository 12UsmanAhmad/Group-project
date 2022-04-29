#include "multimeter.h"
#include "Board_LED.h"
#include "M:\Year 2\Design construction and test\Group-project-main\PB_LCD_Drivers.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Begin interrupt stuff
static volatile uint32_t ticks = 0;
void SysTick_Handler(void)
{
	ticks++;
}

void waitForATime(uint32_t howLongToWait)
{
	uint32_t whenStarted = ticks;
	while (ticks - whenStarted < howLongToWait);
}
// End interrupt stuff

int convertADCValue(uint16_t value)
{
	// configures 4096 is the max value that will be given by ADC so it 
	//will divide by the value then times by the value to give a readable voltage
	value = (int) ((float) value / 4096) * 3;
	return value;
}

double convertADCValue2(uint32_t value) 
{
	// configures 4096 is the max value that will be given by ADC so it 
	//will divide by the value then times by the value to give a readable voltage
	double Val2 = 0;
	Val2 = ( (float) value / 4096) * 3.33;
	return Val2;
}

void AdcConfig(void)
{
	// Enable GPIO-C clock
	RCC->AHB1ENR = (RCC->AHB1ENR & ~RCC_AHB1ENR_GPIOCEN_Msk) | (0x1 << RCC_AHB1ENR_GPIOCEN_Pos); 
	// Enable ADC1 clock
	RCC->APB2ENR = (RCC->APB2ENR & ~RCC_APB2ENR_ADC1EN_Msk) | (0x1 << RCC_APB2ENR_ADC1EN_Pos);
	// Sets mode register 14 to analog mode
	GPIOC->MODER = (GPIOC->MODER & ~GPIO_MODER_MODER4_Msk) | (0x3 << GPIO_MODER_MODER4_Pos);
	// Set ADC to discontinuous mode
	ADC1->CR1 = (ADC1->CR1 & ~ADC_CR1_DISCEN_Msk) | (0x1 << ADC_CR1_DISCEN_Pos);
	// Sets end of conversion flag mode
	ADC1->CR2 = (ADC1->CR2 & ~ADC_CR2_EOCS_Msk) | (0x1 << ADC_CR2_EOCS_Pos);
	// Set ADC to perform one conversion before raising EOC flag
	ADC1->SQR1 = (ADC1->SQR1 & ~ADC_SQR1_L_Msk) | (0x0 << ADC_SQR1_L_Pos);
	// Set ADC to read input 14
	ADC1->SQR3 = (ADC1->SQR3 & ~ADC_SQR3_SQ1_Msk) | (0xE << ADC_SQR3_SQ1_Pos);
	// Enable ADC :D
	ADC1->CR2 = (ADC1->CR2 & ~ADC_CR2_ADON_Msk) | (0x1 << ADC_CR2_ADON_Pos);
}

void DacConfig(void)
{
	// Enable GPIO-A clock
	RCC->AHB1ENR = (RCC->AHB1ENR & ~RCC_AHB1ENR_GPIOAEN_Msk) | (0x1 << RCC_AHB1ENR_GPIOAEN_Pos);
	// Enable DAC1 clock
	RCC->APB1ENR = (RCC->APB1ENR & ~RCC_APB1ENR_DACEN_Msk) | (0x1 << RCC_APB1ENR_DACEN_Pos);
	// Sets mode register 14 to analog mode
	GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODER4_Msk) | (0x3 << GPIO_MODER_MODER4_Pos);
	// Enable channel one of the DAC
	DAC1->CR = (DAC->CR & ~DAC_CR_EN1_Msk) | (0x1 << DAC_CR_EN1_Pos);
}

void PinConfig(void)
{
	// Enable GPIO-E clock
	RCC->AHB1ENR = (RCC->AHB1ENR & ~RCC_AHB1ENR_GPIOEEN_Msk) | (0x1 << RCC_AHB1ENR_GPIOEEN_Pos);
}

void updatePinState(int pin, int state)
{
	// The state parameter will be used as a boolean (logic high, logic low) so simply convert it into a hex value
	int newState = state == 1 ? 0x1 : 0x0;
	switch(pin)
	{
		case 1: GPIOE->PUPDR = (GPIOE->PUPDR & ~GPIO_PUPDR_PUPD10_Msk) | (newState << GPIO_PUPDR_PUPD10_Pos); break;
		case 2: GPIOE->PUPDR = (GPIOE->PUPDR & ~GPIO_PUPDR_PUPD11_Msk) | (newState << GPIO_PUPDR_PUPD11_Pos); break;
		case 3: GPIOE->PUPDR = (GPIOE->PUPDR & ~GPIO_PUPDR_PUPD12_Msk) | (newState << GPIO_PUPDR_PUPD12_Pos); break;
		case 4: GPIOE->PUPDR = (GPIOE->PUPDR & ~GPIO_PUPDR_PUPD13_Msk) | (newState << GPIO_PUPDR_PUPD13_Pos); break;
		case 5: GPIOE->PUPDR = (GPIOE->PUPDR & ~GPIO_PUPDR_PUPD14_Msk) | (newState << GPIO_PUPDR_PUPD14_Pos); break;
		case 6: GPIOE->PUPDR = (GPIOE->PUPDR & ~GPIO_PUPDR_PUPD15_Msk) | (newState << GPIO_PUPDR_PUPD15_Pos); break;
		default: break;
	}
}

double getVoltage(int scale)
{
	double voltage;

	while (true)
	{
		// Get the voltage depending on the scale
		if (scale == 10) voltage = getVoltage10();
		
		// Wait 1 second between readings
		waitForATime(1);

		// Create a readable string to display on LCD
		PB_LCD_Clear();
		char VoltageValue[8];
		strcpy(VoltageValue, "00000000");
		snprintf(VoltageValue, 8, "%0.3f", voltage);

		// Try out adding "Voltage: " to the start
		char FinalText[16] = "Voltage: ";
		strcat(FinalText, VoltageValue);
		
		// Write to LCD
		PB_LCD_WriteString(FinalText, (int) strlen(FinalText));
	}
	
	return -1.0;
}

double getVoltage10(void)
{
	// Declare internal variables to store readings
	uint32_t AdcValue;
	double AdcFinal;
	bool finished = false;

	// Starts conversion of 'standard' channels
	ADC1->CR2 = (ADC1->CR2 & ~ADC_CR2_SWSTART_Msk) | (0x1 << ADC_CR2_SWSTART_Pos);
	while (!finished)
	{
		// Checks if end of conversion flag has been set
		if ((ADC1->SR & ADC_SR_EOC_Msk) == ADC_SR_EOC_Msk) 
		{
			// Retrieve converted value from data register
			AdcValue = ADC1->DR;
			
			// Converts from internal numerical value to actual value
			AdcFinal = convertADCValue2(AdcValue);

			// Mark conversion as finished
			finished = true;
		}
	}

	// Return final value
	return AdcFinal;
}

int Res_values(int AddP1, int AddP2, int AddP3)
{
	if (AddP1 == 0 && AddP2 == 0 && AddP3 == 0) return 10;
	else if (!AddP1 && !AddP2 && AddP3) return 1E2;
	else if (!AddP1 && AddP2 && !AddP3) return 1E3;
	else if (!AddP1 && AddP2 && AddP3) return 1E4;
	else if (AddP1 && !AddP2 && !AddP3) return 1E5;
	else if (AddP1 && AddP2 && !AddP3) return 1E6;
	else return INF;
}

void setup()
{
	// Initialise board
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 2);
	
	// Initialise LCD
	PB_LCD_Init();
	PB_LCD_Clear();	

	// Initialise ADC
	AdcConfig();
	DacConfig();
	PinConfig();
	
	// Write 1.5V to DAC1 at pin PA4
	DAC1->DHR12R1 = (DAC1->DHR12R1 & ~DAC_DHR12R1_DACC1DHR_Msk) | (/* 1.5 volts */ 0x73B << DAC_DHR12R1_DACC1DHR_Pos);
}

int main(void)
{
	setup();
	getVoltage(10);
}
