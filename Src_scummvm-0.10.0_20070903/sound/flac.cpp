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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/sound/flac.cpp $
 * $Id: flac.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "sound/flac.h"

#ifdef USE_FLAC

#include "common/file.h"
#include "common/util.h"

#include "sound/audiostream.h"
#include "sound/audiocd.h"

#define FLAC__NO_DLL // that MS-magic gave me headaches - just link the library you like
#include <FLAC/export.h>


// check if we have FLAC >= 1.1.3; LEGACY_FLAC code can be removed once FLAC-1.1.3 propagates everywhere
#if !defined(FLAC_API_VERSION_CURRENT) || FLAC_API_VERSION_CURRENT < 8
#define LEGACY_FLAC
#else
#undef LEGACY_FLAC
#endif


#ifdef LEGACY_FLAC

// Before FLAC 1.1.3, we needed to use the stream decoder API.
#include <FLAC/seekable_stream_decoder.h>
typedef uint FLAC_size_t;

#else

// With FLAC 1.1.3, the stream decoder API was merged into the regular
// stream API. In order to stay compatible with older FLAC versions, we
// simply add some typedefs and #ifdefs to map between the old and new API.
// We use the typedefs (instead of only #defines) in order to somewhat
// improve the readability of the code.

#include <FLAC/stream_decoder.h>
typedef size_t FLAC_size_t;
// Add aliases for the old names
typedef FLAC__StreamDecoderState FLAC__SeekableStreamDecoderState;
typedef FLAC__StreamDecoderReadStatus FLAC__SeekableStreamDecoderReadStatus;
typedef FLAC__StreamDecoderSeekStatus FLAC__SeekableStreamDecoderSeekStatus;
typedef FLAC__StreamDecoderTellStatus FLAC__SeekableStreamDecoderTellStatus;
typedef FLAC__StreamDecoderLengthStatus FLAC__SeekableStreamDecoderLengthStatus;
typedef FLAC__StreamDecoder FLAC__SeekableStreamDecoder;

#endif


using Common::File;


namespace Audio {

#pragma mark -
#pragma mark --- Flac stream ---
#pragma mark -

static const uint MAX_OUTPUT_CHANNELS = 2;


class FlacInputStream : public AudioStream {
protected:
	Common::SeekableReadStream *_inStream;
	bool _disposeAfterUse;

	uint _numLoops;

	::FLAC__SeekableStreamDecoder *_decoder;

	/** Header of the stream */
	FLAC__StreamMetadata_StreamInfo _streaminfo;

	/** index of the first sample to be played */
	FLAC__uint64 _firstSample;

	/** index + 1(!) of the last sample to be played - 0 is end of stream */
	FLAC__uint64 _lastSample;

	/** true if the last sample was decoded from the FLAC-API - there might still be data in the buffer */
	bool _lastSampleWritten;

	typedef int16 SampleType;
	enum { BUFTYPE_BITS = 16 };

	enum {
		// Maximal buffer size. According to the FLAC format specification, the  block size is
		// a 16 bit value (in fact it seems the maximal block size is 32768, but we play it safe).
		BUFFER_SIZE = 65536
	};
	
	struct {
		SampleType bufData[BUFFER_SIZE];
		SampleType *bufReadPos;
		uint bufFill;
	} _sampleCache;

	SampleType *_outBuffer;
	uint _requestedSamples;

	typedef void (*PFCONVERTBUFFERS)(SampleType*, const FLAC__int32*[], uint, const uint, const uint8);
	PFCONVERTBUFFERS _methodConvertBuffers;


public:
	FlacInputStream(Common::SeekableReadStream *inStream, bool dispose, uint startTime = 0, uint endTime = 0, uint numLoops = 1);
	virtual ~FlacInputStream();

	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const { return _streaminfo.channels >= 2; }
	int getRate() const { return _streaminfo.sample_rate; }
	bool endOfData() const {
		// End of data is reached if there either is no valid stream data available,
		// or if we reached the last sample and completely emptied the sample cache
		return _streaminfo.channels == 0 || (_lastSampleWritten && _sampleCache.bufFill == 0);
	}

