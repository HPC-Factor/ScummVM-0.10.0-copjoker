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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/graphics/colormasks.h $
 * $Id: colormasks.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef GRAPHICS_COLORMASKS_H
#define GRAPHICS_COLORMASKS_H

template<int bitFormat>
struct ColorMasks {
};

/*
The ColorMasks template can be used to map bit format values
(like 555, 565, 1555, 4444) to corresponding bit masks and shift values.
Currently this is only meant for 

The meaning of these is masks is the following:
 kBytesPerPixel
    -> how many bytes per pixel for that format

 kRedMask, kGreenMask, kBlueMask
    -> bitmask, and this with the color to select only the bits of the corresponding color
 
 The k*Bits and k*Shift values can be used to extract R,G,B. I.e. to get
 the red color component of a pixel, as a 8-bit value, you would write
 
 R = ((color & kRedMask) >> kRedShift) << (8-kRedBits)
 
 Actually, instead of the simple left shift, one might want to use somewhat
 more sophisticated code (which fills up the lower most bits.
 
 
 The highBits / lowBits / qhighBits / qlowBits are special values that are
 used in the super-optimized interpolation functions in scaler/intern.h
 and scaler/aspect.cpp. Currently they are only available in 555 and 565 mode.
 To be specific: They pack the masks for two 16 bit pixels at once. The pixels
 are split into "high" and "low" bits, which are then separately interpolated
 and finally re-composed. That way, 2x2 pixels or even 4x2 pixels can
 be interpolated in one go.

*/


template<>
struct ColorMasks<565> {
	enum {
		highBits    = 0xF7DEF7DE,
		lowBits     = 0x08210821,
		qhighBits   = 0xE79CE79C,
		qlowBits    = 0x18631863,


		kBytesPerPixel = 2,

		kAlphaBits  = 0,
		kRedBits    = 5,
		kGreenBits  = 6,
		kBlueBits   = 5,

		kAlphaShift = kRedBits+kGreenBits+kBlueBits,
		kRedShift   = kGreenBits+kBlueBits,
		kGreenShift = kBlueBits,
		kBlueShift  = 0,

		kAlphaMask = ((1 << kAlphaBits) - 1) << kAlphaShift,
		kRedMask   = ((1 << kRedBits) - 1) << kRedShift,
		kGreenMask = ((1 << kGreenBits) - 1) << kGreenShift,
		kBlueMask  = ((1 << kBlueBits) - 1) << kBlueShift,

		kRedBlueMask = kRedMask | kBlueMask

	};
};

template<>
struct ColorMasks<555> {
	enum {
		highBits    = 0x7BDE7BDE,
		lowBits     = 0x04210421,
		qhighBits   = 0x739C739C,
		qlowBits    = 0x0C630C63,


		kBytesPerPixel = 2,

		kAlphaBits  = 0,
		kRedBits    = 5,
		kGreenBits  = 5,
		kBlueBits   = 5,

		kAlphaShift = kRedBits+kGreenBits+kBlueBits,
		kRedShift   = kGreenBits+kBlueBits,
		kGreenShift = kBlueBits,
		kBlueShift  = 0,

		kAlphaMask = ((1 << kAlphaBits) - 1) << kAlphaShift,
		kRedMask   = ((1 << kRedBits) - 1) << kRedShift,
		kGreenMask = ((1 << kGreenBits) - 1) << kGreenShift,
		kBlueMask  = ((1 << kBlueBits) - 1) << kBlueShift,

		kRedBlueMask = kRedMask | kBlueMask
	};
};

template<>
struct ColorMasks<1555> {
	enum {
		kBytesPerPixel = 2,

		kAlphaBits  = 1,
		kRedBits    = 5,
		kGreenBits  = 5,
		kBlueBits   = 5,

		kAlphaShift = kRedBits+kGreenBits+kBlueBits,
		kRedShift   = kGreenBits+kBlueBits,
		kGreenShift = kBlueBits,
		kBlueShift  = 0,

		kAlphaMask = ((1 << kAlphaBits) - 1) << kAlphaShift,
		kRedMask   = ((1 << kRedBits) - 1) << kRedShift,
		kGreenMask = ((1 << kGreenBits) - 1) << kGreenShift,
		kBlueMask  = ((1 << kBlueBits) - 1) << kBlueShift,

		kRedBlueMask = kRedMask | kBlueMask
	};
};

template<>
struct ColorMasks<4444> {
	enum {
		kBytesPerPixel = 2,

