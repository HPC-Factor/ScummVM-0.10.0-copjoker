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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/queen/defs.h $
 * $Id: defs.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef QUEEN_DEFS_H
#define QUEEN_DEFS_H

namespace Queen {

#define SAVEGAME_SIZE	24622

enum {
	COMPRESSION_NONE  = 0,
	COMPRESSION_MP3   = 1,
	COMPRESSION_OGG   = 2,
	COMPRESSION_FLAC  = 3
};

enum {
	GAME_SCREEN_WIDTH  = 320,
	GAME_SCREEN_HEIGHT = 200,
	ROOM_ZONE_HEIGHT   = 150,
	PANEL_ZONE_HEIGHT  =  50
};

enum {
	FRAMES_JOE      = 38,
	FRAMES_JOURNAL  = 40
};

enum Direction {
	DIR_LEFT  = 1,
	DIR_RIGHT = 2,
	DIR_FRONT = 3,
	DIR_BACK  = 4
};

enum InkColor {
	INK_BG_PANEL = 0,
	INK_JOURNAL,
	INK_PINNACLE_ROOM,
	INK_CMD_SELECT,
	INK_CMD_NORMAL,
	INK_TALK_NORMAL,
	INK_JOE,
	INK_OUTLINED_TEXT,
	
	INK_COUNT
};

enum {
	ITEM_NONE                     =  0,
	ITEM_BAT,
	ITEM_JOURNAL,
	ITEM_KNIFE,
	ITEM_COCONUT_HALVES,
	ITEM_BEEF_JERKY,
	ITEM_PROPELLER,
	ITEM_BANANA,
	ITEM_VINE,
	ITEM_SLOTH_HAIR,
	ITEM_COMIC_BOOK,
	ITEM_FLOWER,
	ITEM_BEETLE,
	ITEM_ORCHID,
	ITEM_DICTIONARY,
	ITEM_DEATH_MASH,
	ITEM_PERFUME,
	ITEM_TYRANNO_HORN,
	ITEM_LOTION,
	ITEM_RECORD,
	ITEM_VACUUM_CLEANER,
	ITEM_NET,
	ITEM_ALCOHOL,
	ITEM_ROCKET_PACK,
	ITEM_SOME_MONEY,
	ITEM_CHEESE_BITZ,
	ITEM_DOG_FOOD,
	ITEM_CAN_OPENER,
	ITEM_LETTER,
	ITEM_SQUEAKY_TOY,
	ITEM_KEY,
	ITEM_BOOK,
	ITEM_PIECE_OF_PAPER,
	ITEM_ROCKET_PLAN,
	ITEM_PADLOCK_KEY,
	ITEM_RIB_CAGE,
	ITEM_SKULL,
	ITEM_LEG_BONE,
	ITEM_BAT2,
	ITEM_MAKESHIFT_TOCH,
	ITEM_LIGHTER,
	ITEM_GREEN_JEWEL,
	ITEM_PICK,
	ITEM_STONE_KEY,
	ITEM_BLUE_JEWEL,
	ITEM_CRYSTAL_SKULL,
	ITEM_TREE_SAP,
	ITEM_DINO_RAY_GUN,
	ITEM_BRANCHES,
	ITEM_WIG,
	ITEM_TOWEL,
	ITEM_OTHER_SHEET,
	ITEM_SHEET,
	ITEM_SHEET_ROPE,
	ITEM_CROWBAR,
	ITEM_COMEDY_BREASTS,
	ITEM_DRESS,
	ITEM_KEY2,
	ITEM_CLOTHES,
	ITEM_HAY,
	ITEM_OIL,
	ITEM_CHICKEN,
	ITEM_LIT_TORCH,
	ITEM_OPENED_DOG_FOOD,
	ITEM_SOME_MONEY2,
	ITEM_SOME_MORE_MONEY,
	ITEM_PEELED_BANANA,
	ITEM_STONE_DISC,
	ITEM_GNARLED_VINE,
	ITEM_FLINT,
	ITEM_LIGHTER2,
	ITEM_REST_OF_BEEF_JERKY,
	ITEM_LOTS_OF_MONEY,
	ITEM_HEAPS_OF_MONEY,
	ITEM_OPEN_BOOK,
	ITEM_REST_OF_THE_CHEESE_BITZ,
	ITEM_SCISSORS,
	ITEM_PENCIL,
	ITEM_SUPER_WEENIE_SERUM,
	ITEM_MUMMY_WRAPPINGS,
	ITEM_COCONUT,
	ITEM_ID_CARD,
	ITEM_BIT_OF_STONE,
	ITEM_CHUNK_OF_ROCK,
	ITEM_BIG_STICK,
	ITEM_STICKY_BIT_OF_STONE,
	ITEM_STICKY_CHUNK_OF_ROCK,
	ITEM_DEATH_MASK2,
	ITEM_CHEFS_SURPRISE,
	ITEM_STICKY_BAT,
	ITEM_REST_OF_WRAPPINGS,
	ITEM_BANANA2,
	ITEM_MUG,
	ITEM_FILE,
	ITEM_POCKET_ROCKET_BLUEPRINTS,
	ITEM_HAND_PUPPET,
	ITEM_ARM_BONE,
	ITEM_CROWN,
	ITEM_COMIC_COUPON,
	ITEM_TORN_PAGE
};

enum {
	ROOM_JUNGLE_INSIDE_PLANE  =   1,
	ROOM_JUNGLE_OUTSIDE_PLANE =   2,
	ROOM_JUNGLE_BRIDGE        =   4,
	ROOM_JUNGLE_GORILLA_1     =   6,
	ROOM_JUNGLE_PINNACLE      =   7,
	ROOM_JUNGLE_SLOTH         =   8,
	ROOM_JUNGLE_BUD_SKIP      =   9,
	ROOM_JUNGLE_BEETLE        =  11,
	ROOM_JUNGLE_MISSIONARY    =  13,
	ROOM_JUNGLE_GORILLA_2     =  14,

