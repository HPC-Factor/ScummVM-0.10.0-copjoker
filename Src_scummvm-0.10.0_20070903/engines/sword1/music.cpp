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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/sword1/music.cpp $
 * $Id: music.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/util.h"
#include "common/system.h"

#include "sword1/music.h"
#include "sound/aiff.h"
#include "sound/mixer.h"
#include "sound/mp3.h"
#include "sound/vorbis.h"
#include "sound/wave.h"

#define SMP_BUFSIZE 8192

namespace Sword1 {

class BaseAudioStream : public Audio::AudioStream {
public:
	BaseAudioStream(Common::SeekableReadStream *source, bool loop);
	virtual ~BaseAudioStream();
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo() const { return _isStereo; }
	virtual bool endOfData() const { return (_samplesLeft == 0); }
	virtual int getRate() const { return _rate; }
protected:
	Common::SeekableReadStream	*_sourceStream;
	uint8	*_sampleBuf;
	uint32	_rate;
	bool	_isStereo;
	uint32	_samplesLeft;
	uint16	_bitsPerSample;
	bool	_loop;

	virtual void rewind() = 0;
	virtual void reinit(int size, int rate, byte flags);
};

BaseAudioStream::BaseAudioStream(Common::SeekableReadStream *source, bool loop) {
	_sourceStream = source;
	_sampleBuf = (uint8*)malloc(SMP_BUFSIZE);

	_samplesLeft = 0;
	_isStereo = false;
	_bitsPerSample = 16;
	_rate = 22050;
	_loop = loop;
}

BaseAudioStream::~BaseAudioStream() {
	free(_sampleBuf);
}

void BaseAudioStream::reinit(int size, int rate, byte flags) {
	_isStereo = (flags & Audio::Mixer::FLAG_STEREO) != 0;
	_rate = rate;
	assert((uint)size <= (_sourceStream->size() - _sourceStream->pos()));
	_bitsPerSample = ((flags & Audio::Mixer::FLAG_16BITS) != 0) ? 16 : 8;
	_samplesLeft = (size * 8) / _bitsPerSample;
	if ((_bitsPerSample != 16) && (_bitsPerSample != 8))
		error("BaseAudioStream: unknown sound type");
}

int BaseAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	int retVal = 0;

	while (retVal < numSamples && _samplesLeft > 0) {
		int samples = MIN((int)_samplesLeft, numSamples - retVal);
		retVal += samples;
		_samplesLeft -= samples;
		while (samples > 0) {
			int readBytes = MIN(samples * (_bitsPerSample >> 3), SMP_BUFSIZE);
			_sourceStream->read(_sampleBuf, readBytes);
			if (_bitsPerSample == 16) {
				samples -= (readBytes / 2);
				memcpy(buffer, _sampleBuf, readBytes);
				buffer += (readBytes / 2);
			} else {
				samples -= readBytes;
				int8 *src = (int8*)_sampleBuf;
				while (readBytes--)
					*buffer++ = (int16)*src++ << 8;
			}
		}
		
		if (!_samplesLeft && _loop) {
			rewind();
		}
	}
	
	return retVal;
}

class WaveAudioStream : public BaseAudioStream {
public:
	WaveAudioStream(Common::SeekableReadStream *source, bool loop);
	virtual int readBuffer(int16 *buffer, const int numSamples);
private:
	virtual void rewind();
};

WaveAudioStream::WaveAudioStream(Common::SeekableReadStream *source, bool loop) : BaseAudioStream(source, loop) {
	rewind();

	if (_samplesLeft == 0)
		_loop = false;
}

void WaveAudioStream::rewind() {
	int rate, size;
	byte flags;

	_sourceStream->seek(0);

	if (Audio::loadWAVFromStream(*_sourceStream, size, rate, flags)) {
		reinit(size, rate, flags);
	}
}

int WaveAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	int retVal = BaseAudioStream::readBuffer(buffer, numSamples);

	if (_bitsPerSample == 16) {
		for (int i = 0; i < retVal; i++) {
			buffer[i] = (int16)READ_LE_UINT16(buffer + i);
		}
	}

	return retVal;
}

class AiffAudioStream : public BaseAudioStream {
public:
	AiffAudioStream(Common::SeekableReadStream *source, bool loop);
	virtual int readBuffer(int16 *buffer, const int numSamples);
private:
	void rewind();
};

