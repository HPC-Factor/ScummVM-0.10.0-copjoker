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
 * MorphOS-specific header file
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/platform/morphos/morphos.h $
 * $Id: morphos.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef MORPHOS_MORPHOS_H
#define MORPHOS_MORPHOS_H

#include <dos/dosextens.h>
#include <graphics/regions.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <libraries/cdda.h>
#include <proto/exec.h>

#include "backends/intern.h"
#include "morphos_scaler.h"

class OSystem_MorphOS : public OSystem
{
	public:
		OSystem_MorphOS(SCALERTYPE gfx_mode, bool full_screen);
		virtual ~OSystem_MorphOS();

		bool Initialise();

		// Set colors of the palette
		virtual void setPalette(const byte *colors, uint start, uint num);

		// Set the size of the video bitmap.
		// Typically, 320x200
		virtual void initSize(uint w, uint h);

		// Draw a bitmap to screen.
		// The screen will not be updated to reflect the new bitmap
		virtual void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);

		// Update the dirty areas of the screen
		virtual void updateScreen();

		// Either show or hide the mouse cursor
		virtual bool showMouse(bool visible);

		// Set the position of the mouse cursor
		virtual void set_mouse_pos(int x, int y);

		// Set the bitmap that's used when drawing the cursor.
		virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, byte keycolor);

		// Shaking is used in SCUMM. Set current shake position.
		virtual void setShakePos(int shake_pos);

		// Overlay
		virtual void showOverlay();
		virtual void hideOverlay();
		virtual void clearOverlay();
		virtual void grabOverlay(int16 *buf, int pitch);
		virtual void copyRectToOverlay(const int16 *buf, int pitch, int x, int y, int w, int h);
		virtual int16 getHeight();
		virtual int16 getWidth();

		// Get the number of milliseconds since the program was started.
		virtual uint32 getMillis();

		// Delay for a specified amount of milliseconds
		virtual void delayMillis(uint msecs);

		// Add a new callback timer
		virtual void setTimerCallback(TimerProc callback, int timer);

		// Mutex handling
		virtual MutexRef createMutex();
		virtual void lockMutex(MutexRef mutex);
		virtual void unlockMutex(MutexRef mutex);
		virtual void deleteMutex(MutexRef mutex);

		// Get the next event.
		// Returns true if an event was retrieved.
		virtual bool pollEvent(Common::Event &event);

		// Moves mouse pointer to specified position
		virtual void warpMouse(int x, int y);

		// Set the function to be invoked whenever samples need to be generated
		virtual bool setSoundCallback(SoundProc proc, void *param);
				  void fill_sound    (byte * stream, int len);
				  void clearSoundCallback();

		virtual uint32 property(int param, Property *value);

		// Poll cdrom status
		// Returns true if cd audio is playing
		virtual bool pollCD();

		// Play cdrom audio track
		virtual void playCD(int track, int num_loops, int start_frame, int duration);

		// Stop cdrom audio track
		virtual void stopCD();

		// Update cdrom audio status
		virtual void updateCD();

		// Quit
		virtual void quit();

		static OSystem_MorphOS *create(SCALERTYPE gfx_scaler, bool full_screen);

		static bool OpenATimer(MsgPort **port, IORequest **req, ULONG unit, bool required = true);

	private:
		typedef enum { CSDSPTYPE_WINDOWED, CSDSPTYPE_FULLSCREEN, CSDSPTYPE_TOGGLE, CSDSPTYPE_KEEP } CS_DSPTYPE;

		static const int MAX_MOUSE_W = 80;
		static const int MAX_MOUSE_H = 80;

		void   CreateScreen(CS_DSPTYPE dspType);
		void 	 SwitchScalerTo(SCALERTYPE newScaler);
		bool   AddUpdateRect(WORD x, WORD y, WORD w, WORD h);

		void   DrawMouse();
		void   UndrawMouse();

		/* Display-related attributes */
		Screen  	    *ScummScreen;
		Window  	    *ScummWindow;
		char 			  ScummWndTitle[125];
		APTR          ScummBuffer;
		LONG		     ScummBufferWidth;
		LONG			  ScummBufferHeight;
		ScreenBuffer *ScummScreenBuffer[2];
		BitMap  	    *ScummRenderTo;
		ULONG			  ScummPaintBuffer;
		int			  ScummScrWidth;
		int 			  ScummScrHeight;
		int			  ScummDepth;
		bool 			  Scumm16ColFmt16;
		UWORD 		 *ScummNoCursor;
		ULONG 		  ScummColors[256];
		USHORT 		  ScummColors16[256];
		WORD			  ScummWinX;
		WORD			  ScummWinY;
		bool			  ScummDefaultMouse;
		bool			  ScummOrigMouse;
		int 			  ScummShakePos;
		bool			  FullScreenMode;
		bool 			  ScreenChanged;
		UWORD			**BlockColors;
		bool			 *DirtyBlocks;
		Region 		 *UpdateRegion;
		Region 		 *NewUpdateRegion;
		ULONG			  UpdateRects;
		SignalSemaphore CritSec;

		/* Overlay-related attributes */
		APTR 		 OvlBitMap;
		APTR 		 OvlSavedBuffer;
		ColorMap *OvlCMap;

		/* Sound-related attributes */
		Process 	 *ScummSoundThread;
		SoundProc SoundProc;
		void      *SoundParam;
		MsgPort	 *ThreadPort;
		Message 	  MusicStartup;
		Message 	  SoundStartup;

		/* CD-ROM related attributes */
		CDRIVEPTR CDrive;
		ULONG 	 CDDATrackOffset;

		/* Scaling-related attributes */
		SCALERTYPE ScummScaler;
		int  		  ScummScale;
		MorphOSScaler *Scaler;

		/* Mouse cursor-related attributes */
		bool  MouseVisible, MouseDrawn;
		int   MouseX, MouseY;
		int   MouseWidth, MouseHeight;
		int   MouseOldX, MouseOldY;
		int   MouseOldWidth, MouseOldHeight;
		int   MouseHotspotX, MouseHotspotY;
		byte *MouseImage, MouseBackup[MAX_MOUSE_W*MAX_MOUSE_H];
		byte  MouseKeycolor;
		MsgPort* InputMsgPort;
		IOStdReq*InputIORequest;

		/* Timer-related attributes */
		MsgPort 	   *TimerMsgPort;
		timerequest *TimerIORequest;

		/* Game-related attributes */
		int   GameID;
};

class AutoLock
{
	public:
		AutoLock(SignalSemaphore* s) : sem(s) { ObtainSemaphore(sem); }
		~AutoLock() { ReleaseSemaphore(sem); }

	private:
		SignalSemaphore* sem;
};

#define AUTO_LOCK	 AutoLock cs(&CritSec);


extern OSystem_MorphOS *TheSystem;

#endif

