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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/kyra/resource.h $
 * $Id: resource.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef KYRA_RESOURCE_H
#define KYRA_RESOURCE_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/file.h"
#include "common/list.h"

#include "kyra/kyra.h"

namespace Kyra {

class ResourceFile {
public:
	ResourceFile() : _open(false), _protected(false), _filename() {}
	virtual ~ResourceFile() {}

	virtual uint8 *getFile(uint file) const = 0;
	virtual bool getFileHandle(uint file, Common::File &filehandle) const = 0;
	virtual uint32 getFileSize(uint file) const = 0;

	uint filename() const { return _filename; }

	virtual bool isValid(void) const { return (_filename != 0); }
	bool isOpen(void) const { return _open; }

	virtual void close() { if (!_protected) _open = false; }
	virtual void protect(const bool prot = true) { _protected = prot; }
	virtual void open() { _open = true; }
protected:
	bool _open;
	bool _protected;
	uint _filename;
};

// standard Package format for Kyrandia games
class PAKFile : public ResourceFile {
	struct PakChunk {
		uint _name;
		uint32 _start;
		uint32 _size;
		
		operator uint() const { return _name; }
	};

public:
	PAKFile(const char *file, const char *physfile, Common::File &pakfile, bool isAmiga = false);
	~PAKFile();

	uint8 *getFile(uint file) const;
	bool getFileHandle(uint file, Common::File &filehandle) const;
	uint32 getFileSize(uint file) const;
private:
	bool openFile(Common::File &filehandle) const;

	Common::String _physfile;
	uint32 _physOffset;

	typedef Common::List<PakChunk>::iterator PakIterator;
	typedef Common::List<PakChunk>::const_iterator ConstPakIterator;
	Common::List<PakChunk> _files; // the entries
};

// installation file packages for (Kyra2/)Kyra3
class INSFile : public ResourceFile {
	struct FileEntry {
		uint _name;
		uint32 _start;
		uint32 _size;
		
		operator uint() const { return _name; }
	};
public:
	INSFile(const char *file);
	~INSFile();

	uint8 *getFile(uint file) const;
	bool getFileHandle(uint file, Common::File &filehandle) const;
	uint32 getFileSize(uint file) const;
protected:
	typedef Common::List<FileEntry>::iterator FileIterator;
	typedef Common::List<FileEntry>::const_iterator ConstFileIterator;
	Common::List<FileEntry> _files; // the entries

	Common::String _physfile;
};

class Resource {
public:
	Resource(KyraEngine *vm);
	~Resource();
	
	bool loadPakFile(const Common::String &filename);
	void unloadPakFile(const Common::String &filename);
	bool isInPakList(const Common::String &filename) const;

	uint32 getFileSize(const char *file) const;
	uint8* fileData(const char *file, uint32 *size) const;
	// it gives back a file handle (used for the speech player)
	// it could be that the needed file is embedded in the returned
	// handle
	bool getFileHandle(const char *file, uint32 *size, Common::File &filehandle);

	bool loadFileToBuf(const char *file, void *buf, uint32 maxSize); 

protected:
	typedef Common::List<ResourceFile*>::iterator ResIterator;
	typedef Common::List<ResourceFile*>::const_iterator ConstResIterator;

	KyraEngine *_vm;
	Common::List<ResourceFile*> _pakfiles;
};

// TODO?: maybe prefix all things here with 'kKyra1' instead of 'k'
enum kKyraResources {
	kLoadAll = -1,

	kForestSeq,
	kKallakWritingSeq,
	kKyrandiaLogoSeq,
	kKallakMalcolmSeq,
	kMalcolmTreeSeq,
	kWestwoodLogoSeq,

	kDemo1Seq,
	kDemo2Seq,
	kDemo3Seq,
	kDemo4Seq,

	kAmuleteAnimSeq,

	kOutroReunionSeq,

	kIntroCPSStrings,
	kIntroCOLStrings,
	kIntroWSAStrings,
	kIntroStrings,

	kOutroHomeString,

