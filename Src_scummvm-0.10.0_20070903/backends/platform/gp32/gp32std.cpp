/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/platform/gp32/gp32std.cpp $
 * $Id: gp32std.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "stdafx.h"
#include "common/scummsys.h"
//#include "graphics/scaler.h"
#include "common/system.h"
#include "backends/intern.h"

#include "engines/engine.h"

#include "gp32std.h"
#include "gp32std_grap.h"
#include "gp32std_memory.h"

#define DEBUG_MAX 5
char debline[DEBUG_MAX][256];
static int debnext = 0;

void _dprintf(const char *s, ...) {
	int deba, deb;
	char buf[1024];
	va_list va;

	va_start(va, s);
	vsprintf(buf, s, va);
	va_end(va);

	strcpy(debline[debnext++], buf);

	if (debnext == DEBUG_MAX)
		debnext = 0;
	gp_fillRect(frameBuffer1, 0, 243 - (DEBUG_MAX * 8) - 4, 320, (DEBUG_MAX * 8), 0);
	
	for (deb = debnext, deba = 0; deb < DEBUG_MAX; deb++, deba++) {
		//gp_fillRect(frameBuffer1, 0, (243 - (DEBUG_MAX * 8) - 4) + 8 * deba, 320, 8, 0);
		gp_textOut(frameBuffer1, 0, (240 - (DEBUG_MAX * 8) - 4) + 8 * deba, debline[deb], 0xFFFF);
	}
	for (deb = 0; deb < debnext; deb++, deba++) {
		//gp_fillRect(frameBuffer1, 0, (243 - (DEBUG_MAX * 8) - 4) + 8 * deba, 320, 8, 0);
		gp_textOut(frameBuffer1, 0, (240 - (DEBUG_MAX * 8) - 4) + 8 * deba, debline[deb], 0xFFFF);
	}

//	gp_delay(100);
}

////////////////////
//String functions
char *gp_strcpy(char *dst, const char *src) {
	char *pDst = dst;

	while (*pDst++ = *src++)
		;

	return dst;
}

char *gp_strncpy(char *dst, const char *src, size_t count) {
	char *start = dst;

	while (count && (*dst++ = *src++))
		count--;

	if (count)
		while (--count)
			*dst++ = '\0';

	return start;
}

char *gp_strcat(char *dst, const char *src) {
	char *pDst = dst;

	while (*pDst)
		pDst++;

	while (*pDst++ = *src++)
		;

	return dst;

}

char *gp_strdup(const char *str) {
        char *memory;

        if (!str)
                return NULL;

        if (memory = (char *)gp_malloc(strlen(str) + 1))
                return gp_strcpy(memory, str);

        return NULL;
}

int gp_strcasecmp(const char *dst, const char *src) {
	int f, l;
	do {
		f = tolower((unsigned char)(*(dst++)));
		l = tolower((unsigned char)(*(src++)));
	} while (f && (f == l));

	return f - l;
}

int gp_strncasecmp(const char *dst, const char *src, size_t count) {
	int f,l;

	if (count) {
		do {
			f = tolower((unsigned char)(*(dst++)));
			l = tolower((unsigned char)(*(src++)));
		} while (--count && f && (f == l));
		return f - l;
	}

	return 0;
}

//FIXME: Handle LONG string
void gp_sprintf(char *str, const char *fmt, ...) {
	char s[512];
	va_list marker;

	va_start(marker, fmt);
	vsnprintf(s, 512, fmt, marker);
	va_end(marker);

	gp_strcpy(str, s);
}

int gp_printf(const char *fmt, ...) {
	char s[256];
	va_list marker;

	va_start(marker, fmt);
	vsnprintf(s, 256, fmt, marker);
	va_end(marker);

	_dprintf("%s", s);
	//gp_delay(100);
	return 0;
}

void gp_delay(uint32 msecs) {
	int startTime = GpTickCountGet();
	while (GpTickCountGet() < startTime + msecs);
}

