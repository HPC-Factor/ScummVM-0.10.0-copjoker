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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/platform/ps2/irxboot.cpp $
 * $Id: irxboot.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include <kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <malloc.h>
#include "backends/platform/ps2/irxboot.h"

extern void sioprintf(const char *zFormat, ...);

static const char hddArg[] = "-o" "\0" "4" "\0" "-n" "\0" "20";
static const char pfsArg[] = "-m" "\0" "2" "\0" "-o" "\0" "16" "\0" "-n" "\0" "40" /*"\0" "-debug"*/;

IrxFile irxFiles[] = {
	{ "SIO2MAN", BIOS, NOTHING, NULL, 0 },
	{ "MCMAN",   BIOS, NOTHING, NULL, 0 },
	{ "MCSERV",  BIOS, NOTHING, NULL, 0 },
	{ "PADMAN",  BIOS, NOTHING, NULL, 0 },
	{ "LIBSD",   BIOS, NOTHING, NULL, 0 },

	{ "IOMANX.IRX",   SYSTEM | NOT_HOST, NOTHING, NULL, 0 }, // already loaded by ps2link
	{ "FILEXIO.IRX",  SYSTEM, NOTHING, NULL, 0 },
	{ "CODYVDFS.IRX", SYSTEM, NOTHING, NULL, 0 },
	{ "SJPCM.IRX",    SYSTEM, NOTHING, NULL, 0 },

	{ "USBD.IRX",     USB | OPTIONAL | DEPENDANCY, USB_DRIVER, NULL, 0 },
	{ "PS2MOUSE.IRX", USB | OPTIONAL, MOUSE_DRIVER, NULL, 0 },
	{ "RPCKBD.IRX",   USB | OPTIONAL, KBD_DRIVER, NULL, 0 },
	{ "USB_MASS.IRX", USB | OPTIONAL, MASS_DRIVER, NULL, 0 },

	{ "PS2DEV9.IRX",  HDD | OPTIONAL | DEPENDANCY, HDD_DRIVER, NULL, 0 },
	{ "PS2ATAD.IRX",  HDD | OPTIONAL | DEPENDANCY, HDD_DRIVER, NULL, 0 },
	{ "PS2HDD.IRX",   HDD | OPTIONAL | DEPENDANCY, HDD_DRIVER, hddArg, sizeof(hddArg) },
	{ "PS2FS.IRX",    HDD | OPTIONAL | DEPENDANCY, HDD_DRIVER, pfsArg, sizeof(pfsArg) },
	{ "POWEROFF.IRX", HDD | OPTIONAL | DEPENDANCY, HDD_DRIVER, NULL, 0 }
};

static const int numIrxFiles = sizeof(irxFiles) / sizeof(irxFiles[0]);

BootDevice detectBootPath(const char *elfPath, char *bootPath) {

	BootDevice device;

	if (strncasecmp(elfPath, "cdrom0:", 7) == 0)
		device = CDROM;
	else if (strncasecmp(elfPath, "host", 4) == 0)
		device = HOST;
	else
		device = OTHER;

	sioprintf("elf path: %s, device %d", elfPath, device);
	
	strcpy(bootPath, elfPath);

	char *pathPos = bootPath;
	char seperator;

	if (device == CDROM) {
		// CDVD uses '\' as seperator
		while (*pathPos) {
			if (*pathPos == '/')
				*pathPos = '\\';
			pathPos++;
		}
		seperator = '\\';
	} else {
		// all the other devices use '/'
		while (*pathPos) {
			if (*pathPos == '\\')
				*pathPos = '/';
			pathPos++;
		}
		seperator = '/';
	}
	pathPos = strrchr(bootPath, seperator);
	if (!pathPos)
		pathPos = strchr(bootPath, ':');

	if (pathPos) {
		if ((pathPos[0] == ':') && (device == CDROM)) {
			pathPos[1] = '\\';
			pathPos[2] = '\0';
		} else
			pathPos[1] = '\0';
		sioprintf("done. IRX path: \"%s\"", bootPath);
	} else {
		sioprintf("path not recognized, default to host.");
		strcpy(bootPath, "host:");
		device = UNKNOWN;
	}
	return device;
}

