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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/common/system.h $
 * $Id: system.h 27069 2007-06-03 18:44:03Z knakos $
 *
 */

#ifndef COMMON_SYSTEM_H
#define COMMON_SYSTEM_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "common/noncopyable.h"
#include "common/rect.h"

namespace Audio {
	class Mixer;
}

namespace Graphics {
	struct Surface;
}

namespace Common {
	struct Event;
	class EventManager;
	class SaveFileManager;
	class TimerManager;
}

/**
 * Interface for ScummVM backends. If you want to port ScummVM to a system
 * which is not currently covered by any of our backends, this is the place
 * to start. ScummVM will create an instance of a subclass of this interface
 * and use it to interact with the system.
 *
 * In particular, a backend provides a video surface for ScummVM to draw in;
 * methods to create timers, to handle user input events,
 * control audio CD playback, and sound output.
 */
class OSystem : Common::NonCopyable {
protected:
	OSystem();
	virtual ~OSystem();

public:

	/**
	 * The following method is called once, from main.cpp, after all
	 * config data (including command line params etc.) are fully loaded.
	 *
	 * @note Subclasses should always invoke the implementation of their
	 *       parent class. They should do so near the end of their own
	 *       implementation.
	 */
	virtual void initBackend() { }

	/** @name Feature flags */
	//@{

	/**
	 * A feature in this context means an ability of the backend which can be
	 * either on or off. Examples include:
	 *  - fullscreen mode
	 *  - aspect ration correction
	 *  - a virtual keyboard for text entry (on PDAs)
	 */
	enum Feature {
		/**
		 * If your backend supports both a windowed and a fullscreen mode,
		 * then this feature flag can be used to switch between the two.
		 */
		kFeatureFullscreenMode,

		/**
		 * Control aspect ratio correction. Aspect ratio correction is used to
		 * correct games running at 320x200 (i.e with an aspect ratio of 8:5),
		 * but which on their original hardware were displayed with the
		 * standard 4:3 ratio (that is, the original graphics used non-square
		 * pixels). When the backend support this, then games running at
		 * 320x200 pixels should be scaled up to 320x240 pixels. For all other
		 * resolutions, ignore this feature flag.
		 * @note You can find utility functions in common/scaler.h which can
		 *       be used to implement aspect ratio correction. In particular,
		 *       stretch200To240() can stretch a rect, including (very fast)
		 *       interpolation, and works in-place.
		 */
		kFeatureAspectRatioCorrection,

		/**
		 * Determine whether a virtual keyboard is too be shown or not.
		 * This would mostly be implemented by backends for hand held devices,
		 * like PocketPC, Palms, Symbian phones like the P800, Zaurus, etc.
		 */
		kFeatureVirtualKeyboard,

		/**
		 * This flag is a bit more obscure: it gives a hint to the backend that
		 * the frontend code is very inefficient in doing screen updates. So
		 * the frontend might do a lot of fullscreen blits even though only a
		 * tiny portion of the actual screen data changed. In that case, it
		 * might pay off for the backend to compute which parts actual changed,
		 * and then only mark those as dirty.
		 * Implementing this is purely optional, and no harm should arise
		 * when not doing so (except for decreased speed in said frontends).
		 */
		kFeatureAutoComputeDirtyRects,

		/**
		 * This flag determines either cursor can have its own palette or not
		 * It is currently used only by some Macintosh versions of Humongous
		 * Entertainment games. If backend doesn't implement this feature then
		 * engine switches to b/w version of cursors.
		 */
		kFeatureCursorHasPalette,

		/**
		 * Set to true if the overlay pixel format has an alpha channel.
		 * This should only be set if it offers at least 3-4 bits of accuracy,
		 * as opposed to a single alpha bit.
		 */
		kFeatureOverlaySupportsAlpha,

		/**
		 * Set to true to iconify the window.
		 */
		kFeatureIconifyWindow,

		/**
		 * This feature, set to true, is a hint toward the backend to disable all
		 * key filtering/mapping, in cases where it would be beneficial to do so.
		 * As an example case, this is used in the agi engine's predictive dialog.
		 * When the dialog is displayed this feature is set so that backends with
		 * phone-like keypad temporarily unmap all user actions which leads to
		 * comfortable word entry. Conversely, when the dialog exits the feature
		 * is set to false.
		 * TODO: Fingolfin suggests that the way the feature is used can be 
		 * generalized in this sense: Have a keyboard mapping feature, which the
		 * engine queries for to assign keys to actions ("Here's my default key
		 * map for these actions, what do you want them set to?").
		 */
		kFeatureDisableKeyFiltering
	};

