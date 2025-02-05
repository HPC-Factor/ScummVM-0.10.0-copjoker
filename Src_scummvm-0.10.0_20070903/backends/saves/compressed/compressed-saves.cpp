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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/saves/compressed/compressed-saves.cpp $
 * $Id: compressed-saves.cpp 27063 2007-06-03 14:29:52Z eriktorbjorn $
 *
 */

#include "common/stdafx.h"
#include "common/savefile.h"
#include "common/util.h"
#include "backends/saves/compressed/compressed-saves.h"

#if defined(USE_ZLIB)
#include <zlib.h>

#if ZLIB_VERNUM < 0x1204
#error Version 1.2.0.4 or newer of zlib is required for this code
#endif

/**
 * A simple wrapper class which can be used to wrap around an arbitrary
 * other InSaveFile and will then provide on-the-fly decompression support.
 * Assumes the compressed data to be in gzip format.
 */
class CompressedInSaveFile : public Common::InSaveFile {
protected:
	enum {
		BUFSIZE = 16384		// 1 << MAX_WBITS
	};
	
	byte	_buf[BUFSIZE];

	Common::InSaveFile *_wrapped;
	z_stream _stream;
	int _zlibErr;
	uint32 _pos;
	uint32 _origSize;

public:

	CompressedInSaveFile(Common::InSaveFile *w) : _wrapped(w) {
		assert(w != 0);

		_stream.zalloc = Z_NULL;
		_stream.zfree = Z_NULL;
		_stream.opaque = Z_NULL;
		
		// Verify file header is correct once more
		w->seek(0, SEEK_SET);
		uint16 header = w->readUint16BE();
		assert(header == 0x1F8B ||
		       ((header & 0x0F00) == 0x0800 && header % 31 == 0));
		
		if(header == 0x1F8B) {
			// Retrieve the original file size
			w->seek(-4, SEEK_END);
			_origSize = w->readUint32LE();
		} else {
			// Original size not available in zlib format
			_origSize = 0;
		}
		_pos = 0;
		w->seek(0, SEEK_SET);
		
		// Adding 32 to windowBits indicates to zlib that it is supposed to
		// automatically detect whether gzip or zlib headers are used for
		// the compressed file. This feature was added in zlib 1.2.0.4,
		// released 10 August 2003.
		// Note: This is *crucial* for savegame compatibility, do *not* remove!
		_zlibErr = inflateInit2(&_stream, MAX_WBITS + 32);
		if (_zlibErr != Z_OK)
			return;
		
		// Setup input buffer
		_stream.next_in = _buf;
		_stream.avail_in = 0;
	}

	~CompressedInSaveFile() {
		inflateEnd(&_stream);
		delete _wrapped;
	}

	bool ioFailed() const { return (_zlibErr != Z_OK) && (_zlibErr != Z_STREAM_END); }
	void clearIOFailed() { /* errors here are not recoverable!  */ }

	uint32 read(void *dataPtr, uint32 dataSize) {
		_stream.next_out = (byte *)dataPtr;
		_stream.avail_out = dataSize;

		// Keep going while we get no error
		while (_zlibErr == Z_OK && _stream.avail_out) {
			if (_stream.avail_in == 0 && !_wrapped->eos()) {
				// If we are out of input data: Read more data, if available.
				_stream.next_in = _buf;
				_stream.avail_in = _wrapped->read(_buf, BUFSIZE);
			}
			_zlibErr = inflate(&_stream, Z_NO_FLUSH);
		}
		
		// Update the position counter
		_pos += dataSize - _stream.avail_out;

		return dataSize - _stream.avail_out;
	}

	bool eos() const {
		return (_zlibErr == Z_STREAM_END);
		//return _pos == _origSize;
	}
	uint32 pos() const {
		return _pos;
	}
	uint32 size() const {
		return _origSize;
	}
	void seek(int32 offset, int whence = SEEK_SET) {
		int32 newPos = 0;
		switch(whence) {
		case SEEK_END:
			newPos = size() - offset;
			break;
		case SEEK_SET:
			newPos = offset;
			break;
		case SEEK_CUR:
			newPos = _pos + offset;
		}
		offset = newPos - _pos;

		if (offset < 0)
			error("Backward seeking not supported in compressed savefiles");

		// We could implement backward seeking, but it is tricky to do efficiently.
		// A simple solution would be to restart the whole decompression from the
		// start of the file. Or we could decompress the whole file in one go
		// in the constructor, and wrap it into a MemoryReadStream -- but that
		// would be rather wasteful. As long as we don't need it, I'd rather not
		// implement this at all. -- Fingolfin
		
		// Skip the given amount of data (very inefficient if one tries to skip
		// huge amounts of data, but usually client code will only skip a few
		// bytes, so this should be fine.
		byte tmpBuf[1024];
		while (!ioFailed() && offset > 0) {
			offset -= read(tmpBuf, MIN((int32)sizeof(tmpBuf), offset));
		}
	}
};