AiffAudioStream::AiffAudioStream(Common::SeekableReadStream *source, bool loop) : BaseAudioStream(source, loop) {
	rewind();

	if (_samplesLeft == 0)
		_loop = false;
}

void AiffAudioStream::rewind() {
	int rate, size;
	byte flags;
	
	_sourceStream->seek(0);

	if (Audio::loadAIFFFromStream(*_sourceStream, size, rate, flags)) {
		reinit(size, rate, flags);
	}
}

int AiffAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	int retVal = BaseAudioStream::readBuffer(buffer, numSamples);

	if (_bitsPerSample == 16) {
		for (int i = 0; i < retVal; i++) {
			buffer[i] = (int16)READ_BE_UINT16(buffer + i);
		}
	}
	
	return retVal;
}

// This means fading takes 3 seconds.
#define FADE_LENGTH 3

// These functions are only called from Music, so I'm just going to
// assume that if locking is needed it has already been taken care of.

bool MusicHandle::play(const char *fileBase, bool loop) {
	char fileName[30];
	stop();
	
#ifdef USE_VORBIS
	if (!_audioSource) {
		sprintf(fileName, "%s.ogg", fileBase);
		if (_file.open(fileName))
			_audioSource = Audio::makeVorbisStream(&_file, false, 0, 0, loop ? 0 : 1);
	}
#endif
#ifdef USE_MAD
	if (!_audioSource) {
		sprintf(fileName, "%s.mp3", fileBase);
		if (_file.open(fileName))
			_audioSource = Audio::makeMP3Stream(&_file, false, 0, 0, loop ? 0 : 1);
	}
#endif
	if (!_audioSource) {
		sprintf(fileName, "%s.wav", fileBase);
		if (_file.open(fileName))
			_audioSource = new WaveAudioStream(&_file, loop);
	}

	if (!_audioSource) {
		sprintf(fileName, "%s.aif", fileBase);
		if (_file.open(fileName))
			_audioSource = new AiffAudioStream(&_file, loop);
	}
	
	if (!_audioSource)
		return false;

	fadeUp();
	return true;
}

void MusicHandle::fadeDown() {
	if (streaming()) {
		if (_fading < 0)
			_fading = -_fading;
		else if (_fading == 0)
			_fading = FADE_LENGTH * getRate();
		_fadeSamples = FADE_LENGTH * getRate();
	}
}

void MusicHandle::fadeUp() {
	if (streaming()) {
		if (_fading > 0)
			_fading = -_fading;
		else if (_fading == 0)
			_fading = -1;
		_fadeSamples = FADE_LENGTH * getRate();
	}
}

bool MusicHandle::endOfData() const {
	return !streaming();
}

// is we don't have an audiosource, return some dummy values.
// shouldn't happen anyways.
bool MusicHandle::streaming(void) const {
	return (_audioSource) ? (!_audioSource->endOfStream()) : false;
}

bool MusicHandle::isStereo(void) const {
	return (_audioSource) ? _audioSource->isStereo() : false;
}

int MusicHandle::getRate(void) const {
	return (_audioSource) ? _audioSource->getRate() : 11025;
}

int MusicHandle::readBuffer(int16 *buffer, const int numSamples) {
	int totalSamples = 0;
	int16 *bufStart = buffer;
	if (!_audioSource)
		return 0;
	int expectedSamples = numSamples;
	while ((expectedSamples > 0) && _audioSource) { // _audioSource becomes NULL if we reach EOF and aren't looping
		int samplesReturned = _audioSource->readBuffer(buffer, expectedSamples);
		buffer += samplesReturned;
		totalSamples += samplesReturned;
		expectedSamples -= samplesReturned;
		if ((expectedSamples > 0) && _audioSource->endOfData()) {
			debug(2, "Music reached EOF");
			stop();
		}
	}
	// buffer was filled, now do the fading (if necessary)
	int samplePos = 0;
	while ((_fading > 0) && (samplePos < totalSamples)) { // fade down
		--_fading;
		bufStart[samplePos] = (bufStart[samplePos] * _fading) / _fadeSamples;
		samplePos++;
		if (_fading == 0) {
			stop();
			// clear the rest of the buffer
			memset(bufStart + samplePos, 0, (totalSamples - samplePos) * 2);
			return samplePos;
		}
	}
	while ((_fading < 0) && (samplePos < totalSamples)) { // fade up
		bufStart[samplePos] = -(bufStart[samplePos] * --_fading) / _fadeSamples;
		if (_fading <= -_fadeSamples)
			_fading = 0;
	}
	return totalSamples;
}