void gp_clockSpeedChange(int freq, int magic, int div) {
	#define rTCFG0 (*(volatile unsigned *)0x15100000)
	#define rTCFG1 (*(volatile unsigned *)0x15100004)
	#define rTCNTB4 (*(volatile unsigned *)0x1510003c)
	unsigned int pclk;
	unsigned int prescaler0;

	// Change CPU Speed
	GpClockSpeedChange(freq, magic, div);
	pclk = GpPClkGet();

	// Repair SDK timer - it forgets to set prescaler
	prescaler0 = (pclk / (8000 * 40)) - 1;
	rTCFG0 = (rTCFG0 & 0xFFFFFF00) | prescaler0;
	rTCFG1 = 0x30033;

	// Repair GpTickCountGet
	rTCNTB4 = pclk / 1600;
}

void gp_setCpuSpeed(int freq) {
	// Default value for 40 mhz
	static int CLKDIV = 0x48013;
	static int MCLK = 40000000;
	static int CLKMODE = 0;
	static int HCLK = 40000000;
	static int PCLK = 40000000;

	switch (freq) {
		// overclocked
		case 168: { CLKDIV = 0x14000; MCLK = 168000000; CLKMODE = 3; break; }
		case 172: { CLKDIV = 0x23010; MCLK = 172000000; CLKMODE = 3; break; }
		case 176: { CLKDIV = 0x24010; MCLK = 176000000; CLKMODE = 3; break; }
		case 180: { CLKDIV = 0x16000; MCLK = 180000000; CLKMODE = 3; break; }
		case 184: { CLKDIV = 0x26010; MCLK = 184000000; CLKMODE = 3; break; }
		case 188: { CLKDIV = 0x27010; MCLK = 188000000; CLKMODE = 3; break; }
		case 192: { CLKDIV = 0x18000; MCLK = 192000000; CLKMODE = 3; break; }
		case 196: { CLKDIV = 0x29010; MCLK = 196000000; CLKMODE = 3; break; }
		case 200: { CLKDIV = 0x2A010; MCLK = 200000000; CLKMODE = 3; break; }
		case 204: { CLKDIV = 0x2b010; MCLK = 204000000; CLKMODE = 3; break; }
		case 208: { CLKDIV = 0x2c010; MCLK = 208000000; CLKMODE = 3; break; }
		case 212: { CLKDIV = 0x2d010; MCLK = 212000000; CLKMODE = 3; break; }
		case 216: { CLKDIV = 0x2e010; MCLK = 216000000; CLKMODE = 3; break; }
		case 220: { CLKDIV = 0x2f010; MCLK = 220000000; CLKMODE = 3; break; }
		case 224: { CLKDIV = 0x30010; MCLK = 224000000; CLKMODE = 3; break; }
		case 228: { CLKDIV = 0x1e000; MCLK = 228000000; CLKMODE = 3; break; }
		case 232: { CLKDIV = 0x32010; MCLK = 232000000; CLKMODE = 3; break; }
		case 236: { CLKDIV = 0x33010; MCLK = 236000000; CLKMODE = 3; break; }
		case 240: { CLKDIV = 0x20000; MCLK = 240000000; CLKMODE = 3; break; }
		case 244: { CLKDIV = 0x35010; MCLK = 244000000; CLKMODE = 3; break; }
		case 248: { CLKDIV = 0x36010; MCLK = 248000000; CLKMODE = 3; break; }
		case 252: { CLKDIV = 0x22000; MCLK = 252000000; CLKMODE = 3; break; }
		case 256: { CLKDIV = 0x38010; MCLK = 256000000; CLKMODE = 3; break; }

		// normal
//		case 166: { CLKDIV = 0x4B011; MCLK = 166000000; CLKMODE = 3; break; }
		case 166: { CLKDIV = 0x2f001; MCLK = 165000000; CLKMODE = 3; break; }
		case 164: { CLKDIV = 0x4a011; MCLK = 164000000; CLKMODE = 3; break; }
		case 160: { CLKDIV = 0x48011; MCLK = 160000000; CLKMODE = 3; break; }
		case 156: { CLKDIV = 0x2c001; MCLK = 156000000; CLKMODE = 3; break; }
		case 144: { CLKDIV = 0x28001; MCLK = 144000000; CLKMODE = 3; break; }
		case 133: { CLKDIV = 0x51021; MCLK = 133500000; CLKMODE = 2; break; }
		case 132: { CLKDIV = 0x3a011; MCLK = 132000000; CLKMODE = 3; break; }
		case 120: { CLKDIV = 0x24001; MCLK = 120000000; CLKMODE = 2; break; }
		case 100: { CLKDIV = 0x2b011; MCLK = 102000000; CLKMODE = 2; break; }
		case  66: { CLKDIV = 0x25002; MCLK = 67500000; CLKMODE = 2; break; }
		case  50: { CLKDIV = 0x2a012; MCLK = 50000000; CLKMODE = 0; break; }
//		case  40: { CLKDIV = 0x48013; MCLK = 40000000; CLKMODE = 0; break; }
		case  40: { CLKDIV = 0x48013; MCLK = 40000000; CLKMODE = 1; break; }
//		case  33: { CLKDIV = 0x25003; MCLK = 33750000; CLKMODE = 0; break; }
		case  33: { CLKDIV = 0x25003; MCLK = 33750000; CLKMODE = 2; break; }
		case  22: { CLKDIV = 0x33023; MCLK = 22125000; CLKMODE = 0; break; }
		default:
			error("Invalid CPU frequency!");
	}
	if (CLKMODE == 0) { HCLK = MCLK;     PCLK = MCLK; }
	if (CLKMODE == 1) { HCLK = MCLK;     PCLK = MCLK / 2; }
	if (CLKMODE == 2) { HCLK = MCLK / 2; PCLK = MCLK / 2; }
	if (CLKMODE == 3) { HCLK = MCLK / 2; PCLK = MCLK / 4; }

	gp_clockSpeedChange(MCLK, CLKDIV, CLKMODE);
}