	/**
	 * Determine whether the backend supports the specified feature.
	 */
	virtual bool hasFeature(Feature f) { return false; }

	/**
	 * En-/disable the specified feature. For example, this may be used to
	 * enable fullscreen mode, or to deactivate aspect correction, etc.
	 */
	virtual void setFeatureState(Feature f, bool enable) {}

	/**
	 * Query the state of the specified feature. For example, test whether
	 * fullscreen mode is active or not.
	 */
	virtual bool getFeatureState(Feature f) { return false; }

	//@}



	/**
	 * @name Graphics
	 *
	 * The way graphics work in the class OSystem are meant to make
	 * it possible for game frontends to implement all they need in
	 * an efficient manner. The downside of this is that it may be
	 * rather complicated for backend authors to fully understand and
	 * implement the semantics of the OSystem interface.
	 *
	 *
	 * The graphics visible to the user in the end are actually
	 * composed in three layers: the game graphics, the overlay
	 * graphics, and the mouse.
	 *
	 * First, there are the game graphics. They are always 8bpp, and
	 * the methods in this section deal with them exclusively. In
	 * particular, the size of the game graphics is defined by a call
	 * to initSize(), and copyRectToScreen() blits 8bpp data into the
	 * game layer. Let W and H denote the width and height of the
	 * game graphics.
	 *
	 * Before the user sees these graphics, they may undergo certain
	 * transformations; for example, the may be scaled to better fit
	 * on the visible screen; or aspect ratio correction may be
	 * performed (see kFeatureAspectRatioCorrection). As a result of
	 * this, a pixel of the game graphics may occupy a region bigger
	 * than a single pixel on the screen. We define p_w and p_h to be
	 * the width resp. height of a game pixel on the screen.
	 *
	 * In addition, there is a vertical "shake offset" (as defined by
	 * setShakePos) which is used in some games to provide a shaking
	 * effect. Note that shaking is applied to all three layers, i.e.
	 * also to the overlay and the mouse. We denote the shake offset
	 * by S.
	 *
	 * Putting this together, a pixel (x,y) of the game graphics is
	 * transformed to a rectangle of height p_h and widht p_w
	 * appearing at position (p_w * x, p_hw * (y + S)) on the real
	 * screen (in addition, a backend may choose to offset
	 * everything, e.g. to center the graphics on the screen).
	 *
	 *
	 * The next layer is the overlay. It is composed over the game
	 * graphics. By default, it has exactly the same size and
	 * resolution as the game graphics. However, client code can
	 * specify an overlay scale (as an additional parameter to
	 * initSize()). This is meant to increase the resolution of the
	 * overlay while keeping its size the same as that of the game
	 * graphics. For example, if the overlay scale is 2, and the game
	 * graphics have a resolution of 320x200; then the overlay shall
	 * have a resolution of 640x400, but it still has the same
	 * physical size as the game graphics.
	 *
	 *
	 * Finally, there is the mouse layer. This layer doesn't have to
	 * actually exist within the backend -- it all depends on how a
	 * backend chooses to implement mouse cursors, but in the default
	 * SDL backend, it really is a separate layer. The mouse is
	 * always in 8bpp but can have a palette of its own, if the
	 * backend supports it. The scale of the mouse cursor is called
	 * 'cursorTargetScale'. This is meant as a hint to the backend.
	 * For example, let us assume the overlay is not visible, and the
	 * game graphics are displayed using a 2x scaler. If a mouse
	 * cursor with a cursorTargetScale of 1 is set, then it should be
	 * scaled by factor 2x, too, just like the game graphics. But if
	 * it has a cursorTargetScale of 2, then it shouldn't be scaled
	 * again by the game graphics scaler.
	 */
	//@{

