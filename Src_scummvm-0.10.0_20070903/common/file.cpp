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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/common/file.cpp $
 * $Id: file.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/file.h"
#include "common/fs.h"
#include "common/hashmap.h"
#include "common/util.h"
#include "common/hash-str.h"

#ifdef MACOSX
#include "CoreFoundation/CoreFoundation.h"
#endif

#ifdef __PLAYSTATION2__
	// for those replaced fopen/fread/etc functions
	typedef unsigned long	uint64;
	typedef signed long	int64;
	#include "backends/platform/ps2/fileio.h"

	#define fopen(a, b)			ps2_fopen(a, b)
	#define fclose(a)			ps2_fclose(a)
	#define fseek(a, b, c)			ps2_fseek(a, b, c)
	#define ftell(a)			ps2_ftell(a)
	#define feof(a)				ps2_feof(a)
	#define fread(a, b, c, d)		ps2_fread(a, b, c, d)
	#define fwrite(a, b, c, d)		ps2_fwrite(a, b, c, d)

	//#define fprintf				ps2_fprintf	// used in common/util.cpp
	//#define fflush(a)			ps2_fflush(a)	// used in common/util.cpp

	//#define fgetc(a)			ps2_fgetc(a)	// not used
	//#define fgets(a, b, c)			ps2_fgets(a, b, c)	// not used
	//#define fputc(a, b)			ps2_fputc(a, b)	// not used
	//#define fputs(a, b)			ps2_fputs(a, b)	// not used

	//#define fsize(a)			ps2_fsize(a)	// not used -- and it is not a standard function either
#endif

#ifdef __DS__

	// These functions replease the standard library functions of the same name.
	// As this header is included after the standard one, I have the chance to #define
	// all of these to my own code.
	//
	// A #define is the only way, as redefinig the functions would cause linker errors.
	
	// These functions need to be #undef'ed, as their original definition 
	// in devkitarm is done with #includes (ugh!)
	#undef feof
	#undef clearerr
	//#undef getc
	//#undef ferror
	

	//void 	std_fprintf(FILE* handle, const char* fmt, ...);	// used in common/util.cpp
	//void 	std_fflush(FILE* handle);	// used in common/util.cpp

	//char* 	std_fgets(char* str, int size, FILE* file);	// not used
	//int 	std_getc(FILE* handle);	// not used
	//char* 	std_getcwd(char* dir, int dunno);	// not used
	//void 	std_cwd(char* dir);	// not used
	//int 	std_ferror(FILE* handle);	// not used
	
	// Only functions used in the ScummVM source have been defined here!
	#define fopen(name, mode) 					DS::std_fopen(name, mode)
	#define fclose(handle) 						DS::std_fclose(handle)
	#define fread(ptr, size, items, file)		DS::std_fread(ptr, size, items, file)
	#define fwrite(ptr, size, items, file)		DS::std_fwrite(ptr, size, items, file)
	#define feof(handle)						DS::std_feof(handle)
	#define ftell(handle)						DS::std_ftell(handle)
	#define fseek(handle, offset, whence)		DS::std_fseek(handle, offset, whence)
	#define clearerr(handle)					DS::std_clearerr(handle)

	//#define printf(fmt, ...)					consolePrintf(fmt, ##__VA_ARGS__)

	//#define fprintf(file, fmt, ...)				{ char str[128]; sprintf(str, fmt, ##__VA_ARGS__); DS::std_fwrite(str, strlen(str), 1, file); }
	//#define fflush(file)						DS::std_fflush(file)	// used in common/util.cpp

	//#define fgets(str, size, file)				DS::std_fgets(str, size, file)	// not used
	//#define getc(handle)						DS::std_getc(handle)	// not used
	//#define getcwd(dir, dunno)					DS::std_getcwd(dir, dunno)	// not used
	//#define ferror(handle)						DS::std_ferror(handle)	// not used

#endif

