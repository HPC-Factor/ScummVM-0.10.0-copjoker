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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/platform/dc/dc.h $
 * $Id: dc.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include <common/system.h>
#include <ronin/soundcommon.h>
#include "backends/timer/default/default-timer.h"

#define NUM_BUFFERS 4
#define SOUND_BUFFER_SHIFT 3

class Interactive
{
 public:
  virtual int key(int k, byte &shiftFlags) = 0;
  virtual void mouse(int x, int y) = 0;
};

#include "softkbd.h"

class OSystem_Dreamcast : public OSystem {

 public:
  OSystem_Dreamcast();

  virtual void initBackend();

  // Determine whether the backend supports the specified feature.
  bool hasFeature(Feature f);

  // En-/disable the specified feature.
  void setFeatureState(Feature f, bool enable);

  // Query the state of the specified feature.
  bool getFeatureState(Feature f);

  // Retrieve a list of all graphics modes supported by this backend.
  const GraphicsMode *getSupportedGraphicsModes() const;

  // Return the ID of the 'default' graphics mode.
  int getDefaultGraphicsMode() const;

  // Switch to the specified graphics mode.
  bool setGraphicsMode(int mode);

  // Determine which graphics mode is currently active.
  int getGraphicsMode() const;

  // Set colors of the palette
  void setPalette(const byte *colors, uint start, uint num);
  void grabPalette(byte *colors, uint start, uint num);

  // Set the size of the video bitmap.
  // Typically, 320x200
  void initSize(uint w, uint h);
  int16 getHeight() { return _screen_h; }
  int16 getWidth() { return _screen_w; }

  // Draw a bitmap to screen.
  // The screen will not be updated to reflect the new bitmap
  void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);

  // Copies the current screen contents to a new surface.
  bool grabRawScreen(Graphics::Surface *surf);

  // Clear the screen to black.
  void clearScreen();

  // Update the dirty areas of the screen
  void updateScreen();

  // Either show or hide the mouse cursor
  bool showMouse(bool visible);

  // Move ("warp") the mouse cursor to the specified position.
  void warpMouse(int x, int y);

  // Set the bitmap that's used when drawing the cursor.
  void setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, byte keycolor, int cursorTargetScale);

  // Replace the specified range of cursor the palette with new colors.
  void setCursorPalette(const byte *colors, uint start, uint num);

  // Disable or enable cursor palette.
  void disableCursorPalette(bool disable);

  // Shaking is used in SCUMM. Set current shake position.
  void setShakePos(int shake_pos);

  // Get the number of milliseconds since the program was started.
  uint32 getMillis();

  // Delay for a specified amount of milliseconds
  void delayMillis(uint msecs);

  // Get the next event.
  // Returns true if an event was retrieved.
  bool pollEvent(Common::Event &event);

  // Determine the output sample rate. Audio data provided by the sound
  // callback will be played using this rate.
  int getOutputSampleRate() const;

  // Initialise the specified CD drive for audio playback.
  bool openCD(int drive);

  // Poll cdrom status
  // Returns true if cd audio is playing
  bool pollCD();

  // Play cdrom audio track
  void playCD(int track, int num_loops, int start_frame, int duration);

  // Stop cdrom audio track
  void stopCD();

  // Update cdrom audio status
  void updateCD();

  // Quit
  void quit();

  // Overlay
  int16 getOverlayHeight();
  int16 getOverlayWidth();
  int screenToOverlayX(int x);
  int screenToOverlayY(int y);
  int overlayToScreenX(int x);
  int overlayToScreenY(int y);
  void showOverlay();
  void hideOverlay();
  void clearOverlay();
  void grabOverlay(int16 *buf, int pitch);
  void copyRectToOverlay(const int16 *buf, int pitch, int x, int y, int w, int h);
  OverlayColor RGBToColor(uint8 r, uint8 g, uint8 b) { return ARGBToColor(255, r, g, b); }
  void colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b) {
    uint8 tmp; colorToARGB(color, tmp, r, g, b);
  }
  OverlayColor ARGBToColor(uint8 a, uint8 r, uint8 g, uint8 b) {
    return ((a&0xf0)<<8)|((r&0xf0)<<4)|(g&0xf0)|(b>>4);
  }
  void colorToARGB(OverlayColor color, uint8 &a, uint8 &r, uint8 &g, uint8 &b) {
    a = ((color>>8)&0xf0)|((color>>12)&0x0f);
    r = ((color>>4)&0xf0)|((color>>8)&0x0f);
    g = (color&0xf0)|((color>>4)&0x0f);
    b = ((color<<4)&0xf0)|(color&0x0f);
  }

  // Mutex handling
  MutexRef createMutex();
  void lockMutex(MutexRef mutex);
  void unlockMutex(MutexRef mutex);
  void deleteMutex(MutexRef mutex);

  // Set a window caption or any other comparable status display to the
  // given value.
  void setWindowCaption(const char *caption);

  // Modulatized backend
  Common::SaveFileManager *getSavefileManager() { return _savefile; }
  Audio::Mixer *getMixer() { return _mixer; }
  Common::TimerManager *getTimerManager() { return _timer; }

  // Extra SoftKbd support
  void mouseToSoftKbd(int x, int y, int &rx, int &ry) const;


 private:

  Common::SaveFileManager *_savefile;
  Audio::Mixer *_mixer;
  DefaultTimerManager *_timer;
  SoftKeyboard _softkbd;

  int _ms_cur_x, _ms_cur_y, _ms_cur_w, _ms_cur_h, _ms_old_x, _ms_old_y;
  int _ms_hotspot_x, _ms_hotspot_y, _ms_visible, _devpoll;
  int _current_shake_pos, _screen_w, _screen_h;
  int _overlay_x, _overlay_y;
  unsigned char *_ms_buf;
  unsigned char _ms_keycolor;
  bool _overlay_visible, _overlay_dirty, _screen_dirty;
  int _screen_buffer, _overlay_buffer, _mouse_buffer;
  bool _aspect_stretch, _softkbd_on, _enable_cursor_palette;
  float _overlay_fade, _xscale, _yscale, _top_offset;
  int _softkbd_motion;

  unsigned char *screen;
  unsigned short *mouse;
  unsigned short *overlay;
  void *screen_tx[NUM_BUFFERS];
  void *mouse_tx[NUM_BUFFERS];
  void *ovl_tx[NUM_BUFFERS];
  unsigned short palette[256], cursor_palette[256];

  int temp_sound_buffer[RING_BUFFER_SAMPLES>>SOUND_BUFFER_SHIFT];

  void checkSound();

  void drawMouse(int xdraw, int ydraw, int w, int h,
		 unsigned char *buf, bool visible);

  void setScaling();

  Common::SaveFileManager *createSavefileManager();
};


extern int handleInput(struct mapledev *pad,
		       int &mouse_x, int &mouse_y,
		       byte &shiftFlags, Interactive *inter = NULL);
extern void initSound();
extern bool selectGame(char *&, char *&, class Icon &);