	bool isStreamDecoderReady() const { return getStreamDecoderState() == FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC ; }

protected:
	uint getChannels() const { return MIN(_streaminfo.channels, MAX_OUTPUT_CHANNELS); }

	bool allocateBuffer(uint minSamples);

	inline FLAC__StreamDecoderState getStreamDecoderState() const;

	inline bool processSingleBlock();
	inline bool processUntilEndOfMetadata();
	bool seekAbsolute(FLAC__uint64 sample);

	inline ::FLAC__SeekableStreamDecoderReadStatus callbackRead(FLAC__byte buffer[], FLAC_size_t *bytes);
	inline ::FLAC__SeekableStreamDecoderSeekStatus callbackSeek(FLAC__uint64 absoluteByteOffset);
	inline ::FLAC__SeekableStreamDecoderTellStatus callbackTell(FLAC__uint64 *absoluteByteOffset);
	inline ::FLAC__SeekableStreamDecoderLengthStatus callbackLength(FLAC__uint64 *streamLength);
	inline bool callbackEOF();
	inline ::FLAC__StreamDecoderWriteStatus callbackWrite(const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[]);
	inline void callbackMetadata(const ::FLAC__StreamMetadata *metadata);
	inline void callbackError(::FLAC__StreamDecoderErrorStatus status);

private:
	static ::FLAC__SeekableStreamDecoderReadStatus callWrapRead(const ::FLAC__SeekableStreamDecoder *decoder, FLAC__byte buffer[], FLAC_size_t *bytes, void *clientData);
	static ::FLAC__SeekableStreamDecoderSeekStatus callWrapSeek(const ::FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 absoluteByteOffset, void *clientData);
	static ::FLAC__SeekableStreamDecoderTellStatus callWrapTell(const ::FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *absoluteByteOffset, void *clientData);
	static ::FLAC__SeekableStreamDecoderLengthStatus callWrapLength(const ::FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *streamLength, void *clientData);
	static FLAC__bool callWrapEOF(const ::FLAC__SeekableStreamDecoder *decoder, void *clientData);
	static ::FLAC__StreamDecoderWriteStatus callWrapWrite(const ::FLAC__SeekableStreamDecoder *decoder, const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *clientData);
	static void callWrapMetadata(const ::FLAC__SeekableStreamDecoder *decoder, const ::FLAC__StreamMetadata *metadata, void *clientData);
	static void callWrapError(const ::FLAC__SeekableStreamDecoder *decoder, ::FLAC__StreamDecoderErrorStatus status, void *clientData);