#ifdef __SYMBIAN32__
	#undef feof
	#undef clearerr
	
	#define FILE void
	
	FILE* 	symbian_fopen(const char* name, const char* mode);
	void 	symbian_fclose(FILE* handle);
	size_t 	symbian_fread(const void* ptr, size_t size, size_t numItems, FILE* handle);
	size_t 	symbian_fwrite(const void* ptr, size_t size, size_t numItems, FILE* handle);
	bool 	symbian_feof(FILE* handle);
	long int symbian_ftell(FILE* handle);
	int 	symbian_fseek(FILE* handle, long int offset, int whence);
	void 	symbian_clearerr(FILE* handle);

	// Only functions used in the ScummVM source have been defined here!
	#define fopen(name, mode) 					symbian_fopen(name, mode)
	#define fclose(handle) 						symbian_fclose(handle)
	#define fread(ptr, size, items, file)		symbian_fread(ptr, size, items, file)
	#define fwrite(ptr, size, items, file)		symbian_fwrite(ptr, size, items, file)
	#define feof(handle)						symbian_feof(handle)
	#define ftell(handle)						symbian_ftell(handle)
	#define fseek(handle, offset, whence)		symbian_fseek(handle, offset, whence)
	#define clearerr(handle)					symbian_clearerr(handle)
#endif

namespace Common {

typedef HashMap<String, int, CaseSensitiveString_Hash, CaseSensitiveString_EqualTo> StringIntMap;

// The following two objects could be turned into static members of class
// File. However, then we would be forced to #include hashmap in file.h
// which seems to be a high price just for a simple beautification...
static StringIntMap *_defaultDirectories;
static StringMap *_filesMap;

static FILE *fopenNoCase(const String &filename, const String &directory, const char *mode) {
	FILE *file;
	String buf(directory);
	uint i;

#if !defined(__GP32__) && !defined(PALMOS_MODE)
	// Add a trailing slash, if necessary.
	if (!buf.empty()) {
		const char c = buf.lastChar();
		if (c != ':' && c != '/' && c != '\\')
			buf += '/';
	}
#endif

	// Append the filename to the path string
	const int offsetToFileName = buf.size();
	buf += filename;

	//
	// Try to open the file normally
	//
	file = fopen(buf.c_str(), mode);

	//
	// Try again, with file name converted to upper case
	//
	if (!file) {
		for (i = offsetToFileName; i < buf.size(); ++i) {
			buf[i] = toupper(buf[i]);
		}
		file = fopen(buf.c_str(), mode);
	}

	//
	// Try again, with file name converted to lower case
	//
	if (!file) {
		for (i = offsetToFileName; i < buf.size(); ++i) {
			buf[i] = tolower(buf[i]);
		}
		file = fopen(buf.c_str(), mode);
	}

	//
	// Try again, with file name capitalized
	//
	if (!file) {
		i = offsetToFileName;
		buf[i] = toupper(buf[i]);
		file = fopen(buf.c_str(), mode);
	}

#ifdef __amigaos4__
	//
	// Work around for possibility that someone uses AmigaOS "newlib" build with SmartFileSystem (blocksize 512 bytes), leading
	// to buffer size being only 512 bytes. "Clib2" sets the buffer size to 8KB, resulting smooth movie playback. This forces the buffer
	// to be enough also when using "newlib" compile on SFS.
	//
	if (file) {
		setvbuf(file, NULL, _IOFBF, 8192);
	}
#endif

	return file;
}

void File::addDefaultDirectory(const String &directory) {
	FilesystemNode dir(directory);
	addDefaultDirectoryRecursive(dir, 1);
}

void File::addDefaultDirectoryRecursive(const String &directory, int level, const String &prefix) {
	FilesystemNode dir(directory);
	addDefaultDirectoryRecursive(dir, level, prefix);
}

void File::addDefaultDirectory(const FilesystemNode &directory) {
	addDefaultDirectoryRecursive(directory, 1);
}

void File::addDefaultDirectoryRecursive(const FilesystemNode &dir, int level, const String &prefix) {
	if (level <= 0)
		return;

	FSList fslist;
	if (!dir.listDir(fslist, FilesystemNode::kListAll)) {
		// Failed listing the contents of this node, so it is either not a 
		// directory, or just doesn't exist at all.
		return;
	}

	if (!_defaultDirectories)
		_defaultDirectories = new StringIntMap;

	// Do not add directories multiple times, unless this time they are added
	// with a bigger depth.
	const String &directory(dir.path());
	if (_defaultDirectories->contains(directory) && (*_defaultDirectories)[directory] >= level)
		return;
	(*_defaultDirectories)[directory] = level;

	if (!_filesMap)
		_filesMap = new StringMap;

	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory()) {
			addDefaultDirectoryRecursive(file->path(), level - 1, prefix + file->name() + "/");
		} else {
			String lfn(prefix);
			lfn += file->name();
			lfn.toLowercase();
			if (!_filesMap->contains(lfn)) {
				(*_filesMap)[lfn] = file->path();
			}
		}
	}
}