		kAlphaBits  = 4,
		kRedBits    = 4,
		kGreenBits  = 4,
		kBlueBits   = 4,

		kAlphaShift = kRedBits+kGreenBits+kBlueBits,
		kRedShift   = kGreenBits+kBlueBits,
		kGreenShift = kBlueBits,
		kBlueShift  = 0,

		kAlphaMask = ((1 << kAlphaBits) - 1) << kAlphaShift,
		kRedMask   = ((1 << kRedBits) - 1) << kRedShift,
		kGreenMask = ((1 << kGreenBits) - 1) << kGreenShift,
		kBlueMask  = ((1 << kBlueBits) - 1) << kBlueShift,

		kRedBlueMask = kRedMask | kBlueMask
	};
};

template<>
struct ColorMasks<888> {
	enum {
		kBytesPerPixel = 4,

		kAlphaBits  = 0,
		kRedBits    = 8,
		kGreenBits  = 8,
		kBlueBits   = 8,

		kAlphaShift = kRedBits+kGreenBits+kBlueBits,
		kRedShift   = kGreenBits+kBlueBits,
		kGreenShift = kBlueBits,
		kBlueShift  = 0,

		kAlphaMask = ((1 << kAlphaBits) - 1) << kAlphaShift,
		kRedMask   = ((1 << kRedBits) - 1) << kRedShift,
		kGreenMask = ((1 << kGreenBits) - 1) << kGreenShift,
		kBlueMask  = ((1 << kBlueBits) - 1) << kBlueShift,

		kRedBlueMask = kRedMask | kBlueMask
	};
};

template<>
struct ColorMasks<8888> {
	enum {
		kBytesPerPixel = 4,

		kAlphaBits  = 8,
		kRedBits    = 8,
		kGreenBits  = 8,
		kBlueBits   = 8,

		kAlphaShift = kRedBits+kGreenBits+kBlueBits,
		kRedShift   = kGreenBits+kBlueBits,
		kGreenShift = kBlueBits,
		kBlueShift  = 0,

		kAlphaMask = ((1 << kAlphaBits) - 1) << kAlphaShift,
		kRedMask   = ((1 << kRedBits) - 1) << kRedShift,
		kGreenMask = ((1 << kGreenBits) - 1) << kGreenShift,
		kBlueMask  = ((1 << kBlueBits) - 1) << kBlueShift,

		kRedBlueMask = kRedMask | kBlueMask
	};
};

template<class T>
uint32 RGBToColor(uint8 r, uint8 g, uint8 b) {
	return T::kAlphaMask |
	       (((r << T::kRedShift) >> (8 - T::kRedBits)) & T::kRedMask) |
	       (((g << T::kGreenShift) >> (8 - T::kGreenBits)) & T::kGreenMask) |
	       (((b << T::kBlueShift) >> (8 - T::kBlueBits)) & T::kBlueMask);
}

template<class T>
uint32 ARGBToColor(uint8 a, uint8 r, uint8 g, uint8 b) {
	return (((a << T::kAlphaShift) >> (8 - T::kAlphaBits)) & T::kAlphaMask) |
	       (((r << T::kRedShift) >> (8 - T::kRedBits)) & T::kRedMask) |
	       (((g << T::kGreenShift) >> (8 - T::kGreenBits)) & T::kGreenMask) |
	       (((b << T::kBlueShift) >> (8 - T::kBlueBits)) & T::kBlueMask);
}

template<class T>
void colorToRGB(uint32 color, uint8 &r, uint8 &g, uint8 &b) {
	r = ((color & T::kRedMask) >> T::kRedShift) << (8 - T::kRedBits);
	g = ((color & T::kGreenMask) >> T::kGreenShift) << (8 - T::kGreenBits);
	b = ((color & T::kBlueMask) >> T::kBlueShift) << (8 - T::kBlueBits);
}

template<class T>
void colorToARGB(uint32 color, uint8 &a, uint8 &r, uint8 &g, uint8 &b) {
	a = ((color & T::kAlphaMask) >> T::kAlphaShift) << (8 - T::kAlphaBits);
	r = ((color & T::kRedMask) >> T::kRedShift) << (8 - T::kRedBits);
	g = ((color & T::kGreenMask) >> T::kGreenShift) << (8 - T::kGreenBits);
	b = ((color & T::kBlueMask) >> T::kBlueShift) << (8 - T::kBlueBits);
}

#endif