void gp_Reset() {
   gp_setCpuSpeed(66);
   asm volatile("swi #4\n");
}

void gp_exit(int code) {
	if (!code) {
		printf("  ----------------------------------------");
		printf("       Your GP32 is now restarting...     ");
		printf("  ----------------------------------------");
		printf("");

		gp_delay(3000);

		GpAppExit();
	} else {
		printf("Exit Code %d", code);
		while (1);
	}
}

// Debug functions
void GPDEBUG(const char *fmt, ...) {
	char s[256];
	va_list marker;

	va_start(marker, fmt);
	vsnprintf(s, 256, fmt, marker);
	va_end(marker);

	_dprintf("%s", s);
}

void NP(const char *fmt, ...) {
//	return;
	char s[256];
	va_list marker;

	va_start(marker, fmt);
	vsnprintf(s, 256, fmt, marker);
	va_end(marker);

	_dprintf("NP:%s", s);
	gp_delay(50);
}

void LP(const char *fmt, ...) {
//	return;
	char s[256];
	va_list marker;

	va_start(marker, fmt);
	vsnprintf(s, 256, fmt, marker);
	va_end(marker);

	_dprintf("LP:%s", s);
	gp_delay(300);
}

void SP(const char *fmt, ...) {
//	return;
	char s[256];
	va_list marker;

	va_start(marker, fmt);
	vsnprintf(s, 256, fmt, marker);
	va_end(marker);

	_dprintf("SP:%s", s);
	gp_delay(50);
}

void BP(const char *fmt, ...) {
//	return;
	char s[256];
	va_list marker;

	va_start(marker, fmt);
	vsnprintf(s, 256, fmt, marker);
	va_end(marker);

	_dprintf("BP:%s", s);
	gp_delay(2000);
}