void File::resetDefaultDirectories() {
	delete _defaultDirectories;
	delete _filesMap;
	
	_defaultDirectories = 0;
	_filesMap = 0;
}

File::File()
	: _handle(0), _ioFailed(false) {
}

//#define DEBUG_FILE_REFCOUNT

File::~File() {
#ifdef DEBUG_FILE_REFCOUNT
	warning("File::~File on file '%s'", _name.c_str());
#endif
	close();
}


bool File::open(const String &filename, AccessMode mode) {
	assert(mode == kFileReadMode || mode == kFileWriteMode);

	if (filename.empty()) {
		error("File::open: No filename was specified");
	}

	if (_handle) {
		error("File::open: This file object already is opened (%s), won't open '%s'", _name.c_str(), filename.c_str());
	}

	_name.clear();
	clearIOFailed();

	String fname(filename);
	fname.toLowercase();

	const char *modeStr = (mode == kFileReadMode) ? "rb" : "wb";
	if (mode == kFileWriteMode) {
		_handle = fopenNoCase(filename, "", modeStr);
	} else if (_filesMap && _filesMap->contains(fname)) {
		fname = (*_filesMap)[fname];
		debug(3, "Opening hashed: %s", fname.c_str());
		_handle = fopen(fname.c_str(), modeStr);
	} else if (_filesMap && _filesMap->contains(fname + ".")) {
		// WORKAROUND: Bug #1458388: "SIMON1: Game Detection fails"
		// sometimes instead of "GAMEPC" we get "GAMEPC." (note trailing dot)
		fname = (*_filesMap)[fname + "."];
		debug(3, "Opening hashed: %s", fname.c_str());
		_handle = fopen(fname.c_str(), modeStr);
	} else {

		if (_defaultDirectories) {
			// Try all default directories
			StringIntMap::const_iterator x(_defaultDirectories->begin());
			for (; _handle == NULL && x != _defaultDirectories->end(); ++x) {
				_handle = fopenNoCase(filename, x->_key, modeStr);
			}
		}

		// Last resort: try the current directory
		if (_handle == NULL)
			_handle = fopenNoCase(filename, "", modeStr);

		// Last last (really) resort: try looking inside the application bundle on Mac OS X for the lowercase file.
#ifdef MACOSX
		if (!_handle) {
			CFStringRef cfFileName = CFStringCreateWithBytes(NULL, (const UInt8 *)filename.c_str(), filename.size(), kCFStringEncodingASCII, false);
			CFURLRef fileUrl = CFBundleCopyResourceURL(CFBundleGetMainBundle(), cfFileName, NULL, NULL);
			if (fileUrl) {
				UInt8 buf[256];
				if (CFURLGetFileSystemRepresentation(fileUrl, false, (UInt8 *)buf, 256)) {
					_handle = fopen((char *)buf, modeStr);
				}
				CFRelease(fileUrl);
			}
			CFRelease(cfFileName);
		}
#endif

	}

	if (_handle == NULL) {
		if (mode == kFileReadMode)
			debug(2, "File %s not found", filename.c_str());
		else
			debug(2, "File %s not opened", filename.c_str());
		return false;
	}


	_name = filename;

#ifdef DEBUG_FILE_REFCOUNT
	warning("File::open on file '%s'", _name.c_str());
#endif

	return true;
}