	/**
	 * Description of a graphics mode.
	 */
	struct GraphicsMode {
		/**
		 * The 'name' of the graphics mode. This name is matched when selecting
		 * a mode via the command line, or via the config file.
		 * Examples: "1x", "advmame2x", "hq3x"
		 */
		const char *name;
		/**
		 * Human readable description of the scaler.
		 * Examples: "Normal (no scaling)", "AdvMAME2x", "HQ3x"
		 */
		const char *description;
		/**
		 * ID of the graphics mode. How to use this is completely up to the
		 * backend. This value will be passed to the setGraphicsMode(int)
		 * method by client code.
		 */
		int id;
	};

	/**
	 * Retrieve a list of all graphics modes supported by this backend.
	 * This can be both video modes as well as graphic filters/scalers;
	 * it is completely up to the backend maintainer to decide what is
	 * appropriate here and what not.
	 * The list is terminated by an all-zero entry.
	 * @return a list of supported graphics modes
	 */
	virtual const GraphicsMode *getSupportedGraphicsModes() const = 0;

	/**
	 * Return the ID of the 'default' graphics mode. What exactly this means
	 * is up to the backend. This mode is set by the client code when no user
	 * overrides are present (i.e. if no custom graphics mode is selected via
	 * the command line or a config file).
	 *
	 * @return the ID of the 'default' graphics mode
	 */
	virtual int getDefaultGraphicsMode() const = 0;

	/**
	 * Switch to the specified graphics mode. If switching to the new mode
	 * failed, this method returns false.
	 *
	 * @param mode	the ID of the new graphics mode
	 * @return true if the switch was successful, false otherwise
	 */
	virtual bool setGraphicsMode(int mode) = 0;

	/**
	 * Switch to the graphics mode with the given name. If 'name' is unknown,
	 * or if switching to the new mode failed, this method returns false.
	 *
	 * @param name	the name of the new graphics mode
	 * @return true if the switch was successful, false otherwise
	 * @note This is implemented via the setGraphicsMode(int) method, as well
	 *       as getSupportedGraphicsModes() and getDefaultGraphicsMode().
	 *       In particular, backends do not have to overload this!
	 */
	bool setGraphicsMode(const char *name);

	/**
	 * Determine which graphics mode is currently active.
	 * @return the active graphics mode
	 */
	virtual int getGraphicsMode() const = 0;

	/**
	 * Set the size of the virtual screen. Typical sizes include:
	 *  - 320x200 (e.g. for most SCUMM games, and Simon)
	 *  - 320x240 (e.g. for FM-TOWN SCUMM games)
	 *  - 640x480 (e.g. for Curse of Monkey Island)
	 *
	 * This is the resolution for which the client code generates data;
	 * this is not necessarily equal to the actual display size. For example,
	 * a backend may magnify the graphics to fit on screen (see also the
	 * GraphicsMode); stretch the data to perform aspect ratio correction;
	 * or shrink it to fit on small screens (in cell phones).
	 *
	 * @param width		the new virtual screen width
	 * @param height	the new virtual screen height
	 */
	virtual void initSize(uint width, uint height) = 0;

	/**
	 * Return an int value which is changed whenever any screen
	 * parameters (like the resolution) change. That is, whenever a
	 * EVENT_SCREEN_CHANGED would be sent. You can track this value
	 * in your code to detect screen changes in case you do not have
	 * full control over the event loop(s) being used (like the GUI
	 * code).
	 *
	 * @return an integer which can be used to track screen changes
	 *
	 * @note Backends which generate EVENT_SCREEN_CHANGED events MUST
	 *       overload this method appropriately.
	 */
	virtual int getScreenChangeID() const { return 0; }

	/**
	 * Begin a new GFX transaction, which is a sequence of GFX mode changes.
	 * The idea behind GFX transactions is to make it possible to activate
	 * several different GFX changes at once as a "batch" operation. For
	 * example, assume we are running in 320x200 with a 2x scaler (thus using
	 * 640x400 pixels in total). Now, we want to switch to 640x400 with the 1x
	 * scaler. Without transactions, we have to choose whether we want to first
	 * switch the scaler mode, or first to 640x400 mode. In either case,
	 * depending on the backend implementation, some ugliness may result.
	 * E.g. the window might briefly switch to 320x200 or 1280x800.
	 * Using transactions, this can be avoided.
	 *
	 * @note Transaction support is optional, and the default implementations
	 *       of the relevant methods simply do nothing.
	 * @see endGFXTransaction
	 */
	virtual void beginGFXTransaction() {}


