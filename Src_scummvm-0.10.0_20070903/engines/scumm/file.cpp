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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/file.cpp $
 * $Id: file.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "scumm/file.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/md5.h"

#include "scumm/scumm.h"

using Common::File;

namespace Scumm {

#pragma mark -
#pragma mark --- ScummFile ---
#pragma mark -

ScummFile::ScummFile() : _encbyte(0), _subFileStart(0), _subFileLen(0) {
}

void ScummFile::setEnc(byte value) {
	_encbyte = value;
}

void ScummFile::setSubfileRange(uint32 start, uint32 len) {
	// TODO: Add sanity checks
	const uint32 fileSize = File::size();
	assert(start <= fileSize);
	assert(start + len <= fileSize);
	_subFileStart = start;
	_subFileLen = len;
	seek(0, SEEK_SET);
}

void ScummFile::resetSubfile() {
	_subFileStart = 0;
	_subFileLen = 0;
	seek(0, SEEK_SET);
}

bool ScummFile::open(const Common::String &filename, AccessMode mode) {
	if (File::open(filename, mode)) {
		resetSubfile();
		return true;
	} else {
		return false;
	}
}

bool ScummFile::openSubFile(const Common::String &filename) {
	assert(isOpen());

	// Disable the XOR encryption and reset any current subfile range
	setEnc(0);
	resetSubfile();

	// Read in the filename table and look for the specified file

	unsigned long file_off, file_len;
	char file_name[0x20+1];
	unsigned long i;

	// Get the length of the data file to use for consistency checks
	const uint32 data_file_len = size();

	// Read offset and length to the file records */
	const uint32 file_record_off = readUint32BE();
	const uint32 file_record_len = readUint32BE();

	// Do a quick check to make sure the offset and length are good
	if (file_record_off + file_record_len > data_file_len) {
		return false;
	}

	// Do a little consistancy check on file_record_length
	if (file_record_len % 0x28) {
		return false;
	}

	// Scan through the files
	for (i = 0; i < file_record_len; i += 0x28) {
		// read a file record
		seek(file_record_off + i, SEEK_SET);
		file_off = readUint32BE();
		file_len = readUint32BE();
		read(file_name, 0x20);
		file_name[0x20] = 0;

		assert(file_name[0]);
		//debug(7, "  extracting \'%s\'", file_name);

		// Consistency check. make sure the file data is in the file
		if (file_off + file_len > data_file_len) {
			return false;
		}

		if (scumm_stricmp(file_name, filename.c_str()) == 0) {
			// We got a match!
			setSubfileRange(file_off, file_len);
			return true;
		}
	}

	return false;
}


bool ScummFile::eof() {
	return _subFileLen ? (pos() >= _subFileLen) : File::eof();
}

uint32 ScummFile::pos() {
	return File::pos() - _subFileStart;
}

uint32 ScummFile::size() {
	return _subFileLen ? _subFileLen : File::size();
}

void ScummFile::seek(int32 offs, int whence) {
	if (_subFileLen) {
		// Constrain the seek to the subfile
		switch (whence) {
		case SEEK_END:
			offs = _subFileStart + _subFileLen - offs;
			break;
		case SEEK_SET:
			offs += _subFileStart;
			break;
		case SEEK_CUR:
			offs += File::pos();
			break;
		}
		assert((int32)_subFileStart <= offs && offs <= (int32)(_subFileStart + _subFileLen));
		whence = SEEK_SET;
	}
	File::seek(offs, whence);
}

uint32 ScummFile::read(void *dataPtr, uint32 dataSize) {
	uint32 realLen;

	if (_subFileLen) {
		// Limit the amount we read by the subfile boundaries.
		const uint32 curPos = pos();
		assert(_subFileLen >= curPos);
		uint32 newPos = curPos + dataSize;
		if (newPos > _subFileLen) {
			dataSize = _subFileLen - curPos;
			_ioFailed = true;
		}
	}

	realLen = File::read(dataPtr, dataSize);


	// If an encryption byte was specified, XOR the data we just read by it.
	// This simple kind of "encryption" was used by some of the older SCUMM
	// games.
	if (_encbyte) {
		byte *p = (byte *)dataPtr;
		byte *end = p + realLen;
		while (p < end)
			*p++ ^= _encbyte;
	}

	return realLen;
}

uint32 ScummFile::write(const void *, uint32) {
	error("ScummFile does not support writing!");
	return 0;
}

#pragma mark -
#pragma mark --- ScummNESFile ---
#pragma mark -

enum ResType {
	NES_UNKNOWN,
	NES_GLOBDATA,
	NES_ROOM,
	NES_SCRIPT,
	NES_SOUND,
	NES_COSTUME,
	NES_ROOMGFX,
	NES_COSTUMEGFX,
	NES_SPRPALS,
	NES_SPRDESC,
	NES_SPRLENS,
	NES_SPROFFS,
	NES_SPRDATA,
	NES_CHARSET,
	NES_PREPLIST
};

struct ScummNESFile::Resource {
	uint32 offset;
	uint16 length;
	ResType type;
};

ScummNESFile::ScummNESFile() : _stream(0), _buf(0), _ROMset(kROMsetNum) {
}

uint32 ScummNESFile::write(const void *, uint32) {
	error("ScummNESFile does not support writing!");
	return 0;
}

void ScummNESFile::setEnc(byte enc) {
	_stream->setEnc(enc);
}

#ifdef PALMOS_68K
static const ScummNESFile::Resource *res_roomgfx_usa;
static const ScummNESFile::Resource *res_roomgfx_eur;
static const ScummNESFile::Resource *res_roomgfx_swe;
static const ScummNESFile::Resource *res_roomgfx_fra;
static const ScummNESFile::Resource *res_roomgfx_ger;
#else
static const ScummNESFile::Resource res_roomgfx_usa[40] = {
	{ 0x04001, 0x03C9, NES_ROOMGFX }, { 0x043CA, 0x069E, NES_ROOMGFX }, { 0x04A68, 0x0327, NES_ROOMGFX }, { 0x04D8F, 0x053B, NES_ROOMGFX }, { 0x052CA, 0x06BE, NES_ROOMGFX },
	{ 0x05988, 0x0682, NES_ROOMGFX }, { 0x0600A, 0x0778, NES_ROOMGFX }, { 0x06782, 0x0517, NES_ROOMGFX }, { 0x06C99, 0x07FB, NES_ROOMGFX }, { 0x07494, 0x07BE, NES_ROOMGFX },
	{ 0x08001, 0x07A5, NES_ROOMGFX }, { 0x087A6, 0x06DD, NES_ROOMGFX }, { 0x08E83, 0x04EA, NES_ROOMGFX }, { 0x0936D, 0x0846, NES_ROOMGFX }, { 0x09BB3, 0x08C8, NES_ROOMGFX },
	{ 0x0A47B, 0x0844, NES_ROOMGFX }, { 0x0ACBF, 0x0515, NES_ROOMGFX }, { 0x0B1D4, 0x0799, NES_ROOMGFX }, { 0x0B96D, 0x04BB, NES_ROOMGFX }, { 0x07C52, 0x0319, NES_ROOMGFX },
	{ 0x0C001, 0x0464, NES_ROOMGFX }, { 0x0C465, 0x076D, NES_ROOMGFX }, { 0x0CBD2, 0x0827, NES_ROOMGFX }, { 0x0D3F9, 0x0515, NES_ROOMGFX }, { 0x0D90E, 0x064E, NES_ROOMGFX },
	{ 0x0DF5C, 0x0775, NES_ROOMGFX }, { 0x0E6D1, 0x06DD, NES_ROOMGFX }, { 0x0EDAE, 0x0376, NES_ROOMGFX }, { 0x0F124, 0x05F7, NES_ROOMGFX }, { 0x0F71B, 0x0787, NES_ROOMGFX },
	{ 0x10001, 0x02D6, NES_ROOMGFX }, { 0x102D7, 0x06A3, NES_ROOMGFX }, { 0x1097A, 0x099F, NES_ROOMGFX }, { 0x11319, 0x0361, NES_ROOMGFX }, { 0x1167A, 0x0489, NES_ROOMGFX },
	{ 0x11B03, 0x0437, NES_ROOMGFX }, { 0x11F3A, 0x084D, NES_ROOMGFX }, { 0x0BE28, 0x0199, NES_ROOMGFX }, { 0x12787, 0x09A7, NES_ROOMGFX }, { 0x1312E, 0x037A, NES_ROOMGFX }
};
static const ScummNESFile::Resource res_roomgfx_eur[40] = {
	{ 0x04001, 0x03B9, NES_ROOMGFX }, { 0x043BA, 0x069E, NES_ROOMGFX }, { 0x04A58, 0x0327, NES_ROOMGFX }, { 0x04D7F, 0x053B, NES_ROOMGFX }, { 0x052BA, 0x06BE, NES_ROOMGFX },
	{ 0x05978, 0x0682, NES_ROOMGFX }, { 0x05FFA, 0x0778, NES_ROOMGFX }, { 0x06772, 0x0517, NES_ROOMGFX }, { 0x06C89, 0x07FB, NES_ROOMGFX }, { 0x07484, 0x07BE, NES_ROOMGFX },
	{ 0x08001, 0x07A5, NES_ROOMGFX }, { 0x087A6, 0x06DD, NES_ROOMGFX }, { 0x08E83, 0x04EA, NES_ROOMGFX }, { 0x0936D, 0x0846, NES_ROOMGFX }, { 0x09BB3, 0x08C8, NES_ROOMGFX },
	{ 0x0A47B, 0x0844, NES_ROOMGFX }, { 0x0ACBF, 0x0515, NES_ROOMGFX }, { 0x0B1D4, 0x0799, NES_ROOMGFX }, { 0x0B96D, 0x04BB, NES_ROOMGFX }, { 0x07C42, 0x0319, NES_ROOMGFX },
	{ 0x0C001, 0x0464, NES_ROOMGFX }, { 0x0C465, 0x076D, NES_ROOMGFX }, { 0x0CBD2, 0x0827, NES_ROOMGFX }, { 0x0D3F9, 0x0515, NES_ROOMGFX }, { 0x0D90E, 0x064E, NES_ROOMGFX },
	{ 0x0DF5C, 0x0775, NES_ROOMGFX }, { 0x0E6D1, 0x06DD, NES_ROOMGFX }, { 0x0EDAE, 0x0376, NES_ROOMGFX }, { 0x0F124, 0x05F7, NES_ROOMGFX }, { 0x0F71B, 0x0787, NES_ROOMGFX },
	{ 0x10001, 0x02D6, NES_ROOMGFX }, { 0x102D7, 0x06A3, NES_ROOMGFX }, { 0x1097A, 0x099F, NES_ROOMGFX }, { 0x11319, 0x0361, NES_ROOMGFX }, { 0x1167A, 0x0489, NES_ROOMGFX },
	{ 0x11B03, 0x0437, NES_ROOMGFX }, { 0x11F3A, 0x084D, NES_ROOMGFX }, { 0x12787, 0x0199, NES_ROOMGFX }, { 0x12920, 0x09A7, NES_ROOMGFX }, { 0x132C7, 0x037A, NES_ROOMGFX }
};
static const ScummNESFile::Resource res_roomgfx_swe[40] = {
	{ 0x04001, 0x03F0, NES_ROOMGFX }, { 0x043F1, 0x069E, NES_ROOMGFX }, { 0x04A8F, 0x0327, NES_ROOMGFX }, { 0x04DB6, 0x053B, NES_ROOMGFX }, { 0x052F1, 0x06BE, NES_ROOMGFX },
	{ 0x059AF, 0x0682, NES_ROOMGFX }, { 0x06031, 0x0778, NES_ROOMGFX }, { 0x067A9, 0x0517, NES_ROOMGFX }, { 0x06CC0, 0x07FB, NES_ROOMGFX }, { 0x074BB, 0x07BE, NES_ROOMGFX },
	{ 0x08001, 0x07A5, NES_ROOMGFX }, { 0x087A6, 0x06DD, NES_ROOMGFX }, { 0x08E83, 0x04EA, NES_ROOMGFX }, { 0x0936D, 0x07E2, NES_ROOMGFX }, { 0x09B4F, 0x0791, NES_ROOMGFX },
	{ 0x0A2E0, 0x07B5, NES_ROOMGFX }, { 0x0AA95, 0x0515, NES_ROOMGFX }, { 0x0AFAA, 0x0799, NES_ROOMGFX }, { 0x0B743, 0x04BF, NES_ROOMGFX }, { 0x0BC02, 0x0319, NES_ROOMGFX },
	{ 0x0C001, 0x0464, NES_ROOMGFX }, { 0x0C465, 0x072C, NES_ROOMGFX }, { 0x0CB91, 0x0827, NES_ROOMGFX }, { 0x0D3B8, 0x0515, NES_ROOMGFX }, { 0x0D8CD, 0x064E, NES_ROOMGFX },
	{ 0x0DF1B, 0x0775, NES_ROOMGFX }, { 0x0E690, 0x06DD, NES_ROOMGFX }, { 0x0ED6D, 0x0376, NES_ROOMGFX }, { 0x0F0E3, 0x05F7, NES_ROOMGFX }, { 0x0F6DA, 0x0791, NES_ROOMGFX },
	{ 0x07C79, 0x02D6, NES_ROOMGFX }, { 0x10001, 0x06A3, NES_ROOMGFX }, { 0x106A4, 0x0921, NES_ROOMGFX }, { 0x10FC5, 0x0361, NES_ROOMGFX }, { 0x11326, 0x0489, NES_ROOMGFX },
	{ 0x117AF, 0x0437, NES_ROOMGFX }, { 0x11BE6, 0x084F, NES_ROOMGFX }, { 0x12435, 0x0199, NES_ROOMGFX }, { 0x125CE, 0x0947, NES_ROOMGFX }, { 0x12F15, 0x037A, NES_ROOMGFX }
};
static const ScummNESFile::Resource res_roomgfx_fra[40] = {
	{ 0x04001, 0x0426, NES_ROOMGFX }, { 0x04427, 0x069E, NES_ROOMGFX }, { 0x04AC5, 0x0327, NES_ROOMGFX }, { 0x04DEC, 0x053B, NES_ROOMGFX }, { 0x05327, 0x06BE, NES_ROOMGFX },
	{ 0x059E5, 0x0682, NES_ROOMGFX }, { 0x06067, 0x0778, NES_ROOMGFX }, { 0x067DF, 0x0517, NES_ROOMGFX }, { 0x06CF6, 0x07FB, NES_ROOMGFX }, { 0x074F1, 0x07BE, NES_ROOMGFX },
	{ 0x08001, 0x07A5, NES_ROOMGFX }, { 0x087A6, 0x06DD, NES_ROOMGFX }, { 0x08E83, 0x04EA, NES_ROOMGFX }, { 0x0936D, 0x07E2, NES_ROOMGFX }, { 0x09B4F, 0x0791, NES_ROOMGFX },
	{ 0x0A2E0, 0x07B5, NES_ROOMGFX }, { 0x0AA95, 0x0515, NES_ROOMGFX }, { 0x0AFAA, 0x0799, NES_ROOMGFX }, { 0x0B743, 0x04BB, NES_ROOMGFX }, { 0x0BBFE, 0x0319, NES_ROOMGFX },
	{ 0x0C001, 0x0464, NES_ROOMGFX }, { 0x0C465, 0x072C, NES_ROOMGFX }, { 0x0CB91, 0x0827, NES_ROOMGFX }, { 0x0D3B8, 0x0515, NES_ROOMGFX }, { 0x0D8CD, 0x064E, NES_ROOMGFX },
	{ 0x0DF1B, 0x0775, NES_ROOMGFX }, { 0x0E690, 0x06DD, NES_ROOMGFX }, { 0x0ED6D, 0x0376, NES_ROOMGFX }, { 0x0F0E3, 0x05F7, NES_ROOMGFX }, { 0x0F6DA, 0x0787, NES_ROOMGFX },
	{ 0x10001, 0x02D6, NES_ROOMGFX }, { 0x102D7, 0x06A3, NES_ROOMGFX }, { 0x1097A, 0x0921, NES_ROOMGFX }, { 0x1129B, 0x0361, NES_ROOMGFX }, { 0x115FC, 0x0489, NES_ROOMGFX },
	{ 0x11A85, 0x0437, NES_ROOMGFX }, { 0x11EBC, 0x070D, NES_ROOMGFX }, { 0x07CAF, 0x0199, NES_ROOMGFX }, { 0x125C9, 0x0947, NES_ROOMGFX }, { 0x12F10, 0x037A, NES_ROOMGFX }
};
static const ScummNESFile::Resource res_roomgfx_ger[40] = {
	{ 0x04001, 0x0406, NES_ROOMGFX }, { 0x04407, 0x069E, NES_ROOMGFX }, { 0x04AA5, 0x0327, NES_ROOMGFX }, { 0x04DCC, 0x053B, NES_ROOMGFX }, { 0x05307, 0x06BE, NES_ROOMGFX },
	{ 0x059C5, 0x0682, NES_ROOMGFX }, { 0x06047, 0x0778, NES_ROOMGFX }, { 0x067BF, 0x0517, NES_ROOMGFX }, { 0x06CD6, 0x07FB, NES_ROOMGFX }, { 0x074D1, 0x07BE, NES_ROOMGFX },
	{ 0x08001, 0x07A5, NES_ROOMGFX }, { 0x087A6, 0x06DD, NES_ROOMGFX }, { 0x08E83, 0x04EA, NES_ROOMGFX }, { 0x0936D, 0x07E2, NES_ROOMGFX }, { 0x09B4F, 0x0791, NES_ROOMGFX },
	{ 0x0A2E0, 0x07B5, NES_ROOMGFX }, { 0x0AA95, 0x0515, NES_ROOMGFX }, { 0x0AFAA, 0x0799, NES_ROOMGFX }, { 0x0B743, 0x04BB, NES_ROOMGFX }, { 0x0BBFE, 0x0319, NES_ROOMGFX },
	{ 0x0C001, 0x0464, NES_ROOMGFX }, { 0x0C465, 0x072C, NES_ROOMGFX }, { 0x0CB91, 0x0827, NES_ROOMGFX }, { 0x0D3B8, 0x0515, NES_ROOMGFX }, { 0x0D8CD, 0x064E, NES_ROOMGFX },
	{ 0x0DF1B, 0x0775, NES_ROOMGFX }, { 0x0E690, 0x06DD, NES_ROOMGFX }, { 0x0ED6D, 0x0376, NES_ROOMGFX }, { 0x0F0E3, 0x05F7, NES_ROOMGFX }, { 0x0F6DA, 0x0787, NES_ROOMGFX },
	{ 0x07C8F, 0x02D6, NES_ROOMGFX }, { 0x10001, 0x06A3, NES_ROOMGFX }, { 0x106A4, 0x0921, NES_ROOMGFX }, { 0x10FC5, 0x0361, NES_ROOMGFX }, { 0x11326, 0x0489, NES_ROOMGFX },
	{ 0x117AF, 0x0437, NES_ROOMGFX }, { 0x11BE6, 0x07A0, NES_ROOMGFX }, { 0x12386, 0x0199, NES_ROOMGFX }, { 0x1251F, 0x0947, NES_ROOMGFX }, { 0x12E66, 0x037A, NES_ROOMGFX }
};
#endif
static const ScummNESFile::Resource *res_roomgfx[ScummNESFile::kROMsetNum] = {
	res_roomgfx_usa,
	res_roomgfx_eur,
	res_roomgfx_swe,
	res_roomgfx_fra,
	res_roomgfx_ger,
};

#ifdef PALMOS_68K
static const ScummNESFile::Resource *res_costumegfx_usa;
static const ScummNESFile::Resource *res_costumegfx_eur;
static const ScummNESFile::Resource *res_costumegfx_swe;
static const ScummNESFile::Resource *res_costumegfx_fra;
static const ScummNESFile::Resource *res_costumegfx_ger;
#else
static const ScummNESFile::Resource res_costumegfx_usa[2] = { { 0x30001, 0x0EB8, NES_COSTUMEGFX }, { 0x2F9F1, 0x0340, NES_COSTUMEGFX } };
static const ScummNESFile::Resource res_costumegfx_eur[2] = { { 0x30001, 0x0EB8, NES_COSTUMEGFX }, { 0x2F9F1, 0x0340, NES_COSTUMEGFX } };
static const ScummNESFile::Resource res_costumegfx_swe[2] = { { 0x2EFE1, 0x0EB8, NES_COSTUMEGFX }, { 0x30001, 0x0340, NES_COSTUMEGFX } };
static const ScummNESFile::Resource res_costumegfx_fra[2] = { { 0x30001, 0x0EB8, NES_COSTUMEGFX }, { 0x2F608, 0x0340, NES_COSTUMEGFX } };
static const ScummNESFile::Resource res_costumegfx_ger[2] = { { 0x30001, 0x0EB8, NES_COSTUMEGFX }, { 0x2F4CE, 0x0340, NES_COSTUMEGFX } };
#endif
static const ScummNESFile::Resource *res_costumegfx[ScummNESFile::kROMsetNum] = {
	res_costumegfx_usa,
	res_costumegfx_eur,
	res_costumegfx_swe,
	res_costumegfx_fra,
	res_costumegfx_ger,
};

#ifdef PALMOS_68K
static const ScummNESFile::Resource *res_rooms_usa;
static const ScummNESFile::Resource *res_rooms_eur;
static const ScummNESFile::Resource *res_rooms_swe;
static const ScummNESFile::Resource *res_rooms_fra;
static const ScummNESFile::Resource *res_rooms_ger;
#else
static const ScummNESFile::Resource res_rooms_usa[55] = {
	{ 0x00000, 0x0000, NES_ROOM }, { 0x14001, 0x0D0C, NES_ROOM }, { 0x134A8, 0x04B3, NES_ROOM }, { 0x15397, 0x0849, NES_ROOM }, { 0x15C68, 0x0685, NES_ROOM },
	{ 0x16381, 0x0715, NES_ROOM }, { 0x1395B, 0x04E7, NES_ROOM }, { 0x16CE8, 0x0AC0, NES_ROOM }, { 0x18001, 0x06BA, NES_ROOM }, { 0x17AED, 0x03CB, NES_ROOM },
	{ 0x18BE7, 0x0663, NES_ROOM }, { 0x192A6, 0x0580, NES_ROOM }, { 0x19A44, 0x0443, NES_ROOM }, { 0x1A106, 0x0563, NES_ROOM }, { 0x1A669, 0x0446, NES_ROOM },
	{ 0x1AAAF, 0x03A7, NES_ROOM }, { 0x1AE56, 0x07E3, NES_ROOM }, { 0x1B699, 0x0692, NES_ROOM }, { 0x1C001, 0x0B49, NES_ROOM }, { 0x1CD09, 0x04C6, NES_ROOM },
	{ 0x1D4C2, 0x0568, NES_ROOM }, { 0x1DF6C, 0x0514, NES_ROOM }, { 0x1E8FA, 0x05CC, NES_ROOM }, { 0x1EF83, 0x0389, NES_ROOM }, { 0x1F5E4, 0x0723, NES_ROOM },
	{ 0x20001, 0x049A, NES_ROOM }, { 0x20511, 0x04F8, NES_ROOM }, { 0x21666, 0x05CB, NES_ROOM }, { 0x21DD6, 0x046B, NES_ROOM }, { 0x222F0, 0x0460, NES_ROOM },
	{ 0x227B6, 0x0909, NES_ROOM }, { 0x24001, 0x0366, NES_ROOM }, { 0x23BDF, 0x03CA, NES_ROOM }, { 0x247DB, 0x050D, NES_ROOM }, { 0x25ACF, 0x0346, NES_ROOM },
	{ 0x1BDBD, 0x01CA, NES_ROOM }, { 0x25E15, 0x0457, NES_ROOM }, { 0x2626C, 0x0547, NES_ROOM }, { 0x267B3, 0x064A, NES_ROOM }, { 0x1FD72, 0x024B, NES_ROOM },
	{ 0x2739A, 0x01FA, NES_ROOM }, { 0x2766D, 0x0219, NES_ROOM }, { 0x28001, 0x02F4, NES_ROOM }, { 0x284D6, 0x045C, NES_ROOM }, { 0x289A3, 0x09CF, NES_ROOM },
	{ 0x293C6, 0x05A0, NES_ROOM }, { 0x27B65, 0x0201, NES_ROOM }, { 0x2ADD1, 0x0325, NES_ROOM }, { 0x2B339, 0x01FC, NES_ROOM }, { 0x2B535, 0x02A9, NES_ROOM },
	{ 0x2B7DE, 0x02DE, NES_ROOM }, { 0x2C001, 0x03CE, NES_ROOM }, { 0x2BBC0, 0x0205, NES_ROOM }, { 0x2C53A, 0x0170, NES_ROOM }, { 0x13E42, 0x0169, NES_ROOM }
};
static const ScummNESFile::Resource res_rooms_eur[55] = {
	{ 0x00000, 0x0000, NES_ROOM }, { 0x14001, 0x0D0C, NES_ROOM }, { 0x13641, 0x04B3, NES_ROOM }, { 0x15397, 0x0849, NES_ROOM }, { 0x15C68, 0x0685, NES_ROOM },
	{ 0x16381, 0x0715, NES_ROOM }, { 0x16CE8, 0x04E7, NES_ROOM }, { 0x18001, 0x0ABF, NES_ROOM }, { 0x171CF, 0x06BA, NES_ROOM }, { 0x13AF4, 0x03D2, NES_ROOM },
	{ 0x18E1A, 0x0663, NES_ROOM }, { 0x194D9, 0x04A9, NES_ROOM }, { 0x19BA0, 0x0443, NES_ROOM }, { 0x1A262, 0x047C, NES_ROOM }, { 0x1A6DE, 0x0446, NES_ROOM },
	{ 0x1AB24, 0x03A7, NES_ROOM }, { 0x1AECB, 0x07E3, NES_ROOM }, { 0x1B70E, 0x0692, NES_ROOM }, { 0x1C001, 0x0ACA, NES_ROOM }, { 0x1CC8A, 0x04C6, NES_ROOM },
	{ 0x1D443, 0x0568, NES_ROOM }, { 0x1DEED, 0x0514, NES_ROOM }, { 0x1E87B, 0x05CC, NES_ROOM }, { 0x1EF04, 0x0389, NES_ROOM }, { 0x1F565, 0x0723, NES_ROOM },
	{ 0x20001, 0x049A, NES_ROOM }, { 0x20511, 0x04F8, NES_ROOM }, { 0x21666, 0x05D5, NES_ROOM }, { 0x21DE0, 0x046B, NES_ROOM }, { 0x222FA, 0x0460, NES_ROOM },
	{ 0x227C0, 0x0909, NES_ROOM }, { 0x24001, 0x0366, NES_ROOM }, { 0x247DB, 0x03CA, NES_ROOM }, { 0x24BA5, 0x050D, NES_ROOM }, { 0x23BE9, 0x0346, NES_ROOM },
	{ 0x17DB5, 0x01CA, NES_ROOM }, { 0x25E99, 0x0457, NES_ROOM }, { 0x262F0, 0x0547, NES_ROOM }, { 0x26837, 0x064A, NES_ROOM }, { 0x1FCF3, 0x024B, NES_ROOM },
	{ 0x2741E, 0x01FA, NES_ROOM }, { 0x276F1, 0x0219, NES_ROOM }, { 0x28001, 0x02F4, NES_ROOM }, { 0x284D6, 0x045C, NES_ROOM }, { 0x289A3, 0x09CF, NES_ROOM },
	{ 0x293C6, 0x05A0, NES_ROOM }, { 0x27BE9, 0x0201, NES_ROOM }, { 0x2ADE3, 0x0325, NES_ROOM }, { 0x2B34B, 0x01FC, NES_ROOM }, { 0x2B547, 0x02A9, NES_ROOM },
	{ 0x2B7F0, 0x02DE, NES_ROOM }, { 0x2C001, 0x03CE, NES_ROOM }, { 0x2BBD2, 0x0205, NES_ROOM }, { 0x2C53A, 0x0170, NES_ROOM }, { 0x2BDD7, 0x0169, NES_ROOM }
};
static const ScummNESFile::Resource res_rooms_swe[55] = {
	{ 0x00000, 0x0000, NES_ROOM }, { 0x14001, 0x0D12, NES_ROOM }, { 0x1328F, 0x04B3, NES_ROOM }, { 0x15367, 0x0859, NES_ROOM }, { 0x13742, 0x0694, NES_ROOM },
	{ 0x15C45, 0x0707, NES_ROOM }, { 0x1658F, 0x04E0, NES_ROOM }, { 0x16A6F, 0x0AC8, NES_ROOM }, { 0x18001, 0x06C7, NES_ROOM }, { 0x1789C, 0x03EA, NES_ROOM },
	{ 0x18C09, 0x0649, NES_ROOM }, { 0x192AE, 0x04AB, NES_ROOM }, { 0x19982, 0x0447, NES_ROOM }, { 0x1A04D, 0x047E, NES_ROOM }, { 0x1A4CB, 0x0444, NES_ROOM },
	{ 0x1A90F, 0x03B9, NES_ROOM }, { 0x1ACC8, 0x07E9, NES_ROOM }, { 0x1B511, 0x06A4, NES_ROOM }, { 0x1C001, 0x0B1A, NES_ROOM }, { 0x1CCFD, 0x0486, NES_ROOM },
	{ 0x1D482, 0x0579, NES_ROOM }, { 0x1DF61, 0x051E, NES_ROOM }, { 0x1E8EC, 0x05CF, NES_ROOM }, { 0x1EF73, 0x0398, NES_ROOM }, { 0x1F5F0, 0x071A, NES_ROOM },
	{ 0x20001, 0x049C, NES_ROOM }, { 0x2051E, 0x051E, NES_ROOM }, { 0x21725, 0x05D5, NES_ROOM }, { 0x21EA5, 0x047F, NES_ROOM }, { 0x223D1, 0x0460, NES_ROOM },
	{ 0x22897, 0x090D, NES_ROOM }, { 0x24001, 0x0378, NES_ROOM }, { 0x247C9, 0x03CA, NES_ROOM }, { 0x24B93, 0x050D, NES_ROOM }, { 0x25267, 0x0346, NES_ROOM },
	{ 0x17CD0, 0x01CA, NES_ROOM }, { 0x255AD, 0x0453, NES_ROOM }, { 0x25A00, 0x053E, NES_ROOM }, { 0x25F3E, 0x0647, NES_ROOM }, { 0x1BC49, 0x024B, NES_ROOM },
	{ 0x26B58, 0x01FA, NES_ROOM }, { 0x26E27, 0x0217, NES_ROOM }, { 0x27345, 0x02F4, NES_ROOM }, { 0x27829, 0x045C, NES_ROOM }, { 0x28001, 0x098A, NES_ROOM },
	{ 0x289DF, 0x05A1, NES_ROOM }, { 0x2A442, 0x0201, NES_ROOM }, { 0x2A6E9, 0x0325, NES_ROOM }, { 0x1FD75, 0x01FC, NES_ROOM }, { 0x2AC64, 0x02A9, NES_ROOM },
	{ 0x2AF0D, 0x02D1, NES_ROOM }, { 0x2B2E6, 0x03CC, NES_ROOM }, { 0x23D61, 0x0205, NES_ROOM }, { 0x2B818, 0x0168, NES_ROOM }, { 0x27CF6, 0x0169, NES_ROOM }
};
static const ScummNESFile::Resource res_rooms_fra[55] = {
	{ 0x00000, 0x0000, NES_ROOM }, { 0x14001, 0x0D76, NES_ROOM }, { 0x1328A, 0x04C6, NES_ROOM }, { 0x15451, 0x0885, NES_ROOM }, { 0x13750, 0x0693, NES_ROOM },
	{ 0x15D68, 0x0709, NES_ROOM }, { 0x166D4, 0x0528, NES_ROOM }, { 0x16BFC, 0x0ACC, NES_ROOM }, { 0x18001, 0x06E2, NES_ROOM }, { 0x17A63, 0x03E5, NES_ROOM },
	{ 0x18C3B, 0x066A, NES_ROOM }, { 0x19301, 0x049E, NES_ROOM }, { 0x199C8, 0x044B, NES_ROOM }, { 0x1A0B1, 0x0478, NES_ROOM }, { 0x1A529, 0x043F, NES_ROOM },
	{ 0x1A968, 0x03C8, NES_ROOM }, { 0x1AD30, 0x086F, NES_ROOM }, { 0x1B5FF, 0x069B, NES_ROOM }, { 0x1C001, 0x0AA9, NES_ROOM }, { 0x1CC97, 0x049E, NES_ROOM },
	{ 0x1D42C, 0x05A8, NES_ROOM }, { 0x1DF71, 0x054E, NES_ROOM }, { 0x1E9D1, 0x0606, NES_ROOM }, { 0x1F0A2, 0x039A, NES_ROOM }, { 0x1F74E, 0x071C, NES_ROOM },
	{ 0x20001, 0x04B5, NES_ROOM }, { 0x2052E, 0x04FF, NES_ROOM }, { 0x2172E, 0x05DB, NES_ROOM }, { 0x21EAD, 0x0489, NES_ROOM }, { 0x223E1, 0x0465, NES_ROOM },
	{ 0x228AC, 0x0957, NES_ROOM }, { 0x24001, 0x037E, NES_ROOM }, { 0x2481A, 0x03CA, NES_ROOM }, { 0x24BE4, 0x050D, NES_ROOM }, { 0x252C0, 0x0346, NES_ROOM },
	{ 0x1BD30, 0x01CA, NES_ROOM }, { 0x25606, 0x046D, NES_ROOM }, { 0x25A73, 0x055A, NES_ROOM }, { 0x25FCD, 0x0654, NES_ROOM }, { 0x26C98, 0x024B, NES_ROOM },
	{ 0x26EE3, 0x01FA, NES_ROOM }, { 0x271DD, 0x0217, NES_ROOM }, { 0x27713, 0x02F4, NES_ROOM }, { 0x28001, 0x045C, NES_ROOM }, { 0x284CE, 0x0975, NES_ROOM },
	{ 0x28E97, 0x05E6, NES_ROOM }, { 0x27C3A, 0x0201, NES_ROOM }, { 0x2A9D6, 0x0325, NES_ROOM }, { 0x2AF88, 0x01FC, NES_ROOM }, { 0x2B184, 0x02A9, NES_ROOM },
	{ 0x2B42D, 0x02DF, NES_ROOM }, { 0x2B818, 0x03EC, NES_ROOM }, { 0x2BD67, 0x0209, NES_ROOM }, { 0x2C001, 0x0168, NES_ROOM }, { 0x2C4BF, 0x0169, NES_ROOM }
};
static const ScummNESFile::Resource res_rooms_ger[55] = {
	{ 0x00000, 0x0000, NES_ROOM }, { 0x14001, 0x0D63, NES_ROOM }, { 0x131E0, 0x04A9, NES_ROOM }, { 0x13689, 0x086B, NES_ROOM }, { 0x15421, 0x06A8, NES_ROOM },
	{ 0x15B5D, 0x0731, NES_ROOM }, { 0x16507, 0x0501, NES_ROOM }, { 0x16A08, 0x0AE9, NES_ROOM }, { 0x18001, 0x06DA, NES_ROOM }, { 0x17880, 0x03D0, NES_ROOM },
	{ 0x18C7B, 0x0651, NES_ROOM }, { 0x19328, 0x04A7, NES_ROOM }, { 0x199FE, 0x0447, NES_ROOM }, { 0x1A0F1, 0x0486, NES_ROOM }, { 0x1A577, 0x045D, NES_ROOM },
	{ 0x1A9D4, 0x03AE, NES_ROOM }, { 0x1AD82, 0x0840, NES_ROOM }, { 0x1B622, 0x06C3, NES_ROOM }, { 0x1C001, 0x0B07, NES_ROOM }, { 0x1CD05, 0x0494, NES_ROOM },
	{ 0x1D4A5, 0x05AC, NES_ROOM }, { 0x1DFD6, 0x0524, NES_ROOM }, { 0x1E9C0, 0x05F7, NES_ROOM }, { 0x1F09A, 0x038E, NES_ROOM }, { 0x1F75F, 0x0733, NES_ROOM },
	{ 0x20001, 0x04A9, NES_ROOM }, { 0x2052A, 0x052E, NES_ROOM }, { 0x2177C, 0x0621, NES_ROOM }, { 0x21F57, 0x0495, NES_ROOM }, { 0x2249A, 0x045E, NES_ROOM },
	{ 0x2295E, 0x0951, NES_ROOM }, { 0x24001, 0x036E, NES_ROOM }, { 0x247F9, 0x03CA, NES_ROOM }, { 0x24BC3, 0x050D, NES_ROOM }, { 0x252A8, 0x0346, NES_ROOM },
	{ 0x17CA2, 0x01CA, NES_ROOM }, { 0x255EE, 0x046F, NES_ROOM }, { 0x25A5D, 0x054D, NES_ROOM }, { 0x25FAA, 0x064B, NES_ROOM }, { 0x26BE2, 0x024B, NES_ROOM },
	{ 0x26E2D, 0x01FA, NES_ROOM }, { 0x2710F, 0x0217, NES_ROOM }, { 0x27663, 0x02F4, NES_ROOM }, { 0x28001, 0x045C, NES_ROOM }, { 0x284CE, 0x0A8F, NES_ROOM },
	{ 0x28FB1, 0x05FF, NES_ROOM }, { 0x27B69, 0x0201, NES_ROOM }, { 0x2AAA9, 0x0325, NES_ROOM }, { 0x1BD7C, 0x01FC, NES_ROOM }, { 0x2B031, 0x02A9, NES_ROOM },
	{ 0x2B2DA, 0x02D8, NES_ROOM }, { 0x2B6D2, 0x03D2, NES_ROOM }, { 0x2BC0D, 0x020D, NES_ROOM }, { 0x2C001, 0x0168, NES_ROOM }, { 0x27E11, 0x0169, NES_ROOM }
};
#endif
static const ScummNESFile::Resource *res_rooms[ScummNESFile::kROMsetNum] = {
	res_rooms_usa,
	res_rooms_eur,
	res_rooms_swe,
	res_rooms_fra,
	res_rooms_ger,
};

#ifdef PALMOS_68K
static const ScummNESFile::Resource *res_scripts_usa;
static const ScummNESFile::Resource *res_scripts_eur;
static const ScummNESFile::Resource *res_scripts_swe;
static const ScummNESFile::Resource *res_scripts_fra;
static const ScummNESFile::Resource *res_scripts_ger;
#else
static const ScummNESFile::Resource res_scripts_usa[179] = {
	{ 0x00000, 0x0000, NES_SCRIPT }, { 0x29966, 0x044D, NES_SCRIPT }, { 0x29DB3, 0x0207, NES_SCRIPT }, { 0x29FBA, 0x009F, NES_SCRIPT }, { 0x2A059, 0x03F4, NES_SCRIPT },
	{ 0x2A44D, 0x01A1, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2A5EE, 0x004A, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2A638, 0x0005, NES_SCRIPT },
	{ 0x2C6AA, 0x000D, NES_SCRIPT }, { 0x2C6B7, 0x000D, NES_SCRIPT }, { 0x186BB, 0x0040, NES_SCRIPT }, { 0x186FB, 0x0016, NES_SCRIPT }, { 0x1B639, 0x0046, NES_SCRIPT },
	{ 0x1EEC6, 0x00BD, NES_SCRIPT }, { 0x21C31, 0x0055, NES_SCRIPT }, { 0x177A8, 0x0027, NES_SCRIPT }, { 0x1FD07, 0x0027, NES_SCRIPT }, { 0x1FD2E, 0x0027, NES_SCRIPT },
	{ 0x1BD2B, 0x0022, NES_SCRIPT }, { 0x15BE0, 0x0088, NES_SCRIPT }, { 0x22241, 0x0020, NES_SCRIPT }, { 0x22261, 0x008F, NES_SCRIPT }, { 0x1924A, 0x002B, NES_SCRIPT },
	{ 0x1CB4A, 0x0061, NES_SCRIPT }, { 0x1CBAB, 0x003C, NES_SCRIPT }, { 0x1CBE7, 0x0042, NES_SCRIPT }, { 0x1CC29, 0x004F, NES_SCRIPT }, { 0x2049B, 0x0076, NES_SCRIPT },
	{ 0x16A96, 0x0035, NES_SCRIPT }, { 0x16ACB, 0x001C, NES_SCRIPT }, { 0x16AE7, 0x0014, NES_SCRIPT }, { 0x16AFB, 0x001C, NES_SCRIPT }, { 0x16B17, 0x0027, NES_SCRIPT },
	{ 0x16B3E, 0x01AA, NES_SCRIPT }, { 0x1D1CF, 0x0096, NES_SCRIPT }, { 0x1D265, 0x010E, NES_SCRIPT }, { 0x1D373, 0x001C, NES_SCRIPT }, { 0x1D38F, 0x0056, NES_SCRIPT },
	{ 0x1D3E5, 0x0072, NES_SCRIPT }, { 0x1E480, 0x0028, NES_SCRIPT }, { 0x1E4A8, 0x017D, NES_SCRIPT }, { 0x1E625, 0x0229, NES_SCRIPT }, { 0x28932, 0x0071, NES_SCRIPT },
	{ 0x17EB8, 0x004D, NES_SCRIPT }, { 0x162ED, 0x0039, NES_SCRIPT }, { 0x18711, 0x028B, NES_SCRIPT }, { 0x1899C, 0x00BB, NES_SCRIPT }, { 0x18A57, 0x018B, NES_SCRIPT },
	{ 0x00000, 0x0000, NES_SCRIPT }, { 0x19E87, 0x00ED, NES_SCRIPT }, { 0x21C86, 0x00F6, NES_SCRIPT }, { 0x1E84E, 0x009B, NES_SCRIPT }, { 0x21D7C, 0x0047, NES_SCRIPT },
	{ 0x2C6C4, 0x004D, NES_SCRIPT }, { 0x16326, 0x0024, NES_SCRIPT }, { 0x14D0D, 0x0014, NES_SCRIPT }, { 0x177CF, 0x0059, NES_SCRIPT }, { 0x17828, 0x0109, NES_SCRIPT },
	{ 0x17931, 0x0009, NES_SCRIPT }, { 0x14D21, 0x01B6, NES_SCRIPT }, { 0x2B0F6, 0x0243, NES_SCRIPT }, { 0x230BF, 0x067F, NES_SCRIPT }, { 0x2C711, 0x001C, NES_SCRIPT },
	{ 0x2C72D, 0x001A, NES_SCRIPT }, { 0x2C747, 0x0021, NES_SCRIPT }, { 0x2C768, 0x0024, NES_SCRIPT }, { 0x2C78C, 0x0017, NES_SCRIPT }, { 0x2C7A3, 0x0017, NES_SCRIPT },
	{ 0x2C7BA, 0x0014, NES_SCRIPT }, { 0x2C7CE, 0x0024, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2C7F2, 0x0011, NES_SCRIPT }, { 0x1793A, 0x009D, NES_SCRIPT },
	{ 0x22750, 0x0066, NES_SCRIPT }, { 0x14ED7, 0x0075, NES_SCRIPT }, { 0x1F30C, 0x0120, NES_SCRIPT }, { 0x1FD55, 0x001D, NES_SCRIPT }, { 0x1F42C, 0x008F, NES_SCRIPT },
	{ 0x1F4BB, 0x0097, NES_SCRIPT }, { 0x179D7, 0x006A, NES_SCRIPT }, { 0x17A41, 0x0030, NES_SCRIPT }, { 0x1F552, 0x0092, NES_SCRIPT }, { 0x2C803, 0x00CC, NES_SCRIPT },
	{ 0x2C8CF, 0x00BA, NES_SCRIPT }, { 0x2C989, 0x0088, NES_SCRIPT }, { 0x20A09, 0x01B0, NES_SCRIPT }, { 0x20BB9, 0x0168, NES_SCRIPT }, { 0x20D21, 0x006C, NES_SCRIPT },
	{ 0x20D8D, 0x0037, NES_SCRIPT }, { 0x20DC4, 0x00E4, NES_SCRIPT }, { 0x20EA8, 0x0045, NES_SCRIPT }, { 0x20EED, 0x00E1, NES_SCRIPT }, { 0x20FCE, 0x00F6, NES_SCRIPT },
	{ 0x210C4, 0x0141, NES_SCRIPT }, { 0x21205, 0x0183, NES_SCRIPT }, { 0x21388, 0x0034, NES_SCRIPT }, { 0x213BC, 0x00A9, NES_SCRIPT }, { 0x24367, 0x011B, NES_SCRIPT },
	{ 0x1BD4D, 0x0070, NES_SCRIPT }, { 0x1CC78, 0x0091, NES_SCRIPT }, { 0x29372, 0x0054, NES_SCRIPT }, { 0x19F74, 0x00CE, NES_SCRIPT }, { 0x1A042, 0x0077, NES_SCRIPT },
	{ 0x14F4C, 0x0057, NES_SCRIPT }, { 0x27886, 0x02DF, NES_SCRIPT }, { 0x1DA2A, 0x0219, NES_SCRIPT }, { 0x1DC43, 0x00F9, NES_SCRIPT }, { 0x1DD3C, 0x0056, NES_SCRIPT },
	{ 0x1DD92, 0x01C2, NES_SCRIPT }, { 0x14FA3, 0x004D, NES_SCRIPT }, { 0x27594, 0x00D9, NES_SCRIPT }, { 0x21DC3, 0x0013, NES_SCRIPT }, { 0x2A63D, 0x00F0, NES_SCRIPT },
	{ 0x24482, 0x00E7, NES_SCRIPT }, { 0x21465, 0x00F2, NES_SCRIPT }, { 0x24569, 0x002B, NES_SCRIPT }, { 0x2C3CF, 0x010F, NES_SCRIPT }, { 0x24594, 0x00AA, NES_SCRIPT },
	{ 0x24CE8, 0x0DAB, NES_SCRIPT }, { 0x1B67F, 0x000D, NES_SCRIPT }, { 0x1B68C, 0x000D, NES_SCRIPT }, { 0x2373E, 0x017C, NES_SCRIPT }, { 0x282F5, 0x01E1, NES_SCRIPT },
	{ 0x238BA, 0x0153, NES_SCRIPT }, { 0x23A0D, 0x019C, NES_SCRIPT }, { 0x23BA9, 0x0016, NES_SCRIPT }, { 0x2C4DE, 0x005C, NES_SCRIPT }, { 0x23BBF, 0x0020, NES_SCRIPT },
	{ 0x27D66, 0x00A5, NES_SCRIPT }, { 0x2A72D, 0x034D, NES_SCRIPT }, { 0x14FF0, 0x00E3, NES_SCRIPT }, { 0x2BABC, 0x005F, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT },
	{ 0x25A93, 0x003C, NES_SCRIPT }, { 0x1E8E9, 0x0011, NES_SCRIPT }, { 0x1634A, 0x0018, NES_SCRIPT }, { 0x26DFD, 0x001F, NES_SCRIPT }, { 0x26E1C, 0x0054, NES_SCRIPT },
	{ 0x26E70, 0x0149, NES_SCRIPT }, { 0x26FB9, 0x004B, NES_SCRIPT }, { 0x27004, 0x017D, NES_SCRIPT }, { 0x27181, 0x0027, NES_SCRIPT }, { 0x271A8, 0x0041, NES_SCRIPT },
	{ 0x271E9, 0x01B1, NES_SCRIPT }, { 0x16362, 0x001F, NES_SCRIPT }, { 0x2463E, 0x002A, NES_SCRIPT }, { 0x150D3, 0x019E, NES_SCRIPT }, { 0x19275, 0x0031, NES_SCRIPT },
	{ 0x17A71, 0x007C, NES_SCRIPT }, { 0x21557, 0x00DC, NES_SCRIPT }, { 0x1D457, 0x0018, NES_SCRIPT }, { 0x1D46F, 0x0053, NES_SCRIPT }, { 0x18BE2, 0x0005, NES_SCRIPT },
	{ 0x15271, 0x011B, NES_SCRIPT }, { 0x1538C, 0x000B, NES_SCRIPT }, { 0x24668, 0x0138, NES_SCRIPT }, { 0x247A0, 0x0014, NES_SCRIPT }, { 0x1DF54, 0x0018, NES_SCRIPT },
	{ 0x247B4, 0x0027, NES_SCRIPT }, { 0x1A0B9, 0x004D, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2BB1B, 0x00A5, NES_SCRIPT }, { 0x2AA7A, 0x00C1, NES_SCRIPT },
	{ 0x2AB3B, 0x0140, NES_SCRIPT }, { 0x19826, 0x00BF, NES_SCRIPT }, { 0x198E5, 0x014D, NES_SCRIPT }, { 0x19A32, 0x0012, NES_SCRIPT }, { 0x2AC7B, 0x0005, NES_SCRIPT },
	{ 0x2AC80, 0x0005, NES_SCRIPT }, { 0x2AC85, 0x0005, NES_SCRIPT }, { 0x2AC8A, 0x0005, NES_SCRIPT }, { 0x2AC8F, 0x0005, NES_SCRIPT }, { 0x21633, 0x0033, NES_SCRIPT },
	{ 0x2AC94, 0x0005, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2AC99, 0x009C, NES_SCRIPT }, { 0x2AD35, 0x009C, NES_SCRIPT }
};
static const ScummNESFile::Resource res_scripts_eur[179] = {
	{ 0x00000, 0x0000, NES_SCRIPT }, { 0x29966, 0x044D, NES_SCRIPT }, { 0x29DB3, 0x0207, NES_SCRIPT }, { 0x29FBA, 0x009F, NES_SCRIPT }, { 0x2A059, 0x03F4, NES_SCRIPT },
	{ 0x2A44D, 0x01A1, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2A5EE, 0x005C, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2A64A, 0x0005, NES_SCRIPT },
	{ 0x2C6AA, 0x000D, NES_SCRIPT }, { 0x2C6B7, 0x000D, NES_SCRIPT }, { 0x17889, 0x0040, NES_SCRIPT }, { 0x178C9, 0x0016, NES_SCRIPT }, { 0x1B6AE, 0x0046, NES_SCRIPT },
	{ 0x1EE47, 0x00BD, NES_SCRIPT }, { 0x21C3B, 0x0055, NES_SCRIPT }, { 0x18AC0, 0x0027, NES_SCRIPT }, { 0x1FC88, 0x0027, NES_SCRIPT }, { 0x1FCAF, 0x0027, NES_SCRIPT },
	{ 0x1BDA0, 0x0022, NES_SCRIPT }, { 0x15BE0, 0x0088, NES_SCRIPT }, { 0x2224B, 0x0020, NES_SCRIPT }, { 0x2226B, 0x008F, NES_SCRIPT }, { 0x1947D, 0x002B, NES_SCRIPT },
	{ 0x1CACB, 0x0061, NES_SCRIPT }, { 0x1CB2C, 0x003C, NES_SCRIPT }, { 0x1CB68, 0x0042, NES_SCRIPT }, { 0x1CBAA, 0x004F, NES_SCRIPT }, { 0x2049B, 0x0076, NES_SCRIPT },
	{ 0x16A96, 0x0035, NES_SCRIPT }, { 0x16ACB, 0x001C, NES_SCRIPT }, { 0x16AE7, 0x0014, NES_SCRIPT }, { 0x16AFB, 0x001C, NES_SCRIPT }, { 0x16B17, 0x0027, NES_SCRIPT },
	{ 0x16B3E, 0x01AA, NES_SCRIPT }, { 0x1D150, 0x0096, NES_SCRIPT }, { 0x1D1E6, 0x010E, NES_SCRIPT }, { 0x1D2F4, 0x001C, NES_SCRIPT }, { 0x1D310, 0x0056, NES_SCRIPT },
	{ 0x1D366, 0x0072, NES_SCRIPT }, { 0x1E401, 0x0028, NES_SCRIPT }, { 0x1E429, 0x017D, NES_SCRIPT }, { 0x1E5A6, 0x0229, NES_SCRIPT }, { 0x28932, 0x0071, NES_SCRIPT },
	{ 0x13EC6, 0x004D, NES_SCRIPT }, { 0x162ED, 0x0039, NES_SCRIPT }, { 0x178DF, 0x028B, NES_SCRIPT }, { 0x17B6A, 0x00BB, NES_SCRIPT }, { 0x17C25, 0x018B, NES_SCRIPT },
	{ 0x00000, 0x0000, NES_SCRIPT }, { 0x19FE3, 0x00ED, NES_SCRIPT }, { 0x21C90, 0x00F6, NES_SCRIPT }, { 0x1E7CF, 0x009B, NES_SCRIPT }, { 0x21D86, 0x0047, NES_SCRIPT },
	{ 0x2C6C4, 0x004D, NES_SCRIPT }, { 0x16326, 0x0024, NES_SCRIPT }, { 0x14D0D, 0x0014, NES_SCRIPT }, { 0x18AE7, 0x0059, NES_SCRIPT }, { 0x18B40, 0x011E, NES_SCRIPT },
	{ 0x18C5E, 0x0009, NES_SCRIPT }, { 0x14D21, 0x01B6, NES_SCRIPT }, { 0x2B108, 0x0243, NES_SCRIPT }, { 0x230C9, 0x067F, NES_SCRIPT }, { 0x2C711, 0x001C, NES_SCRIPT },
	{ 0x2C72D, 0x001A, NES_SCRIPT }, { 0x2C747, 0x0021, NES_SCRIPT }, { 0x2C768, 0x0024, NES_SCRIPT }, { 0x2C78C, 0x0017, NES_SCRIPT }, { 0x2C7A3, 0x0017, NES_SCRIPT },
	{ 0x2C7BA, 0x0014, NES_SCRIPT }, { 0x2C7CE, 0x0024, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2C7F2, 0x0011, NES_SCRIPT }, { 0x18C67, 0x009D, NES_SCRIPT },
	{ 0x2275A, 0x0066, NES_SCRIPT }, { 0x14ED7, 0x0075, NES_SCRIPT }, { 0x1F28D, 0x0120, NES_SCRIPT }, { 0x1FCD6, 0x001D, NES_SCRIPT }, { 0x1F3AD, 0x008F, NES_SCRIPT },
	{ 0x1F43C, 0x0097, NES_SCRIPT }, { 0x18D04, 0x006A, NES_SCRIPT }, { 0x18D6E, 0x0030, NES_SCRIPT }, { 0x1F4D3, 0x0092, NES_SCRIPT }, { 0x2C803, 0x00CC, NES_SCRIPT },
	{ 0x2C8CF, 0x00BA, NES_SCRIPT }, { 0x2C989, 0x0088, NES_SCRIPT }, { 0x20A09, 0x01B0, NES_SCRIPT }, { 0x20BB9, 0x0168, NES_SCRIPT }, { 0x20D21, 0x006C, NES_SCRIPT },
	{ 0x20D8D, 0x0037, NES_SCRIPT }, { 0x20DC4, 0x00E4, NES_SCRIPT }, { 0x20EA8, 0x0045, NES_SCRIPT }, { 0x20EED, 0x00E1, NES_SCRIPT }, { 0x20FCE, 0x00F6, NES_SCRIPT },
	{ 0x210C4, 0x0141, NES_SCRIPT }, { 0x21205, 0x0183, NES_SCRIPT }, { 0x21388, 0x0034, NES_SCRIPT }, { 0x213BC, 0x00A9, NES_SCRIPT }, { 0x24367, 0x011B, NES_SCRIPT },
	{ 0x1BDC2, 0x0070, NES_SCRIPT }, { 0x1CBF9, 0x0091, NES_SCRIPT }, { 0x29372, 0x0054, NES_SCRIPT }, { 0x1A0D0, 0x00CE, NES_SCRIPT }, { 0x1A19E, 0x0077, NES_SCRIPT },
	{ 0x14F4C, 0x0057, NES_SCRIPT }, { 0x2790A, 0x02DF, NES_SCRIPT }, { 0x1D9AB, 0x0219, NES_SCRIPT }, { 0x1DBC4, 0x00F9, NES_SCRIPT }, { 0x1DCBD, 0x0056, NES_SCRIPT },
	{ 0x1DD13, 0x01C2, NES_SCRIPT }, { 0x14FA3, 0x004D, NES_SCRIPT }, { 0x27618, 0x00D9, NES_SCRIPT }, { 0x21DCD, 0x0013, NES_SCRIPT }, { 0x2A64F, 0x00F0, NES_SCRIPT },
	{ 0x24482, 0x00E7, NES_SCRIPT }, { 0x21465, 0x00F2, NES_SCRIPT }, { 0x24569, 0x002B, NES_SCRIPT }, { 0x2C3CF, 0x010F, NES_SCRIPT }, { 0x24594, 0x00AA, NES_SCRIPT },
	{ 0x250B2, 0x0DAB, NES_SCRIPT }, { 0x1B6F4, 0x000D, NES_SCRIPT }, { 0x1B701, 0x000D, NES_SCRIPT }, { 0x23748, 0x017C, NES_SCRIPT }, { 0x282F5, 0x01E1, NES_SCRIPT },
	{ 0x238C4, 0x0153, NES_SCRIPT }, { 0x23A17, 0x019C, NES_SCRIPT }, { 0x23BB3, 0x0016, NES_SCRIPT }, { 0x2C4DE, 0x005C, NES_SCRIPT }, { 0x23BC9, 0x0020, NES_SCRIPT },
	{ 0x27DEA, 0x00A5, NES_SCRIPT }, { 0x2A73F, 0x034D, NES_SCRIPT }, { 0x14FF0, 0x00E3, NES_SCRIPT }, { 0x2BACE, 0x005F, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT },
	{ 0x25E5D, 0x003C, NES_SCRIPT }, { 0x1E86A, 0x0011, NES_SCRIPT }, { 0x1634A, 0x0018, NES_SCRIPT }, { 0x26E81, 0x001F, NES_SCRIPT }, { 0x26EA0, 0x0054, NES_SCRIPT },
	{ 0x26EF4, 0x0149, NES_SCRIPT }, { 0x2703D, 0x004B, NES_SCRIPT }, { 0x27088, 0x017D, NES_SCRIPT }, { 0x27205, 0x0027, NES_SCRIPT }, { 0x2722C, 0x0041, NES_SCRIPT },
	{ 0x2726D, 0x01B1, NES_SCRIPT }, { 0x16362, 0x001F, NES_SCRIPT }, { 0x2463E, 0x002A, NES_SCRIPT }, { 0x150D3, 0x019E, NES_SCRIPT }, { 0x194A8, 0x0031, NES_SCRIPT },
	{ 0x18D9E, 0x007C, NES_SCRIPT }, { 0x21557, 0x00DC, NES_SCRIPT }, { 0x1D3D8, 0x0018, NES_SCRIPT }, { 0x1D3F0, 0x0053, NES_SCRIPT }, { 0x17DB0, 0x0005, NES_SCRIPT },
	{ 0x15271, 0x011B, NES_SCRIPT }, { 0x1538C, 0x000B, NES_SCRIPT }, { 0x24668, 0x0138, NES_SCRIPT }, { 0x247A0, 0x0014, NES_SCRIPT }, { 0x1DED5, 0x0018, NES_SCRIPT },
	{ 0x247B4, 0x0027, NES_SCRIPT }, { 0x1A215, 0x004D, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2BB2D, 0x00A5, NES_SCRIPT }, { 0x2AA8C, 0x00C1, NES_SCRIPT },
	{ 0x2AB4D, 0x0140, NES_SCRIPT }, { 0x19982, 0x00BF, NES_SCRIPT }, { 0x19A41, 0x014D, NES_SCRIPT }, { 0x19B8E, 0x0012, NES_SCRIPT }, { 0x2AC8D, 0x0005, NES_SCRIPT },
	{ 0x2AC92, 0x0005, NES_SCRIPT }, { 0x2AC97, 0x0005, NES_SCRIPT }, { 0x2AC9C, 0x0005, NES_SCRIPT }, { 0x2ACA1, 0x0005, NES_SCRIPT }, { 0x21633, 0x0033, NES_SCRIPT },
	{ 0x2ACA6, 0x0005, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2ACAB, 0x009C, NES_SCRIPT }, { 0x2AD47, 0x009C, NES_SCRIPT }
};
static const ScummNESFile::Resource res_scripts_swe[179] = {
	{ 0x00000, 0x0000, NES_SCRIPT }, { 0x28F80, 0x043B, NES_SCRIPT }, { 0x293BB, 0x0209, NES_SCRIPT }, { 0x295C4, 0x00AB, NES_SCRIPT }, { 0x2966F, 0x03FD, NES_SCRIPT },
	{ 0x29A6C, 0x01A1, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x29C0D, 0x005C, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x29C69, 0x0005, NES_SCRIPT },
	{ 0x2B980, 0x000D, NES_SCRIPT }, { 0x2B98D, 0x000D, NES_SCRIPT }, { 0x186C8, 0x0040, NES_SCRIPT }, { 0x18708, 0x0016, NES_SCRIPT }, { 0x1B4B1, 0x0046, NES_SCRIPT },
	{ 0x1EEBB, 0x00B8, NES_SCRIPT }, { 0x21CFA, 0x005C, NES_SCRIPT }, { 0x17537, 0x0027, NES_SCRIPT }, { 0x1FD0A, 0x0027, NES_SCRIPT }, { 0x1FD31, 0x0027, NES_SCRIPT },
	{ 0x1BBB5, 0x0022, NES_SCRIPT }, { 0x15BC0, 0x0085, NES_SCRIPT }, { 0x22324, 0x001E, NES_SCRIPT }, { 0x22342, 0x008F, NES_SCRIPT }, { 0x19252, 0x002B, NES_SCRIPT },
	{ 0x1CB1B, 0x006D, NES_SCRIPT }, { 0x1CB88, 0x004C, NES_SCRIPT }, { 0x1CBD4, 0x0044, NES_SCRIPT }, { 0x1CC18, 0x0053, NES_SCRIPT }, { 0x2049D, 0x0081, NES_SCRIPT },
	{ 0x1634C, 0x0035, NES_SCRIPT }, { 0x16381, 0x001C, NES_SCRIPT }, { 0x1639D, 0x0014, NES_SCRIPT }, { 0x163B1, 0x001C, NES_SCRIPT }, { 0x163CD, 0x0027, NES_SCRIPT },
	{ 0x163F4, 0x019B, NES_SCRIPT }, { 0x1D183, 0x0094, NES_SCRIPT }, { 0x1D217, 0x0117, NES_SCRIPT }, { 0x1D32E, 0x001C, NES_SCRIPT }, { 0x1D34A, 0x0056, NES_SCRIPT },
	{ 0x1D3A0, 0x0072, NES_SCRIPT }, { 0x1E47F, 0x0028, NES_SCRIPT }, { 0x1E4A7, 0x0175, NES_SCRIPT }, { 0x1E61C, 0x022B, NES_SCRIPT }, { 0x27C85, 0x0071, NES_SCRIPT },
	{ 0x17C86, 0x004A, NES_SCRIPT }, { 0x13DD6, 0x0039, NES_SCRIPT }, { 0x1871E, 0x0270, NES_SCRIPT }, { 0x1898E, 0x00C0, NES_SCRIPT }, { 0x18A4E, 0x01B6, NES_SCRIPT },
	{ 0x00000, 0x0000, NES_SCRIPT }, { 0x19DC9, 0x00EE, NES_SCRIPT }, { 0x21D56, 0x00F5, NES_SCRIPT }, { 0x1E847, 0x0094, NES_SCRIPT }, { 0x21E4B, 0x0047, NES_SCRIPT },
	{ 0x2B99A, 0x004D, NES_SCRIPT }, { 0x13E0F, 0x0024, NES_SCRIPT }, { 0x14D13, 0x0014, NES_SCRIPT }, { 0x1755E, 0x0054, NES_SCRIPT }, { 0x175B2, 0x011A, NES_SCRIPT },
	{ 0x176CC, 0x0009, NES_SCRIPT }, { 0x14D27, 0x01B9, NES_SCRIPT }, { 0x2AA0E, 0x0256, NES_SCRIPT }, { 0x231A4, 0x06D2, NES_SCRIPT }, { 0x2B9E7, 0x001D, NES_SCRIPT },
	{ 0x2BA04, 0x0016, NES_SCRIPT }, { 0x2BA1A, 0x002D, NES_SCRIPT }, { 0x2BA47, 0x0027, NES_SCRIPT }, { 0x2BA6E, 0x0016, NES_SCRIPT }, { 0x2BA84, 0x0014, NES_SCRIPT },
	{ 0x2BA98, 0x0015, NES_SCRIPT }, { 0x2BAAD, 0x0029, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2BAD6, 0x0010, NES_SCRIPT }, { 0x176D5, 0x00A2, NES_SCRIPT },
	{ 0x22831, 0x0066, NES_SCRIPT }, { 0x14EE0, 0x0077, NES_SCRIPT }, { 0x1F30B, 0x011A, NES_SCRIPT }, { 0x1FD58, 0x001D, NES_SCRIPT }, { 0x1F425, 0x0095, NES_SCRIPT },
	{ 0x1F4BA, 0x009E, NES_SCRIPT }, { 0x17777, 0x006F, NES_SCRIPT }, { 0x177E6, 0x002F, NES_SCRIPT }, { 0x1F558, 0x0098, NES_SCRIPT }, { 0x2BAE6, 0x00C4, NES_SCRIPT },
	{ 0x2BBAA, 0x00AE, NES_SCRIPT }, { 0x2BC58, 0x0088, NES_SCRIPT }, { 0x20A3C, 0x01BB, NES_SCRIPT }, { 0x20BF7, 0x0197, NES_SCRIPT }, { 0x20D8E, 0x006E, NES_SCRIPT },
	{ 0x20DFC, 0x0028, NES_SCRIPT }, { 0x20E24, 0x00EA, NES_SCRIPT }, { 0x20F0E, 0x0049, NES_SCRIPT }, { 0x20F57, 0x00E7, NES_SCRIPT }, { 0x2103E, 0x010C, NES_SCRIPT },
	{ 0x2114A, 0x0151, NES_SCRIPT }, { 0x2129B, 0x01B0, NES_SCRIPT }, { 0x2144B, 0x0034, NES_SCRIPT }, { 0x2147F, 0x00A9, NES_SCRIPT }, { 0x24379, 0x010E, NES_SCRIPT },
	{ 0x1BBD7, 0x0072, NES_SCRIPT }, { 0x1CC6B, 0x0092, NES_SCRIPT }, { 0x2898B, 0x0054, NES_SCRIPT }, { 0x19EB7, 0x00D3, NES_SCRIPT }, { 0x19F8A, 0x0077, NES_SCRIPT },
	{ 0x14F57, 0x0057, NES_SCRIPT }, { 0x2703E, 0x0307, NES_SCRIPT }, { 0x1D9FB, 0x024F, NES_SCRIPT }, { 0x1DC4A, 0x00E4, NES_SCRIPT }, { 0x1DD2E, 0x0059, NES_SCRIPT },
	{ 0x1DD87, 0x01C2, NES_SCRIPT }, { 0x14FAE, 0x004D, NES_SCRIPT }, { 0x26D52, 0x00D5, NES_SCRIPT }, { 0x21E92, 0x0013, NES_SCRIPT }, { 0x29C6E, 0x00F0, NES_SCRIPT },
	{ 0x24487, 0x00E0, NES_SCRIPT }, { 0x21528, 0x00F2, NES_SCRIPT }, { 0x24567, 0x0023, NES_SCRIPT }, { 0x2B6B2, 0x010B, NES_SCRIPT }, { 0x2458A, 0x00A1, NES_SCRIPT },
	{ 0x250A0, 0x018B, NES_SCRIPT }, { 0x1B4F7, 0x000D, NES_SCRIPT }, { 0x1B504, 0x000D, NES_SCRIPT }, { 0x23876, 0x018E, NES_SCRIPT }, { 0x27639, 0x01F0, NES_SCRIPT },
	{ 0x23A04, 0x017B, NES_SCRIPT }, { 0x23B7F, 0x01AC, NES_SCRIPT }, { 0x23D2B, 0x0016, NES_SCRIPT }, { 0x2B7BD, 0x005B, NES_SCRIPT }, { 0x23D41, 0x0020, NES_SCRIPT },
	{ 0x2A643, 0x00A6, NES_SCRIPT }, { 0x29D5E, 0x0399, NES_SCRIPT }, { 0x14FFB, 0x00D2, NES_SCRIPT }, { 0x2B1DE, 0x0063, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT },
	{ 0x2522B, 0x003C, NES_SCRIPT }, { 0x1E8DB, 0x0011, NES_SCRIPT }, { 0x13E33, 0x0018, NES_SCRIPT }, { 0x26585, 0x001F, NES_SCRIPT }, { 0x265A4, 0x0054, NES_SCRIPT },
	{ 0x265F8, 0x017D, NES_SCRIPT }, { 0x26775, 0x004B, NES_SCRIPT }, { 0x267C0, 0x0165, NES_SCRIPT }, { 0x26925, 0x0027, NES_SCRIPT }, { 0x2694C, 0x0041, NES_SCRIPT },
	{ 0x2698D, 0x01CB, NES_SCRIPT }, { 0x13E4B, 0x001F, NES_SCRIPT }, { 0x2462B, 0x002A, NES_SCRIPT }, { 0x150CD, 0x0187, NES_SCRIPT }, { 0x1927D, 0x0031, NES_SCRIPT },
	{ 0x17815, 0x0087, NES_SCRIPT }, { 0x2161A, 0x00D8, NES_SCRIPT }, { 0x1D412, 0x0018, NES_SCRIPT }, { 0x1D42A, 0x0058, NES_SCRIPT }, { 0x18C04, 0x0005, NES_SCRIPT },
	{ 0x15254, 0x0108, NES_SCRIPT }, { 0x1535C, 0x000B, NES_SCRIPT }, { 0x24655, 0x0139, NES_SCRIPT }, { 0x2478E, 0x0014, NES_SCRIPT }, { 0x1DF49, 0x0018, NES_SCRIPT },
	{ 0x247A2, 0x0027, NES_SCRIPT }, { 0x1A001, 0x004C, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2B241, 0x00A5, NES_SCRIPT }, { 0x2A0F7, 0x00B5, NES_SCRIPT },
	{ 0x2A1AC, 0x0140, NES_SCRIPT }, { 0x19759, 0x00CA, NES_SCRIPT }, { 0x19823, 0x014D, NES_SCRIPT }, { 0x19970, 0x0012, NES_SCRIPT }, { 0x2A2EC, 0x0005, NES_SCRIPT },
	{ 0x2A2F1, 0x0005, NES_SCRIPT }, { 0x2A2F6, 0x0005, NES_SCRIPT }, { 0x2A2FB, 0x0005, NES_SCRIPT }, { 0x2A300, 0x0005, NES_SCRIPT }, { 0x216F2, 0x0033, NES_SCRIPT },
	{ 0x2A305, 0x0005, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2A30A, 0x009C, NES_SCRIPT }, { 0x2A3A6, 0x009C, NES_SCRIPT }
};
static const ScummNESFile::Resource res_scripts_fra[179] = {
	{ 0x00000, 0x0000, NES_SCRIPT }, { 0x2947D, 0x0480, NES_SCRIPT }, { 0x298FD, 0x0226, NES_SCRIPT }, { 0x29B23, 0x0092, NES_SCRIPT }, { 0x29BB5, 0x040C, NES_SCRIPT },
	{ 0x29FC1, 0x01A1, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2A162, 0x005C, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2A1BE, 0x0005, NES_SCRIPT },
	{ 0x2C169, 0x000D, NES_SCRIPT }, { 0x2C176, 0x000D, NES_SCRIPT }, { 0x186E3, 0x0040, NES_SCRIPT }, { 0x18723, 0x0016, NES_SCRIPT }, { 0x1B59F, 0x0046, NES_SCRIPT },
	{ 0x1EFD7, 0x00CB, NES_SCRIPT }, { 0x21D09, 0x0054, NES_SCRIPT }, { 0x176C8, 0x0027, NES_SCRIPT }, { 0x1FE6A, 0x0027, NES_SCRIPT }, { 0x1FE91, 0x0027, NES_SCRIPT },
	{ 0x1BC9A, 0x0022, NES_SCRIPT }, { 0x15CD6, 0x0092, NES_SCRIPT }, { 0x22336, 0x001C, NES_SCRIPT }, { 0x22352, 0x008F, NES_SCRIPT }, { 0x192A5, 0x002B, NES_SCRIPT },
	{ 0x1CAAA, 0x0069, NES_SCRIPT }, { 0x1CB13, 0x0054, NES_SCRIPT }, { 0x1CB67, 0x0048, NES_SCRIPT }, { 0x1CBAF, 0x0058, NES_SCRIPT }, { 0x204B6, 0x0078, NES_SCRIPT },
	{ 0x16471, 0x0035, NES_SCRIPT }, { 0x164A6, 0x001C, NES_SCRIPT }, { 0x164C2, 0x0014, NES_SCRIPT }, { 0x164D6, 0x001C, NES_SCRIPT }, { 0x164F2, 0x0027, NES_SCRIPT },
	{ 0x16519, 0x01BB, NES_SCRIPT }, { 0x1D135, 0x008D, NES_SCRIPT }, { 0x1D1C2, 0x0119, NES_SCRIPT }, { 0x1D2DB, 0x001C, NES_SCRIPT }, { 0x1D2F7, 0x0056, NES_SCRIPT },
	{ 0x1D34D, 0x0072, NES_SCRIPT }, { 0x1E4BF, 0x0028, NES_SCRIPT }, { 0x1E4E7, 0x01E0, NES_SCRIPT }, { 0x1E6C7, 0x0241, NES_SCRIPT }, { 0x2845D, 0x0071, NES_SCRIPT },
	{ 0x17E48, 0x004C, NES_SCRIPT }, { 0x13DE3, 0x0039, NES_SCRIPT }, { 0x18739, 0x0296, NES_SCRIPT }, { 0x189CF, 0x00C2, NES_SCRIPT }, { 0x18A91, 0x01A5, NES_SCRIPT },
	{ 0x00000, 0x0000, NES_SCRIPT }, { 0x19E13, 0x00F3, NES_SCRIPT }, { 0x21D5D, 0x00F6, NES_SCRIPT }, { 0x1E908, 0x00B8, NES_SCRIPT }, { 0x21E53, 0x0047, NES_SCRIPT },
	{ 0x2C183, 0x004D, NES_SCRIPT }, { 0x13E1C, 0x0024, NES_SCRIPT }, { 0x14D77, 0x0014, NES_SCRIPT }, { 0x176EF, 0x0059, NES_SCRIPT }, { 0x17748, 0x013F, NES_SCRIPT },
	{ 0x17887, 0x0009, NES_SCRIPT }, { 0x14D8B, 0x01D4, NES_SCRIPT }, { 0x2ACFB, 0x028D, NES_SCRIPT }, { 0x23203, 0x0779, NES_SCRIPT }, { 0x2C1D0, 0x001B, NES_SCRIPT },
	{ 0x2C1EB, 0x001F, NES_SCRIPT }, { 0x2C20A, 0x0024, NES_SCRIPT }, { 0x2C22E, 0x0019, NES_SCRIPT }, { 0x2C247, 0x0018, NES_SCRIPT }, { 0x2C25F, 0x001D, NES_SCRIPT },
	{ 0x2C27C, 0x0016, NES_SCRIPT }, { 0x2C292, 0x0027, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2C2B9, 0x0011, NES_SCRIPT }, { 0x17890, 0x00AA, NES_SCRIPT },
	{ 0x22846, 0x0066, NES_SCRIPT }, { 0x14F5F, 0x0083, NES_SCRIPT }, { 0x1F43C, 0x013A, NES_SCRIPT }, { 0x1FEB8, 0x001D, NES_SCRIPT }, { 0x1F576, 0x0098, NES_SCRIPT },
	{ 0x1F60E, 0x009B, NES_SCRIPT }, { 0x1793A, 0x006E, NES_SCRIPT }, { 0x179A8, 0x0033, NES_SCRIPT }, { 0x1F6A9, 0x00A5, NES_SCRIPT }, { 0x2C2CA, 0x00BA, NES_SCRIPT },
	{ 0x2C384, 0x00AC, NES_SCRIPT }, { 0x2C430, 0x008F, NES_SCRIPT }, { 0x20A2D, 0x01BE, NES_SCRIPT }, { 0x20BEB, 0x0158, NES_SCRIPT }, { 0x20D43, 0x0079, NES_SCRIPT },
	{ 0x20DBC, 0x002B, NES_SCRIPT }, { 0x20DE7, 0x00E8, NES_SCRIPT }, { 0x20ECF, 0x004A, NES_SCRIPT }, { 0x20F19, 0x0110, NES_SCRIPT }, { 0x21029, 0x0136, NES_SCRIPT },
	{ 0x2115F, 0x0152, NES_SCRIPT }, { 0x212B1, 0x01B3, NES_SCRIPT }, { 0x21464, 0x0032, NES_SCRIPT }, { 0x21496, 0x00A9, NES_SCRIPT }, { 0x2437F, 0x0133, NES_SCRIPT },
	{ 0x1BCBC, 0x0074, NES_SCRIPT }, { 0x1CC07, 0x0090, NES_SCRIPT }, { 0x28E43, 0x0054, NES_SCRIPT }, { 0x19F06, 0x00DB, NES_SCRIPT }, { 0x19FE1, 0x0080, NES_SCRIPT },
	{ 0x14FE2, 0x0057, NES_SCRIPT }, { 0x273F4, 0x031F, NES_SCRIPT }, { 0x1D9D4, 0x0238, NES_SCRIPT }, { 0x1DC0C, 0x00FE, NES_SCRIPT }, { 0x1DD0A, 0x005A, NES_SCRIPT },
	{ 0x1DD64, 0x01F5, NES_SCRIPT }, { 0x15039, 0x004D, NES_SCRIPT }, { 0x270DD, 0x0100, NES_SCRIPT }, { 0x21E9A, 0x0013, NES_SCRIPT }, { 0x2A1C3, 0x00F0, NES_SCRIPT },
	{ 0x244B2, 0x00E4, NES_SCRIPT }, { 0x2153F, 0x00EC, NES_SCRIPT }, { 0x24596, 0x0033, NES_SCRIPT }, { 0x2BC04, 0x0108, NES_SCRIPT }, { 0x245C9, 0x009F, NES_SCRIPT },
	{ 0x250F1, 0x0193, NES_SCRIPT }, { 0x1B5E5, 0x000D, NES_SCRIPT }, { 0x1B5F2, 0x000D, NES_SCRIPT }, { 0x2397C, 0x0199, NES_SCRIPT }, { 0x27A07, 0x0233, NES_SCRIPT },
	{ 0x23B15, 0x0171, NES_SCRIPT }, { 0x23C86, 0x01BC, NES_SCRIPT }, { 0x23E42, 0x0016, NES_SCRIPT }, { 0x2BD0C, 0x005B, NES_SCRIPT }, { 0x23E58, 0x0020, NES_SCRIPT },
	{ 0x27E3B, 0x00B9, NES_SCRIPT }, { 0x2A2B3, 0x03D3, NES_SCRIPT }, { 0x15086, 0x00E4, NES_SCRIPT }, { 0x2B70C, 0x0067, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT },
	{ 0x25284, 0x003C, NES_SCRIPT }, { 0x1E9C0, 0x0011, NES_SCRIPT }, { 0x13E40, 0x0018, NES_SCRIPT }, { 0x26621, 0x001F, NES_SCRIPT }, { 0x26640, 0x0054, NES_SCRIPT },
	{ 0x26694, 0x0173, NES_SCRIPT }, { 0x26807, 0x004B, NES_SCRIPT }, { 0x26852, 0x0190, NES_SCRIPT }, { 0x269E2, 0x0027, NES_SCRIPT }, { 0x26A09, 0x0041, NES_SCRIPT },
	{ 0x26A4A, 0x024E, NES_SCRIPT }, { 0x13E58, 0x001F, NES_SCRIPT }, { 0x24668, 0x002A, NES_SCRIPT }, { 0x1516A, 0x01C9, NES_SCRIPT }, { 0x192D0, 0x0031, NES_SCRIPT },
	{ 0x179DB, 0x0088, NES_SCRIPT }, { 0x2162B, 0x00D0, NES_SCRIPT }, { 0x1D3BF, 0x0018, NES_SCRIPT }, { 0x1D3D7, 0x0055, NES_SCRIPT }, { 0x18C36, 0x0005, NES_SCRIPT },
	{ 0x15333, 0x0113, NES_SCRIPT }, { 0x15446, 0x000B, NES_SCRIPT }, { 0x24692, 0x014D, NES_SCRIPT }, { 0x247DF, 0x0014, NES_SCRIPT }, { 0x1DF59, 0x0018, NES_SCRIPT },
	{ 0x247F3, 0x0027, NES_SCRIPT }, { 0x1A061, 0x0050, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2B773, 0x00A5, NES_SCRIPT }, { 0x2A686, 0x00BA, NES_SCRIPT },
	{ 0x2A740, 0x0140, NES_SCRIPT }, { 0x1979F, 0x00CA, NES_SCRIPT }, { 0x19869, 0x014D, NES_SCRIPT }, { 0x199B6, 0x0012, NES_SCRIPT }, { 0x2A880, 0x0005, NES_SCRIPT },
	{ 0x2A885, 0x0005, NES_SCRIPT }, { 0x2A88A, 0x0005, NES_SCRIPT }, { 0x2A88F, 0x0005, NES_SCRIPT }, { 0x2A894, 0x0005, NES_SCRIPT }, { 0x216FB, 0x0033, NES_SCRIPT },
	{ 0x2A899, 0x0005, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2A89E, 0x009C, NES_SCRIPT }, { 0x2A93A, 0x009C, NES_SCRIPT }
};
static const ScummNESFile::Resource res_scripts_ger[179] = {
	{ 0x00000, 0x0000, NES_SCRIPT }, { 0x295B0, 0x045A, NES_SCRIPT }, { 0x29A0A, 0x0218, NES_SCRIPT }, { 0x29C22, 0x00B1, NES_SCRIPT }, { 0x29CD3, 0x0408, NES_SCRIPT },
	{ 0x2A0DB, 0x01A1, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2A27C, 0x005C, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2A2D8, 0x0005, NES_SCRIPT },
	{ 0x2C169, 0x000D, NES_SCRIPT }, { 0x2C176, 0x000D, NES_SCRIPT }, { 0x186DB, 0x0040, NES_SCRIPT }, { 0x1871B, 0x0016, NES_SCRIPT }, { 0x1B5C2, 0x0046, NES_SCRIPT },
	{ 0x1EFB7, 0x00E3, NES_SCRIPT }, { 0x21D9D, 0x0069, NES_SCRIPT }, { 0x174F1, 0x0027, NES_SCRIPT }, { 0x1FE92, 0x0027, NES_SCRIPT }, { 0x1FEB9, 0x0027, NES_SCRIPT },
	{ 0x1BCE5, 0x0022, NES_SCRIPT }, { 0x13EF4, 0x0087, NES_SCRIPT }, { 0x223EC, 0x001F, NES_SCRIPT }, { 0x2240B, 0x008F, NES_SCRIPT }, { 0x192CC, 0x002B, NES_SCRIPT },
	{ 0x1CB08, 0x006E, NES_SCRIPT }, { 0x1CB76, 0x004E, NES_SCRIPT }, { 0x1CBC4, 0x004D, NES_SCRIPT }, { 0x1CC11, 0x0059, NES_SCRIPT }, { 0x204AA, 0x0080, NES_SCRIPT },
	{ 0x1628E, 0x0035, NES_SCRIPT }, { 0x162C3, 0x001C, NES_SCRIPT }, { 0x162DF, 0x0014, NES_SCRIPT }, { 0x162F3, 0x001C, NES_SCRIPT }, { 0x1630F, 0x0027, NES_SCRIPT },
	{ 0x16336, 0x01D1, NES_SCRIPT }, { 0x1D199, 0x00A0, NES_SCRIPT }, { 0x1D239, 0x011C, NES_SCRIPT }, { 0x1D355, 0x001C, NES_SCRIPT }, { 0x1D371, 0x0056, NES_SCRIPT },
	{ 0x1D3C7, 0x0072, NES_SCRIPT }, { 0x1E4FA, 0x0028, NES_SCRIPT }, { 0x1E522, 0x019D, NES_SCRIPT }, { 0x1E6BF, 0x023B, NES_SCRIPT }, { 0x2845D, 0x0071, NES_SCRIPT },
	{ 0x17C50, 0x0052, NES_SCRIPT }, { 0x15AC9, 0x0039, NES_SCRIPT }, { 0x18731, 0x02E7, NES_SCRIPT }, { 0x18A18, 0x00BC, NES_SCRIPT }, { 0x18AD4, 0x01A2, NES_SCRIPT },
	{ 0x00000, 0x0000, NES_SCRIPT }, { 0x19E45, 0x00F8, NES_SCRIPT }, { 0x21E06, 0x00F7, NES_SCRIPT }, { 0x1E8FA, 0x00B5, NES_SCRIPT }, { 0x21EFD, 0x0047, NES_SCRIPT },
	{ 0x2C183, 0x004D, NES_SCRIPT }, { 0x15B02, 0x0024, NES_SCRIPT }, { 0x14D64, 0x0014, NES_SCRIPT }, { 0x17518, 0x005E, NES_SCRIPT }, { 0x17576, 0x0125, NES_SCRIPT },
	{ 0x1769B, 0x0009, NES_SCRIPT }, { 0x14D78, 0x01C7, NES_SCRIPT }, { 0x2ADCE, 0x0263, NES_SCRIPT }, { 0x232AF, 0x077F, NES_SCRIPT }, { 0x2C1D0, 0x001E, NES_SCRIPT },
	{ 0x2C1EE, 0x0024, NES_SCRIPT }, { 0x2C212, 0x002E, NES_SCRIPT }, { 0x2C240, 0x0022, NES_SCRIPT }, { 0x2C262, 0x0013, NES_SCRIPT }, { 0x2C275, 0x001E, NES_SCRIPT },
	{ 0x2C293, 0x0016, NES_SCRIPT }, { 0x2C2A9, 0x0027, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2C2D0, 0x0012, NES_SCRIPT }, { 0x176A4, 0x00A4, NES_SCRIPT },
	{ 0x228F8, 0x0066, NES_SCRIPT }, { 0x14F3F, 0x007F, NES_SCRIPT }, { 0x1F428, 0x013A, NES_SCRIPT }, { 0x1FEE0, 0x001D, NES_SCRIPT }, { 0x1F562, 0x00A0, NES_SCRIPT },
	{ 0x1F602, 0x00A4, NES_SCRIPT }, { 0x17748, 0x0076, NES_SCRIPT }, { 0x177BE, 0x0036, NES_SCRIPT }, { 0x1F6A6, 0x00B9, NES_SCRIPT }, { 0x2C2E2, 0x00CB, NES_SCRIPT },
	{ 0x2C3AD, 0x00B7, NES_SCRIPT }, { 0x2C464, 0x008A, NES_SCRIPT }, { 0x20A58, 0x01BD, NES_SCRIPT }, { 0x20C15, 0x0181, NES_SCRIPT }, { 0x20D96, 0x0078, NES_SCRIPT },
	{ 0x20E0E, 0x003C, NES_SCRIPT }, { 0x20E4A, 0x00E9, NES_SCRIPT }, { 0x20F33, 0x0046, NES_SCRIPT }, { 0x20F79, 0x00F6, NES_SCRIPT }, { 0x2106F, 0x0118, NES_SCRIPT },
	{ 0x21187, 0x015B, NES_SCRIPT }, { 0x212E2, 0x01AC, NES_SCRIPT }, { 0x2148E, 0x003F, NES_SCRIPT }, { 0x214CD, 0x00A9, NES_SCRIPT }, { 0x2436F, 0x0126, NES_SCRIPT },
	{ 0x1BD07, 0x0075, NES_SCRIPT }, { 0x1CC6A, 0x009B, NES_SCRIPT }, { 0x28F5D, 0x0054, NES_SCRIPT }, { 0x19F3D, 0x00E1, NES_SCRIPT }, { 0x1A01E, 0x0086, NES_SCRIPT },
	{ 0x14FBE, 0x0057, NES_SCRIPT }, { 0x27326, 0x033D, NES_SCRIPT }, { 0x1DA51, 0x023B, NES_SCRIPT }, { 0x1DC8C, 0x00FB, NES_SCRIPT }, { 0x1DD87, 0x0056, NES_SCRIPT },
	{ 0x1DDDD, 0x01E1, NES_SCRIPT }, { 0x15015, 0x004D, NES_SCRIPT }, { 0x27027, 0x00E8, NES_SCRIPT }, { 0x21F44, 0x0013, NES_SCRIPT }, { 0x2A2DD, 0x00F0, NES_SCRIPT },
	{ 0x24495, 0x00F8, NES_SCRIPT }, { 0x21576, 0x00F9, NES_SCRIPT }, { 0x2458D, 0x002B, NES_SCRIPT }, { 0x2BAA4, 0x010F, NES_SCRIPT }, { 0x245B8, 0x00A5, NES_SCRIPT },
	{ 0x250D0, 0x019C, NES_SCRIPT }, { 0x1B608, 0x000D, NES_SCRIPT }, { 0x1B615, 0x000D, NES_SCRIPT }, { 0x23A2E, 0x0185, NES_SCRIPT }, { 0x27957, 0x0212, NES_SCRIPT },
	{ 0x23BB3, 0x0158, NES_SCRIPT }, { 0x23D0B, 0x01C4, NES_SCRIPT }, { 0x23ECF, 0x0016, NES_SCRIPT }, { 0x2BBB3, 0x005A, NES_SCRIPT }, { 0x23EE5, 0x0020, NES_SCRIPT },
	{ 0x27D6A, 0x00A7, NES_SCRIPT }, { 0x2A3CD, 0x038C, NES_SCRIPT }, { 0x15062, 0x00F6, NES_SCRIPT }, { 0x2B5B2, 0x007B, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT },
	{ 0x2526C, 0x003C, NES_SCRIPT }, { 0x1E9AF, 0x0011, NES_SCRIPT }, { 0x15B26, 0x0018, NES_SCRIPT }, { 0x265F5, 0x001F, NES_SCRIPT }, { 0x26614, 0x0054, NES_SCRIPT },
	{ 0x26668, 0x018E, NES_SCRIPT }, { 0x267F6, 0x004B, NES_SCRIPT }, { 0x26841, 0x0196, NES_SCRIPT }, { 0x269D7, 0x0027, NES_SCRIPT }, { 0x269FE, 0x0041, NES_SCRIPT },
	{ 0x26A3F, 0x01A3, NES_SCRIPT }, { 0x15B3E, 0x001F, NES_SCRIPT }, { 0x2465D, 0x002A, NES_SCRIPT }, { 0x15158, 0x0198, NES_SCRIPT }, { 0x192F7, 0x0031, NES_SCRIPT },
	{ 0x177F4, 0x008C, NES_SCRIPT }, { 0x2166F, 0x00DA, NES_SCRIPT }, { 0x1D439, 0x0018, NES_SCRIPT }, { 0x1D451, 0x0054, NES_SCRIPT }, { 0x18C76, 0x0005, NES_SCRIPT },
	{ 0x152F0, 0x0126, NES_SCRIPT }, { 0x15416, 0x000B, NES_SCRIPT }, { 0x24687, 0x0137, NES_SCRIPT }, { 0x247BE, 0x0014, NES_SCRIPT }, { 0x1DFBE, 0x0018, NES_SCRIPT },
	{ 0x247D2, 0x0027, NES_SCRIPT }, { 0x1A0A4, 0x004D, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2B62D, 0x00A5, NES_SCRIPT }, { 0x2A759, 0x00BA, NES_SCRIPT },
	{ 0x2A813, 0x0140, NES_SCRIPT }, { 0x197CF, 0x00D0, NES_SCRIPT }, { 0x1989F, 0x014D, NES_SCRIPT }, { 0x199EC, 0x0012, NES_SCRIPT }, { 0x2A953, 0x0005, NES_SCRIPT },
	{ 0x2A958, 0x0005, NES_SCRIPT }, { 0x2A95D, 0x0005, NES_SCRIPT }, { 0x2A962, 0x0005, NES_SCRIPT }, { 0x2A967, 0x0005, NES_SCRIPT }, { 0x21749, 0x0033, NES_SCRIPT },
	{ 0x2A96C, 0x0005, NES_SCRIPT }, { 0x00000, 0x0000, NES_SCRIPT }, { 0x2A971, 0x009C, NES_SCRIPT }, { 0x2AA0D, 0x009C, NES_SCRIPT }
};
#endif
static const ScummNESFile::Resource *res_scripts[ScummNESFile::kROMsetNum] = {
	res_scripts_usa,
	res_scripts_eur,
	res_scripts_swe,
	res_scripts_fra,
	res_scripts_ger,
};

#ifdef PALMOS_68K
static const ScummNESFile::Resource *res_sounds_usa;
static const ScummNESFile::Resource *res_sounds_eur;
static const ScummNESFile::Resource *res_sounds_swe;
static const ScummNESFile::Resource *res_sounds_fra;
static const ScummNESFile::Resource *res_sounds_ger;
#else
static const ScummNESFile::Resource res_sounds_usa[82] = {
	{ 0x0FFE8, 0x000A, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND },
	{ 0x30ECA, 0x0832, NES_SOUND }, { 0x17FCA, 0x0011, NES_SOUND }, { 0x27E0B, 0x0073, NES_SOUND }, { 0x17FDB, 0x0011, NES_SOUND }, { 0x17FEC, 0x0011, NES_SOUND },
	{ 0x27E7E, 0x0056, NES_SOUND }, { 0x27ED4, 0x001F, NES_SOUND }, { 0x23FEE, 0x0011, NES_SOUND }, { 0x0FFF2, 0x000A, NES_SOUND }, { 0x27EF3, 0x000A, NES_SOUND },
	{ 0x27EFD, 0x0019, NES_SOUND }, { 0x27F16, 0x004B, NES_SOUND }, { 0x27F61, 0x000A, NES_SOUND }, { 0x27F6B, 0x000F, NES_SOUND }, { 0x27F7A, 0x001D, NES_SOUND },
	{ 0x27F97, 0x0045, NES_SOUND }, { 0x27FDC, 0x000F, NES_SOUND }, { 0x2FD42, 0x001B, NES_SOUND }, { 0x2FD5D, 0x0033, NES_SOUND }, { 0x27FEB, 0x0011, NES_SOUND },
	{ 0x2BFEF, 0x000F, NES_SOUND }, { 0x2FD90, 0x0075, NES_SOUND }, { 0x2FE05, 0x0014, NES_SOUND }, { 0x0FFE8, 0x000A, NES_SOUND }, { 0x2FE19, 0x00FF, NES_SOUND },
	{ 0x2FF18, 0x000F, NES_SOUND }, { 0x2FF27, 0x000F, NES_SOUND }, { 0x2FF36, 0x0092, NES_SOUND }, { 0x2FF36, 0x0092, NES_SOUND }, { 0x2FFC8, 0x002D, NES_SOUND },
	{ 0x316FC, 0x00F8, NES_SOUND }, { 0x317F4, 0x0016, NES_SOUND }, { 0x3180A, 0x0011, NES_SOUND }, { 0x3181B, 0x004B, NES_SOUND }, { 0x31866, 0x0011, NES_SOUND },
	{ 0x31877, 0x003B, NES_SOUND }, { 0x318B2, 0x008A, NES_SOUND }, { 0x3193C, 0x0011, NES_SOUND }, { 0x3194D, 0x000F, NES_SOUND }, { 0x3195C, 0x00A2, NES_SOUND },
	{ 0x319FE, 0x00D3, NES_SOUND }, { 0x31AD1, 0x0097, NES_SOUND }, { 0x2BFEF, 0x000F, NES_SOUND }, { 0x3195C, 0x00A2, NES_SOUND }, { 0x31B68, 0x05D1, NES_SOUND },
	{ 0x31B68, 0x05D1, NES_SOUND }, { 0x32139, 0x0011, NES_SOUND }, { 0x0FFE8, 0x000A, NES_SOUND }, { 0x2FD90, 0x0075, NES_SOUND }, { 0x27ED4, 0x001F, NES_SOUND },
	{ 0x3214A, 0x098E, NES_SOUND }, { 0x3181B, 0x004B, NES_SOUND }, { 0x32AD8, 0x0011, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND }, { 0x32AE9, 0x000F, NES_SOUND },
	{ 0x32AF8, 0x002F, NES_SOUND }, { 0x32B27, 0x001D, NES_SOUND }, { 0x32B44, 0x0018, NES_SOUND }, { 0x32B5C, 0x0016, NES_SOUND }, { 0x32B72, 0x001B, NES_SOUND },
	{ 0x32B8D, 0x0088, NES_SOUND }, { 0x32C15, 0x0065, NES_SOUND }, { 0x32C7A, 0x0065, NES_SOUND }, { 0x32CDF, 0x0073, NES_SOUND }, { 0x32D52, 0x00F9, NES_SOUND },
	{ 0x32E4B, 0x049E, NES_SOUND }, { 0x34001, 0x0EA8, NES_SOUND }, { 0x332E9, 0x0B18, NES_SOUND }, { 0x34EA9, 0x0B9C, NES_SOUND }, { 0x35A45, 0x0C6B, NES_SOUND },
	{ 0x366B0, 0x0E56, NES_SOUND }, { 0x38001, 0x0C70, NES_SOUND }, { 0x38C71, 0x0DEC, NES_SOUND }, { 0x39A5D, 0x0B77, NES_SOUND }, { 0x37506, 0x042F, NES_SOUND },
	{ 0x3A5D4, 0x0AC5, NES_SOUND }, { 0x3B099, 0x0BE4, NES_SOUND }
};
static const ScummNESFile::Resource res_sounds_eur[82] = {
	{ 0x0BF54, 0x000A, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND },
	{ 0x30ECA, 0x0832, NES_SOUND }, { 0x0BF5E, 0x0011, NES_SOUND }, { 0x27ECB, 0x0073, NES_SOUND }, { 0x0BF6F, 0x0011, NES_SOUND }, { 0x0FF5D, 0x0011, NES_SOUND },
	{ 0x316FC, 0x0056, NES_SOUND }, { 0x13F4E, 0x001F, NES_SOUND }, { 0x0FF6E, 0x0011, NES_SOUND }, { 0x13F6D, 0x000A, NES_SOUND }, { 0x1BF47, 0x000A, NES_SOUND },
	{ 0x1BF51, 0x0019, NES_SOUND }, { 0x31752, 0x004B, NES_SOUND }, { 0x1BF6A, 0x000A, NES_SOUND }, { 0x27F3E, 0x000F, NES_SOUND }, { 0x27F4D, 0x001D, NES_SOUND },
	{ 0x3179D, 0x0045, NES_SOUND }, { 0x27F6A, 0x000F, NES_SOUND }, { 0x2BF40, 0x001B, NES_SOUND }, { 0x317E2, 0x0033, NES_SOUND }, { 0x2BF5B, 0x0011, NES_SOUND },
	{ 0x2BF6C, 0x000F, NES_SOUND }, { 0x31815, 0x0075, NES_SOUND }, { 0x2FF6C, 0x0014, NES_SOUND }, { 0x0BF54, 0x000A, NES_SOUND }, { 0x3188A, 0x00FF, NES_SOUND },
	{ 0x31989, 0x000F, NES_SOUND }, { 0x31998, 0x000F, NES_SOUND }, { 0x319A7, 0x0092, NES_SOUND }, { 0x319A7, 0x0092, NES_SOUND }, { 0x31A39, 0x002D, NES_SOUND },
	{ 0x31A66, 0x00F8, NES_SOUND }, { 0x31B5E, 0x0016, NES_SOUND }, { 0x31B74, 0x0011, NES_SOUND }, { 0x31B85, 0x004B, NES_SOUND }, { 0x31BD0, 0x0011, NES_SOUND },
	{ 0x31BE1, 0x003B, NES_SOUND }, { 0x31C1C, 0x008A, NES_SOUND }, { 0x31CA6, 0x0011, NES_SOUND }, { 0x31CB7, 0x000F, NES_SOUND }, { 0x31CC6, 0x00A2, NES_SOUND },
	{ 0x31D68, 0x00D3, NES_SOUND }, { 0x31E3B, 0x0097, NES_SOUND }, { 0x2BF6C, 0x000F, NES_SOUND }, { 0x31CC6, 0x00A2, NES_SOUND }, { 0x31ED2, 0x05D1, NES_SOUND },
	{ 0x31ED2, 0x05D1, NES_SOUND }, { 0x324A3, 0x0011, NES_SOUND }, { 0x0BF54, 0x000A, NES_SOUND }, { 0x31815, 0x0075, NES_SOUND }, { 0x13F4E, 0x001F, NES_SOUND },
	{ 0x324B4, 0x098E, NES_SOUND }, { 0x31B85, 0x004B, NES_SOUND }, { 0x32E42, 0x0011, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND }, { 0x32E53, 0x000F, NES_SOUND },
	{ 0x32E62, 0x002F, NES_SOUND }, { 0x32E91, 0x001D, NES_SOUND }, { 0x32EAE, 0x0018, NES_SOUND }, { 0x32EC6, 0x0016, NES_SOUND }, { 0x32EDC, 0x001B, NES_SOUND },
	{ 0x32EF7, 0x0088, NES_SOUND }, { 0x32F7F, 0x0065, NES_SOUND }, { 0x32FE4, 0x0065, NES_SOUND }, { 0x33049, 0x0073, NES_SOUND }, { 0x330BC, 0x00F9, NES_SOUND },
	{ 0x331B5, 0x049E, NES_SOUND }, { 0x34001, 0x0EA8, NES_SOUND }, { 0x34EA9, 0x0B18, NES_SOUND }, { 0x359C1, 0x0B9C, NES_SOUND }, { 0x3655D, 0x0C6B, NES_SOUND },
	{ 0x38001, 0x0E56, NES_SOUND }, { 0x371C8, 0x0C70, NES_SOUND }, { 0x38E57, 0x0DEC, NES_SOUND }, { 0x39C43, 0x0B77, NES_SOUND }, { 0x33653, 0x042F, NES_SOUND },
	{ 0x3A7BA, 0x0AC5, NES_SOUND }, { 0x3B27F, 0x0BE4, NES_SOUND }
};
static const ScummNESFile::Resource res_sounds_swe[82] = {
	{ 0x0BF58, 0x000A, NES_SOUND }, { 0x30352, 0x0832, NES_SOUND }, { 0x30352, 0x0832, NES_SOUND }, { 0x30352, 0x0832, NES_SOUND }, { 0x30352, 0x0832, NES_SOUND },
	{ 0x30352, 0x0832, NES_SOUND }, { 0x0BF62, 0x0011, NES_SOUND }, { 0x27E5F, 0x0073, NES_SOUND }, { 0x17F5A, 0x0011, NES_SOUND }, { 0x17F6B, 0x0011, NES_SOUND },
	{ 0x27ED2, 0x0056, NES_SOUND }, { 0x1BF55, 0x001F, NES_SOUND }, { 0x23F66, 0x0011, NES_SOUND }, { 0x0BF73, 0x000A, NES_SOUND }, { 0x1BF74, 0x000A, NES_SOUND },
	{ 0x27F28, 0x0019, NES_SOUND }, { 0x2BF0A, 0x004B, NES_SOUND }, { 0x1FF71, 0x000A, NES_SOUND }, { 0x27F41, 0x000F, NES_SOUND }, { 0x27F50, 0x001D, NES_SOUND },
	{ 0x2FEAA, 0x0045, NES_SOUND }, { 0x27F6D, 0x000F, NES_SOUND }, { 0x2BF55, 0x001B, NES_SOUND }, { 0x2FEEF, 0x0033, NES_SOUND }, { 0x2FF22, 0x0011, NES_SOUND },
	{ 0x2BF70, 0x000F, NES_SOUND }, { 0x30B84, 0x0075, NES_SOUND }, { 0x2FF33, 0x0014, NES_SOUND }, { 0x0BF58, 0x000A, NES_SOUND }, { 0x30BF9, 0x00FF, NES_SOUND },
	{ 0x2FF47, 0x000F, NES_SOUND }, { 0x2FF56, 0x000F, NES_SOUND }, { 0x30CF8, 0x0092, NES_SOUND }, { 0x30CF8, 0x0092, NES_SOUND }, { 0x30D8A, 0x002D, NES_SOUND },
	{ 0x30DB7, 0x00F8, NES_SOUND }, { 0x2FF65, 0x0016, NES_SOUND }, { 0x30EAF, 0x0011, NES_SOUND }, { 0x30EC0, 0x004B, NES_SOUND }, { 0x30F0B, 0x0011, NES_SOUND },
	{ 0x30F1C, 0x003B, NES_SOUND }, { 0x30F57, 0x008A, NES_SOUND }, { 0x30FE1, 0x0011, NES_SOUND }, { 0x30FF2, 0x000F, NES_SOUND }, { 0x31001, 0x00A2, NES_SOUND },
	{ 0x310A3, 0x00D3, NES_SOUND }, { 0x31176, 0x0097, NES_SOUND }, { 0x2BF70, 0x000F, NES_SOUND }, { 0x31001, 0x00A2, NES_SOUND }, { 0x3120D, 0x05D1, NES_SOUND },
	{ 0x3120D, 0x05D1, NES_SOUND }, { 0x317DE, 0x0011, NES_SOUND }, { 0x0BF58, 0x000A, NES_SOUND }, { 0x30B84, 0x0075, NES_SOUND }, { 0x1BF55, 0x001F, NES_SOUND },
	{ 0x317EF, 0x098E, NES_SOUND }, { 0x30EC0, 0x004B, NES_SOUND }, { 0x3217D, 0x0011, NES_SOUND }, { 0x30352, 0x0832, NES_SOUND }, { 0x3218E, 0x000F, NES_SOUND },
	{ 0x3219D, 0x002F, NES_SOUND }, { 0x321CC, 0x001D, NES_SOUND }, { 0x321E9, 0x0018, NES_SOUND }, { 0x32201, 0x0016, NES_SOUND }, { 0x32217, 0x001B, NES_SOUND },
	{ 0x32232, 0x0088, NES_SOUND }, { 0x322BA, 0x0065, NES_SOUND }, { 0x3231F, 0x0065, NES_SOUND }, { 0x32384, 0x0073, NES_SOUND }, { 0x323F7, 0x00F9, NES_SOUND },
	{ 0x324F0, 0x049E, NES_SOUND }, { 0x3298E, 0x0EA8, NES_SOUND }, { 0x34001, 0x0B18, NES_SOUND }, { 0x34B19, 0x0B9C, NES_SOUND }, { 0x356B5, 0x0C6B, NES_SOUND },
	{ 0x36320, 0x0E56, NES_SOUND }, { 0x37176, 0x0C70, NES_SOUND }, { 0x38001, 0x0DEC, NES_SOUND }, { 0x38DED, 0x0B77, NES_SOUND }, { 0x33836, 0x042F, NES_SOUND },
	{ 0x39964, 0x0AC5, NES_SOUND }, { 0x3A429, 0x0BE4, NES_SOUND }
};
static const ScummNESFile::Resource res_sounds_fra[82] = {
	{ 0x07F74, 0x000A, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND },
	{ 0x30ECA, 0x0832, NES_SOUND }, { 0x0BF6C, 0x0011, NES_SOUND }, { 0x1BEFA, 0x0073, NES_SOUND }, { 0x17F10, 0x0011, NES_SOUND }, { 0x17F21, 0x0011, NES_SOUND },
	{ 0x1FED5, 0x0056, NES_SOUND }, { 0x17F32, 0x001F, NES_SOUND }, { 0x17F51, 0x0011, NES_SOUND }, { 0x0FF76, 0x000A, NES_SOUND }, { 0x17F62, 0x000A, NES_SOUND },
	{ 0x1FF2B, 0x0019, NES_SOUND }, { 0x23E78, 0x004B, NES_SOUND }, { 0x17F6C, 0x000A, NES_SOUND }, { 0x1BF6D, 0x000F, NES_SOUND }, { 0x1FF44, 0x001D, NES_SOUND },
	{ 0x23EC3, 0x0045, NES_SOUND }, { 0x1FF61, 0x000F, NES_SOUND }, { 0x23F08, 0x001B, NES_SOUND }, { 0x23F23, 0x0033, NES_SOUND }, { 0x23F56, 0x0011, NES_SOUND },
	{ 0x1FF70, 0x000F, NES_SOUND }, { 0x27EF4, 0x0075, NES_SOUND }, { 0x23F67, 0x0014, NES_SOUND }, { 0x07F74, 0x000A, NES_SOUND }, { 0x2FB83, 0x00FF, NES_SOUND },
	{ 0x27F69, 0x000F, NES_SOUND }, { 0x2BF70, 0x000F, NES_SOUND }, { 0x2FC82, 0x0092, NES_SOUND }, { 0x2FC82, 0x0092, NES_SOUND }, { 0x2FD14, 0x002D, NES_SOUND },
	{ 0x2FD41, 0x00F8, NES_SOUND }, { 0x2FE39, 0x0016, NES_SOUND }, { 0x2FE4F, 0x0011, NES_SOUND }, { 0x2FE60, 0x004B, NES_SOUND }, { 0x2FEAB, 0x0011, NES_SOUND },
	{ 0x2FEBC, 0x003B, NES_SOUND }, { 0x316FC, 0x008A, NES_SOUND }, { 0x2FEF7, 0x0011, NES_SOUND }, { 0x2FF08, 0x000F, NES_SOUND }, { 0x31786, 0x00A2, NES_SOUND },
	{ 0x31828, 0x00D3, NES_SOUND }, { 0x318FB, 0x0097, NES_SOUND }, { 0x1FF70, 0x000F, NES_SOUND }, { 0x31786, 0x00A2, NES_SOUND }, { 0x31992, 0x05D1, NES_SOUND },
	{ 0x31992, 0x05D1, NES_SOUND }, { 0x2FF17, 0x0011, NES_SOUND }, { 0x07F74, 0x000A, NES_SOUND }, { 0x27EF4, 0x0075, NES_SOUND }, { 0x17F32, 0x001F, NES_SOUND },
	{ 0x31F63, 0x098E, NES_SOUND }, { 0x2FE60, 0x004B, NES_SOUND }, { 0x2FF28, 0x0011, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND }, { 0x2FF39, 0x000F, NES_SOUND },
	{ 0x2FF48, 0x002F, NES_SOUND }, { 0x328F1, 0x001D, NES_SOUND }, { 0x3290E, 0x0018, NES_SOUND }, { 0x32926, 0x0016, NES_SOUND }, { 0x3293C, 0x001B, NES_SOUND },
	{ 0x32957, 0x0088, NES_SOUND }, { 0x329DF, 0x0065, NES_SOUND }, { 0x32A44, 0x0065, NES_SOUND }, { 0x32AA9, 0x0073, NES_SOUND }, { 0x32B1C, 0x00F9, NES_SOUND },
	{ 0x32C15, 0x049E, NES_SOUND }, { 0x330B3, 0x0EA8, NES_SOUND }, { 0x34001, 0x0B18, NES_SOUND }, { 0x34B19, 0x0B9C, NES_SOUND }, { 0x356B5, 0x0C6B, NES_SOUND },
	{ 0x36320, 0x0E56, NES_SOUND }, { 0x37176, 0x0C70, NES_SOUND }, { 0x38001, 0x0DEC, NES_SOUND }, { 0x38DED, 0x0B77, NES_SOUND }, { 0x39964, 0x042F, NES_SOUND },
	{ 0x39D93, 0x0AC5, NES_SOUND }, { 0x3A858, 0x0BE4, NES_SOUND }
};
static const ScummNESFile::Resource res_sounds_ger[82] = {
	{ 0x0BF6D, 0x000A, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND },
	{ 0x30ECA, 0x0832, NES_SOUND }, { 0x23F05, 0x0011, NES_SOUND }, { 0x2FA49, 0x0073, NES_SOUND }, { 0x23F16, 0x0011, NES_SOUND }, { 0x23F27, 0x0011, NES_SOUND },
	{ 0x2FABC, 0x0056, NES_SOUND }, { 0x23F38, 0x001F, NES_SOUND }, { 0x23F57, 0x0011, NES_SOUND }, { 0x0FF76, 0x000A, NES_SOUND }, { 0x17F71, 0x000A, NES_SOUND },
	{ 0x2BF2F, 0x0019, NES_SOUND }, { 0x2FB12, 0x004B, NES_SOUND }, { 0x23F68, 0x000A, NES_SOUND }, { 0x2BF48, 0x000F, NES_SOUND }, { 0x2BF57, 0x001D, NES_SOUND },
	{ 0x2FB5D, 0x0045, NES_SOUND }, { 0x2FBA2, 0x000F, NES_SOUND }, { 0x2FBB1, 0x001B, NES_SOUND }, { 0x2FBCC, 0x0033, NES_SOUND }, { 0x2FBFF, 0x0011, NES_SOUND },
	{ 0x2FC10, 0x000F, NES_SOUND }, { 0x2FC1F, 0x0075, NES_SOUND }, { 0x2FC94, 0x0014, NES_SOUND }, { 0x0BF6D, 0x000A, NES_SOUND }, { 0x2FCA8, 0x00FF, NES_SOUND },
	{ 0x2FDA7, 0x000F, NES_SOUND }, { 0x2FDB6, 0x000F, NES_SOUND }, { 0x2FDC5, 0x0092, NES_SOUND }, { 0x2FDC5, 0x0092, NES_SOUND }, { 0x2FE57, 0x002D, NES_SOUND },
	{ 0x2FE84, 0x00F8, NES_SOUND }, { 0x316FC, 0x0016, NES_SOUND }, { 0x31712, 0x0011, NES_SOUND }, { 0x31723, 0x004B, NES_SOUND }, { 0x3176E, 0x0011, NES_SOUND },
	{ 0x3177F, 0x003B, NES_SOUND }, { 0x317BA, 0x008A, NES_SOUND }, { 0x31844, 0x0011, NES_SOUND }, { 0x31855, 0x000F, NES_SOUND }, { 0x31864, 0x00A2, NES_SOUND },
	{ 0x31906, 0x00D3, NES_SOUND }, { 0x319D9, 0x0097, NES_SOUND }, { 0x2FC10, 0x000F, NES_SOUND }, { 0x31864, 0x00A2, NES_SOUND }, { 0x31A70, 0x05D1, NES_SOUND },
	{ 0x31A70, 0x05D1, NES_SOUND }, { 0x32041, 0x0011, NES_SOUND }, { 0x0BF6D, 0x000A, NES_SOUND }, { 0x2FC1F, 0x0075, NES_SOUND }, { 0x23F38, 0x001F, NES_SOUND },
	{ 0x32052, 0x098E, NES_SOUND }, { 0x31723, 0x004B, NES_SOUND }, { 0x329E0, 0x0011, NES_SOUND }, { 0x30ECA, 0x0832, NES_SOUND }, { 0x329F1, 0x000F, NES_SOUND },
	{ 0x32A00, 0x002F, NES_SOUND }, { 0x32A2F, 0x001D, NES_SOUND }, { 0x32A4C, 0x0018, NES_SOUND }, { 0x32A64, 0x0016, NES_SOUND }, { 0x32A7A, 0x001B, NES_SOUND },
	{ 0x32A95, 0x0088, NES_SOUND }, { 0x32B1D, 0x0065, NES_SOUND }, { 0x32B82, 0x0065, NES_SOUND }, { 0x32BE7, 0x0073, NES_SOUND }, { 0x32C5A, 0x00F9, NES_SOUND },
	{ 0x32D53, 0x049E, NES_SOUND }, { 0x34001, 0x0EA8, NES_SOUND }, { 0x331F1, 0x0B18, NES_SOUND }, { 0x34EA9, 0x0B9C, NES_SOUND }, { 0x35A45, 0x0C6B, NES_SOUND },
	{ 0x366B0, 0x0E56, NES_SOUND }, { 0x38001, 0x0C70, NES_SOUND }, { 0x38C71, 0x0DEC, NES_SOUND }, { 0x39A5D, 0x0B77, NES_SOUND }, { 0x37506, 0x042F, NES_SOUND },
	{ 0x3A5D4, 0x0AC5, NES_SOUND }, { 0x3B099, 0x0BE4, NES_SOUND }
};
#endif
static const ScummNESFile::Resource *res_sounds[ScummNESFile::kROMsetNum] = {
	res_sounds_usa,
	res_sounds_eur,
	res_sounds_swe,
	res_sounds_fra,
	res_sounds_ger,
};

#ifdef PALMOS_68K
static const ScummNESFile::Resource *res_costumes_usa;
static const ScummNESFile::Resource *res_costumes_eur;
static const ScummNESFile::Resource *res_costumes_swe;
static const ScummNESFile::Resource *res_costumes_fra;
static const ScummNESFile::Resource *res_costumes_ger;
#else
static const ScummNESFile::Resource res_costumes_usa[25] = {
	{ 0x17F05, 0x0055, NES_COSTUME }, { 0x17F05, 0x0055, NES_COSTUME }, { 0x17F05, 0x0055, NES_COSTUME }, { 0x17F05, 0x0055, NES_COSTUME }, { 0x17F05, 0x0055, NES_COSTUME },
	{ 0x17F05, 0x0055, NES_COSTUME }, { 0x17F05, 0x0055, NES_COSTUME }, { 0x17F05, 0x0055, NES_COSTUME }, { 0x13FAB, 0x004B, NES_COSTUME }, { 0x17F05, 0x0055, NES_COSTUME },
	{ 0x17F05, 0x0055, NES_COSTUME }, { 0x17F05, 0x0055, NES_COSTUME }, { 0x17F5A, 0x0036, NES_COSTUME }, { 0x17F90, 0x003A, NES_COSTUME }, { 0x17F90, 0x003A, NES_COSTUME },
	{ 0x17F05, 0x0055, NES_COSTUME }, { 0x1BF87, 0x003B, NES_COSTUME }, { 0x17F05, 0x0055, NES_COSTUME }, { 0x23FA9, 0x0045, NES_COSTUME }, { 0x1FFBD, 0x0040, NES_COSTUME },
	{ 0x1BFC2, 0x003C, NES_COSTUME }, { 0x17F90, 0x003A, NES_COSTUME }, { 0x17F90, 0x003A, NES_COSTUME }, { 0x17F05, 0x0055, NES_COSTUME }, { 0x13FAB, 0x004B, NES_COSTUME }
};
static const ScummNESFile::Resource res_costumes_eur[25] = {
	{ 0x0BEFF, 0x0055, NES_COSTUME }, { 0x0BEFF, 0x0055, NES_COSTUME }, { 0x0BEFF, 0x0055, NES_COSTUME }, { 0x0BEFF, 0x0055, NES_COSTUME }, { 0x0BEFF, 0x0055, NES_COSTUME },
	{ 0x0BEFF, 0x0055, NES_COSTUME }, { 0x0BEFF, 0x0055, NES_COSTUME }, { 0x0BEFF, 0x0055, NES_COSTUME }, { 0x0FEA2, 0x004B, NES_COSTUME }, { 0x0BEFF, 0x0055, NES_COSTUME },
	{ 0x0BEFF, 0x0055, NES_COSTUME }, { 0x0BEFF, 0x0055, NES_COSTUME }, { 0x0FEED, 0x0036, NES_COSTUME }, { 0x0FF23, 0x003A, NES_COSTUME }, { 0x0FF23, 0x003A, NES_COSTUME },
	{ 0x0BEFF, 0x0055, NES_COSTUME }, { 0x13F13, 0x003B, NES_COSTUME }, { 0x0BEFF, 0x0055, NES_COSTUME }, { 0x23F2F, 0x0045, NES_COSTUME }, { 0x1FF3E, 0x0040, NES_COSTUME },
	{ 0x27E8F, 0x003C, NES_COSTUME }, { 0x0FF23, 0x003A, NES_COSTUME }, { 0x0FF23, 0x003A, NES_COSTUME }, { 0x0BEFF, 0x0055, NES_COSTUME }, { 0x0FEA2, 0x004B, NES_COSTUME }
};
static const ScummNESFile::Resource res_costumes_swe[25] = {
	{ 0x0FEF5, 0x0055, NES_COSTUME }, { 0x0FEF5, 0x0055, NES_COSTUME }, { 0x0FEF5, 0x0055, NES_COSTUME }, { 0x0FEF5, 0x0055, NES_COSTUME }, { 0x0FEF5, 0x0055, NES_COSTUME },
	{ 0x0FEF5, 0x0055, NES_COSTUME }, { 0x0FEF5, 0x0055, NES_COSTUME }, { 0x0FEF5, 0x0055, NES_COSTUME }, { 0x17E9A, 0x004B, NES_COSTUME }, { 0x0FEF5, 0x0055, NES_COSTUME },
	{ 0x0FEF5, 0x0055, NES_COSTUME }, { 0x0FEF5, 0x0055, NES_COSTUME }, { 0x0FF4A, 0x0036, NES_COSTUME }, { 0x17EE5, 0x003A, NES_COSTUME }, { 0x17EE5, 0x003A, NES_COSTUME },
	{ 0x0FEF5, 0x0055, NES_COSTUME }, { 0x17F1F, 0x003B, NES_COSTUME }, { 0x0FEF5, 0x0055, NES_COSTUME }, { 0x1BE94, 0x0045, NES_COSTUME }, { 0x1BED9, 0x0040, NES_COSTUME },
	{ 0x1BF19, 0x003C, NES_COSTUME }, { 0x17EE5, 0x003A, NES_COSTUME }, { 0x17EE5, 0x003A, NES_COSTUME }, { 0x0FEF5, 0x0055, NES_COSTUME }, { 0x17E9A, 0x004B, NES_COSTUME }
};
static const ScummNESFile::Resource res_costumes_fra[25] = {
	{ 0x0BF17, 0x0055, NES_COSTUME }, { 0x0BF17, 0x0055, NES_COSTUME }, { 0x0BF17, 0x0055, NES_COSTUME }, { 0x0BF17, 0x0055, NES_COSTUME }, { 0x0BF17, 0x0055, NES_COSTUME },
	{ 0x0BF17, 0x0055, NES_COSTUME }, { 0x0BF17, 0x0055, NES_COSTUME }, { 0x0BF17, 0x0055, NES_COSTUME }, { 0x13E77, 0x004B, NES_COSTUME }, { 0x0BF17, 0x0055, NES_COSTUME },
	{ 0x0BF17, 0x0055, NES_COSTUME }, { 0x0BF17, 0x0055, NES_COSTUME }, { 0x07F3E, 0x0036, NES_COSTUME }, { 0x13EC2, 0x003A, NES_COSTUME }, { 0x13EC2, 0x003A, NES_COSTUME },
	{ 0x0BF17, 0x0055, NES_COSTUME }, { 0x13EFC, 0x003B, NES_COSTUME }, { 0x0BF17, 0x0055, NES_COSTUME }, { 0x13F37, 0x0045, NES_COSTUME }, { 0x17E94, 0x0040, NES_COSTUME },
	{ 0x17ED4, 0x003C, NES_COSTUME }, { 0x13EC2, 0x003A, NES_COSTUME }, { 0x13EC2, 0x003A, NES_COSTUME }, { 0x0BF17, 0x0055, NES_COSTUME }, { 0x13E77, 0x004B, NES_COSTUME }
};
static const ScummNESFile::Resource res_costumes_ger[25] = {
	{ 0x0FEEB, 0x0055, NES_COSTUME }, { 0x0FEEB, 0x0055, NES_COSTUME }, { 0x0FEEB, 0x0055, NES_COSTUME }, { 0x0FEEB, 0x0055, NES_COSTUME }, { 0x0FEEB, 0x0055, NES_COSTUME },
	{ 0x0FEEB, 0x0055, NES_COSTUME }, { 0x0FEEB, 0x0055, NES_COSTUME }, { 0x0FEEB, 0x0055, NES_COSTUME }, { 0x17E6C, 0x004B, NES_COSTUME }, { 0x0FEEB, 0x0055, NES_COSTUME },
	{ 0x0FEEB, 0x0055, NES_COSTUME }, { 0x0FEEB, 0x0055, NES_COSTUME }, { 0x0FF40, 0x0036, NES_COSTUME }, { 0x17EB7, 0x003A, NES_COSTUME }, { 0x17EB7, 0x003A, NES_COSTUME },
	{ 0x0FEEB, 0x0055, NES_COSTUME }, { 0x17EF1, 0x003B, NES_COSTUME }, { 0x0FEEB, 0x0055, NES_COSTUME }, { 0x17F2C, 0x0045, NES_COSTUME }, { 0x1FEFD, 0x0040, NES_COSTUME },
	{ 0x1FF3D, 0x003C, NES_COSTUME }, { 0x17EB7, 0x003A, NES_COSTUME }, { 0x17EB7, 0x003A, NES_COSTUME }, { 0x0FEEB, 0x0055, NES_COSTUME }, { 0x17E6C, 0x004B, NES_COSTUME }
};
#endif
static const ScummNESFile::Resource *res_costumes[ScummNESFile::kROMsetNum] = {
	res_costumes_usa,
	res_costumes_eur,
	res_costumes_swe,
	res_costumes_fra,
	res_costumes_ger,
};

#ifdef PALMOS_68K
static const ScummNESFile::Resource *res_globdata_usa;
static const ScummNESFile::Resource *res_globdata_eur;
static const ScummNESFile::Resource *res_globdata_swe;
static const ScummNESFile::Resource *res_globdata_fra;
static const ScummNESFile::Resource *res_globdata_ger;
#else
static const ScummNESFile::Resource res_globdata_usa[1] = { { 0x2CA11, 0x0307, NES_GLOBDATA } };
static const ScummNESFile::Resource res_globdata_eur[1] = { { 0x2CA11, 0x0307, NES_GLOBDATA } };
static const ScummNESFile::Resource res_globdata_swe[1] = { { 0x2C001, 0x0307, NES_GLOBDATA } };
static const ScummNESFile::Resource res_globdata_fra[1] = { { 0x2C628, 0x0307, NES_GLOBDATA } };
static const ScummNESFile::Resource res_globdata_ger[1] = { { 0x2C4EE, 0x0307, NES_GLOBDATA } };
#endif
static const ScummNESFile::Resource *res_globdata[ScummNESFile::kROMsetNum] = {
	res_globdata_usa,
	res_globdata_eur,
	res_globdata_swe,
	res_globdata_fra,
	res_globdata_ger,
};

#ifdef PALMOS_68K
static const ScummNESFile::Resource *res_sprpals_usa;
static const ScummNESFile::Resource *res_sprpals_eur;
static const ScummNESFile::Resource *res_sprpals_swe;
static const ScummNESFile::Resource *res_sprpals_fra;
static const ScummNESFile::Resource *res_sprpals_ger;
#else
// sprite palette data
static const ScummNESFile::Resource res_sprpals_usa[2] = { { 0x0BFC1, 0x0010, NES_SPRPALS }, { 0x0BFD1, 0x0010, NES_SPRPALS } };
static const ScummNESFile::Resource res_sprpals_eur[2] = { { 0x07F61, 0x0010, NES_SPRPALS }, { 0x0BEB2, 0x0010, NES_SPRPALS } };
static const ScummNESFile::Resource res_sprpals_swe[2] = { { 0x07F55, 0x0010, NES_SPRPALS }, { 0x07F65, 0x0010, NES_SPRPALS } };
static const ScummNESFile::Resource res_sprpals_fra[2] = { { 0x07ED8, 0x0010, NES_SPRPALS }, { 0x07EE8, 0x0010, NES_SPRPALS } };
static const ScummNESFile::Resource res_sprpals_ger[2] = { { 0x07F6B, 0x0010, NES_SPRPALS }, { 0x0BF17, 0x0010, NES_SPRPALS } };
#endif
static const ScummNESFile::Resource *res_sprpals[ScummNESFile::kROMsetNum] = {
	res_sprpals_usa,
	res_sprpals_eur,
	res_sprpals_swe,
	res_sprpals_fra,
	res_sprpals_ger,
};

#ifdef PALMOS_68K
static const ScummNESFile::Resource *res_sprdesc_usa;
static const ScummNESFile::Resource *res_sprdesc_eur;
static const ScummNESFile::Resource *res_sprdesc_swe;
static const ScummNESFile::Resource *res_sprdesc_fra;
static const ScummNESFile::Resource *res_sprdesc_ger;
#else
// associates costume IDs with sprite sets (indexes into SPRLENS/SPROFFS)
static const ScummNESFile::Resource res_sprdesc_usa[2] = { { 0x0FFB7, 0x0031, NES_SPRDESC }, { 0x0BFE1, 0x0009, NES_SPRDESC } };
static const ScummNESFile::Resource res_sprdesc_eur[2] = { { 0x0BEC2, 0x0031, NES_SPRDESC }, { 0x07F71, 0x0009, NES_SPRDESC } };
static const ScummNESFile::Resource res_sprdesc_swe[2] = { { 0x0BF1B, 0x0031, NES_SPRDESC }, { 0x07F75, 0x0009, NES_SPRDESC } };
static const ScummNESFile::Resource res_sprdesc_fra[2] = { { 0x07EF8, 0x0031, NES_SPRDESC }, { 0x07F29, 0x0009, NES_SPRDESC } };
static const ScummNESFile::Resource res_sprdesc_ger[2] = { { 0x0BF27, 0x0031, NES_SPRDESC }, { 0x0BF58, 0x0009, NES_SPRDESC } };
#endif
static const ScummNESFile::Resource *res_sprdesc[ScummNESFile::kROMsetNum] = {
	res_sprdesc_usa,
	res_sprdesc_eur,
	res_sprdesc_swe,
	res_sprdesc_fra,
	res_sprdesc_ger,
};

#ifdef PALMOS_68K
static const ScummNESFile::Resource *res_sprlens_usa;
static const ScummNESFile::Resource *res_sprlens_eur;
static const ScummNESFile::Resource *res_sprlens_swe;
static const ScummNESFile::Resource *res_sprlens_fra;
static const ScummNESFile::Resource *res_sprlens_ger;
#else
// number of sprites in each set (indicates length within SPRDATA)
static const ScummNESFile::Resource res_sprlens_usa[2] = { { 0x0FEA2, 0x0115, NES_SPRLENS }, { 0x07FF5, 0x0006, NES_SPRLENS } };
static const ScummNESFile::Resource res_sprlens_eur[2] = { { 0x1BE32, 0x0115, NES_SPRLENS }, { 0x07F5B, 0x0006, NES_SPRLENS } };
static const ScummNESFile::Resource res_sprlens_swe[2] = { { 0x13E6A, 0x0115, NES_SPRLENS }, { 0x07F4F, 0x0006, NES_SPRLENS } };
static const ScummNESFile::Resource res_sprlens_fra[2] = { { 0x0FE61, 0x0115, NES_SPRLENS }, { 0x07ED2, 0x0006, NES_SPRLENS } };
static const ScummNESFile::Resource res_sprlens_ger[2] = { { 0x2BE1A, 0x0115, NES_SPRLENS }, { 0x07F65, 0x0006, NES_SPRLENS } };
#endif
static const ScummNESFile::Resource *res_sprlens[ScummNESFile::kROMsetNum] = {
	res_sprlens_usa,
	res_sprlens_eur,
	res_sprlens_swe,
	res_sprlens_fra,
	res_sprlens_ger,
};

#ifdef PALMOS_68K
static const ScummNESFile::Resource *res_sproffs_usa;
static const ScummNESFile::Resource *res_sproffs_eur;
static const ScummNESFile::Resource *res_sproffs_swe;
static const ScummNESFile::Resource *res_sproffs_fra;
static const ScummNESFile::Resource *res_sproffs_ger;
#else
// offset of each sprite set (indexes into SPRDATA)
static const ScummNESFile::Resource res_sproffs_usa[2] = { { 0x2BDC5, 0x022A, NES_SPROFFS }, { 0x0BFEA, 0x000C, NES_SPROFFS } };
static const ScummNESFile::Resource res_sproffs_eur[2] = { { 0x2FD42, 0x022A, NES_SPROFFS }, { 0x0BEF3, 0x000C, NES_SPROFFS } };
static const ScummNESFile::Resource res_sproffs_swe[2] = { { 0x2BCE0, 0x022A, NES_SPROFFS }, { 0x0BF4C, 0x000C, NES_SPROFFS } };
static const ScummNESFile::Resource res_sproffs_fra[2] = { { 0x2F959, 0x022A, NES_SPROFFS }, { 0x07F32, 0x000C, NES_SPROFFS } };
static const ScummNESFile::Resource res_sproffs_ger[2] = { { 0x2F81F, 0x022A, NES_SPROFFS }, { 0x0BF61, 0x000C, NES_SPROFFS } };
#endif
static const ScummNESFile::Resource *res_sproffs[ScummNESFile::kROMsetNum] = {
	res_sproffs_usa,
	res_sproffs_eur,
	res_sproffs_swe,
	res_sproffs_fra,
	res_sproffs_ger,
};

#ifdef PALMOS_68K
static const ScummNESFile::Resource *res_sprdata_usa;
static const ScummNESFile::Resource *res_sprdata_eur;
static const ScummNESFile::Resource *res_sprdata_swe;
static const ScummNESFile::Resource *res_sprdata_fra;
static const ScummNESFile::Resource *res_sprdata_ger;
#else
// sprite data sets (packed NES sprite data)
static const ScummNESFile::Resource res_sprdata_usa[2] = { { 0x2CE11, 0x2BE0, NES_SPRDATA }, { 0x07F6B, 0x008A, NES_SPRDATA } };
static const ScummNESFile::Resource res_sprdata_eur[2] = { { 0x2CE11, 0x2BE0, NES_SPRDATA }, { 0x0BE28, 0x008A, NES_SPRDATA } };
static const ScummNESFile::Resource res_sprdata_swe[2] = { { 0x2C401, 0x2BE0, NES_SPRDATA }, { 0x0FE6B, 0x008A, NES_SPRDATA } };
static const ScummNESFile::Resource res_sprdata_fra[2] = { { 0x2CA28, 0x2BE0, NES_SPRDATA }, { 0x07E48, 0x008A, NES_SPRDATA } };
static const ScummNESFile::Resource res_sprdata_ger[2] = { { 0x2C8EE, 0x2BE0, NES_SPRDATA }, { 0x0FE61, 0x008A, NES_SPRDATA } };
static const ScummNESFile::Resource *res_sprdata[ScummNESFile::kROMsetNum] = {
	res_sprdata_usa,
	res_sprdata_eur,
	res_sprdata_swe,
	res_sprdata_fra,
	res_sprdata_ger,
};
#endif

#ifdef PALMOS_68K
static const ScummNESFile::Resource *res_charset_usa;
static const ScummNESFile::Resource *res_charset_eur;
static const ScummNESFile::Resource *res_charset_swe;
static const ScummNESFile::Resource *res_charset_fra;
static const ScummNESFile::Resource *res_charset_ger;
#else
static const ScummNESFile::Resource res_charset_usa[1] = { { 0x3F6EE, 0x0090, NES_CHARSET } };
static const ScummNESFile::Resource res_charset_eur[1] = { { 0x3F724, 0x0090, NES_CHARSET } };
static const ScummNESFile::Resource res_charset_swe[1] = { { 0x3F739, 0x0090, NES_CHARSET } };
static const ScummNESFile::Resource res_charset_fra[1] = { { 0x3F739, 0x0090, NES_CHARSET } };
static const ScummNESFile::Resource res_charset_ger[1] = { { 0x3F739, 0x0090, NES_CHARSET } };
#endif
static const ScummNESFile::Resource *res_charset[ScummNESFile::kROMsetNum] = {
	res_charset_usa,
	res_charset_eur,
	res_charset_swe,
	res_charset_fra,
	res_charset_ger,
};

#ifdef PALMOS_68K
static const ScummNESFile::Resource *res_preplist_usa;
static const ScummNESFile::Resource *res_preplist_eur;
static const ScummNESFile::Resource *res_preplist_swe;
static const ScummNESFile::Resource *res_preplist_fra;
static const ScummNESFile::Resource *res_preplist_ger;
#else
static const ScummNESFile::Resource res_preplist_usa[1] = { { 0x3FB5A, 0x000E, NES_PREPLIST } };
static const ScummNESFile::Resource res_preplist_eur[1] = { { 0x3FB90, 0x000E, NES_PREPLIST } };
static const ScummNESFile::Resource res_preplist_swe[1] = { { 0x3FBA9, 0x000E, NES_PREPLIST } };
static const ScummNESFile::Resource res_preplist_fra[1] = { { 0x3FBAF, 0x0010, NES_PREPLIST } };
static const ScummNESFile::Resource res_preplist_ger[1] = { { 0x3FBAB, 0x000F, NES_PREPLIST } };
#endif
static const ScummNESFile::Resource *res_preplist[ScummNESFile::kROMsetNum] = {
	res_preplist_usa,
	res_preplist_eur,
	res_preplist_swe,
	res_preplist_fra,
	res_preplist_ger,
};

uint16 write_byte(Common::WriteStream *out, byte val) {
	val ^= 0xFF;
	if (out != 0)
		out->writeByte(val);
	return 1;
}

uint16 write_word(Common::WriteStream *out, uint16 val) {
	val ^= 0xFFFF;
	if (out != 0)
		out->writeUint16LE(val);
	return 2;
}

byte ScummNESFile::fileReadByte() {
	byte b = 0;
	File::read(&b, 1);
	return b;
}

uint16 ScummNESFile::fileReadUint16LE() {
	uint16 a = fileReadByte();
	uint16 b = fileReadByte();
	return a | (b << 8);
}

uint16 ScummNESFile::extractResource(Common::WriteStream *output, const Resource *res) {
	uint16 len, i, j;
	byte val;
	byte cnt;
	uint16 reslen = 0;

	if (res == NULL)
		error("extract_resource - no resource specified");

	if ((res->offset == 0) && (res->length == 0))
		return 0;	/* there are 8 scripts that are zero bytes long, so we should skip them */

	File::seek(res->offset,SEEK_SET);

	switch (res->type) {
	case NES_GLOBDATA:
		len = res->length;

		for (i = 0; i < len; i++)
			reslen += write_byte(output, fileReadByte());

		break;

	case NES_ROOMGFX:
	case NES_COSTUMEGFX:
		reslen += write_word(output, (uint16)(res->length + 2));
		len = fileReadByte();
		reslen += write_byte(output, (byte)len);

		if (!len)
			len = 256;
		len = len << 4;

		for (i = 0; i < len;) {
			reslen += write_byte(output, cnt = fileReadByte());
			for (j = 0; j < (cnt & 0x7F); j++, i++)
				if ((cnt & 0x80) || (j == 0))
					reslen += write_byte(output, fileReadByte());
		}

		if (File::pos() - res->offset != res->length)
			error("extract_resource - length mismatch while extracting graphics resource (was %04X, should be %04X)", File::pos() - res->offset, res->length);

		break;

	case NES_ROOM:
	case NES_SCRIPT:
		len = fileReadUint16LE();

		if (len != res->length)
			error("extract_resource - length mismatch while extracting room/script resource (was %04X, should be %04X)", len, res->length);

		File::seek(-2, SEEK_CUR);

		for (i = 0; i < len; i++)
			reslen += write_byte(output, fileReadByte());

		break;

	case NES_SOUND:
		len = res->length + 2;
		val = fileReadByte();
		cnt = fileReadByte();

		if ((val == 2) && (cnt == 100)) {
			reslen += write_word(output, len);
			reslen += write_byte(output, val);
			reslen += write_byte(output, cnt);

			cnt = fileReadByte();
			reslen += write_byte(output, cnt);
			for (i = 0; i < cnt; i++)
				reslen += write_byte(output, fileReadByte());
			for (i = 0; i < cnt; i++)
				reslen += write_byte(output, fileReadByte());

			while (1) {
				reslen += write_byte(output, val = fileReadByte());
				if (val >= 0xFE)
					break;
			}
		} else if (((val == 0) || (val == 1) || (val == 4)) && (cnt == 10)) {
			reslen += write_word(output, len);
			reslen += write_byte(output, val);
			reslen += write_byte(output, cnt);
			while (1) {
				reslen += write_byte(output, val = fileReadByte());

				if (val >= 0xFE)
					break;

				if (val >= 0x10)
					reslen += write_byte(output, fileReadByte());
				else {
					reslen += write_byte(output, fileReadByte());
					reslen += write_byte(output, fileReadByte());
					reslen += write_byte(output, fileReadByte());
					reslen += write_byte(output, fileReadByte());
				}
			}
		} else
			error("extract_resource - unknown sound type %d/%d detected",val,cnt);

		if (File::pos() - res->offset != res->length)
			error("extract_resource - length mismatch while extracting sound resource (was %04X, should be %04X)", File::pos() - res->offset, res->length);

		break;

	case NES_COSTUME:
	case NES_SPRPALS:
	case NES_SPRDESC:
	case NES_SPRLENS:
	case NES_SPROFFS:
	case NES_SPRDATA:
	case NES_CHARSET:
		len = res->length;
		reslen += write_word(output, (uint16)(len + 2));

		for (i = 0; i < len; i++)
			reslen += write_byte(output, fileReadByte());

		break;

	case NES_PREPLIST:
		len = res->length;
                reslen += write_word(output, 0x002A);

		reslen += write_byte(output, ' ');
		for (i = 1; i < 8; i++)
			reslen += write_byte(output, 0);

		for (j = 0; j < 4; j++)
		{
			reslen += write_byte(output,' ');
			for (i = 1; (val = fileReadByte()); i++)
				reslen += write_byte(output, val);
			for (; i < 8; i++)
				reslen += write_byte(output, 0);
		}
		break;

	default:
		error("extract_resource - unknown resource type %d specified!", res->type);
	}

	return reslen;
}

struct ScummNESFile::LFLEntry {
	const Resource **type;
	int index;
};

// based on structure of Classic PC Maniac Mansion LFL files
// (roomgfx resources are arranged in order, one per file,
// after the room blocks)
static const ScummNESFile::LFLEntry lfl_01[] = { {res_rooms, 1}, {res_roomgfx, 1}, {res_scripts, 57}, {res_scripts, 61}, {res_scripts, 76}, {res_scripts, 105}, {res_scripts, 111}, {res_sounds, 5}, {res_scripts, 132}, {res_scripts, 148}, {res_scripts, 155}, {res_scripts, 156}, {res_sounds, 39}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_02[] = { {res_rooms, 2}, {res_roomgfx, 2}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_03[] = { {res_rooms, 3}, {res_roomgfx, 3}, {res_scripts, 21}, {res_sounds, 26}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_04[] = { {res_rooms, 4}, {res_roomgfx, 4}, {res_scripts, 46}, {res_scripts, 56}, {res_scripts, 137}, {res_scripts, 146}, {res_sounds, 12}, {res_sounds, 11}, {res_sounds, 13}, {res_sounds, 42}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_05[] = { {res_rooms, 5}, {res_roomgfx, 5}, {res_scripts, 30}, {res_scripts, 31}, {res_scripts, 32}, {res_scripts, 33}, {res_scripts, 34}, {res_scripts, 35}, {res_sounds, 22}, {res_sounds, 23}, {res_sounds, 24}, {res_sounds, 21}, {res_sounds, 46}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_06[] = { {res_rooms, 6}, {res_roomgfx, 6}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_07[] = { {res_rooms, 7}, {res_roomgfx, 7}, {res_scripts, 17}, {res_scripts, 58}, {res_scripts, 59}, {res_scripts, 60}, {res_scripts, 74}, {res_scripts, 81}, {res_scripts, 82}, {res_scripts, 150}, {res_sounds, 14}, {res_sounds, 15}, {res_sounds, 16}, {res_sounds, 17}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_08[] = { {res_rooms, 8}, {res_roomgfx, 8}, {res_scripts, 7}, {res_scripts, 12}, {res_scripts, 13}, {res_scripts, 47}, {res_scripts, 48}, {res_scripts, 49}, {res_scripts, 154}, {res_sounds, 32}, {res_sounds, 33}, {res_sounds, 36}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_09[] = { {res_rooms, 9}, {res_roomgfx, 9}, {res_scripts, 10}, {res_scripts, 11}, {res_scripts, 45}, {res_scripts, 55}, {res_scripts, 84}, {res_scripts, 85}, {res_scripts, 86}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_10[] = { {res_rooms, 10}, {res_roomgfx, 10}, {res_scripts, 24}, {res_scripts, 149}, {res_sounds, 28}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_11[] = { {res_rooms, 11}, {res_roomgfx, 11}, {res_scripts, 166}, {res_scripts, 167}, {res_scripts, 168}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_12[] = { {res_rooms, 12}, {res_roomgfx, 12}, {res_scripts, 51}, {res_scripts, 103}, {res_scripts, 104}, {res_scripts, 161}, {res_sounds, 63}, {res_costumes, 14}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_13[] = { {res_rooms, 13}, {res_roomgfx, 13}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_14[] = { {res_rooms, 14}, {res_roomgfx, 14}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_15[] = { {res_rooms, 15}, {res_roomgfx, 15}, {res_sounds, 27}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_16[] = { {res_rooms, 16}, {res_roomgfx, 16}, {res_scripts, 14}, {res_scripts, 121}, {res_scripts, 122}, {res_sounds, 40}, {res_sounds, 64}, {res_sounds, 68}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_17[] = { {res_rooms, 17}, {res_roomgfx, 17}, {res_scripts, 20}, {res_scripts, 100}, {res_sounds, 25}, {res_sounds, 44}, {res_sounds, 2}, {res_sounds, 50}, {res_sounds, 52}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_18[] = { {res_rooms, 18}, {res_roomgfx, 18}, {res_scripts, 25}, {res_scripts, 26}, {res_scripts, 27}, {res_scripts, 28}, {res_scripts, 64}, {res_scripts, 65}, {res_scripts, 66}, {res_scripts, 67}, {res_scripts, 68}, {res_scripts, 69}, {res_scripts, 70}, {res_scripts, 71}, {res_scripts, 73}, {res_scripts, 101}, {res_sounds, 35}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_19[] = { {res_rooms, 19}, {res_roomgfx, 19}, {res_scripts, 36}, {res_scripts, 37}, {res_scripts, 38}, {res_scripts, 39}, {res_scripts, 40}, {res_scripts, 152}, {res_scripts, 153}, {res_costumes, 10}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_20[] = { {res_rooms, 20}, {res_roomgfx, 20}, {res_scripts, 107}, {res_scripts, 108}, {res_scripts, 109}, {res_scripts, 110}, {res_scripts, 159}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_21[] = { {res_rooms, 21}, {res_roomgfx, 21}, {res_scripts, 41}, {res_scripts, 42}, {res_scripts, 43}, {res_scripts, 53}, {res_scripts, 136}, {res_sounds, 29}, {res_sounds, 20}, {res_sounds, 37}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_22[] = { {res_rooms, 22}, {res_roomgfx, 22}, {res_scripts, 15}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_23[] = { {res_rooms, 23}, {res_roomgfx, 23}, {res_scripts, 77}, {res_scripts, 79}, {res_scripts, 80}, {res_scripts, 83}, {res_sounds, 41}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_24[] = { {res_rooms, 24}, {res_roomgfx, 24}, {res_scripts, 18}, {res_scripts, 19}, {res_scripts, 78}, {res_sounds, 7}, {res_sounds, 3}, {res_sounds, 18}, {res_sounds, 34}, {res_costumes, 12}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_25[] = { {res_rooms, 25}, {res_roomgfx, 25}, {res_scripts, 29}, {res_sounds, 30}, {res_sounds, 31}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_26[] = { {res_rooms, 26}, {res_roomgfx, 26}, {res_scripts, 87}, {res_scripts, 88}, {res_scripts, 89}, {res_scripts, 90}, {res_scripts, 91}, {res_scripts, 92}, {res_scripts, 93}, {res_scripts, 94}, {res_scripts, 95}, {res_scripts, 96}, {res_scripts, 97}, {res_scripts, 98}, {res_scripts, 116}, {res_scripts, 151}, {res_scripts, 174}, {res_costumes, 11}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_27[] = { {res_rooms, 27}, {res_roomgfx, 27}, {res_scripts, 16}, {res_scripts, 52}, {res_scripts, 54}, {res_scripts, 113}, {res_sounds, 45}, {res_costumes, 19}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_28[] = { {res_rooms, 28}, {res_roomgfx, 28}, {res_scripts, 22}, {res_scripts, 23}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_29[] = { {res_rooms, 29}, {res_roomgfx, 29}, {res_scripts, 75}, {res_sounds, 43}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_30[] = { {res_rooms, 30}, {res_roomgfx, 30}, {res_scripts, 63}, {res_sounds, 0}, {res_scripts, 123}, {res_scripts, 125}, {res_scripts, 126}, {res_scripts, 127}, {res_scripts, 129}, {res_sounds, 55}, {res_sounds, 59}, {res_sounds, 60}, {res_costumes, 8}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_31[] = { {res_rooms, 31}, {res_roomgfx, 31}, {res_scripts, 99}, {res_scripts, 115}, {res_scripts, 117}, {res_scripts, 119}, {res_scripts, 147}, {res_scripts, 157}, {res_scripts, 158}, {res_scripts, 160}, {res_costumes, 13}, {res_costumes, 9}, {res_costumes, 23}, {res_costumes, 24}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_32[] = { {res_rooms, 32}, {res_roomgfx, 32}, {res_costumes, 15}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_33[] = { {res_rooms, 33}, {res_roomgfx, 33}, {res_scripts, 120}, {res_scripts, 135}, {res_sounds, 56}, {res_sounds, 57}, {res_sounds, 58}, {res_sounds, 1}, {res_costumes, 22}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_34[] = { {res_rooms, 34}, {res_roomgfx, 34}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_35[] = { {res_rooms, 35}, {res_roomgfx, 35}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_36[] = { {res_rooms, 36}, {res_roomgfx, 36}, {res_sounds, 10}, {res_sounds, 4}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_37[] = { {res_rooms, 37}, {res_roomgfx, 37}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_38[] = { {res_rooms, 38}, {res_roomgfx, 38}, {res_scripts, 138}, {res_scripts, 139}, {res_scripts, 140}, {res_scripts, 141}, {res_scripts, 142}, {res_scripts, 143}, {res_scripts, 144}, {res_scripts, 145}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_39[] = { {res_rooms, 39}, {res_roomgfx, 39}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_40[] = { {res_rooms, 40}, {res_roomgfx, 0}, {res_scripts, 112}, {res_costumes, 17}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_41[] = { {res_rooms, 41}, {res_scripts, 106}, {res_sounds, 47}, {res_sounds, 48}, {res_sounds, 53}, {res_sounds, 49}, {res_sounds, 51}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_42[] = { {res_rooms, 42}, {res_scripts, 124}, {res_costumes, 18}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_43[] = { {res_rooms, 43}, {res_scripts, 44}, {res_sounds, 19}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_44[] = { {res_rooms, 44}, {res_scripts, 102}, {res_sounds, 6}, {res_sounds, 38}, {res_sounds, 8}, {res_sounds, 9}, {res_costumes, 1}, {res_costumes, 2}, {res_costumes, 5}, {res_costumes, 6}, {res_costumes, 3}, {res_costumes, 4}, {res_costumes, 7}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_45[] = { {res_rooms, 45}, {res_scripts, 1}, {res_scripts, 2}, {res_scripts, 3}, {res_scripts, 4}, {res_scripts, 5}, {res_scripts, 9}, {res_scripts, 114}, {res_scripts, 131}, {res_scripts, 164}, {res_scripts, 165}, {res_scripts, 169}, {res_scripts, 170}, {res_scripts, 171}, {res_scripts, 172}, {res_scripts, 173}, {res_scripts, 175}, {res_sounds, 54}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_46[] = { {res_rooms, 46}, {res_scripts, 130}, {res_sounds, 65}, {res_costumes, 0}, {res_costumes, 21}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_47[] = { {res_rooms, 47}, {res_scripts, 62}, {res_sounds, 69}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_48[] = { {res_rooms, 48}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_49[] = { {res_rooms, 49}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_50[] = { {res_rooms, 50}, {res_scripts, 133}, {res_scripts, 163}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_51[] = { {res_rooms, 51}, {res_scripts, 118}, {res_scripts, 128}, {res_sounds, 61}, {res_sounds, 62}, {res_sounds, 67}, {res_sounds, 66}, {res_costumes, 16}, {res_costumes, 20}, {NULL, 0} };
static const ScummNESFile::LFLEntry lfl_52[] = { {res_rooms, 52}, {NULL, 0} };
// remaining 'standard' resources (not used by any of the original LFL files)
static const ScummNESFile::LFLEntry lfl_53[] = { {res_rooms, 53}, {res_scripts, 177}, {res_scripts, 178}, {res_sounds, 70}, {res_sounds, 71}, {res_sounds, 72}, {res_sounds, 73}, {res_sounds, 74}, {res_sounds, 75}, {res_sounds, 76}, {res_sounds, 77}, {res_sounds, 78}, {res_sounds, 79}, {res_sounds, 80}, {res_sounds, 81}, {NULL, 0} };
// all 'non-standard' resources (the costume-related stuff)
static const ScummNESFile::LFLEntry lfl_54[] = { {res_rooms, 54}, {res_sprdesc, 0}, {res_sprdesc, 1}, {res_sprlens, 0}, {res_sprlens, 1}, {res_sproffs, 0}, {res_sproffs, 1}, {res_sprdata, 0}, {res_sprdata, 1}, {res_costumegfx, 0}, {res_costumegfx, 1}, {res_sprpals, 0}, {res_sprpals, 1}, {res_charset, 0}, {res_preplist, 0}, {NULL, 0} };

struct ScummNESFile::LFL {
	int num;
	const ScummNESFile::LFLEntry *entries;
};

static const ScummNESFile::LFL lfls[] = {
	{  1, lfl_01 },
	{  2, lfl_02 },
	{  3, lfl_03 },
	{  4, lfl_04 },
	{  5, lfl_05 },
	{  6, lfl_06 },
	{  7, lfl_07 },
	{  8, lfl_08 },
	{  9, lfl_09 },
	{ 10, lfl_10 },
	{ 11, lfl_11 },
	{ 12, lfl_12 },
	{ 13, lfl_13 },
	{ 14, lfl_14 },
	{ 15, lfl_15 },
	{ 16, lfl_16 },
	{ 17, lfl_17 },
	{ 18, lfl_18 },
	{ 19, lfl_19 },
	{ 20, lfl_20 },
	{ 21, lfl_21 },
	{ 22, lfl_22 },
	{ 23, lfl_23 },
	{ 24, lfl_24 },
	{ 25, lfl_25 },
	{ 26, lfl_26 },
	{ 27, lfl_27 },
	{ 28, lfl_28 },
	{ 29, lfl_29 },
	{ 30, lfl_30 },
	{ 31, lfl_31 },
	{ 32, lfl_32 },
	{ 33, lfl_33 },
	{ 34, lfl_34 },
	{ 35, lfl_35 },
	{ 36, lfl_36 },
	{ 37, lfl_37 },
	{ 38, lfl_38 },
	{ 39, lfl_39 },
	{ 40, lfl_40 },
	{ 41, lfl_41 },
	{ 42, lfl_42 },
	{ 43, lfl_43 },
	{ 44, lfl_44 },
	{ 45, lfl_45 },
	{ 46, lfl_46 },
	{ 47, lfl_47 },
	{ 48, lfl_48 },
	{ 49, lfl_49 },
	{ 50, lfl_50 },
	{ 51, lfl_51 },
	{ 52, lfl_52 },
	{ 53, lfl_53 },
	{ 54, lfl_54 },
	{ -1, NULL }
};

#include "common/pack-start.h"	// START STRUCT PACKING

struct _lfl_index {
	byte	room_lfl[55];
	uint16	room_addr[55];
	byte	costume_lfl[80];
	uint16	costume_addr[80];
	byte	script_lfl[200];
	uint16	script_addr[200];
	byte	sound_lfl[100];
	uint16	sound_addr[100];
} lfl_index;

#include "common/pack-end.h"	// END STRUCT PACKING


bool ScummNESFile::generateResource(int res) {
	const LFL *lfl = &lfls[res - 1];
	int j;
	int bufsize = 2;

	for (j = 0; lfl->entries[j].type != NULL; j++)
		bufsize += extractResource(0, &lfl->entries[j].type[_ROMset][lfl->entries[j].index]);

	free(_buf);
	_buf = (byte *)calloc(1, bufsize);

	Common::MemoryWriteStream out(_buf, bufsize);

	for (j = 0; lfl->entries[j].type != NULL; j++) {
		const Resource *entry = &lfl->entries[j].type[_ROMset][lfl->entries[j].index];
		extractResource(&out, entry);
	}
	write_byte(&out, 0xD1);
	write_byte(&out, 0xF5);

	if (_stream)
		delete _stream;

	_stream = new Common::MemoryReadStream(_buf, bufsize);

	return true;
}

bool ScummNESFile::generateIndex() {
	int i, j;

	for (i = 0; lfls[i].num != -1; i++) {
		const LFL *lfl = &lfls[i];
		uint16 respos = 0;

		for (j = 0; lfl->entries[j].type != NULL; j++) {
			const LFLEntry *entry = &lfl->entries[j];

			switch (entry->type[_ROMset][entry->index].type) {
			case NES_ROOM:
				lfl_index.room_lfl[entry->index] = lfl->num;
				lfl_index.room_addr[entry->index] = TO_LE_16(respos);
				break;
			case NES_COSTUME:
				lfl_index.costume_lfl[entry->index] = lfl->num;
				lfl_index.costume_addr[entry->index] = TO_LE_16(respos);
				break;
			case NES_SPRDESC:
				lfl_index.costume_lfl[entry->index + 25] = lfl->num;
				lfl_index.costume_addr[entry->index + 25] = TO_LE_16(respos);
				break;
			case NES_SPRLENS:
				lfl_index.costume_lfl[entry->index + 27] = lfl->num;
				lfl_index.costume_addr[entry->index + 27] = TO_LE_16(respos);
				break;
			case NES_SPROFFS:
				lfl_index.costume_lfl[entry->index + 29] = lfl->num;
				lfl_index.costume_addr[entry->index + 29] = TO_LE_16(respos);
				break;
			case NES_SPRDATA:
				lfl_index.costume_lfl[entry->index + 31] = lfl->num;
				lfl_index.costume_addr[entry->index + 31] = TO_LE_16(respos);
				break;
			case NES_COSTUMEGFX:
				lfl_index.costume_lfl[entry->index + 33] = lfl->num;
				lfl_index.costume_addr[entry->index + 33] = TO_LE_16(respos);
				break;
			case NES_SPRPALS:
				lfl_index.costume_lfl[entry->index + 35] = lfl->num;
				lfl_index.costume_addr[entry->index + 35] = TO_LE_16(respos);
				break;
			case NES_ROOMGFX:
				lfl_index.costume_lfl[entry->index + 37] = lfl->num;
				lfl_index.costume_addr[entry->index + 37] = TO_LE_16(respos);
				break;
			case NES_SCRIPT:
				lfl_index.script_lfl[entry->index] = lfl->num;
				lfl_index.script_addr[entry->index] = TO_LE_16(respos);
				break;
			case NES_SOUND:
				lfl_index.sound_lfl[entry->index] = lfl->num;
				lfl_index.sound_addr[entry->index] = TO_LE_16(respos);
				break;
			case NES_CHARSET:
				lfl_index.costume_lfl[77] = lfl->num;
				lfl_index.costume_addr[77] = TO_LE_16(respos);
				break;
			case NES_PREPLIST:
				lfl_index.costume_lfl[78] = lfl->num;
				lfl_index.costume_addr[78] = TO_LE_16(respos);
				break;
			default:
				error("Unindexed entry found!");
				break;
			}
			respos += extractResource(0, &entry->type[_ROMset][entry->index]);
		}
	}

	int bufsize = 2;

	bufsize += 775;
	bufsize += sizeof(lfl_index);

	free(_buf);
	_buf = (byte *)calloc(1, bufsize);

	Common::MemoryWriteStream out(_buf, bufsize);

	write_byte(&out, 0x43);
	write_byte(&out, 0x46);

	extractResource(&out, &res_globdata[_ROMset][0]);

	for (i = 0; i < (int)sizeof(lfl_index); i++)
		write_byte(&out, ((byte *)&lfl_index)[i]);

	if (_stream)
		delete _stream;

	_stream = new Common::MemoryReadStream(_buf, bufsize);

	return true;
}

bool ScummNESFile::open(const Common::String &filename, AccessMode mode) {

	if (_ROMset == kROMsetNum) {
		char md5str[32+1];
		if (Common::md5_file_string(filename.c_str(), md5str)) {

			if (!strcmp(md5str, "3905799e081b80a61d4460b7b733c206")) {
				_ROMset = kROMsetUSA;
				debug(1, "ROM contents verified as Maniac Mansion (USA)");
			} else if (!strcmp(md5str, "d8d07efcb88f396bee0b402b10c3b1c9")) {
				_ROMset = kROMsetEurope;
				debug(1, "ROM contents verified as Maniac Mansion (Europe)");
			} else if (!strcmp(md5str, "22d07d6c386c9c25aca5dac2a0c0d94b")) {
				_ROMset = kROMsetSweden;
				debug(1, "ROM contents verified as Maniac Mansion (Sweden)");
			} else if (!strcmp(md5str, "81bbfa181184cb494e7a81dcfa94fbd9")) {
				_ROMset = kROMsetFrance;
				debug(2, "ROM contents verified as Maniac Mansion (France)");
			} else if (!strcmp(md5str, "257f8c14d8c584f7ddd601bcb00920c7")) {
				_ROMset = kROMsetGermany;
				debug(2, "ROM contents verified as Maniac Mansion (Germany)");
			} else {
				error("Unsupported Maniac Mansion ROM, md5: %s", md5str);
				return false;
			}
		} else {
			return false;
		}
	}

	if (File::open(filename, mode)) {
		if (_stream)
			delete _stream;
		_stream = 0;

		free(_buf);
		_buf = 0;

		return true;
	} else {
		return false;
	}
}

void ScummNESFile::close() {
	if (_stream)
		delete _stream;
	_stream = 0;

	free(_buf);
	_buf = 0;

	File::close();
}

bool ScummNESFile::openSubFile(const Common::String &filename) {
	assert(isOpen());

	const char *ext = strrchr(filename.c_str(), '.');
	char resNum[3];
	int res;

	// We always have file name in form of XX.lfl
	resNum[0] = ext[-2];
	resNum[1] = ext[-1];
	resNum[2] = 0;

	res = atoi(resNum);

	if (res == 0) {
		return generateIndex();
	} else {
		return generateResource(res);
	}
}

#pragma mark -
#pragma mark --- ScummDiskImage ---
#pragma mark -

static const int maniacResourcesPerFile[55] = {
	 0, 11,  1,  3,  9, 12,  1, 13, 10,  6,
	 4,  1,  7,  1,  1,  2,  7,  8, 19,  9,
	 6,  9,  2,  6,  8,  4, 16,  8,  3,  3,
	12, 12,  2,  8,  1,  1,  2,  1,  9,  1,
	 3,  7,  3,  3, 13,  5,  4,  3,  1,  1,
	 3, 10,  1,  0,  0
};

static const int zakResourcesPerFile[59] = {
	 0, 29, 12, 14, 13,  4,  4, 10,  7,  4,
	14, 19,  5,  4,  7,  6, 11,  9,  4,  4,
	 1,  3,  3,  5,  1,  9,  4, 10, 13,  6,
	 7, 10,  2,  6,  1, 11,  2,  5,  7,  1,
	 7,  1,  4,  2,  8,  6,  6,  6,  4, 13,
	 3,  1,  2,  1,  2,  1, 10,  1,  1
};


ScummDiskImage::ScummDiskImage(const char *disk1, const char *disk2, GameSettings game) : _stream(0), _buf(0) {
	_disk1 = disk1;
	_disk2 = disk2;
	_game = game;

	_openedDisk = 0;

	if (_game.id == GID_MANIAC) {
		_numGlobalObjects = 256;
		_numRooms = 55;
		_numCostumes = 25;
		_numScripts = 160;
		_numSounds = 70;
		_resourcesPerFile = maniacResourcesPerFile;
	} else {
		_numGlobalObjects = 775;
		_numRooms = 59;
		_numCostumes = 38;
		_numScripts = 155;
		_numSounds = 127;
		_resourcesPerFile = zakResourcesPerFile;
	}
}

uint32 ScummDiskImage::write(const void *, uint32) {
	error("ScummDiskImage does not support writing!");
	return 0;
}

void ScummDiskImage::setEnc(byte enc) {
	_stream->setEnc(enc);
}

byte ScummDiskImage::fileReadByte() {
	byte b = 0;
	File::read(&b, 1);
	return b;
}

uint16 ScummDiskImage::fileReadUint16LE() {
	uint16 a = fileReadByte();
	uint16 b = fileReadByte();
	return a | (b << 8);
}

bool ScummDiskImage::openDisk(char num) {
	if (num == '1')
		num = 1;
	if (num == '2')
		num = 2;

	if (_openedDisk != num || !File::isOpen()) {
		if (File::isOpen())
			File::close();

		if (num == 1)
			File::open(_disk1.c_str());
		else if (num == 2)
			File::open(_disk2.c_str());
		else {
			error("ScummDiskImage::open(): wrong disk (%c)", num);
			return false;
		}

		_openedDisk = num;

		if (!File::isOpen()) {
			error("ScummDiskImage::open(): cannot open disk (%d)", num);
			return false;
		}
	}
	return true;
}

bool ScummDiskImage::open(const Common::String &filename, AccessMode mode) {
	uint16 signature;

	// check signature
	openDisk(1);

	if (_game.platform == Common::kPlatformApple2GS) {
		File::seek(142080);
	} else {
		File::seek(0);
	}

	signature = fileReadUint16LE();
	if (signature != 0x0A31) {
		error("ScummDiskImage::open(): signature not found in disk 1!");
		return false;
	}

	extractIndex(0); // Fill in resource arrays

	openDisk(2);

	if (_game.platform == Common::kPlatformApple2GS) {
		File::seek(143104);
		signature = fileReadUint16LE();
		if (signature != 0x0032)
			error("Error: signature not found in disk 2!\n");
	} else {
		File::seek(0);
		signature = fileReadUint16LE();
		if (signature != 0x0132)
			error("Error: signature not found in disk 2!\n");
	}


	return true;
}


uint16 ScummDiskImage::extractIndex(Common::WriteStream *out) {
	int i;
	uint16 reslen = 0;

	openDisk(1);

	if (_game.platform == Common::kPlatformApple2GS) {
		File::seek(142080);
	} else {
		File::seek(0);
	}

	// skip signature
	fileReadUint16LE();

	// write expected signature
	if (_game.platform == Common::kPlatformApple2GS) {
		reslen += write_word(out, 0x0032);
	} else {
		reslen += write_word(out, 0x0132);
	}

	// copy object flags
	for (i = 0; i < _numGlobalObjects; i++)
		reslen += write_byte(out, fileReadByte());

	// copy room offsets
	for (i = 0; i < _numRooms; i++) {
		_roomDisks[i] = fileReadByte();
		reslen += write_byte(out, _roomDisks[i]);
	}
	for (i = 0; i < _numRooms; i++) {
		_roomSectors[i] = fileReadByte();
		reslen += write_byte(out, _roomSectors[i]);
		_roomTracks[i] = fileReadByte();
		reslen += write_byte(out, _roomTracks[i]);
	}
	for (i = 0; i < _numCostumes; i++)
		reslen += write_byte(out, fileReadByte());
	for (i = 0; i < _numCostumes; i++)
		reslen += write_word(out, fileReadUint16LE());

	for (i = 0; i < _numScripts; i++)
		reslen += write_byte(out, fileReadByte());
	for (i = 0; i < _numScripts; i++)
		reslen += write_word(out, fileReadUint16LE());

	for (i = 0; i < _numSounds; i++)
		reslen += write_byte(out, fileReadByte());
	for (i = 0; i < _numSounds; i++)
		reslen += write_word(out, fileReadUint16LE());

	return reslen;
}

bool ScummDiskImage::generateIndex() {
	int bufsize;

	bufsize = extractIndex(0);

	free(_buf);
	_buf = (byte *)calloc(1, bufsize);

	Common::MemoryWriteStream out(_buf, bufsize);

	extractIndex(&out);

	if (_stream)
		delete _stream;

	_stream = new Common::MemoryReadStream(_buf, bufsize);

	return true;
}

uint16 ScummDiskImage::extractResource(Common::WriteStream *out, int res) {
	const int AppleSectorOffset[36] = {
		0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256,
		272, 288, 304, 320, 336, 352, 368,
		384, 400, 416, 432, 448, 464,
		480, 496, 512, 528, 544, 560
	};
	const int C64SectorOffset[36] = {
		0,
		0, 21, 42, 63, 84, 105, 126, 147, 168, 189, 210, 231, 252, 273, 294, 315, 336,
		357, 376, 395, 414, 433, 452, 471,
		490, 508, 526, 544, 562, 580,
		598, 615, 632, 649, 666
	};
	int i;
	uint16 reslen = 0;

	openDisk(_roomDisks[res]);

	if (_game.platform == Common::kPlatformApple2GS) {
		File::seek((AppleSectorOffset[_roomTracks[res]] + _roomSectors[res]) * 256);
	} else {
		File::seek((C64SectorOffset[_roomTracks[res]] + _roomSectors[res]) * 256);
	}

	for (i = 0; i < _resourcesPerFile[res]; i++) {
		uint16 len = fileReadUint16LE();
		reslen += write_word(out, len);

		for (len -= 2; len > 0; len--)
			reslen += write_byte(out, fileReadByte());
	}

	return reslen;
}

bool ScummDiskImage::generateResource(int res) {
	int bufsize;

	if (res >= _numRooms)
		return false;

	bufsize = extractResource(0, res);

	free(_buf);
	_buf = (byte *)calloc(1, bufsize);

	Common::MemoryWriteStream out(_buf, bufsize);

	extractResource(&out, res);

	if (_stream)
		delete _stream;

	_stream = new Common::MemoryReadStream(_buf, bufsize);

	return true;
}

void ScummDiskImage::close() {
	if (_stream)
		delete _stream;
	_stream = 0;

	free(_buf);
	_buf = 0;

	File::close();
}

bool ScummDiskImage::openSubFile(const Common::String &filename) {
	assert(isOpen());

	const char *ext = strrchr(filename.c_str(), '.');
	char resNum[3];
	int res;

	// We always have file name in form of XX.lfl
	resNum[0] = ext[-2];
	resNum[1] = ext[-1];
	resNum[2] = 0;

	res = atoi(resNum);

	if (res == 0) {
		return generateIndex();
	} else {
		return generateResource(res);
	}

	return true;
}

} // End of namespace Scumm