	ROOM_AMAZON_ENTRANCE      =  16,
	ROOM_AMAZON_HIDEOUT       =  17,
	ROOM_AMAZON_THRONE        =  18,
	ROOM_AMAZON_JAIL          =  19,

	ROOM_VILLAGE              =  20,
	ROOM_TRADER_BOBS          =  21,

	ROOM_FLODA_OUTSIDE        =  22,
	ROOM_FLODA_KITCHEN        =  26,
	ROOM_FLODA_LOCKERROOM     =  27,
	ROOM_FLODA_KLUNK          =  30,
	ROOM_FLODA_HENRY          =  32,
	ROOM_FLODA_OFFICE         =  35,
	ROOM_FLODA_JAIL           =  41,
	ROOM_FLODA_FRONTDESK      = 103,

	ROOM_TEMPLE_OUTSIDE       =  43,
	ROOM_TEMPLE_MUMMIES       =  46,
	ROOM_TEMPLE_ZOMBIES       =  50,
	ROOM_TEMPLE_TREE          =  51,
	ROOM_TEMPLE_SNAKE         =  53,
	ROOM_TEMPLE_LIZARD_LASER  =  55,
	ROOM_TEMPLE_MAZE          =  58,
	ROOM_TEMPLE_MAZE_2        =  59,
	ROOM_TEMPLE_MAZE_3        =  60,
	ROOM_TEMPLE_MAZE_4        =  61,
	ROOM_TEMPLE_MAZE_5        = 100,
	ROOM_TEMPLE_MAZE_6        = 101,

	ROOM_VALLEY_CARCASS       =  67,

	ROOM_HOTEL_UPSTAIRS       =  70,
	ROOM_HOTEL_DOWNSTAIRS     =  71,
	ROOM_HOTEL_LOLA           =  72,
	ROOM_HOTEL_LOBBY          =  73,

	ROOM_CAR_CHASE            =  74,

	ROOM_FINAL_FIGHT          =  69,

	ROOM_INTRO_RITA_JOE_HEADS = 116,
	ROOM_INTRO_EXPLOSION      = 123,

	//special
	SPARKY_OUTSIDE_HOTEL      =  77,
	DEATH_MASK                =  79,
	IBI_LOGO                  =  82,
	COMIC_1                   =  87,
	COMIC_2                   =  88,
	COMIC_3                   =  89,
	ROOM_UNUSED_INTRO_1       =  90,
	ROOM_UNUSED_INTRO_2       =  91,
	ROOM_UNUSED_INTRO_3       =  92,
	ROOM_UNUSED_INTRO_4       =  93,
	ROOM_UNUSED_INTRO_5       =  94,
	FOTAQ_LOGO                =  95,
	WARNER_LOGO               = 126,

	FAYE_HEAD                 =  37,
	AZURA_HEAD                = 106,
	FRANK_HEAD                = 107,

	ROOM_ENDING_CREDITS       = 110,

	ROOM_JOURNAL              = 200 // dummy value to keep Display methods happy
};

//! GameState vars
enum {
	VAR_HOTEL_ITEMS_REMOVED       =   3,
	VAR_JOE_DRESSING_MODE         =  19,
	VAR_BYPASS_ZOMBIES            =  21,
	VAR_BYPASS_FLODA_RECEPTIONIST =  35,
	VAR_GUARDS_TURNED_ON          =  85,
	VAR_HOTEL_ESCAPE_STATE        =  93,
	VAR_INTRO_PLAYED              = 117,
	VAR_AZURA_IN_LOVE             = 167
};

enum Verb {
	VERB_NONE = 0,

	VERB_PANEL_COMMAND_FIRST = 1,
	VERB_OPEN        = 1,
	VERB_CLOSE       = 2,
	VERB_MOVE        = 3,
	// no verb 4
	VERB_GIVE        = 5,
	VERB_USE         = 6,
	VERB_PICK_UP     = 7,
	VERB_LOOK_AT     = 9,
	VERB_TALK_TO     = 8,
	VERB_PANEL_COMMAND_LAST = 9,

	VERB_WALK_TO     = 10,
	VERB_SCROLL_UP   = 11,
	VERB_SCROLL_DOWN = 12,

	VERB_DIGIT_FIRST = 13,
	VERB_DIGIT_1   = 13,
	VERB_DIGIT_2   = 14,
	VERB_DIGIT_3   = 15,
	VERB_DIGIT_4   = 16,
	VERB_DIGIT_LAST = 16,

	VERB_INV_FIRST = VERB_DIGIT_FIRST,
	VERB_INV_1 = VERB_DIGIT_1,
	VERB_INV_2 = VERB_DIGIT_2,
	VERB_INV_3 = VERB_DIGIT_3,
	VERB_INV_4 = VERB_DIGIT_4,
	VERB_INV_LAST = VERB_DIGIT_LAST,

	VERB_USE_JOURNAL = 20,
	VERB_SKIP_TEXT   = 101,

	VERB_PREP_WITH = 11,
	VERB_PREP_TO   = 12
};

} // End of namespace Queen

#endif