	kRoomFilenames,
	kRoomList,

	kCharacterImageFilenames,
	
	kItemNames,
	kTakenStrings,
	kPlacedStrings,
	kDroppedStrings,
	kNoDropStrings,

	kPutDownString,
	kWaitAmuletString,
	kBlackJewelString,
	kPoisonGoneString,
	kHealingTipString,
	kWispJewelStrings,
	kMagicJewelStrings,

	kThePoisonStrings,
	kFluteStrings,

	kFlaskFullString,
	kFullFlaskString,

	kVeryCleverString,
	kNewGameString,
	
	kDefaultShapes,
	kHealing1Shapes,
	kHealing2Shapes,
	kPoisonDeathShapes,
	kFluteShapes,
	kWinter1Shapes,
	kWinter2Shapes,
	kWinter3Shapes,
	kDrinkShapes,
	kWispShapes,
	kMagicAnimShapes,
	kBranStoneShapes,

	kPaletteList,

	kGUIStrings,
	kConfigStrings,
	
	kKyra1TownsSFXTable,
	kCreditsStrings,
	
	kMaxResIDs
};

struct Shape;
struct Room;

class StaticResource {
public:
	StaticResource(KyraEngine *vm) : _vm(vm), _resList(), _fileLoader(0), _builtIn(0), _filenameTable(0) {}
	~StaticResource() { deinit(); }
	
	static bool checkKyraDat();

	bool init();
	void deinit();

	const char * const*loadStrings(int id, int &strings);
	const uint8 *loadRawData(int id, int &size);
	const Shape *loadShapeTable(int id, int &entries);
	const Room *loadRoomTable(int id, int &entries);
	const uint8 * const*loadPaletteTable(int id, int &entries);

	// use '-1' to prefetch/unload all ids
	// prefetchId retruns false if only on of the resources
	// can't be loaded and it breaks then the first res
	// can't be loaded
	bool prefetchId(int id);
	void unloadId(int id);
private:
	KyraEngine *_vm;

	struct FilenameTable;
	struct ResData;
	struct FileType;

	bool checkResList(int id, int &type, const void *&ptr, int &size);
	const void *checkForBuiltin(int id, int &type, int &size);
	const FilenameTable *searchFile(int id);
	const FileType *getFiletype(int type);
	const void *getData(int id, int requesttype, int &size);

	bool loadLanguageTable(const char *filename, void *&ptr, int &size);
	bool loadStringTable(const char *filename, void *&ptr, int &size);
	bool loadRawData(const char *filename, void *&ptr, int &size);
	bool loadShapeTable(const char *filename, void *&ptr, int &size);
	bool loadRoomTable(const char *filename, void *&ptr, int &size);
	bool loadPaletteTable(const char *filename, void *&ptr, int &size);
	
	void freeRawData(void *&ptr, int &size);
	void freeStringTable(void *&ptr, int &size);
	void freeShapeTable(void *&ptr, int &size);
	void freeRoomTable(void *&ptr, int &size);
	void freePaletteTable(void *&ptr, int &size);

	uint8 *getFile(const char *name, int &size);

	enum kResTypes {
		kLanguageList,
		kStringList,
		kRoomList,
		kShapeList,
		kRawData,
		kPaletteTable
	};

	struct BuiltinRes {
		int id;
		int type;
		int size;
		const void *data;
	};

	struct FilenameTable {
		int id;
		int type;
		const char *filename;
	};

	struct FileType {
		int type;
		typedef bool (StaticResource::*LoadFunc)(const char *filename, void *&ptr, int &size);
		typedef void (StaticResource::*FreeFunc)(void *&ptr, int &size);

		LoadFunc load;
		FreeFunc free;
	};

	struct ResData {
		int id;
		int type;
		int size;
		void *data;
	};

	Common::List<ResData> _resList;

	const FileType *_fileLoader;
	const BuiltinRes *_builtIn;
	const FilenameTable *_filenameTable;
};

} // end of namespace Kyra

#endif

