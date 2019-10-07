/*---------------------------------------------------------------------------------
	$Id: template.c,v 1.2 2005/09/07 20:06:06 wntrmute Exp $

	Simple ARM7 stub (sends RTC, TSC, and X/Y data to the ARM 9)

	$Log: template.c,v $
	Revision 1.2  2005/09/07 20:06:06  wntrmute
	updated for latest libnds changes

	Revision 1.8  2005/08/03 05:13:16  wntrmute
	corrected sound code


---------------------------------------------------------------------------------*/
#include <nds.h>

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
	u32 temp=0;
	if (IPC->mailData == 0xDEADC0DE) {
		SerialWaitBusy();
		REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER | SPI_BAUD_1MHz | SPI_CONTINUOUS;
		REG_SPIDATA = 0x80;
		SerialWaitBusy();
		REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER | SPI_BAUD_1MHz;
		REG_SPIDATA = 0;
		SerialWaitBusy();
		temp = REG_SPIDATA & 0xFF;

		REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER | SPI_BAUD_1MHz | SPI_CONTINUOUS;
		REG_SPIDATA = 0;
		SerialWaitBusy();
		REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER | SPI_BAUD_1MHz;
		REG_SPIDATA = temp | PM_SYSTEM_PWR;
	}

	// Read the touch data
	inputGetAndSend();
}

//---------------------------------------------------------------------------------
int main(int argc, char ** argv) {
//---------------------------------------------------------------------------------

	// Setup the FIFO system
	irqInit();
	fifoInit();

	// Initialize touch (offset, scaling, etc.)
	touchInit();

	// Reset the clock if needed
	rtcReset();

	while (IPC->mailData != 0x00424242);	// wait for arm9 init
// check if it is running on DS Lite or not
	SerialWaitBusy();
	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER | SPI_BAUD_1MHz | SPI_CONTINUOUS;
	REG_SPIDATA = 0x80;
	SerialWaitBusy();
	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER | SPI_BAUD_1MHz;
	REG_SPIDATA = 0;
	SerialWaitBusy();

// bit 6 set means it's a DS lite
	if((REG_SPIDATA & BIT(6))) {
		IPC->mailData = 0x42424201;
	} else {
		IPC->mailData = 0x42424200;
	}

	irqInit();
	irqSet(IRQ_VBLANK, VblankHandler);
	irqEnable(IRQ_VBLANK);

	// Keep the ARM7 out of main RAM
	while (1) swiWaitForVBlank();
}


