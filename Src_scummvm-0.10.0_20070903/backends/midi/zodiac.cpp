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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/midi/zodiac.cpp $
 * $Id: zodiac.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 */

#include "common/stdafx.h"
#include "sound/mpu401.h"
#include "common/util.h"

#ifndef DISABLE_TAPWAVE

#include <tapwave.h>


class MidiDriver_Zodiac:public MidiDriver_MPU401 {
public:
	MidiDriver_Zodiac();
	int open();
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);

private:
	TwMidiHandle _midiHandle;
	Boolean _isOpen;
	Int32 _oldVol;
 };

MidiDriver_Zodiac::MidiDriver_Zodiac() {
	_isOpen = false;
	_midiHandle = 0;
}

int MidiDriver_Zodiac::open() {
	Err e;

	if (e = TwMidiOpen(&_midiHandle))
 		return MERR_DEVICE_NOT_AVAILABLE;

	TwMidiGetMasterVolume(&_oldVol);
	TwMidiSetMasterVolume(twMidiMaxVolume); // TODO : set volume based on gVars

	_isOpen = true;
	return 0;
}

void MidiDriver_Zodiac::close() {
	if (_isOpen) {
		_isOpen = false;
		MidiDriver_MPU401::close();

		TwMidiSetMasterVolume(_oldVol);
		TwMidiClose(_midiHandle);
	}
}

void MidiDriver_Zodiac::send(uint32 b) {
	if (!_isOpen)
		return;

	UInt8 midiCmd[4];
	UInt8 chanID,mdCmd;

	midiCmd[3] = (b & 0xFF000000) >> 24;
	midiCmd[2] = (b & 0x00FF0000) >> 16;
	midiCmd[1] = (b & 0x0000FF00) >> 8;
	midiCmd[0] = (b & 0x000000FF);

	chanID = (midiCmd[0] & 0x0F) ;
	mdCmd = midiCmd[0] & 0xF0;

	switch (mdCmd) {
		case 0x80:	// note off
			TwMidiNoteOff(_midiHandle, chanID, midiCmd[1], 0);
			break;

		case 0x90:	// note on
			TwMidiNoteOn(_midiHandle, chanID, midiCmd[1], midiCmd[2]);
			break;

		case 0xB0:	// control change
			TwMidiControlChange(_midiHandle, chanID, midiCmd[1], midiCmd[2]);
			break;

		case 0xC0:	// progam change
			TwMidiProgramChange(_midiHandle, chanID, midiCmd[1]);
			break;

		case 0xE0:	// pitchBend
			TwMidiPitchBend(_midiHandle, chanID, (short)(midiCmd[1] | (midiCmd[2] << 8)));
			break;
	}
}

void MidiDriver_Zodiac::sysEx(const byte *msg, uint16 length) {
	unsigned char buf[256];
	
	buf[0] = 0xF0;
	memcpy(buf + 1, msg, length);
	buf[length + 1] = 0xF7;

	TwMidiSysEx(_midiHandle, 0, (byte *)buf, length + 2);
}

MidiDriver *MidiDriver_Zodiac_create() {
	return new MidiDriver_Zodiac();
}

#endif
