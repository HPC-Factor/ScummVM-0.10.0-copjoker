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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/common/stream.h $
 * $Id: stream.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef COMMON_STREAM_H
#define COMMON_STREAM_H

#include "common/stdafx.h"
#include "common/scummsys.h"

namespace Common {

class String;
class MemoryReadStream;

/**
 * Virtual base class for both ReadStream and WriteStream.
 */
class Stream {
public:
	virtual ~Stream() {}

	/**
	 * Returns true if any I/O failure occurred.
	 * This flag is never cleared automatically. In order to clear it,
	 * client code has to call clearIOFailed() explicitly.
	 *
	 * @todo Instead of returning a plain bool, maybe we should define
	 *       a list of error codes which can be returned here.
	 */
	virtual bool ioFailed() const { return false; }

	/**
	 * Reset the I/O error status.
	 */
	virtual void clearIOFailed() {}
};

/**
 * Generic interface for a writable data stream.
 */
class WriteStream : virtual public Stream {
public:
	/**
	 * Write data into the stream. Subclasses must implement this
	 * method; all other write methods are implemented using it.
	 *
	 * @param dataPtr	pointer to the data to be written
	 * @param dataSize	number of bytes to be written
	 * @return the number of bytes which were actually written.
	 */
	virtual uint32 write(const void *dataPtr, uint32 dataSize) = 0;

	/**
	 * Commit any buffered data to the underlying channel or
	 * storage medium; unbuffered streams can use the default
	 * implementation.
	 */
	virtual void flush() {}

	// The remaining methods all have default implementations; subclasses
	// need not (and should not) overload them.

	void writeByte(byte value) {
		write(&value, 1);
	}

	void writeSByte(int8 value) {
		write(&value, 1);
	}

	void writeUint16LE(uint16 value) {
		writeByte((byte)(value & 0xff));
		writeByte((byte)(value >> 8));
	}

	void writeUint32LE(uint32 value) {
		writeUint16LE((uint16)(value & 0xffff));
		writeUint16LE((uint16)(value >> 16));
	}

	void writeUint16BE(uint16 value) {
		writeByte((byte)(value >> 8));
		writeByte((byte)(value & 0xff));
	}

	void writeUint32BE(uint32 value) {
		writeUint16BE((uint16)(value >> 16));
		writeUint16BE((uint16)(value & 0xffff));
	}

	void writeSint16LE(int16 value) {
		writeUint16LE((uint16)value);
	}

	void writeSint32LE(int32 value) {
		writeUint32LE((uint32)value);
	}

	void writeSint16BE(int16 value) {
		writeUint16BE((uint16)value);
	}

	void writeSint32BE(int32 value) {
		writeUint32BE((uint32)value);
	}

	void writeString(const String &str);
};


/**
 * Generic interface for a readable data stream.
 */
class ReadStream : virtual public Stream {
public:
	/**
	 * Returns true if the end of the stream has been reached.
	 */
	virtual bool eos() const = 0;

	/**
	 * Read data from the stream. Subclasses must implement this
	 * method; all other read methods are implemented using it.
	 *
	 * @param dataPtr	pointer to a buffer into which the data is read
	 * @param dataSize	number of bytes to be read
	 * @return the number of bytes which were actually read.
	 */
	virtual uint32 read(void *dataPtr, uint32 dataSize) = 0;


	// The remaining methods all have default implementations; subclasses
	// need not (and should not) overload them.

	/**
	 * Read am unsigned byte from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling ioFailed()).
	 */
	byte readByte() {
		byte b = 0;
		read(&b, 1);
		return b;
	}

	/**
	 * Read a signed byte from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling ioFailed()).
	 */
	int8 readSByte() {
		int8 b = 0;
		read(&b, 1);
		return b;
	}

	/**
	 * Read an unsigned 16-bit word stored in little endian (LSB first) order
	 * from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling ioFailed()).
	 */
	uint16 readUint16LE() {
		uint16 a = readByte();
		uint16 b = readByte();
		return a | (b << 8);
	}

	/**
	 * Read an unsigned 32-bit word stored in little endian (LSB first) order
	 * from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling ioFailed()).
	 */
	uint32 readUint32LE() {
		uint32 a = readUint16LE();
		uint32 b = readUint16LE();
		return (b << 16) | a;
	}

	/**
	 * Read an unsigned 16-bit word stored in big endian (MSB first) order
	 * from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling ioFailed()).
	 */
	uint16 readUint16BE() {
		uint16 b = readByte();
		uint16 a = readByte();
		return a | (b << 8);
	}

	/**
	 * Read an unsigned 32-bit word stored in big endian (MSB first) order
	 * from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling ioFailed()).
	 */
	uint32 readUint32BE() {
		uint32 b = readUint16BE();
		uint32 a = readUint16BE();
		return (b << 16) | a;
	}

	/**
	 * Read a signed 16-bit word stored in little endian (LSB first) order
	 * from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling ioFailed()).
	 */
	int16 readSint16LE() {
		return (int16)readUint16LE();
	}

	/**
	 * Read a signed 32-bit word stored in little endian (LSB first) order
	 * from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling ioFailed()).
	 */
	int32 readSint32LE() {
		return (int32)readUint32LE();
	}

