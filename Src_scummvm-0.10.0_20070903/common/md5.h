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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/common/md5.h $
 * $Id: md5.h 27024 2007-05-30 21:56:52Z fingolfin $
 */

#ifndef COMMON_MD5_H
#define COMMON_MD5_H

#include "common/scummsys.h"
#include "common/fs.h"
#include "common/stream.h"

namespace Common {

bool md5_file(const char *name, uint8 digest[16], uint32 length = 0);
bool md5_file(const FilesystemNode &file, uint8 digest[16], uint32 length = 0);
bool md5_file(ReadStream &stream, uint8 digest[16], uint32 length = 0);

// The following two methods work similar to the above two, but 
// instead of computing the binary MD5 digest, they produce
// a human readable lowercase hexstring representing the digest.
// The md5str parameter must point to a buffer of 32+1 chars.
bool md5_file_string(const char *name, char *md5str, uint32 length = 0);
bool md5_file_string(const FilesystemNode &file, char *md5str, uint32 length = 0);
bool md5_file_string(ReadStream &stream, char *md5str, uint32 length = 0);


} // End of namespace Common

#endif
