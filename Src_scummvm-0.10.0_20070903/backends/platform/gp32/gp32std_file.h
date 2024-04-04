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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/platform/gp32/gp32std_file.h $
 * $Id: gp32std_file.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef __GP32STD_FILE_H
#define __GP32STD_FILE_H

#define FCACHE_SIZE 8 * 1024

typedef struct {
	F_HANDLE handle;
	unsigned long mode;
	unsigned long fileSize;
	unsigned long filePos;
	unsigned long physFilePos;
	unsigned long cachePos;
	unsigned long cacheBufOffs;
	unsigned long bytesInCache;
	unsigned char cacheData[FCACHE_SIZE];
} GPFILE;

extern GPFILE *gp_stderr;
extern GPFILE *gp_stdout;
extern GPFILE *gp_stdin;

extern GPFILE *	gp_fopen(const char *filename, const char *mode);
extern int		gp_fclose(GPFILE *stream);
extern int		gp_fseek(GPFILE *stream, long offset, int whence);
extern size_t	gp_fread(void *ptr, size_t size, size_t n, GPFILE *stream);
extern size_t	gp_fwrite(const void *ptr, size_t size, size_t n, GPFILE *stream);
extern long		gp_ftell(GPFILE *stream);
extern void		gp_clearerr(GPFILE *stream);
extern int		gp_feof(GPFILE *stream);
extern char		gp_fgetc(GPFILE *stream);
extern char *	gp_fgets(char *s, int n, GPFILE *stream);
extern int		gp_fflush(GPFILE *stream);
extern int		gp_ferror(GPFILE *stream);
extern int		gp_fprintf(GPFILE *stream, const char *fmt, ...);

#endif