/**
 * A simple wrapper class which can be used to wrap around an arbitrary
 * other OutSaveFile and will then provide on-the-fly compression support.
 * The compressed data is written in the gzip format.
 */
class CompressedOutSaveFile : public Common::OutSaveFile {
protected:
	enum {
		BUFSIZE = 16384		// 1 << MAX_WBITS
	};
	
	byte	_buf[BUFSIZE];
	Common::OutSaveFile *_wrapped;
	z_stream _stream;
	int _zlibErr;

	void processData(int flushType) {
		// This function is called by both write() and finalize().
		while (_zlibErr == Z_OK && (_stream.avail_in || flushType == Z_FINISH)) {
			if (_stream.avail_out == 0) {
				if (_wrapped->write(_buf, BUFSIZE) != BUFSIZE) {
					_zlibErr = Z_ERRNO;
					break;
				}
				_stream.next_out = _buf;
				_stream.avail_out = BUFSIZE;
			}
			_zlibErr = deflate(&_stream, flushType);
		}
	}

public:
	CompressedOutSaveFile(Common::OutSaveFile *w) : _wrapped(w) {
		assert(w != 0);
		_stream.zalloc = Z_NULL;
		_stream.zfree = Z_NULL;
		_stream.opaque = Z_NULL;
		
		// Adding 16 to windowBits indicates to zlib that it is supposed to
		// write gzip headers. This feature was added in zlib 1.2.0.4,
		// released 10 August 2003.
		// Note: This is *crucial* for savegame compatibility, do *not* remove!
		_zlibErr = deflateInit2(&_stream,
		                 Z_DEFAULT_COMPRESSION,
		                 Z_DEFLATED,
		                 MAX_WBITS + 16,
		                 8,
                         Z_DEFAULT_STRATEGY);
		assert(_zlibErr == Z_OK);

		_stream.next_out = _buf;
		_stream.avail_out = BUFSIZE;
		_stream.avail_in = 0;
		_stream.next_in = 0;
	}

	~CompressedOutSaveFile() {
		finalize();
		deflateEnd(&_stream);
		delete _wrapped;
	}

	bool ioFailed() const {
		return (_zlibErr != Z_OK && _zlibErr != Z_STREAM_END) || _wrapped->ioFailed();
	}

	void clearIOFailed() {
		// Note: we don't reset the _zlibErr here, as it is not
		// clear in general ho
		_wrapped->clearIOFailed();
	}

	void finalize() {
		if (_zlibErr != Z_OK)
			return;

		// Process whatever remaining data there is.
		processData(Z_FINISH);

		// Since processData only writes out blocks of size BUFSIZE,
		// we may have to flush some stragglers.
		uint remainder = BUFSIZE - _stream.avail_out;
		if (remainder > 0) {
			if (_wrapped->write(_buf, remainder) != remainder) {
				_zlibErr = Z_ERRNO;
			}
		}

		// Finalize the wrapped savefile, too
		_wrapped->finalize();
	}

	uint32 write(const void *dataPtr, uint32 dataSize) {
		if (ioFailed())
			return 0;

		// Hook in the new data ...
		// Note: We need to make a const_cast here, as zlib is not aware
		// of the const keyword.
		_stream.next_in = const_cast<byte *>((const byte *)dataPtr);
		_stream.avail_in = dataSize;
	
		// ... and flush it to disk
		processData(Z_NO_FLUSH);

		return dataSize - _stream.avail_in;
	}
};

#endif	// USE_ZLIB

Common::InSaveFile *wrapInSaveFile(Common::InSaveFile *toBeWrapped) {
#if defined(USE_ZLIB)
	if (toBeWrapped) {
		uint16 header = toBeWrapped->readUint16BE();
		bool isCompressed = (header == 0x1F8B ||
				     ((header & 0x0F00) == 0x0800 &&
				      header % 31 == 0));
		toBeWrapped->seek(-2, SEEK_CUR);
		if (isCompressed)
			return new CompressedInSaveFile(toBeWrapped);
	}
#endif
	return toBeWrapped;
}

Common::OutSaveFile *wrapOutSaveFile(Common::OutSaveFile *toBeWrapped) {
#if defined(USE_ZLIB)
	if (toBeWrapped)
		return new CompressedOutSaveFile(toBeWrapped);
#endif
	return toBeWrapped;
}
