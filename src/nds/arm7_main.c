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

s32 xscale, yscale, xoffset, yoffset;

#define abs(n)	(n<0?-n:n)

//---------------------------------------------------------------------------------
s32 myReadTouchValue(int measure, int retry , int range) {
//---------------------------------------------------------------------------------
	int i;
	s32 this_value=0, this_range;

	s32 last_value = touchRead(measure | 1);

	for ( i=0; i < retry; i++) {
		touchRead(measure | 1);
		this_value = touchRead(measure);
		this_range = abs(last_value - this_value);
		if (this_range <= range) break;
	}

	if ( i == range) this_value = 0;
	return this_value;

}

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

	u16 but=0, x=0, y=0, xpx=0, ypx=0;

	but = REG_KEYXY;
	if (!(but & (1<<6))) {
		x = myReadTouchValue(TSC_MEASURE_X, 5, 30);
		y = myReadTouchValue(TSC_MEASURE_Y, 5, 30);

		xpx = ( x * xscale - xoffset + (xscale>>1) ) >>19;
		ypx = ( y * yscale - yoffset + (yscale>>1) ) >>19;

		if ( xpx < 0) xpx = 0;
		if ( ypx < 0) ypx = 0;
		if ( xpx > (SCREEN_WIDTH -1)) xpx = SCREEN_WIDTH -1;
		if ( ypx > (SCREEN_HEIGHT -1)) ypx = SCREEN_HEIGHT -1;
	}

	// Read the touch data
	IPC->touchXpx		= xpx;
	IPC->touchYpx		= ypx;
}

//---------------------------------------------------------------------------------
int main(int argc, char ** argv) {
//---------------------------------------------------------------------------------

	// Reset the clock if needed
	rtcReset();

	xscale = ((PersonalData->calX2px - PersonalData->calX1px) << 19) / ((PersonalData->calX2) - (PersonalData->calX1));
	yscale = ((PersonalData->calY2px - PersonalData->calY1px) << 19) / ((PersonalData->calY2) - (PersonalData->calY1));

	xoffset = ((PersonalData->calX1 + PersonalData->calX2) * xscale  - ((PersonalData->calX1px + PersonalData->calX2px) << 19) ) /2;
	yoffset = ((PersonalData->calY1 + PersonalData->calY2) * yscale  - ((PersonalData->calY1px + PersonalData->calY2px) << 19) ) /2;

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