	/**
	 * End (and thereby commit) the current GFX transaction.
	 * @see beginGFXTransaction
	 */
	virtual void endGFXTransaction() {}


	/**
	 * Returns the currently set virtual screen height.
	 * @see initSize
	 * @return the currently set virtual screen height
	 */
	virtual int16 getHeight() = 0;

	/**
	 * Returns the currently set virtual screen width.
	 * @see initSize
	 * @return the currently set virtual screen width
	 */
	virtual int16 getWidth() = 0;

	/**
	 * Replace the specified range of the palette with new colors.
	 * The palette entries from 'start' till (start+num-1) will be replaced - so
	 * a full palette update is accomplished via start=0, num=256.
	 *
	 * The palette data is specified in interleaved RGBA format. That is, the
	 * first byte of the memory block 'colors' points at is the red component
	 * of the first new color; the second byte the blue component of the first
	 * new color; the third byte the green component, the last byte to the alpha
	 * (transparency) value. Then the second color starts, and so on. So memory
	 * looks like this: R1-G1-B1-A1-R2-G2-B2-A2-R3-...
	 *
	 * @param colors	the new palette data, in interleaved RGB format
	 * @param start		the first palette entry to be updated
	 * @param num		the number of palette entries to be updated
	 *
	 * @note It is an error if start+num exceeds 256, behaviour is undefined
	 *       in that case (the backend may ignore it silently or assert).
	 * @note The alpha value is not actually used, and future revisions of this
	 *       API are probably going to remove it.
	 */
	virtual void setPalette(const byte *colors, uint start, uint num) = 0;

	/**
	 * Grabs a specified part of the currently active palette.
	 * The format is the same as for setPalette.
	 *
	 * @param colors	the palette data, in interleaved RGB format
	 * @param start		the first platte entry to be read
	 * @param num		the number of palette entries to be read
	 */
	virtual void grabPalette(byte *colors, uint start, uint num) = 0;