	/**
	 * Read a signed 16-bit word stored in big endian (MSB first) order
	 * from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling ioFailed()).
	 */
	int16 readSint16BE() {
		return (int16)readUint16BE();
	}

	/**
	 * Read a signed 32-bit word stored in big endian (MSB first) order
	 * from the stream and return it.
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which client code can check by
	 * calling ioFailed()).
	 */
	int32 readSint32BE() {
		return (int32)readUint32BE();
	}
	
	/**
	 * Read the specified amount of data into a malloc'ed buffer
	 * which then is wrapped into a MemoryReadStream.
	 * The returned stream might contain less data than requested,
	 * if reading more failed.
	 */
	MemoryReadStream *readStream(uint32 dataSize);

};


/**
 * Interface for a seekable & readable data stream.
 *
 * @todo We really need better error handling here!
 *       Like seek should somehow indicate whether it failed.
 */
class SeekableReadStream : virtual public ReadStream {
public:

	virtual uint32 pos() const = 0;
	virtual uint32 size() const = 0;

	virtual void seek(int32 offset, int whence = SEEK_SET) = 0;

	void skip(uint32 offset) { seek(offset, SEEK_CUR); }

	/**
	 * Read one line of text from a CR or CR/LF terminated plain text file.
	 * This method is a rough analog of the (f)gets function.
	 *
	 * @param buf	the buffer to store into
	 * @param bufSize	the size of the buffer
	 * @return a pointer to the read string, or NULL if an error occurred
	 * @note The line terminator (CR or CR/LF) is stripped and not inserted
	 *       into the buffer.
	 */
	virtual char *readLine(char *buf, size_t bufSize);
};

/**
 * SubReadStream provides access to a ReadStream restricted to the range
 * [currentPosition, currentPosition+end).
 * Manipulating the parent stream directly /will/ mess up a substream.
 * Likewise, manipulating two substreams of a parent stream will cause them to
 * step on each others toes.
 */
class SubReadStream : virtual public ReadStream {
protected:
	ReadStream *_parentStream;
	uint32 _pos;
	uint32 _end;
	bool _disposeParentStream;
public:
	SubReadStream(ReadStream *parentStream, uint32 end, bool disposeParentStream = false)
		: _parentStream(parentStream),
		  _pos(0),
		  _end(end),
		  _disposeParentStream(disposeParentStream) {}
	~SubReadStream() {
		if (_disposeParentStream) delete _parentStream;
	}

	virtual bool eos() const { return _pos == _end; }
	virtual uint32 read(void *dataPtr, uint32 dataSize);
};

/*
 * SeekableSubReadStream provides access to a SeekableReadStream restricted to
 * the range [begin, end).
 * The same caveats apply to SeekableSubReadStream as do to SeekableReadStream.
 */
class SeekableSubReadStream : public SubReadStream, public SeekableReadStream {
protected:
	SeekableReadStream *_parentStream;
	uint32 _begin;
public:
	SeekableSubReadStream(SeekableReadStream *parentStream, uint32 begin, uint32 end, bool disposeParentStream = false);

	virtual uint32 pos() const { return _pos - _begin; }
	virtual uint32 size() const { return _end - _begin; }

	virtual void seek(int32 offset, int whence = SEEK_SET);
};

/**
 * Simple memory based 'stream', which implements the ReadStream interface for
 * a plain memory block.
 */
class MemoryReadStream : public SeekableReadStream {
private:
	const byte * const _ptrOrig;
	const byte *_ptr;
	const uint32 _size;
	uint32 _pos;
	byte _encbyte;
	bool _disposeMemory;

public:

	/**
	 * This constructor takes a pointer to a memory buffer and a length, and
	 * wraps it. If disposeMemory is true, the MemoryReadStream takes ownership
	 * of the buffer and hence free's it when destructed.
	 */
	MemoryReadStream(const byte *dataPtr, uint32 dataSize, bool disposeMemory = false) :
		_ptrOrig(dataPtr),
		_ptr(dataPtr),
		_size(dataSize),
		_pos(0),
		_encbyte(0),
		_disposeMemory(disposeMemory) {}

	~MemoryReadStream() {
		if (_disposeMemory)
			free(const_cast<byte *>(_ptrOrig));
	}

	void setEnc(byte value) { _encbyte = value; }

	uint32 read(void *dataPtr, uint32 dataSize);

	bool eos() const { return _pos == _size; }
	uint32 pos() const { return _pos; }
	uint32 size() const { return _size; }

	void seek(int32 offs, int whence = SEEK_SET);
};

/**
 * Simple memory based 'stream', which implements the WriteStream interface for
 * a plain memory block.
 */
class MemoryWriteStream : public WriteStream {
private:
	byte *_ptr;
	const uint32 _bufSize;
	uint32 _pos;
public:
	MemoryWriteStream(byte *buf, uint32 len) : _ptr(buf), _bufSize(len), _pos(0) {}

	uint32 write(const void *dataPtr, uint32 dataSize) {
		// Write at most as many bytes as are still available...
		if (dataSize > _bufSize - _pos)
			dataSize = _bufSize - _pos;
		memcpy(_ptr, dataPtr, dataSize);
		_ptr += dataSize;
		_pos += dataSize;
		return dataSize;
	}

	bool eos() const { return _pos == _bufSize; }
	uint32 pos() const { return _pos; }
	uint32 size() const { return _bufSize; }
};

}	// End of namespace Common

#endif