	void setBestConvertBufferMethod();
	static void convertBuffersGeneric(SampleType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits);
	static void convertBuffersStereoNS(SampleType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits);
	static void convertBuffersStereo8Bit(SampleType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits);
	static void convertBuffersMonoNS(SampleType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits);
	static void convertBuffersMono8Bit(SampleType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits);
};

FlacInputStream::FlacInputStream(Common::SeekableReadStream *inStream, bool dispose, uint startTime, uint endTime, uint numLoops)
#ifdef LEGACY_FLAC
			:	_decoder(::FLAC__seekable_stream_decoder_new()),
#else
			:	_decoder(::FLAC__stream_decoder_new()),
#endif
		_inStream(inStream),
		_disposeAfterUse(dispose),
		_numLoops(numLoops),
		_firstSample(0), _lastSample(0),
		_outBuffer(NULL), _requestedSamples(0), _lastSampleWritten(false),
		_methodConvertBuffers(&FlacInputStream::convertBuffersGeneric)
{
	assert(_inStream);
	memset(&_streaminfo, 0, sizeof(_streaminfo));

	_sampleCache.bufReadPos = NULL;
	_sampleCache.bufFill = 0;

	_methodConvertBuffers = &FlacInputStream::convertBuffersGeneric;

	bool success;
#ifdef LEGACY_FLAC
	::FLAC__seekable_stream_decoder_set_read_callback(_decoder, &FlacInputStream::callWrapRead);
	::FLAC__seekable_stream_decoder_set_seek_callback(_decoder, &FlacInputStream::callWrapSeek);
	::FLAC__seekable_stream_decoder_set_tell_callback(_decoder, &FlacInputStream::callWrapTell);
	::FLAC__seekable_stream_decoder_set_length_callback(_decoder, &FlacInputStream::callWrapLength);
	::FLAC__seekable_stream_decoder_set_eof_callback(_decoder, &FlacInputStream::callWrapEOF);
	::FLAC__seekable_stream_decoder_set_write_callback(_decoder, &FlacInputStream::callWrapWrite);
	::FLAC__seekable_stream_decoder_set_metadata_callback(_decoder, &FlacInputStream::callWrapMetadata);
	::FLAC__seekable_stream_decoder_set_error_callback(_decoder, &FlacInputStream::callWrapError);
	::FLAC__seekable_stream_decoder_set_client_data(_decoder, (void*)this);

	success = (::FLAC__seekable_stream_decoder_init(_decoder) == FLAC__SEEKABLE_STREAM_DECODER_OK);
#else
	success = (::FLAC__stream_decoder_init_stream(
		_decoder,
		&FlacInputStream::callWrapRead, 
		&FlacInputStream::callWrapSeek, 
		&FlacInputStream::callWrapTell, 
		&FlacInputStream::callWrapLength, 
		&FlacInputStream::callWrapEOF, 
		&FlacInputStream::callWrapWrite, 
		&FlacInputStream::callWrapMetadata, 
		&FlacInputStream::callWrapError, 
		(void*)this
	) == FLAC__STREAM_DECODER_INIT_STATUS_OK);
#endif
	if (success) {
		if (processUntilEndOfMetadata() && _streaminfo.channels > 0) {
			// Compute the start/end sample (we use floating point arithmetics here to
			// avoid overflows).
			_firstSample = (FLAC__uint64)(startTime * (_streaminfo.sample_rate / 1000.0));
			_lastSample = (FLAC__uint64)(endTime * (_streaminfo.sample_rate / 1000.0));
			if (_firstSample == 0 || seekAbsolute(_firstSample)) {
				return; // no error occured
			}
		}
	}

	warning("FlacInputStream: could not create audio stream");
}

FlacInputStream::~FlacInputStream() {
	if (_decoder != NULL) {
#ifdef LEGACY_FLAC
		(void) ::FLAC__seekable_stream_decoder_finish(_decoder);
		::FLAC__seekable_stream_decoder_delete(_decoder);
#else
		(void) ::FLAC__stream_decoder_finish(_decoder);
		::FLAC__stream_decoder_delete(_decoder);
#endif
	}
	if (_disposeAfterUse)
		delete _inStream;
}

inline FLAC__StreamDecoderState FlacInputStream::getStreamDecoderState() const {
	assert(_decoder != NULL);
#ifdef LEGACY_FLAC
	return ::FLAC__seekable_stream_decoder_get_stream_decoder_state(_decoder);
#else
	return ::FLAC__stream_decoder_get_state(_decoder);
#endif
}

inline bool FlacInputStream::processSingleBlock() {
	assert(_decoder != NULL);
#ifdef LEGACY_FLAC
	return 0 != ::FLAC__seekable_stream_decoder_process_single(_decoder);
#else
	return 0 != ::FLAC__stream_decoder_process_single(_decoder);
#endif
}

inline bool FlacInputStream::processUntilEndOfMetadata() {
	assert(_decoder != NULL);
#ifdef LEGACY_FLAC
	return 0 != ::FLAC__seekable_stream_decoder_process_until_end_of_metadata(_decoder);
#else
	return 0 != ::FLAC__stream_decoder_process_until_end_of_metadata(_decoder);
#endif
}

bool FlacInputStream::seekAbsolute(FLAC__uint64 sample) {
	assert(_decoder != NULL);
#ifdef LEGACY_FLAC
	const bool result = (0 != ::FLAC__seekable_stream_decoder_seek_absolute(_decoder, sample));
#else
	const bool result = (0 != ::FLAC__stream_decoder_seek_absolute(_decoder, sample));
#endif
	if (result) {
		_lastSampleWritten = (_lastSample != 0 && sample >= _lastSample); // only set if we are SURE
	}
	return result;
}

int FlacInputStream::readBuffer(int16 *buffer, const int numSamples) {
	const uint numChannels = getChannels();

	if (numChannels == 0) {
		warning("FlacInputStream: Stream not sucessfully initialised, cant playback");
		return -1; // streaminfo wasnt read!
	}

	assert(numSamples % numChannels == 0); // must be multiple of channels!
	assert(buffer != NULL);
	assert(_outBuffer == NULL);
	assert(_requestedSamples == 0);

	_outBuffer = buffer;
	_requestedSamples = numSamples;

	// If there is still data in our buffer from the last time around,
	// copy that first.
	if (_sampleCache.bufFill > 0) {
		assert(_sampleCache.bufReadPos >= _sampleCache.bufData);
		assert(_sampleCache.bufFill % numChannels == 0);

		const uint copySamples = MIN((uint)numSamples, _sampleCache.bufFill);
		memcpy(buffer, _sampleCache.bufReadPos, copySamples*sizeof(buffer[0]));

		_outBuffer = buffer + copySamples;
		_requestedSamples = numSamples - copySamples;
		_sampleCache.bufReadPos += copySamples;
		_sampleCache.bufFill -= copySamples;
	}

	bool decoderOk = true;

	if (!_lastSampleWritten) {
		FLAC__StreamDecoderState state = getStreamDecoderState();

		// Keep poking FLAC to process more samples until we completely satisfied the request
		while (_requestedSamples > 0 && state == FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC) {
			assert(_sampleCache.bufFill == 0);
			assert(_requestedSamples % numChannels == 0);
			processSingleBlock();
			state = getStreamDecoderState();
			
			if (state == FLAC__STREAM_DECODER_END_OF_STREAM) {
				_lastSampleWritten = true;
			}

			// If we reached the end of the stream, and looping is enabled: Try to rewind
			if (_lastSampleWritten && _numLoops != 1) {
				if (_numLoops != 0)
					_numLoops--;
				seekAbsolute(_firstSample);
				state = getStreamDecoderState();
			}
		}

		// Error handling
		switch (state) {
		case FLAC__STREAM_DECODER_END_OF_STREAM:
			_lastSampleWritten = true;
			break;
		case FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC:
			break;
		default:
			decoderOk = false;
			warning("FlacInputStream: An error occured while decoding. DecoderState is: %s",
				FLAC__StreamDecoderStateString[getStreamDecoderState()]);
		}
	}

	// Compute how many samples we actually produced
	const int samples = (int)(_outBuffer - buffer);
	assert(samples % numChannels == 0);

	_outBuffer = NULL; // basically unnecessary, only for the purpose of the asserts
	_requestedSamples = 0; // basically unnecessary, only for the purpose of the asserts

	return decoderOk ? samples : -1;
}

inline ::FLAC__SeekableStreamDecoderReadStatus FlacInputStream::callbackRead(FLAC__byte buffer[], FLAC_size_t *bytes) {
	if (*bytes == 0)
#ifdef LEGACY_FLAC
		return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_ERROR; /* abort to avoid a deadlock */
#else
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT; /* abort to avoid a deadlock */
#endif

	const uint32 bytesRead = _inStream->read(buffer, *bytes);

	if (bytesRead == 0 && _inStream->ioFailed())
#ifdef LEGACY_FLAC
		return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_ERROR;
#else
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
#endif

	*bytes = static_cast<uint>(bytesRead);
#ifdef LEGACY_FLAC
	return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_OK;
#else
	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
#endif
}

void FlacInputStream::setBestConvertBufferMethod() {
	PFCONVERTBUFFERS tempMethod = &FlacInputStream::convertBuffersGeneric;

	const uint numChannels = getChannels();
	const uint8 numBits = (uint8)_streaminfo.bits_per_sample;

	assert(numChannels >= 1);
	assert(numBits >= 4 && numBits <=32);

	if (numChannels == 1) {
		if (numBits == 8)
			tempMethod = &FlacInputStream::convertBuffersMono8Bit;
		if (numBits == BUFTYPE_BITS)
			tempMethod = &FlacInputStream::convertBuffersMonoNS;
	} else if (numChannels == 2) {
		if (numBits == 8)
			tempMethod = &FlacInputStream::convertBuffersStereo8Bit;
		if (numBits == BUFTYPE_BITS)
			tempMethod = &FlacInputStream::convertBuffersStereoNS;
	} /* else ... */

	_methodConvertBuffers = tempMethod;
}

// 1 channel, no scaling
void FlacInputStream::convertBuffersMonoNS(SampleType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits) {
	assert(numChannels == 1);
	assert(numBits == BUFTYPE_BITS);

	FLAC__int32 const* inChannel1 = inChannels[0];

	while (numSamples >= 4) {
		bufDestination[0] = static_cast<SampleType>(inChannel1[0]);
		bufDestination[1] = static_cast<SampleType>(inChannel1[1]);
		bufDestination[2] = static_cast<SampleType>(inChannel1[2]);
		bufDestination[3] = static_cast<SampleType>(inChannel1[3]);
		bufDestination += 4;
		inChannel1 += 4;
		numSamples -= 4;
	}

	for (; numSamples > 0; --numSamples) {
		*bufDestination++ = static_cast<SampleType>(*inChannel1++);
	}

	inChannels[0] = inChannel1;
	assert(numSamples == 0); // dint copy too many samples
}

// 1 channel, scaling from 8Bit
void FlacInputStream::convertBuffersMono8Bit(SampleType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits) {
	assert(numChannels == 1);
	assert(numBits == 8);
	assert(8 < BUFTYPE_BITS);

	FLAC__int32 const* inChannel1 = inChannels[0];

	while (numSamples >= 4) {
		bufDestination[0] = static_cast<SampleType>(inChannel1[0]) << (BUFTYPE_BITS - 8);
		bufDestination[1] = static_cast<SampleType>(inChannel1[1]) << (BUFTYPE_BITS - 8);
		bufDestination[2] = static_cast<SampleType>(inChannel1[2]) << (BUFTYPE_BITS - 8);
		bufDestination[3] = static_cast<SampleType>(inChannel1[3]) << (BUFTYPE_BITS - 8);
		bufDestination += 4;
		inChannel1 += 4;
		numSamples -= 4;
	}

	for (; numSamples > 0; --numSamples) {
		*bufDestination++ = static_cast<SampleType>(*inChannel1++) << (BUFTYPE_BITS - 8);
	}

	inChannels[0] = inChannel1;
	assert(numSamples == 0); // dint copy too many samples
}

// 2 channels, no scaling
void FlacInputStream::convertBuffersStereoNS(SampleType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits) {
	assert(numChannels == 2);
	assert(numBits == BUFTYPE_BITS);
	assert(numSamples % 2 == 0); // must be integral multiply of channels


	FLAC__int32 const* inChannel1 = inChannels[0];	// Left Channel
	FLAC__int32 const* inChannel2 = inChannels[1];	// Right Channel

	while (numSamples >= 2*2) {
		bufDestination[0] = static_cast<SampleType>(inChannel1[0]);
		bufDestination[1] = static_cast<SampleType>(inChannel2[0]);
		bufDestination[2] = static_cast<SampleType>(inChannel1[1]);
		bufDestination[3] = static_cast<SampleType>(inChannel2[1]);
		bufDestination += 2 * 2;
		inChannel1 += 2;
		inChannel2 += 2;
		numSamples -= 2 * 2;
	}

	while (numSamples > 0) {
		bufDestination[0] = static_cast<SampleType>(*inChannel1++);
		bufDestination[1] = static_cast<SampleType>(*inChannel2++);
		bufDestination += 2;
		numSamples -= 2;
	}

	inChannels[0] = inChannel1;
	inChannels[1] = inChannel2;
	assert(numSamples == 0); // dint copy too many samples
}

// 2 channels, scaling from 8Bit
void FlacInputStream::convertBuffersStereo8Bit(SampleType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits) {
	assert(numChannels == 2);
	assert(numBits == 8);
	assert(numSamples % 2 == 0); // must be integral multiply of channels
	assert(8 < BUFTYPE_BITS);

	FLAC__int32 const* inChannel1 = inChannels[0];	// Left Channel
	FLAC__int32 const* inChannel2 = inChannels[1];	// Right Channel

	while (numSamples >= 2*2) {
		bufDestination[0] = static_cast<SampleType>(inChannel1[0]) << (BUFTYPE_BITS - 8);
		bufDestination[1] = static_cast<SampleType>(inChannel2[0]) << (BUFTYPE_BITS - 8);
		bufDestination[2] = static_cast<SampleType>(inChannel1[1]) << (BUFTYPE_BITS - 8);
		bufDestination[3] = static_cast<SampleType>(inChannel2[1]) << (BUFTYPE_BITS - 8);
		bufDestination += 2 * 2;
		inChannel1 += 2;
		inChannel2 += 2;
		numSamples -= 2 * 2;
	}

	while (numSamples > 0) {
		bufDestination[0] = static_cast<SampleType>(*inChannel1++) << (BUFTYPE_BITS - 8);
		bufDestination[1] = static_cast<SampleType>(*inChannel2++) << (BUFTYPE_BITS - 8);
		bufDestination += 2;
		numSamples -= 2;
	}

	inChannels[0] = inChannel1;
	inChannels[1] = inChannel2;
	assert(numSamples == 0); // dint copy too many samples
}

// all Purpose-conversion - slowest of em all
void FlacInputStream::convertBuffersGeneric(SampleType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits) {
	assert(numSamples % numChannels == 0); // must be integral multiply of channels

	if (numBits < BUFTYPE_BITS) {
		const uint8 kPower = (uint8)(BUFTYPE_BITS - numBits);

		for (; numSamples > 0; numSamples -= numChannels) {
			for (uint i = 0; i < numChannels; ++i)
				*bufDestination++ = static_cast<SampleType>(*(inChannels[i]++)) << kPower;
		}
	} else if (numBits > BUFTYPE_BITS) {
		const uint8 kPower = (uint8)(numBits - BUFTYPE_BITS);

		for (; numSamples > 0; numSamples -= numChannels) {
			for (uint i = 0; i < numChannels; ++i)
				*bufDestination++ = static_cast<SampleType>(*(inChannels[i]++) >> kPower) ;
		}
	} else {
		for (; numSamples > 0; numSamples -= numChannels) {
			for (uint i = 0; i < numChannels; ++i)
				*bufDestination++ = static_cast<SampleType>(*(inChannels[i]++));
		}
	}

	assert(numSamples == 0); // dint copy too many samples
}

inline ::FLAC__StreamDecoderWriteStatus FlacInputStream::callbackWrite(const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[]) {
    assert(frame->header.channels == _streaminfo.channels);
	assert(frame->header.sample_rate == _streaminfo.sample_rate);
	assert(frame->header.bits_per_sample == _streaminfo.bits_per_sample);
	assert(frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER || _streaminfo.min_blocksize == _streaminfo.max_blocksize);

	// We require that either the sample cache is empty, or that no samples were requested
	assert(_sampleCache.bufFill == 0 || _requestedSamples == 0);

	uint numSamples = frame->header.blocksize;
	const uint numChannels = getChannels();
	const uint8 numBits = (uint8)_streaminfo.bits_per_sample;

	assert(_requestedSamples % numChannels == 0); // must be integral multiply of channels

	const FLAC__uint64 firstSampleNumber = (frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER) ?
		frame->header.number.sample_number : (static_cast<FLAC__uint64>(frame->header.number.frame_number)) * _streaminfo.max_blocksize;

	// Check whether we are about to reach beyond the last sample we are supposed to play.
	if (_lastSample != 0 && firstSampleNumber + numSamples >= _lastSample) {
		numSamples = (uint)(firstSampleNumber >= _lastSample ? 0 : _lastSample - firstSampleNumber);
		_lastSampleWritten = true;
	}

	// The value in _requestedSamples counts raw samples, so if there are more than one
	// channel, we have to multiply the number of available sample "pairs" by numChannels
	numSamples *= numChannels;

	const FLAC__int32 *inChannels[MAX_OUTPUT_CHANNELS];
	for (uint i = 0; i < numChannels; ++i)
		inChannels[i] = buffer[i];

	// write the incoming samples directly into the buffer provided to us by the mixer
	if (_requestedSamples > 0) {
		assert(_requestedSamples % numChannels == 0);
		assert(_outBuffer != NULL);

		// Copy & convert the available samples (limited both by how many we have available, and
		// by how many are actually needed).
		const uint copySamples = MIN(_requestedSamples, numSamples);
		(*_methodConvertBuffers)(_outBuffer, inChannels, copySamples, numChannels, numBits);

		_requestedSamples -= copySamples;
		numSamples -= copySamples;
		_outBuffer += copySamples;
	}

	// Write all remaining samples (i.e. those which didn't fit into the mixer buffer)
	// into the sample cache.
	if (_sampleCache.bufFill == 0)
		_sampleCache.bufReadPos = _sampleCache.bufData;
	const uint cacheSpace = (_sampleCache.bufData + BUFFER_SIZE) - (_sampleCache.bufReadPos + _sampleCache.bufFill);
	assert(numSamples <= cacheSpace);
	(*_methodConvertBuffers)(_sampleCache.bufReadPos + _sampleCache.bufFill, inChannels, numSamples, numChannels, numBits);

	_sampleCache.bufFill += numSamples;

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

inline ::FLAC__SeekableStreamDecoderSeekStatus FlacInputStream::callbackSeek(FLAC__uint64 absoluteByteOffset) {
	_inStream->seek(absoluteByteOffset, SEEK_SET);
	const bool result = (absoluteByteOffset == _inStream->pos());

#ifdef LEGACY_FLAC
	return result ? FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_OK : FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_ERROR;
#else
	return result ? FLAC__STREAM_DECODER_SEEK_STATUS_OK : FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
#endif
}

inline ::FLAC__SeekableStreamDecoderTellStatus FlacInputStream::callbackTell(FLAC__uint64 *absoluteByteOffset) {
	*absoluteByteOffset = static_cast<FLAC__uint64>(_inStream->pos());
#ifdef LEGACY_FLAC
	return FLAC__SEEKABLE_STREAM_DECODER_TELL_STATUS_OK;
#else
	return FLAC__STREAM_DECODER_TELL_STATUS_OK;
#endif
}

inline ::FLAC__SeekableStreamDecoderLengthStatus FlacInputStream::callbackLength(FLAC__uint64 *streamLength) {
	*streamLength = static_cast<FLAC__uint64>(_inStream->size());
#ifdef LEGACY_FLAC
	return FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_OK;
#else
	return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
#endif
}

inline bool FlacInputStream::callbackEOF() {
	return _inStream->eos();
}


inline void FlacInputStream::callbackMetadata(const ::FLAC__StreamMetadata *metadata) {
	assert(_decoder != NULL);
	assert(metadata->type == FLAC__METADATA_TYPE_STREAMINFO); // others arent really interesting

	_streaminfo = metadata->data.stream_info;
	setBestConvertBufferMethod(); // should be set after getting stream-information. FLAC always parses the info first
}
inline void FlacInputStream::callbackError(::FLAC__StreamDecoderErrorStatus status) {
	// some of these are non-critical-Errors
	debug(1, "FlacInputStream: An error occured while decoding. DecoderState is: %s",
			FLAC__StreamDecoderErrorStatusString[status]);
}

/* Static Callback Wrappers */
::FLAC__SeekableStreamDecoderReadStatus FlacInputStream::callWrapRead(const ::FLAC__SeekableStreamDecoder *decoder, FLAC__byte buffer[], FLAC_size_t *bytes, void *clientData) {
	FlacInputStream *instance = (FlacInputStream *)clientData;
	assert(0 != instance);
	return instance->callbackRead(buffer, bytes);
}

::FLAC__SeekableStreamDecoderSeekStatus FlacInputStream::callWrapSeek(const ::FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 absoluteByteOffset, void *clientData) {
	FlacInputStream *instance = (FlacInputStream *)clientData;
	assert(0 != instance);
	return instance->callbackSeek(absoluteByteOffset);
}

::FLAC__SeekableStreamDecoderTellStatus FlacInputStream::callWrapTell(const ::FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *absoluteByteOffset, void *clientData) {
	FlacInputStream *instance = (FlacInputStream *)clientData;
	assert(0 != instance);
	return instance->callbackTell(absoluteByteOffset);
}

::FLAC__SeekableStreamDecoderLengthStatus FlacInputStream::callWrapLength(const ::FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *streamLength, void *clientData) {
	FlacInputStream *instance = (FlacInputStream *)clientData;
	assert(0 != instance);
	return instance->callbackLength(streamLength);
}

FLAC__bool FlacInputStream::callWrapEOF(const ::FLAC__SeekableStreamDecoder *decoder, void *clientData) {
	FlacInputStream *instance = (FlacInputStream *)clientData;
	assert(0 != instance);
	return instance->callbackEOF();
}

::FLAC__StreamDecoderWriteStatus FlacInputStream::callWrapWrite(const ::FLAC__SeekableStreamDecoder *decoder, const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *clientData) {
	FlacInputStream *instance = (FlacInputStream *)clientData;
	assert(0 != instance);
	return instance->callbackWrite(frame, buffer);
}

void FlacInputStream::callWrapMetadata(const ::FLAC__SeekableStreamDecoder *decoder, const ::FLAC__StreamMetadata *metadata, void *clientData) {
	FlacInputStream *instance = (FlacInputStream *)clientData;
	assert(0 != instance);
	instance->callbackMetadata(metadata);
}

void FlacInputStream::callWrapError(const ::FLAC__SeekableStreamDecoder *decoder, ::FLAC__StreamDecoderErrorStatus status, void *clientData) {
	FlacInputStream *instance = (FlacInputStream *)clientData;
	assert(0 != instance);
	instance->callbackError(status);
}


#pragma mark -
#pragma mark --- Flac factory functions ---
#pragma mark -


AudioStream *makeFlacStream(File *file, uint32 size) {
	assert(file);

	// FIXME: For now, just read the whole data into memory, and be done
	// with it. Of course this is in general *not* a nice thing to do...

	// If no size was specified, read the whole remainder of the file
	if (!size)
		size = file->size() - file->pos();

	// Read 'size' bytes of data into a MemoryReadStream
	Common::MemoryReadStream *stream = file->readStream(size);

	// .. and create an MP3InputStream from all this
	FlacInputStream *input = new FlacInputStream(stream, true);
	if (!input->isStreamDecoderReady()) {
		delete input;
		return 0;
	}
	return input;
}

AudioStream *makeFlacStream(
	Common::SeekableReadStream *stream,
	bool disposeAfterUse,
	uint32 startTime,
	uint32 duration,
	uint numLoops) {

	uint32 endTime = duration ? (startTime + duration) : 0;

	FlacInputStream *input = new FlacInputStream(stream, disposeAfterUse, startTime, endTime, numLoops);
	if (!input->isStreamDecoderReady()) {
		delete input;
		return 0;
	}
	return input;
}

} // End of namespace Audio

#endif // #ifdef USE_FLAC