	/**
	 * Blit a bitmap to the virtual screen.
	 * The real screen will not immediately be updated to reflect the changes.
	 * Client code has to to call updateScreen to ensure any changes are
	 * visible to the user. This can be used to optimize drawing and reduce
	 * flicker.
	 * The graphics data uses 8 bits per pixel, using the palette specified
	 * via setPalette.
	 *
	 * @param buf		the buffer containing the graphics data source
	 * @param pitch		the pitch of the buffer (number of bytes in a scanline)
	 * @param x			the x coordinate of the destination rectangle
	 * @param y			the y coordinate of the destination rectangle
	 * @param w			the width of the destination rectangle
	 * @param h			the height of the destination rectangle
	 *
	 * @note The specified destination rectangle must be completly contained
	 *       in the visible screen space, and must be non-empty. If not, a
	 *       backend may or may not perform clipping, trigger an assert or
	 *       silently corrupt memory.
	 *
	 * @see updateScreen
	 */
	virtual void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) = 0;

	/**
	 * Copies the current screen contents to a new surface, with the original
	 * bit depth. This will allocate memory for the pixel data.
	 * WARNING: surf->free() must be called by the user to avoid leaking.
	 *
	 * @param surf	the surfce to store the data in it
	 * @return true if all went well, false if an error occured
	 */
	virtual bool grabRawScreen(Graphics::Surface *surf) = 0;

	/**
	 * Clear the screen to black.
	 */
	virtual void clearScreen() {}

	/** Update the dirty areas of the screen. */
	virtual void updateScreen() = 0;

	/**
	 * Set current shake position, a feature needed for some SCUMM screen effects.
	 * The effect causes the displayed graphics to be shifted upwards by the specified
	 * (always positive) offset. The area at the bottom of the screen which is moved
	 * into view by this is filled by black. This does not cause any graphic data to
	 * be lost - that is, to restore the original view, the game engine only has to
	 * call this method again with a 0 offset. No calls to copyRectToScreen are necessary.
	 * @param shakeOffset	the shake offset
	 *
	 * @todo This is a rather special screen effect, only used by the SCUMM
	 *       frontend - we should consider removing it from the backend API
	 *       and instead implement the functionality in the frontend.
	 */
	virtual void setShakePos(int shakeOffset) = 0;
		
	/**
	 * Sets the area of the screen that has the focus.  For example, when a character
	 * is speaking, they will have the focus.  Allows for pan-and-scan style views
	 * where the backend could follow the speaking character or area of interest on 
	 * the screen.
	 *
	 * The backend is responsible for clipping the rectangle and deciding how best to
	 * zoom the screen to show any shape and size rectangle the engine provides.
	 *
	 * @param rect A rectangle on the screen to be focused on
	 * @see clearFocusRectangle
	 */	
	virtual void setFocusRectangle(const Common::Rect& rect) {}
	
	/**
	 * Clears the focus set by a call to setFocusRectangle().  This allows the engine
	 * to clear the focus during times when no particular area of the screen has the
	 * focus.
	 * @see setFocusRectangle
	 */
	virtual void clearFocusRectangle() {}

	//@}



	/**
	 * @name Overlay
	 * In order to be able to display dialogs atop the game graphics, backends
	 * must provide an overlay mode.
	 *
	 * While the game graphics are always 8 bpp, the overlay can be 8 or 16 bpp.
	 * Depending on which it is, OverlayColor is 8 or 16 bit.
	 *
	 * For 'coolness' we usually want to have an overlay which is blended over
	 * the game graphics. On backends which support alpha blending, this is
	 * no issue; but on other systems (in particular those which only support
	 * 8bpp), this needs some trickery.
	 *
	 * Essentially, we fake (alpha) blending on these systems by copying the
	 * game graphics into the overlay buffer, then manually compose whatever
	 * graphics we want to show in the overlay.
	 */
	//@{

	/** Activate the overlay mode. */
	virtual void showOverlay() = 0;

	/** Deactivate the overlay mode. */
	virtual void hideOverlay() = 0;

	/**
	 * Reset the overlay.
	 *
	 * After calling this method while the overlay mode is active, the user
	 * should be seeing only the game graphics. How this is achieved depends
	 * on how the backend implements the overlay. Either it sets all pixels of
	 * the overlay to be transparent (when alpha blending is used).
	 *
	 * Or, in case of fake alpha blending, it might just put a copy of the
	 * current game graphics screen into the overlay.
	 */
	virtual void clearOverlay() = 0;

	/**
	 * Copy the content of the overlay into a buffer provided by the caller.
	 * This is only used to implement fake alpha blending.
	 */
	virtual void grabOverlay(OverlayColor *buf, int pitch) = 0;

	/**
	 * Blit a graphics buffer to the overlay.
	 * In a sense, this is the reverse of grabOverlay.
	 * @see copyRectToScreen
	 * @see grabOverlay
	 */
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) = 0;

	/**
	 * Return the height of the overlay.
	 * @see getHeight
	 */
	virtual int16 getOverlayHeight()  { return getHeight(); }

	/**
	 * Return the width of the overlay.
	 * @see getWidth
	 */
	virtual int16 getOverlayWidth()   { return getWidth(); }

	virtual int screenToOverlayX(int x) { return x; }
	virtual int screenToOverlayY(int y) { return y; }
	virtual int overlayToScreenX(int x) { return x; }
	virtual int overlayToScreenY(int y) { return y; }

	/**
	* Convert the given RGB triplet into an OverlayColor. A OverlayColor can
	 * be 8bit, 16bit or 32bit, depending on the target system. The default
	 * implementation generates a 16 bit color value, in the 565 format
	 * (that is, 5 bits red, 6 bits green, 5 bits blue).
	 * @see colorToRGB
	 * @see ARGBToColor
	 */
	virtual OverlayColor RGBToColor(uint8 r, uint8 g, uint8 b) {
		return ((((r >> 3) & 0x1F) << 11) | (((g >> 2) & 0x3F) << 5) | ((b >> 3) & 0x1F));
	}

	/**
	 * Convert the given OverlayColor into a RGB triplet. An OverlayColor can
	 * be 8bit, 16bit or 32bit, depending on the target system. The default
	 * implementation takes a 16 bit color value and assumes it to be in 565 format
	 * (that is, 5 bits red, 6 bits green, 5 bits blue).
	 * @see RGBToColor
	 * @see colorToARGB
	 */
	virtual void colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b) {
		r = (((color >> 11) & 0x1F) << 3);
		g = (((color >> 5) & 0x3F) << 2);
		b = ((color&0x1F) << 3);
	}

	/**
	* Convert the given ARGB quadruplet into an OverlayColor. A OverlayColor can
	 * be 8bit, 16bit or 32bit, depending on the target system. The default
	 * implementation generates a 16 bit color value, in the 565 format
	 * (that is, 5 bits red, 6 bits green, 5 bits blue).
	 * @note The alpha component, ranges from 0 (transparent) to 255 (opaque).
	 * @see colorToRGB
	 * @see RGBToColor
	 */
	virtual OverlayColor ARGBToColor(uint8 a, uint8 r, uint8 g, uint8 b) {
		return RGBToColor(r, g, b);
	}

	/**
	 * Convert the given OverlayColor into an ARGB quadruplet. An OverlayColor can
	 * be 8bit, 16bit or 32bit, depending on the target system. The default
	 * implementation takes a 16 bit color value and assumes it to be in 565 format
	 * (that is, 5 bits red, 6 bits green, 5 bits blue).
	 * @note The alpha component, ranges from 0 (transparent) to 255 (opaque).
	 * @see ARGBToColor
	 * @see colorToRGB
	 */
	virtual void colorToARGB(OverlayColor color, uint8 &a, uint8 &r, uint8 &g, uint8 &b) {
		colorToRGB(color, r, g, b);
		a = 255;
	}

	//@}



	/** @name Mouse */
	//@{

	/** Show or hide the mouse cursor. */
	virtual bool showMouse(bool visible) = 0;

	/**
	 * Move ("warp") the mouse cursor to the specified position in virtual
	 * screen coordinates.
	 * @param x		the new x position of the mouse
	 * @param y		the new x position of the mouse
	 */
	virtual void warpMouse(int x, int y) = 0;

	/**
	 * Set the bitmap used for drawing the cursor.
	 *
	 * @param buf				the pixmap data to be used (8bit/pixel)
	 * @param w					width of the mouse cursor
	 * @param h					height of the mouse cursor
	 * @param hotspotX			horizontal offset from the left side to the hotspot
	 * @param hotspotY			vertical offset from the top side to the hotspot
	 * @param keycolor			transparency color index
	 * @param cursorTargetScale	scale factor which cursor is designed for
	 */
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor = 255, int cursorTargetScale = 1) = 0;

	/**
	 * Replace the specified range of cursor the palette with new colors.
	 * The palette entries from 'start' till (start+num-1) will be replaced - so
	 * a full palette update is accomplished via start=0, num=256.
	 *
	 * Backends which implement it should have kFeatureCursorHasPalette flag set
	 *
	 * @see setPalette
	 * @see kFeatureCursorHasPalette
	 */
	virtual void setCursorPalette(const byte *colors, uint start, uint num) {}

	/**
	 * Disable or enable cursor palette.
	 *
	 * Backends which implement it should have kFeatureCursorHasPalette flag set
	 *
	 * @param disable  True to disable, false to enable.
	 *
	 * @see setPalette
	 * @see kFeatureCursorHasPalette
	 */
	virtual void disableCursorPalette(bool disable) {}

	//@}



	/** @name Events and Time */
	//@{

