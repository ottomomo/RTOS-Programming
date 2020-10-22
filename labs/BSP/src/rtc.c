#include <s3c44b0x.h>
#include <s3cev40.h>
#include <rtc.h>

extern void isr_TICK_dummy(void);

void rtc_init(void) {
	TICNT = 0x00;
	RTCALM = 0x0;
	RTCRST = 0x0;

	RTCCON = 0x1;

	BCDYEAR = 0x2013; // 2013
	BCDMON = 0x1; // Enero
	BCDDAY = 0x1; // El dia del mes
	BCDDATE = 0x3; // dia de la semana (1-7)
	BCDHOUR = 0x0; // Hora 00
	BCDMIN = 0x0; // Minuto 00
	BCDSEC = 0x0; // Segundo 00
	ALMYEAR = 0x0;
	ALMMON = 0x0;
	ALMDAY = 0x0;
	ALMHOUR = 0x0;
	ALMMIN = 0x0;
	ALMSEC = 0x0;
	RTCCON &= 0x0; // Deshabilita la posibilidad de leer/escribir los registros de hora/fecha del RTC
}

void rtc_puttime(rtc_time_t *rtc_time) {
	RTCCON |= 0x1;

	BCDYEAR = ((rtc_time->year / 10) << 4) + (rtc_time->year % 10);
	if (rtc_time->mon < 10)
		BCDMON = rtc_time->mon;
	else
		BCDMON = ((rtc_time->mon / 10) << 4) + (rtc_time->mon % 10);

	BCDDATE = rtc_time->wday;

	if (rtc_time->mday < 10)
		BCDDAY = rtc_time->mday;
	else
		BCDDAY = ((rtc_time->mday / 10) << 4) + (rtc_time->mday % 10);

	if (rtc_time->hour < 10)
		BCDHOUR = rtc_time->hour;
	else
		BCDHOUR = ((rtc_time->hour / 10) << 4) + (rtc_time->hour % 10);

	if (rtc_time->min < 10)
		BCDMIN = rtc_time->min;
	else
		BCDMIN = ((rtc_time->min / 10) << 4) + (rtc_time->min % 10);

	if (rtc_time->sec < 10)
		BCDSEC = rtc_time->sec;
	else
		BCDSEC = ((rtc_time->sec / 10) << 4) + (rtc_time->sec % 10);

	RTCCON &= 0x0;
}

void rtc_gettime(rtc_time_t *rtc_time) {
	RTCCON |= 0x1;

	rtc_time->year = (((BCDYEAR & 0xF0) >> 4) * 10) + (BCDYEAR & 0x0F);
	rtc_time->mon = (((BCDMON & 0xF0) >> 4) * 10) + (BCDMON & 0x0F);
	rtc_time->mday = (((BCDDAY & 0xF0) >> 4) * 10) + (BCDDAY & 0x0F);
	rtc_time->wday = (((BCDDATE & 0xF0) >> 4) * 10) + (BCDDATE & 0x0F);
	rtc_time->hour = (((BCDHOUR & 0xF0) >> 4) * 10) + (BCDHOUR & 0x0F);
	rtc_time->min = (((BCDMIN & 0xF0) >> 4) * 10) + (BCDMIN & 0x0F);
	rtc_time->sec = (((BCDSEC & 0xF0) >> 4) * 10) + (BCDSEC & 0x0F);
	if (!rtc_time->sec) {
		rtc_time->year = (((BCDYEAR & 0xF0) >> 4) * 10) + (BCDYEAR & 0x0F);
		rtc_time->mon = (((BCDMON & 0xF0) >> 4) * 10) + (BCDMON & 0x0F);
		rtc_time->mday = (((BCDDAY & 0xF0) >> 4) * 10) + (BCDDAY & 0x0F);
		rtc_time->wday = (((BCDDATE & 0xF0) >> 4) * 10) + (BCDDATE & 0x0F);
		rtc_time->hour = (((BCDHOUR & 0xF0) >> 4) * 10) + (BCDHOUR & 0x0F);
		rtc_time->min = (((BCDMIN & 0xF0) >> 4) * 10) + (BCDMIN & 0x0F);
		rtc_time->sec = (((BCDSEC & 0xF0) >> 4) * 10) + (BCDSEC & 0x0F);
	};

	RTCCON &= 0x0;
}

void rtc_open(void (*isr)(void), uint8 tick_count) {
	pISR_TICK = (uint32) isr;
	I_ISPC = BIT_TICK;
	INTMSK &= ~(BIT_GLOBAL | BIT_TICK);
	TICNT =  (1<<7) | tick_count;
}

void rtc_close(void) {
	TICNT = 0 << 7;
	INTMSK |= BIT_TICK;
	pISR_TICK = (uint32) isr_TICK_dummy;
}
