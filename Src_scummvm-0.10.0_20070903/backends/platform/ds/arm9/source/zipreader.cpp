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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */


#include "stdafx.h"
#include "zipreader.h"

#define SWAP_U16(v) (((v & 0xFF) << 8) + (v & 0xFF00))
#define SWAP_U32(v) ((SWAP_U16((v & 0XFFFF0000) >> 16) + (SWAP_U16(v & 0x0000FFFF) << 16)))


ZipFile::ZipFile() {
	// Locate a zip file in cartridge memory space
	
	consolePrintf("ZIP file check...");
	
	char* p = (char *) ZF_SEARCH_START;
	bool found = false;
	
	_zipFile = NULL;
	
	while ((p != (char *) ZF_SEARCH_END) && (!found)) {
		// Zip file header is: 0x504B0304
		
		if ( (*p == 0x50) && (*(p + 1) == 0x4B) && (*(p + 2) == 0x03) && (*(p + 3) == 0x04) ) {
			// Found header!
			found = true;
			_zipFile = p;
		}
	
		if (!found) p += ZF_SEARCH_STRIDE;
		
	}

	if (_zipFile) {
		consolePrintf("Ok!\n");
	} else {
		consolePrintf("Not in use!\n");
		return;
	}
	
	changeToRoot();
	restartFile();
	
	if (_currentFile->compSize != (u32) getFileSize()) {
		consolePrintf("Error: ZIP file contains compression!\n");
	}
	
	_allFilesVisible = false;
}

bool ZipFile::isReady() {
	return _zipFile != NULL;
}

bool ZipFile::restartFile() {
	_currentFile = (FileHeader *) _zipFile;
	char name[128];
	getFileName(name);

	bool more = true;

	while (!currentFileInFolder() && more) {
		char name[128];
		getFileName(name);
		more = skipFile();
	}
	
	return more;
}

bool ZipFile::currentFileInFolder() {
	char name[128];
	
	if (_allFilesVisible) return true;
	
	getFileName(name);

//	consolePrintf("N:'%s'D:'%s'\n", name, _directory);
	
	if (_directory[0] == 0) { // Root directory
		name[strlen(name) - 1] = 0;
		return !strchr(name, '\\'); 	// Not in root if contains a / character before the last character
	} else {
/*		if (name starts with directory && it's not the directory
			&& (no slashes after the directory || it's the last character)
			&& (slash follows directory)
	*/
		if ((strstr(name, _directory) == name) && (strlen(name) != strlen(_directory))	
			&& ((strchr(name + strlen(_directory) + 1, '\\') == NULL)
			|| (strchr(name + strlen(_directory) + 1, '\\') == name + strlen(name) - 1))
			&& (*(name + strlen(_directory)) == '\\')) {
			return true;
		}
	}
	
	return false;
}

void ZipFile::getFileName(char* name) {
	strncpy(name, (char *) (_currentFile + 1), _currentFile->nameLength);
	
	for (int r = 0; r < (int) strlen(name); r++) {
		if (name[r] == '/') name[r] = '\\';
	}

	name[_currentFile->nameLength] = 0;
	
	if (name[strlen(name) - 1] == '\\') {
		name[strlen(name) - 1] = 0;
	}	
}

bool ZipFile::skipFile() {
	bool valid;

	do {
	
		// Move on to the next file
		_currentFile = (FileHeader *) (
			((char *) (_currentFile)) + sizeof(*_currentFile) + _currentFile->nameLength + _currentFile->fileSize + _currentFile->extraLength
		);
		
			// Return true if there are more files.  Check this by looking for the magic number
		valid =  (_currentFile->magic[0] == 0x50) &&
				(_currentFile->magic[1] == 0x4B) &&
				(_currentFile->magic[2] == 0x03) &&
				(_currentFile->magic[3] == 0x04);
		
			
	} while (valid && !currentFileInFolder());
	
	return valid;
	
	// Currently doesn't handle data descriptors!
}



u32 ZipFile::misaligned32(u32* v) {
	char* b = (char *) v;
	return (b[0] << 24) + (b[1] << 16) + (b[2] << 8) + b[3];
}

u16 ZipFile::misaligned16(u16* v) {
	char* b = (char *) v;
	return (b[0] << 8) + b[1];
}

int ZipFile::getFileSize() {
	return _currentFile->fileSize;
}

bool ZipFile::isDirectory() {
	return _currentFile->fileSize == 0; 		// This is a bit wrong, but seems to work.
}

char* ZipFile::getFile() {
	return ((char *) (_currentFile)) + sizeof(*_currentFile) + _currentFile->nameLength + _currentFile->extraLength;
}

bool ZipFile::findFile(char* search) {
	changeToRoot();
	restartFile();
	
	char searchName[128];
	strcpy(searchName, search);
	for (int r = 0; r < (int) strlen(searchName); r++) {
		if (searchName[r] == '/') searchName[r] = '\\';
	}

	if (*(searchName + strlen(searchName) - 1) == '\\') {	// Directories have a terminating slash
		*(searchName + strlen(searchName) - 1) = '\0';		// which we need to dispose of.
	}

	
	do {
		char name[128];
		getFileName(name);
		if (*(name + strlen(name) - 1) == '\\') {	// Directories have a terminating slash
			*(name + strlen(name) - 1) = '\0';		// which we need to dispose of.
		}
		
		
		if (!stricmp(name, searchName)) {
//			consolePrintf("'%s'=='%s'\n", name, searchName);
			return true;		// Got it!
		} else {
//			consolePrintf("'%s'!='%s'\n", name, searchName);
		}
	} while (skipFile());

	return false;		// File wasn't found
}

void ZipFile::changeToRoot() {
	_directory[0] = 0;
}

void ZipFile::changeDirectory(char* dir) {
//	consolePrintf("Current dir now '%s'\n", dir);

	strcpy(_directory, dir);
	for (int r = 0; r < (int) strlen(_directory); r++) {
		if (_directory[r] == '/') _directory[r] = '\\';
	}
	
	if (_directory[strlen(_directory) - 1] == '\\')	{
		_directory[strlen(_directory) - 1] = '\0';
	}
}

ZipFile::~ZipFile() {

}