void MusicHandle::stop() {
	if (_audioSource) {
		delete _audioSource;
		_audioSource = NULL;
	}
	if (_file.isOpen())
		_file.close();
	_fading = 0;
}

Music::Music(Audio::Mixer *pMixer) {
	_mixer = pMixer;
	_sampleRate = pMixer->getOutputRate();
	_converter[0] = NULL;
	_converter[1] = NULL;
	_volumeL = _volumeR = 192;
	_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, false, true);
}

Music::~Music() {
	_mixer->stopHandle(_soundHandle);
	delete _converter[0];
	delete _converter[1];
}

void Music::mixer(int16 *buf, uint32 len) {
	Common::StackLock lock(_mutex);
	memset(buf, 0, 2 * len * sizeof(int16));
	for (int i = 0; i < ARRAYSIZE(_handles); i++)
		if (_handles[i].streaming() && _converter[i])
			_converter[i]->flow(_handles[i], buf, len, _volumeL, _volumeR);
}

void Music::setVolume(uint8 volL, uint8 volR) {
	_volumeL = (Audio::st_volume_t)volL;
	_volumeR = (Audio::st_volume_t)volR;
}

void Music::giveVolume(uint8 *volL, uint8 *volR) {
	*volL = (uint8)_volumeL;
	*volR = (uint8)_volumeR;
}

void Music::startMusic(int32 tuneId, int32 loopFlag) {
	if (strlen(_tuneList[tuneId]) > 0) {
		int newStream = 0;
		_mutex.lock();
		if (_handles[0].streaming() && _handles[1].streaming()) {
			int streamToStop;
			// Both streams playing - one must be forced to stop.
			if (!_handles[0].fading() && !_handles[1].fading()) {
				// None of them are fading. Shouldn't happen,
				// so it doesn't matter which one we pick.
				streamToStop = 0;
			} else if (_handles[0].fading() && !_handles[1].fading()) {
				// Stream 0 is fading, so pick that one.
				streamToStop = 0;
			} else if (!_handles[0].fading() && _handles[1].fading()) {
				// Stream 1 is fading, so pick that one.
				streamToStop = 1;
			} else {
				// Both streams are fading. Pick the one that
				// is closest to silent.
				if (ABS(_handles[0].fading()) < ABS(_handles[1].fading()))
					streamToStop = 0;
				else
					streamToStop = 1;
			}
			_handles[streamToStop].stop();
		}
		if (_handles[0].streaming()) {
			_handles[0].fadeDown();
			newStream = 1;
		} else if (_handles[1].streaming()) {
			_handles[1].fadeDown();
			newStream = 0;
		}
		delete _converter[newStream];
		_converter[newStream] = NULL;
		_mutex.unlock();

		/* The handle will load the music file now. It can take a while, so unlock
		   the mutex before, to have the soundthread playing normally.
		   As the corresponding _converter is NULL, the handle will be ignored by the playing thread */
		if (_handles[newStream].play(_tuneList[tuneId], loopFlag != 0)) {
			_mutex.lock();
			_converter[newStream] = Audio::makeRateConverter(_handles[newStream].getRate(), _mixer->getOutputRate(), _handles[newStream].isStereo(), false);
			_mutex.unlock();
		} else {
			if (tuneId != 81) // file 81 was apparently removed from BS.
				warning("Can't find music file %s", _tuneList[tuneId]);
		}
	} else {
		_mutex.lock();
		if (_handles[0].streaming())
			_handles[0].fadeDown();
		if (_handles[1].streaming())
			_handles[1].fadeDown();
		_mutex.unlock();
	}
}

void Music::fadeDown() {
	Common::StackLock lock(_mutex);
	for (int i = 0; i < ARRAYSIZE(_handles); i++)
		if (_handles[i].streaming())
			_handles[i].fadeDown();
}

} // End of namespace Sword1
