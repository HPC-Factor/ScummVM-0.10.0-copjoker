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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/platform/gp32/gp32std_sound.h $
 * $Id: gp32std_sound.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef __GP32STD_SOUND_H
#define __GP32STD_SOUND_H

// GPSOUNDBUF.userdata / Pointer to the buffer which needs to be refilled / Length of the buffer in bytes
typedef void SoundProc(void *param, byte *buf, int len);

typedef struct TGPSOUNDBUF {
	int32 freq;
	uint16 format;
	uint16 samples;				// Buffer length (in samples)
	uint16 channels;
	uint16 padding;				// Fix alignment problem
	void *userdata;				// Userdata which gets passed to the callback function
	SoundProc *callback;		// Callback function (just like in SDL)
	unsigned int pollfreq;		// Frequency of the timer interrupt which polls the playing position
								// recommended value: 2*(playingfreq in Hz/GPSOUNDBUF.samples)
	unsigned int samplesize;	// Size of one sample (8bit mono->1, 16bit stereo->4) - don't touch this
} GPSOUNDBUF;

int gp_soundBufStart(GPSOUNDBUF *sb);
void gp_soundBufStop();

#endif