protected:
	friend class DefaultEventManager;

	/**
	 * Get the next event in the event queue.
	 * @param event	point to an Common::Event struct, which will be filled with the event data.
	 * @return true if an event was retrieved.
	 */
	virtual bool pollEvent(Common::Event &event) = 0;

public:
	/** Get the number of milliseconds since the program was started. */
	virtual uint32 getMillis() = 0;

	/** Delay/sleep for the specified amount of milliseconds. */
	virtual void delayMillis(uint msecs) = 0;

	/**
	 * Return the timer manager singleton. For more information, refer
	 * to the TimerManager documentation.
	 */
	virtual Common::TimerManager *getTimerManager() = 0;

	/**
	 * Return the event manager singleton. For more information, refer
	 * to the EventManager documentation.
	 */
	virtual Common::EventManager *getEventManager();

	//@}



	/**
	 * @name Mutex handling
	 * Historically, the OSystem API used to have a method which allowed
	 * creating threads. Hence mutex support was needed for thread syncing.
	 * To ease portability, though, we decided to remove the threading API.
	 * Instead, we now use timers (see setTimerCallback() and Common::Timer).
	 * But since those may be implemented using threads (and in fact, that's
	 * how our primary backend, the SDL one, does it on many systems), we
	 * still have to do mutex syncing in our timer callbacks.
	 *
	 * Hence backends which do not use threads to implement the timers simply
	 * can use dummy implementations for these methods.
	 */
	//@{

	typedef Common::MutexRef	MutexRef;

	/**
	 * Create a new mutex.
	 * @return the newly created mutex, or 0 if an error occured.
	 */
	virtual MutexRef createMutex() = 0;

	/**
	 * Lock the given mutex.
	 * @param mutex	the mutex to lock.
	 */
	virtual void lockMutex(MutexRef mutex) = 0;

	/**
	 * Unlock the given mutex.
	 * @param mutex	the mutex to unlock.
	 */
	virtual void unlockMutex(MutexRef mutex) = 0;

	/**
	 * Delete the given mutex. Make sure the mutex is unlocked before you delete it.
	 * If you delete a locked mutex, the behavior is undefined, in particular, your
	 * program may crash.
	 * @param mutex	the mutex to delete.
	 */
	virtual void deleteMutex(MutexRef mutex) = 0;

	//@}



	/** @name Sound */
	//@{

	/**
	 * Returh the audio mixer. For more information, refer to the
	 * Audio::Mixer documentation.
	 */
	virtual Audio::Mixer *getMixer() = 0;

	/**
	 * Determine the output sample rate. Audio data provided by the sound
	 * callback will be played using this rate.
	 * @note Client code other than the sound mixer should _not_ use this
	 *       method. Instead, call Mixer::getOutputRate()!
	 * @return the output sample rate
	 */
	virtual int getOutputSampleRate() const = 0;

	//@}



	/**
	 * @name Audio CD
	 * The methods in this group deal with Audio CD playback.
	 * The default implementation simply does nothing.
	 */
	//@{

	/**
	 * Initialise the specified CD drive for audio playback.
	 * @return true if the CD drive was inited succesfully
	 */
	virtual bool openCD(int drive);

	/**
	 * Poll CD status.
	 * @return true if CD audio is playing
	 */
	virtual bool pollCD();

	/**
	 * Start audio CD playback.
	 * @param track			the track to play.
	 * @param num_loops		how often playback should be repeated (-1 = infinitely often).
	 * @param start_frame	the frame at which playback should start (75 frames = 1 second).
	 * @param duration		the number of frames to play.
	 */
	virtual void playCD(int track, int num_loops, int start_frame, int duration);

	/**
	 * Stop audio CD playback.
	 */
	virtual void stopCD();

	/**
	 * Update cdrom audio status.
	 */
	virtual void updateCD();

	//@}



	/** @name Miscellaneous */
	//@{
	/** Quit (exit) the application. */
	virtual void quit() = 0;

	/**
	 * Set a window caption or any other comparable status display to the
	 * given value. The caption must be a pure ASCII string. Passing a
	 * non-ASCII string may lead to unexpected behavior, even crashes.
	 *
	 * In a future revision of this API, this may be changed to allowing
	 * UTF-8 or UTF-16 encoded data, or maybe ISO LATIN 1.
	 *
	 * @param caption	the window caption to use, as an ASCII string
	 */
	virtual void setWindowCaption(const char *caption) {}

	/**
	 * Display a message in an 'on screen display'. That is, display it in a
	 * fashion where it is visible on or near the screen (e.g. in a transparent
	 * rectangle over the regular screen content; or in a message box beneath
	 * it; etc.).
	 *
	 * @note There is a default implementation which uses a TimedMessageDialog
	 *       to display the message. Hence implementing this is optional.
	 *
	 * @param msg	the message to display on screen
	 */
	virtual void displayMessageOnOSD(const char *msg);

	/**
	 * Return the SaveFileManager, used to store and load savestates
	 * and other modifiable persistent game data. For more information,
	 * refer to the SaveFileManager documentation.
	 */
	virtual Common::SaveFileManager *getSavefileManager() = 0;

	//@}
};


/** The global OSystem instance. Initialised in main(). */
extern OSystem *g_system;

#endif
