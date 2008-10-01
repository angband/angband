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

#include <nds/bios.h>
#include <nds/arm7/touch.h>
#include <nds/arm7/clock.h>

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
//*/
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

	static int heartbeat = 0;

	u16 but=0, x=0, y=0, xpx=0, ypx=0, z1=0, z2=0, batt=0, aux=0;
	s32 t1=0, t2=0;
	u8 ct[sizeof(IPC->time.curtime)];
	u32 i;

	// Update the heartbeat
	heartbeat++;

	// Read the touch screen

	but = REG_KEYXY;
#define REPEATS	9
	if (!(but & (1<<6))) {
/*	
		s16 xarr[REPEATS], yarr[REPEATS];
		u16 xtop=0, ytop=0, xbtm=0, ybtm=0, i,j;

		for (i=0;i<REPEATS;i++) {
			xarr[i] = myReadTouchValue(TSC_MEASURE_X, 5, 30);
			yarr[i] = myReadTouchValue(TSC_MEASURE_Y, 5, 30);
		}
		for (j=0;j<REPEATS/2;j++) {
			for (i=0;i<REPEATS;i++) {
				if (xarr[i] != -1) {
					if (xarr[i] < xarr[xbtm]) xbtm = i;
					if (xarr[i] > xarr[xtop]) xtop = i;
				}
				if (yarr[i] != -1) {
					if (yarr[i] < yarr[ybtm]) ybtm = i;
					if (yarr[i] > yarr[ytop]) ytop = i;
				}
			}
			xarr[xtop]=xarr[xbtm]=yarr[ytop]=yarr[ybtm]=-1;
			xtop=ytop=xbtm=ybtm=0;
			while (xarr[xtop] == -1) xtop++;
			while (xarr[ytop] == -1) ytop++;
			xbtm = xtop;
			ybtm = ytop;
		}
  		for (i=0;i<REPEATS;i++) {
			if (i != xtop && i != xbtm && xarr[i] != -1) x = xarr[i];
			if (i != ytop && i != ybtm && yarr[i] != -1) t = yarr[i];
		}
	/*/
	
		x = myReadTouchValue(TSC_MEASURE_X, 5, 30);
		y = myReadTouchValue(TSC_MEASURE_Y, 5, 30);		//*/
		
		xpx = ( x * xscale - xoffset + (xscale>>1) ) >>19;
		ypx = ( y * yscale - yoffset + (yscale>>1) ) >>19;
	
		if ( xpx < 0) xpx = 0;
		if ( ypx < 0) ypx = 0;
		if ( xpx > (SCREEN_WIDTH -1)) xpx = SCREEN_WIDTH -1;
		if ( ypx > (SCREEN_HEIGHT -1)) ypx = SCREEN_HEIGHT -1;//*/
		

/*		touchPosition tempPos = touchReadXY();

		x = tempPos.x;
		y = tempPos.y;
		xpx = tempPos.px;
		ypx = tempPos.py;//*/
	}
#undef REPEATS

	z1 = touchRead(TSC_MEASURE_Z1);
	z2 = touchRead(TSC_MEASURE_Z2);

	
	batt = touchRead(TSC_MEASURE_BATTERY);
	aux  = touchRead(TSC_MEASURE_AUX);

	// Read the time
	rtcGetTime((uint8 *)ct);
	BCDToInteger((uint8 *)&(ct[1]), 7);

	// Read the temperature
	temp = touchReadTemperature(&t1, &t2);

	// Update the IPC struct
//	IPC->heartbeat	= heartbeat;
	IPC->buttons		= but;
	IPC->touchX			= x;
	IPC->touchY			= y;
	IPC->touchXpx		= xpx;
	IPC->touchYpx		= ypx;
	IPC->touchZ1		= z1;
	IPC->touchZ2		= z2;
	IPC->battery		= batt;
	IPC->aux			= aux;

	for(i=0; i<sizeof(ct); i++) {
		IPC->time.curtime[i] = ct[i];
	}

	IPC->temperature = temp;
	IPC->tdiode1 = t1;
	IPC->tdiode2 = t2;


	//sound code  :)
	/*TransferSound *snd = IPC->soundData;
	IPC->soundData = 0;

	if (0 != snd) {

		for (i=0; i<snd->count; i++) {
			s32 chan = getFreeSoundChannel();

			if (chan >= 0) {
				startSound(snd->data[i].rate, snd->data[i].data, snd->data[i].len, chan, snd->data[i].vol, snd->data[i].pan, snd->data[i].format);
			}
		}
	}*/

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
//*/
	//enable sound
//	powerON(POWER_SOUND);
//	SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);
//	IPC->soundData = 0;

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