bool File::open(const FilesystemNode &node, AccessMode mode) {
	assert(mode == kFileReadMode || mode == kFileWriteMode);

	if (!node.isValid()) {
		warning("File::open: Trying to open an invalid FilesystemNode object");
		return false;
	} else if (node.isDirectory()) {
		warning("File::open: Trying to open a FilesystemNode which is a directory");
		return false;
	}

	String filename(node.name());

	if (_handle) {
		error("File::open: This file object already is opened (%s), won't open '%s'", _name.c_str(), filename.c_str());
	}

	clearIOFailed();
	_name.clear();

	const char *modeStr = (mode == kFileReadMode) ? "rb" : "wb";

	_handle = fopen(node.path().c_str(), modeStr);

	if (_handle == NULL) {
		if (mode == kFileReadMode)
			debug(2, "File %s not found", filename.c_str());
		else
			debug(2, "File %s not opened", filename.c_str());
		return false;
	}

	_name = filename;

#ifdef DEBUG_FILE_REFCOUNT
	warning("File::open on file '%s'", _name.c_str());
#endif

	return true;
}

bool File::exists(const String &filename) {
	// First try to find the file it via a FilesystemNode (in case an absolute
	// path was passed). But we only use this to filter out directories.
	FilesystemNode file(filename);
	// FIXME: can't use isValid() here since at the time of writing
	// FilesystemNode is to be unable to find for example files
	// added in extrapath
	if (file.isDirectory())
		return false;

	// Next, try to locate the file by *opening* it in read mode. This has
	// multiple effects:
	// 1) It takes _filesMap and _defaultDirectories into consideration -> good
	// 2) It returns true if and only if File::open is possible on the file -> good
	// 3) If this method is misused, it could lead to an fopen call on a directory
	//    -> bad!
	// 4) It also checks whether we can read the file. This is not 100%
	//    desirable; after all, even when we can't read it, the file is present.
	//    Since this method is often used to check whether a file should be
	//    re-created, that's not nice.
	//
	// TODO/FIXME: We should clarify the semantics of this method, and then
	// maybe should introduce several new methods:
	//   fileExistsAndReadable
	//   fileExists
	//   fileExistsAtPath
	//   dirExists
	//   dirExistsAtPath
	// or maybe only 1-2 methods which take some params :-).
	
	File tmp;
	return tmp.open(filename, kFileReadMode);
}

void File::close() {
	if (_handle)
		fclose((FILE *)_handle);
	_handle = NULL;
}

bool File::isOpen() const {
	return _handle != NULL;
}

bool File::ioFailed() const {
	return _ioFailed != 0;
}

void File::clearIOFailed() {
	_ioFailed = false;
}

bool File::eof() const {
	if (_handle == NULL) {
		error("File::eof: File is not open!");
		return false;
	}

	return feof((FILE *)_handle) != 0;
}

uint32 File::pos() const {
	if (_handle == NULL) {
		error("File::pos: File is not open!");
		return 0;
	}

	return ftell((FILE *)_handle);
}

uint32 File::size() const {
	if (_handle == NULL) {
		error("File::size: File is not open!");
		return 0;
	}

	uint32 oldPos = ftell((FILE *)_handle);
	fseek((FILE *)_handle, 0, SEEK_END);
	uint32 length = ftell((FILE *)_handle);
	fseek((FILE *)_handle, oldPos, SEEK_SET);

	return length;
}

void File::seek(int32 offs, int whence) {
	if (_handle == NULL) {
		error("File::seek: File is not open!");
		return;
	}

	if (fseek((FILE *)_handle, offs, whence) != 0)
		clearerr((FILE *)_handle);
}

uint32 File::read(void *ptr, uint32 len) {
	byte *ptr2 = (byte *)ptr;
	uint32 real_len;

	if (_handle == NULL) {
		error("File::read: File is not open!");
		return 0;
	}

	if (len == 0)
		return 0;

	real_len = fread(ptr2, 1, len, (FILE *)_handle);
	if (real_len < len) {
		_ioFailed = true;
	}

	return real_len;
}

uint32 File::write(const void *ptr, uint32 len) {
	if (_handle == NULL) {
		error("File::write: File is not open!");
		return 0;
	}

	if (len == 0)
		return 0;

	if ((uint32)fwrite(ptr, 1, len, (FILE *)_handle) != len) {
		_ioFailed = true;
	}

	return len;
}

}	// End of namespace Common
