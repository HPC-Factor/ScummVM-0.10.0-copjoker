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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/saga/font.cpp $
 * $Id: font.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

// Font management and font drawing module
#include "saga/saga.h"
#include "saga/gfx.h"
#include "saga/rscfile.h"

#include "saga/font.h"
#include "saga/stream.h"

namespace Saga {

Font::Font(SagaEngine *vm) : _vm(vm), _initialized(false) {
	int i;

	// Load font module resource context

	assert(_vm->getFontsCount() > 0);

	_fonts = (FontData **)calloc(_vm->getFontsCount(), sizeof(*_fonts));
	_loadedFonts = 0;

	for (i = 0; i < _vm->getFontsCount(); i++) {
		loadFont(_vm->getFontDescription(i)->fontResourceId);
	}

	_initialized = true;
}

Font::~Font(void) {
	debug(8, "Font::~Font(): Freeing fonts.");
	int i;

	for (i = 0 ; i < _loadedFonts ; i++) {
		if (_fonts[i] != NULL) {
			free(_fonts[i]->normal.font);
			free(_fonts[i]->outline.font);
		}

		free(_fonts[i]);
	}
}


void Font::loadFont(uint32 fontResourceId) {
	FontData *font;
	byte *fontResourcePointer;
	size_t fontResourceLength;
	int numBits;
	int c;
	ResourceContext *fontContext;

	debug(1, "Font::loadFont(): Reading fontResourceId %d...", fontResourceId);

	fontContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
	if (fontContext == NULL) {
		error("Font::Font() resource context not found");
	}

	// Load font resource
	_vm->_resource->loadResource(fontContext, fontResourceId, fontResourcePointer, fontResourceLength);

	if (fontResourceLength < FONT_DESCSIZE) {
		error("Font::loadFont() Invalid font length (%i < %i)", (int)fontResourceLength, FONT_DESCSIZE);
	}

	MemoryReadStreamEndian readS(fontResourcePointer, fontResourceLength, fontContext->isBigEndian);

	// Create new font structure
	font = (FontData *)malloc(sizeof(*font));

	// Read font header
	font->normal.header.charHeight = readS.readUint16();
	font->normal.header.charWidth = readS.readUint16();
	font->normal.header.rowLength = readS.readUint16();


	debug(2, "Character width: %d", font->normal.header.charWidth);
	debug(2, "Character height: %d", font->normal.header.charHeight);
	debug(2, "Row padding: %d", font->normal.header.rowLength);

	for (c = 0; c < FONT_CHARCOUNT; c++) {
		font->normal.fontCharEntry[c].index = readS.readUint16();
	}

	for (c = 0; c < FONT_CHARCOUNT; c++) {
		numBits = font->normal.fontCharEntry[c].width = readS.readByte();
		font->normal.fontCharEntry[c].byteWidth = getByteLen(numBits);
	}

	for (c = 0; c < FONT_CHARCOUNT; c++) {
		font->normal.fontCharEntry[c].flag = readS.readByte();
	}

	for (c = 0; c < FONT_CHARCOUNT; c++) {
		font->normal.fontCharEntry[c].tracking = readS.readByte();
	}

	if (readS.pos() != FONT_DESCSIZE) {
		error("Invalid font resource size.");
	}

	font->normal.font = (byte*)malloc(fontResourceLength - FONT_DESCSIZE);
	memcpy(font->normal.font, fontResourcePointer + FONT_DESCSIZE, fontResourceLength - FONT_DESCSIZE);

	free(fontResourcePointer);


	// Create outline font style
	createOutline(font);

	// Set font data
	_fonts[_loadedFonts++] = font;
}

void Font::createOutline(FontData *font) {
	int i;
	int row;
	int newByteWidth;
	int oldByteWidth;
	int newRowLength = 0;
	size_t indexOffset = 0;
	int index;
	int currentByte;
	unsigned char *basePointer;
	unsigned char *srcPointer;
	unsigned char *destPointer1;
	unsigned char *destPointer2;
	unsigned char *destPointer3;
	unsigned char charRep;


	// Populate new font style character data
	for (i = 0; i < FONT_CHARCOUNT; i++) {
		newByteWidth = 0;
		oldByteWidth = 0;
		index = font->normal.fontCharEntry[i].index;
		if ((index > 0) || (i == FONT_FIRSTCHAR)) {
			index += indexOffset;
		}

		font->outline.fontCharEntry[i].index = index;
		font->outline.fontCharEntry[i].tracking = font->normal.fontCharEntry[i].tracking;
		font->outline.fontCharEntry[i].flag = font->normal.fontCharEntry[i].flag;

		if (font->normal.fontCharEntry[i].width != 0) {
			newByteWidth = getByteLen(font->normal.fontCharEntry[i].width + 2);
			oldByteWidth = getByteLen(font->normal.fontCharEntry[i].width);

			if (newByteWidth > oldByteWidth) {
				indexOffset++;
			}
		}

		font->outline.fontCharEntry[i].width = font->normal.fontCharEntry[i].width + 2;
		font->outline.fontCharEntry[i].byteWidth = newByteWidth;
		newRowLength += newByteWidth;
	}

	debug(2, "New row length: %d", newRowLength);

	font->outline.header = font->normal.header;
	font->outline.header.charWidth += 2;
	font->outline.header.charHeight += 2;
	font->outline.header.rowLength = newRowLength;

	// Allocate new font representation storage
	font->outline.font = (unsigned char *)calloc(newRowLength, font->outline.header.charHeight);


	// Generate outline font representation
	for (i = 0; i < FONT_CHARCOUNT; i++) {
		for (row = 0; row < font->normal.header.charHeight; row++) {
			for (currentByte = 0; currentByte < font->outline.fontCharEntry[i].byteWidth; currentByte++) {
				basePointer = font->outline.font + font->outline.fontCharEntry[i].index + currentByte;
				destPointer1 = basePointer + newRowLength * row;
				destPointer2 = basePointer + newRowLength * (row + 1);
				destPointer3 = basePointer + newRowLength * (row + 2);
				if (currentByte > 0) {
					// Get last two columns from previous byte
					srcPointer = font->normal.font + font->normal.header.rowLength * row + font->normal.fontCharEntry[i].index + (currentByte - 1);
					charRep = *srcPointer;
					*destPointer1 |= ((charRep << 6) | (charRep << 7));
					*destPointer2 |= ((charRep << 6) | (charRep << 7));
					*destPointer3 |= ((charRep << 6) | (charRep << 7));
				}

				if (currentByte < font->normal.fontCharEntry[i].byteWidth) {
					srcPointer = font->normal.font + font->normal.header.rowLength * row + font->normal.fontCharEntry[i].index + currentByte;
					charRep = *srcPointer;
					*destPointer1 |= charRep | (charRep >> 1) | (charRep >> 2);
					*destPointer2 |= charRep | (charRep >> 1) | (charRep >> 2);
					*destPointer3 |= charRep | (charRep >> 1) | (charRep >> 2);
				}
			}
		}

		// "Hollow out" character to prevent overdraw
		for (row = 0; row < font->normal.header.charHeight; row++) {
			for (currentByte = 0; currentByte < font->outline.fontCharEntry[i].byteWidth; currentByte++) {
				destPointer2 = font->outline.font + font->outline.header.rowLength * (row + 1) + font->outline.fontCharEntry[i].index + currentByte;
				if (currentByte > 0) {
					// Get last two columns from previous byte
					srcPointer = font->normal.font + font->normal.header.rowLength * row + font->normal.fontCharEntry[i].index + (currentByte - 1);
					*destPointer2 &= ((*srcPointer << 7) ^ 0xFFU);
				}

				if (currentByte < font->normal.fontCharEntry[i].byteWidth) {
					srcPointer = font->normal.font + font->normal.header.rowLength * row + font->normal.fontCharEntry[i].index + currentByte;
					*destPointer2 &= ((*srcPointer >> 1) ^ 0xFFU);
				}
			}
		}
	}
}

// Returns the horizontal length in pixels of the graphical representation
// of at most 'count' characters of the string 'text', taking
// into account any formatting options specified by 'flags'.
// If 'count' is 0, all characters of 'test' are counted.
int Font::getStringWidth(FontId fontId, const char *text, size_t count, FontEffectFlags flags) {
	FontData *font;
	size_t ct;
	int width = 0;
	int ch;
	const byte *txt;

	
	font = getFont(fontId);

	txt = (const byte *) text;

	for (ct = count; *txt && (!count || ct > 0); txt++, ct--) {
		ch = *txt & 0xFFU;
		// Translate character
		ch = _charMap[ch];
		assert(ch < FONT_CHARCOUNT);
		width += font->normal.fontCharEntry[ch].tracking;
	}

	if ((flags & kFontBold) || (flags & kFontOutline)) {
		width += 1;
	}

	return width;
}


void Font::draw(FontId fontId, Surface *ds, const char *text, size_t count, const Common::Point &point,
			   int color, int effectColor, FontEffectFlags flags) {
	FontData *font;
	Point offsetPoint(point);

	font = getFont(fontId);

	if (flags & kFontOutline) {
		offsetPoint.x--;
		offsetPoint.y--;
		outFont(font->outline, ds, text, count, offsetPoint, effectColor, flags);
		outFont(font->normal, ds, text, count, point, color, flags);
	} else if (flags & kFontShadow) {
		offsetPoint.x--;
		offsetPoint.y++;
		outFont(font->normal, ds, text, count, offsetPoint, effectColor, flags);
		outFont(font->normal, ds, text, count, point, color, flags);
	} else { // FONT_NORMAL
		outFont(font->normal, ds, text, count, point, color, flags);
	}
}

void Font::outFont(const FontStyle &drawFont, Surface *ds, const char *text, size_t count, const Common::Point &point, int color, FontEffectFlags flags) {
	const byte *textPointer;
	byte *c_dataPointer;
	int c_code;
	int charRow;
	Point textPoint(point);

	byte *outputPointer;
	byte *outputPointer_min;
	byte *outputPointer_max;

	int row;
	int rowLimit;

	int c_byte_len;
	int c_byte;
	int c_bit;
	int ct;

	if ((point.x > ds->w) || (point.y > ds->h)) {
		// Output string can't be visible
		return;
	}

	textPointer = (const byte *)text;
	ct = count;

	// Draw string one character at a time, maximum of 'draw_str'_ct
	// characters, or no limit if 'draw_str_ct' is 0
	for (; *textPointer && (!count || ct); textPointer++, ct--) {
		c_code = *textPointer & 0xFFU;

		// Translate character
		if (!(flags & kFontDontmap))
			c_code = _charMap[c_code];
		assert(c_code < FONT_CHARCOUNT);

		// Check if character is defined
		if ((drawFont.fontCharEntry[c_code].index == 0) && (c_code != FONT_FIRSTCHAR)) {
#if FONT_SHOWUNDEFINED
			if (c_code == FONT_CH_SPACE) {
				textPoint.x += drawFont.fontCharEntry[c_code].tracking;
				continue;
			}
			c_code = FONT_CH_QMARK;
#else
			// Character code is not defined, but advance tracking
			// ( Not defined if offset is 0, except for 33 ('!') which
			//   is defined )
			textPoint.x += drawFont.fontCharEntry[c_code].tracking;
			continue;
#endif
		}

		// Get length of character in bytes
		c_byte_len = ((drawFont.fontCharEntry[c_code].width - 1) / 8) + 1;
		rowLimit = (ds->h < (textPoint.y + drawFont.header.charHeight)) ? ds->h : textPoint.y + drawFont.header.charHeight;
		charRow = 0;

		for (row = textPoint.y; row < rowLimit; row++, charRow++) {
			// Clip negative rows */
			if (row < 0) {
				continue;
			}

			outputPointer = (byte *)ds->pixels + (ds->pitch * row) + textPoint.x;
			outputPointer_min = (byte *)ds->pixels + (ds->pitch * row) + (textPoint.x > 0 ? textPoint.x : 0);
			outputPointer_max = outputPointer + (ds->pitch - textPoint.x);

			// If character starts off the screen, jump to next character
			if (outputPointer < outputPointer_min) {
				break;
			}

			c_dataPointer = drawFont.font + charRow * drawFont.header.rowLength + drawFont.fontCharEntry[c_code].index;

			for (c_byte = 0; c_byte < c_byte_len; c_byte++, c_dataPointer++) {
				// Check each bit, draw pixel if bit is set
				for (c_bit = 7; c_bit >= 0 && (outputPointer < outputPointer_max); c_bit--) {
					if ((*c_dataPointer >> c_bit) & 0x01) {
						*outputPointer = (byte)color;
					}
					outputPointer++;
				} // end per-bit processing
			} // end per-byte processing
		} // end per-row processing

		// Advance tracking position
		textPoint.x += drawFont.fontCharEntry[c_code].tracking;
	} // end per-character processing
}


void Font::textDraw(FontId fontId, Surface *ds, const char *text, const Common::Point &point, int color, int effectColor, FontEffectFlags flags) {
	int textWidth;
	int textLength;
	int fitWidth;
	Common::Point textPoint(point);

	textLength = strlen(text);

	if (!(flags & kFontCentered)) {
		// Text is not centered; No formatting required
		draw(fontId, ds, text, textLength, point, color, effectColor, flags);
		return;
	}

	// Text is centered... format output
	// Enforce minimum and maximum center points for centered text
	if (textPoint.x < TEXT_CENTERLIMIT) {
		textPoint.x = TEXT_CENTERLIMIT;
	}

	if (textPoint.x > ds->w - TEXT_CENTERLIMIT) {
		textPoint.x = ds->w - TEXT_CENTERLIMIT;
	}

	if (textPoint.x < (TEXT_MARGIN * 2)) {
		// Text can't be centered if it's too close to the margin
		return;
	}

	textWidth = getStringWidth(fontId, text, textLength, flags);

	if (textPoint.x < (ds->w / 2)) {
		// Fit to right side
		fitWidth = (textPoint.x - TEXT_MARGIN) * 2;
	} else {
		// Fit to left side
		fitWidth = ((ds->w - TEXT_MARGIN) - textPoint.x) * 2;
	}

	if (fitWidth < textWidth) {
		warning("text too long to be displayed in one line");
		textWidth = fitWidth;
	}
	// Entire string fits, draw it
	textPoint.x = textPoint.x - (textWidth / 2);
	draw(fontId, ds, text, textLength, textPoint, color, effectColor, flags);
}

int Font::getHeight(FontId fontId, const char *text, int width, FontEffectFlags flags) {
	int textWidth;
	int textLength;
	int fitWidth;
	const char *startPointer;
	const char *searchPointer;
	const char *measurePointer;
	const char *foundPointer;
	int len;
	int w;
	const char *endPointer;
	int h;
	int wc;
	int w_total;
	int len_total;
	Common::Point textPoint;
	Common::Point textPoint2;

	textLength = strlen(text);

	textWidth = getStringWidth(fontId, text, textLength, flags);
	h = getHeight(fontId);
	fitWidth = width;

	textPoint.x = (fitWidth / 2);
	textPoint.y = 0;

	if (fitWidth >= textWidth) {
		return h;
	}

	// String won't fit on one line
	w_total = 0;
	len_total = 0;
	wc = 0;

	startPointer = text;
	measurePointer = text;
	searchPointer = text;
	endPointer = text + textLength;

	for (;;) {
		foundPointer = strchr(searchPointer, ' ');
		if (foundPointer == NULL) {
			// Ran to the end of the buffer
			len = endPointer - measurePointer;
		} else {
			len = foundPointer - measurePointer;
		}

		w = getStringWidth(fontId, measurePointer, len, flags);
		measurePointer = foundPointer;

		if ((w_total + w) > fitWidth) {
			// This word won't fit
			if (wc == 0) {
				// The first word in the line didn't fit. Still print it
				searchPointer = measurePointer + 1;
			}
			// Wrap what we've got and restart
			textPoint.y += h + TEXT_LINESPACING;
			if (foundPointer == NULL) {
				// Since word hit NULL but fit, we are done
				return textPoint.y + h;
			}
			w_total = 0;
			len_total = 0;
			wc = 0;
			measurePointer = searchPointer;
			startPointer = searchPointer;
		} else {
			// Word will fit ok
			w_total += w;
			len_total += len;
			wc++;
			if (foundPointer == NULL) {
				// Since word hit NULL but fit, we are done
				return textPoint.y + h;
			}
			searchPointer = measurePointer + 1;
		}
	}
}

void Font::textDrawRect(FontId fontId, Surface *ds, const char *text, const Common::Rect &rect, int color, int effectColor, FontEffectFlags flags) {
	int textWidth;
	int textLength;
	int fitWidth;
	const char *startPointer;
	const char *searchPointer;
	const char *measurePointer;
	const char *foundPointer;
	int len;
	int w;
	const char *endPointer;
	int h;
	int wc;
	int w_total;
	int len_total;
	Common::Point textPoint;
	Common::Point textPoint2;

	textLength = strlen(text);

	textWidth = getStringWidth(fontId, text, textLength, flags);
	fitWidth = rect.width();

	textPoint.x = rect.left + (fitWidth / 2);
	textPoint.y = rect.top;

	if (fitWidth >= textWidth) {
		// Entire string fits, draw it
		textPoint.x -= (textWidth / 2);
		draw(fontId, ds, text, textLength, textPoint, color, effectColor, flags);
		return;
	}

	// String won't fit on one line
	h = getHeight(fontId);
	w_total = 0;
	len_total = 0;
	wc = 0;

	startPointer = text;
	measurePointer = text;
	searchPointer = text;
	endPointer = text + textLength;

	for (;;) {
		foundPointer = strchr(searchPointer, ' ');
		if (foundPointer == NULL) {
			// Ran to the end of the buffer
			len = endPointer - measurePointer;
		} else {
			len = foundPointer - measurePointer;
		}

		w = getStringWidth(fontId, measurePointer, len, flags);
		measurePointer = foundPointer;

		if ((w_total + w) > fitWidth) {
			// This word won't fit
			if (wc == 0) {
				w_total = fitWidth;
				len_total = len;
			}

			// Wrap what we've got and restart
			textPoint2.x = textPoint.x - (w_total / 2);
			textPoint2.y = textPoint.y;
			draw(fontId, ds, startPointer, len_total, textPoint2, color, effectColor, flags);
			textPoint.y += h + TEXT_LINESPACING;
			if (textPoint.y >= rect.bottom) {
				return;
			}
			w_total = 0;
			len_total = 0;
			if (wc == 0) {
				searchPointer = measurePointer + 1;
			}
			wc = 0;

			// Advance the search pointer to the next non-space.
			// Otherwise, the first "word" to be measured will be
			// an empty string. Measuring or drawing a string of
			// length 0 is interpreted as measure/draw the entire
			// buffer, which certainly is not what we want here.
			//
			// This happes because a string may contain several
			// spaces in a row, e.g. after a period.

			while (*searchPointer == ' ')
				searchPointer++;

			measurePointer = searchPointer;
			startPointer = searchPointer;
		} else {
			// Word will fit ok
			w_total += w;
			len_total += len;
			wc++;
			if (foundPointer == NULL) {
				// Since word hit NULL but fit, we are done
				textPoint2.x = textPoint.x - (w_total / 2);
				textPoint2.y = textPoint.y;
				draw(fontId, ds, startPointer, len_total, textPoint2, color,
					effectColor, flags);
				return;
			}
			searchPointer = measurePointer + 1;
		}
	}
}

Font::FontId Font::knownFont2FontIdx(KnownFont font) {
	FontId fontId = kSmallFont;

	if (_vm->getGameType() == GType_ITE) {
		switch (font)
		{
		case (kKnownFontSmall):
			fontId = kSmallFont;
			break;
		case (kKnownFontMedium):
			fontId = kMediumFont;
			break;
		case (kKnownFontBig):
			fontId = kBigFont;
			break;

		case (kKnownFontVerb):
			fontId = kSmallFont;
			break;
		case (kKnownFontScript):
			fontId = kMediumFont;
			break;
		case (kKnownFontPause):
			fontId = _vm->_font->valid(kBigFont) ? kBigFont : kMediumFont;
			break;
		}
	} else if (_vm->getGameType() == GType_IHNM) {
		switch (font)
		{
		case (kKnownFontSmall):
			fontId = kSmallFont;
			break;
		case (kKnownFontMedium):
			fontId = kMediumFont;
			break;
		case (kKnownFontBig):
			fontId = kBigFont;
			break;

		case (kKnownFontVerb):
			fontId = kIHNMFont8;
			break;
		case (kKnownFontScript):
			fontId = kIHNMMainFont;
			break;
		case (kKnownFontPause):
			fontId = kMediumFont; // unchecked
			break;
		}
	}
	return fontId;
}

} // End of namespace Saga