int loadIrxModules(int device, const char *irxPath, IrxReference **modules) {	

	IrxReference *resModules = (IrxReference *)malloc(numIrxFiles * sizeof(IrxReference));
	IrxReference *curModule = resModules;

	for (int i = 0; i < numIrxFiles; i++) {
		curModule->fileRef = irxFiles + i;
		if ((device == HOST) && (irxFiles[i].flags & NOT_HOST))
			continue;

		if ((irxFiles[i].flags & TYPEMASK) == BIOS) {
			curModule->loc = IRX_FILE;
			curModule->path = (char *)malloc(32);
			sprintf(curModule->path, "rom0:%s", irxFiles[i].name);
			curModule->buffer = NULL;
			curModule->size = 0;
			curModule->argSize = 0;
			curModule->args = NULL;
			curModule->errorCode = 0;
		} else {
			curModule->loc = IRX_BUFFER;
			curModule->path = (char *)malloc(256);

			sprintf(curModule->path, "%s%s%s", irxPath, irxFiles[i].name, (device == CDROM) ? ";1" : "");
			int fd = fioOpen(curModule->path, O_RDONLY);
			if (fd < 0) {
				// IRX not found
				sioprintf("Can't open %s: %d", curModule->path, fd);
				// we keep the error code of the path where we originally expected the file
				curModule->errorCode = fd;

				// try cdrom root directory
				sprintf(curModule->path, "cdrom0:\\%s;1", irxFiles[i].name);
				fd = fioOpen(curModule->path, O_RDONLY);
				if (fd < 0) {
					// still not found, try host:
					sioprintf("Can't open %s: %d", curModule->path, fd);
					sprintf(curModule->path, "host:%s", irxFiles[i].name);
					fd = fioOpen(curModule->path, O_RDONLY);
					if (fd < 0) {
						// we simply can't find it.
						sioprintf("Can't open %s: %d", curModule->path, fd);
						// restore the path where we originally expected the file, for error message (later, after boot up)
						sprintf(curModule->path, "%s%s%s", irxPath, irxFiles[i].name, (device == CDROM) ? ";1" : "");
					}
				}
			}

			if (fd >= 0) {
				curModule->size = fioLseek(fd, 0, SEEK_END);
				fioLseek(fd, 0, SEEK_SET);
				curModule->buffer = (uint8 *)memalign(64, (curModule->size + 63) & ~63);
				fioRead(fd, curModule->buffer, curModule->size);

				curModule->argSize = irxFiles[i].argSize;
				curModule->args = irxFiles[i].args;
				curModule->errorCode = 0;
				fioClose(fd);
			} else {
				if (irxFiles[i].flags & DEPENDANCY) {
					// other modules depend on this one.
					// kill the modules we already loaded, if they depend on the one that failed.
					IrxReference *pos = resModules;
					while (pos < curModule) {
						if ((pos->fileRef->flags & TYPEMASK) == (irxFiles[i].flags & TYPEMASK)) {
							if (pos->path)
								free(pos->path);
							if (pos->buffer)
								free(pos->buffer);

							IrxReference *copyPos = pos;
							while (copyPos < curModule) {
								copyPos[0] = copyPos[1];
								copyPos++;
							}
							curModule--;
						} else
							pos++;
					}
					// and skip any remaining modules that depend on the missing one, too.
					while ((i < numIrxFiles - 1) && ((irxFiles[i + 1].flags & TYPEMASK) == (curModule->fileRef->flags & TYPEMASK)))
						i++;
					// the module that actually failed (curModule) is kept in the array for displaying an error message
				}
				curModule->size = 0;
				curModule->buffer = NULL;
				curModule->argSize = 0;
				curModule->args = NULL;
			}
		}
		curModule++;
	}
	*modules = resModules;
	sioprintf("List of %d modules:", curModule - resModules);
	for (int i = 0; i < curModule - resModules; i++)
		sioprintf("%s", resModules[i].path);
	return curModule - resModules;
}

