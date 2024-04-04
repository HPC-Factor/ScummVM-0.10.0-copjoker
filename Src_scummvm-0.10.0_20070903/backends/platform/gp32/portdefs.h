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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/platform/gp32/portdefs.h $
 * $Id: portdefs.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef PORTDEFS_H
#define PORTDEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <cctype>

#include "gp32std.h"
#include "gp32std_file.h"
#include "gp32std_memory.h"

#undef byte

#undef malloc
#undef calloc
#undef free

#undef stderr
#undef stdout
#undef stdin

#undef fopen
#undef fread
#undef fwrite
#undef fclose
#undef ftell
#undef rewind
#undef fseek

#undef ferror
#undef clearerr 

#undef feof	
#undef getc

#define FILE GPFILE

#define stderr	gp_stderr
#define stdout	gp_stdout
#define stdin	gp_stdin

#define fopen	gp_fopen
#define fread	gp_fread
#define fwrite	gp_fwrite
#define fclose	gp_fclose
#define fseek	gp_fseek
#define ftell	gp_ftell
#define ferror	gp_ferror
#define clearerr gp_clearerr

#define feof	gp_feof
#define fgets	gp_fgets
#define fgetc	gp_fgetc
#define getc	gp_fgetc

#define fprintf	gp_fprintf
#define fflush	gp_fflush

#define printf	gp_printf

#define exit	gp_exit

#define strcpy	gp_strcpy
#define strncpy	gp_strncpy
#define strcat	gp_strcat
#define strdup	gp_strdup
#define sprintf	gp_sprintf
#define strcasecmp	gp_strcasecmp
#define strncasecmp gp_strncasecmp

#define memcpy	gp_memcpy
#define memset	gp_memset
#define malloc	gp_malloc
#define calloc	gp_calloc //gm_calloc
#define free	gp_free

#define FUNCTION GPDEBUG("FUNC: %s", __FUNCTION__);

#define DELAY gp_delay(500);

#define MARK printf("MARK: %s, %s, %d", __FILE__, __FUNCTION__, __LINE__);

#define assert(e) ((e) ? 0 : (printf("!AS: " #e " (%s, %d)\n", __FILE__, __LINE__)))
#define ASSERT assert

#define ENDLESSLOOP while (1)

#endif /* PORTDEFS_H */
