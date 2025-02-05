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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/base/commandLine.cpp $
 * $Id: commandLine.cpp 27355 2007-06-11 08:41:26Z sev $
 *
 */

#include "common/stdafx.h"

#include "engines/engine.h"
#include "base/commandLine.h"
#include "base/plugins.h"
#include "base/version.h"

#include "common/config-manager.h"
#include "common/system.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#ifdef UNIX
#ifdef MACOSX
#define DEFAULT_SAVE_PATH "Documents/ScummVM Savegames"
#else
#define DEFAULT_SAVE_PATH ".scummvm"
#endif
#elif defined(__SYMBIAN32__)
#define DEFAULT_SAVE_PATH "Savegames"
#endif

#define DETECTOR_TESTING_HACK

#ifdef DETECTOR_TESTING_HACK
#include "common/fs.h"
#endif

namespace Base {

static const char USAGE_STRING[] =
	"%s: %s\n"
	"Usage: %s [OPTIONS]... [GAME]\n"
	"\n"
	"Try '%s --help' for more options.\n"
;

// DONT FIXME: DO NOT ORDER ALPHABETICALLY, THIS IS ORDERED BY IMPORTANCE/CATEGORY! :)
#if defined(PALMOS_MODE) || defined(__SYMBIAN32__) || defined(__GP32__)
static const char HELP_STRING[] = "NoUsageString"; // save more data segment space
#else
static const char HELP_STRING[] =
	"ScummVM - Graphical Adventure Game Interpreter\n"
	"Usage: %s [OPTIONS]... [GAME]\n"
	"  -v, --version            Display ScummVM version information and exit\n"
	"  -h, --help               Display a brief help text and exit\n"
	"  -z, --list-games         Display list of supported games and exit\n"
	"  -t, --list-targets       Display list of configured targets and exit\n"
	"\n"
	"  -c, --config=CONFIG      Use alternate configuration file\n"
	"  -p, --path=PATH          Path to where the game is installed\n"
	"  -x, --save-slot[=NUM]    Save game slot to load (default: autosave)\n"
	"  -f, --fullscreen         Force full-screen mode\n"
	"  -F, --no-fullscreen      Force windowed mode\n"
	"  -g, --gfx-mode=MODE      Select graphics scaler (1x,2x,3x,2xsai,super2xsai,\n"
	"                           supereagle,advmame2x,advmame3x,hq2x,hq3x,tv2x,\n"
	"                           dotmatrix)\n"
	"  --gui-theme=THEME        Select GUI theme (default, modern, classic)\n"
	"  --themepath=PATH         Path to where GUI themes are stored\n"
	"  -e, --music-driver=MODE  Select music driver (see README for details)\n"
	"  -q, --language=LANG      Select language (en,de,fr,it,pt,es,jp,zh,kr,se,gb,\n"
	"                           hb,ru,cz)\n"
	"  -m, --music-volume=NUM   Set the music volume, 0-255 (default: 192)\n"
	"  -s, --sfx-volume=NUM     Set the sfx volume, 0-255 (default: 192)\n"
	"  -r, --speech-volume=NUM  Set the speech volume, 0-255 (default: 192)\n"
	"  --midi-gain=NUM          Set the gain for MIDI playback, 0-1000 (default:\n"
	"                           100) (only supported by some MIDI drivers)\n"
	"  -n, --subtitles          Enable subtitles (use with games that have voice)\n"
	"  -b, --boot-param=NUM     Pass number to the boot script (boot param)\n"
	"  -d, --debuglevel=NUM     Set debug verbosity level\n"
	"  --debugflags=FLAGS       Enables engine specific debug flags\n"
	"                           (separated by commas)\n"
	"  -u, --dump-scripts       Enable script dumping if a directory called 'dumps'\n"
	"                           exists in the current directory\n"
	"\n"
	"  --cdrom=NUM              CD drive to play CD audio from (default: 0 = first\n"
	"                           drive)\n"
	"  --joystick[=NUM]         Enable input with joystick (default: 0 = first\n"
	"                           joystick)\n"
	"  --platform=WORD          Specify platform of game (allowed values: 2gs, 3do,\n"
	"                           acorn, amiga, atari, c64, fmtowns, nes, mac, pc,\n"
	"                           pce, segacd, windows)\n"
	"  --savepath=PATH          Path to where savegames are stored\n"
	"  --soundfont=FILE         Select the SoundFont for MIDI playback (only\n"
	"                           supported by some MIDI drivers)\n"
	"  --multi-midi             Enable combination Adlib and native MIDI\n"
	"  --native-mt32            True Roland MT-32 (disable GM emulation)\n"
	"  --enable-gs              Enable Roland GS mode for MIDI playback\n"
	"  --output-rate=RATE       Select output sample rate in Hz (e.g. 22050)\n"
	"  --aspect-ratio           Enable aspect ratio correction\n"
	"  --render-mode=MODE       Enable additional render modes (cga, ega, hercGreen,\n"
	"                           hercAmber, amiga)\n"
	"\n"
#if !defined(DISABLE_SKY) || !defined(DISABLE_QUEEN)
	"  --alt-intro              Use alternative intro for CD versions of Beneath a\n"
	"                           Steel Sky and Flight of the Amazon Queen\n"
#endif
	"  --copy-protection        Enable copy protection in SCUMM games, when\n"
	"                           ScummVM disables it by default.\n"
	"  --talkspeed=NUM          Set talk speed for games (default: 60)\n"
#ifndef DISABLE_SCUMM
	"  --demo-mode              Start demo mode of Maniac Mansion\n"
	"  --tempo=NUM              Set music tempo (in percent, 50-200) for SCUMM games\n"
	"                           (default: 100)\n"
#endif
	"\n"
	"The meaning of boolean long options can be inverted by prefixing them with\n"
	"\"no-\", e.g. \"--no-aspect-ratio\".\n"
;
#endif

static const char *s_appName = "scummvm";

static void usage(const char *s, ...) GCC_PRINTF(1, 2);

static void usage(const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__) || defined (__SYMBIAN32__))
	printf(USAGE_STRING, s_appName, buf, s_appName, s_appName);
#endif
	exit(1);
}

void registerDefaults() {

	// Graphics
	ConfMan.registerDefault("fullscreen", false);
	ConfMan.registerDefault("aspect_ratio", false);
	ConfMan.registerDefault("gfx_mode", "normal");
	ConfMan.registerDefault("render_mode", "default");

	// Sound & Music
	ConfMan.registerDefault("music_volume", 192);
	ConfMan.registerDefault("sfx_volume", 192);
	ConfMan.registerDefault("speech_volume", 192);

	ConfMan.registerDefault("music_mute", false);
	ConfMan.registerDefault("sfx_mute", false);
	ConfMan.registerDefault("speech_mute", false);

	ConfMan.registerDefault("multi_midi", false);
	ConfMan.registerDefault("native_mt32", false);
	ConfMan.registerDefault("enable_gs", false);
	ConfMan.registerDefault("midi_gain", 100);
//	ConfMan.registerDefault("music_driver", ???);

	ConfMan.registerDefault("cdrom", 0);

	// Game specific
	ConfMan.registerDefault("path", "");
	ConfMan.registerDefault("savepath", "");

//	ConfMan.registerDefault("amiga", false);
	ConfMan.registerDefault("platform", Common::kPlatformPC);
	ConfMan.registerDefault("language", "en");
	ConfMan.registerDefault("subtitles", false);
	ConfMan.registerDefault("boot_param", 0);
	ConfMan.registerDefault("dump_scripts", false);
	ConfMan.registerDefault("save_slot", -1);
	ConfMan.registerDefault("autosave_period", 5 * 60);	// By default, trigger autosave every 5 minutes

#if !defined(DISABLE_SCUMM) || !defined(DISABLE_SWORD2)
	ConfMan.registerDefault("object_labels", true);
#endif

	ConfMan.registerDefault("copy_protection", false);
	ConfMan.registerDefault("talkspeed", 60);

#ifndef DISABLE_SCUMM
	ConfMan.registerDefault("demo_mode", false);
	ConfMan.registerDefault("tempo", 0);
#endif

#if !defined(DISABLE_SKY) || !defined(DISABLE_QUEEN)
	ConfMan.registerDefault("alt_intro", false);
#endif

	// Miscellaneous
	ConfMan.registerDefault("joystick_num", -1);
	ConfMan.registerDefault("confirm_exit", false);
	ConfMan.registerDefault("disable_sdl_parachute", false);
#ifdef USE_ALSA
	ConfMan.registerDefault("alsa_port", "65:0");
#endif

	// Register default savepath
#ifdef DEFAULT_SAVE_PATH
	char savePath[MAXPATHLEN];
#ifdef UNIX
	const char *home = getenv("HOME");
	if (home && *home && strlen(home) < MAXPATHLEN) {
		snprintf(savePath, MAXPATHLEN, "%s/%s", home, DEFAULT_SAVE_PATH);
		ConfMan.registerDefault("savepath", savePath);
	}
#elif defined(__SYMBIAN32__)
	strcpy(savePath, Symbian::GetExecutablePath());
	strcat(savePath, DEFAULT_SAVE_PATH);
	ConfMan.registerDefault("savepath", savePath);
#endif
#endif // #ifdef DEFAULT_SAVE_PATH
}

//
// Various macros used by the command line parser.
//

// Use this for options which have an *optional* value
#define DO_OPTION_OPT(shortCmd, longCmd, defaultVal) \
	if (isLongCmd ? (!strcmp(s+2, longCmd) || !memcmp(s+2, longCmd"=", sizeof(longCmd"=") - 1)) : (tolower(s[1]) == shortCmd)) { \
		s += 2; \
		if (isLongCmd) { \
			s += sizeof(longCmd) - 1; \
			if (*s == '=') \
				s++; \
		} \
		const char *option = s; \
		if (*s == '\0' && !isLongCmd) { option = s2; i++; } \
		if (!option || *option == '\0') option = defaultVal; \
		if (option) settings[longCmd] = option;

// Use this for options which have a required (string) value
#define DO_OPTION(shortCmd, longCmd) \
	DO_OPTION_OPT(shortCmd, longCmd, 0) \
	if (!option) usage("Option '%s' requires an argument", argv[isLongCmd ? i : i-1]);

// Use this for options which have a required integer value
#define DO_OPTION_INT(shortCmd, longCmd) \
	DO_OPTION(shortCmd, longCmd) \
	char *endptr = 0; \
	int intValue; intValue = (int)strtol(option, &endptr, 0); \
	if (endptr == NULL || *endptr != 0) usage("--%s: Invalid number '%s'", longCmd, option);

// Use this for boolean options; this distinguishes between "-x" and "-X",
// resp. between "--some-option" and "--no-some-option".
#define DO_OPTION_BOOL(shortCmd, longCmd) \
	if (isLongCmd ? (!strcmp(s+2, longCmd) || !strcmp(s+2, "no-"longCmd)) : (tolower(s[1]) == shortCmd)) { \
		bool boolValue = (islower(s[1]) != 0); \
		s += 2; \
		if (isLongCmd) { \
			boolValue = !strcmp(s, longCmd); \
			s += boolValue ? (sizeof(longCmd) - 1) : (sizeof("no-"longCmd) - 1); \
		} \
		if (*s != '\0') goto unknownOption; \
		const char *option = boolValue ? "true" : "false"; \
		settings[longCmd] = option;

// Use this for options which never have a value, i.e. for 'commands', like "--help".
#define DO_COMMAND(shortCmd, longCmd) \
	if (isLongCmd ? (!strcmp(s+2, longCmd)) : (tolower(s[1]) == shortCmd)) { \
		s += 2; \
		if (isLongCmd) \
			s += sizeof(longCmd) - 1; \
		if (*s != '\0') goto unknownOption; \
		return longCmd;


#define DO_LONG_OPTION_OPT(longCmd, d)  DO_OPTION_OPT(0, longCmd, d)
#define DO_LONG_OPTION(longCmd)         DO_OPTION(0, longCmd)
#define DO_LONG_OPTION_INT(longCmd)     DO_OPTION_INT(0, longCmd)
#define DO_LONG_OPTION_BOOL(longCmd)    DO_OPTION_BOOL(0, longCmd)
#define DO_LONG_COMMAND(longCmd)        DO_COMMAND(0, longCmd)

// End an option handler
#define END_OPTION \
		continue; \
	}


Common::String parseCommandLine(Common::StringMap &settings, int argc, char **argv) {
	const char *s, *s2;
	
	// argv[0] contains the name of the executable.
	if (argv && argv[0]) {
		s = strrchr(argv[0], '/');
		s_appName = s ? (s+1) : argv[0];
	}

	// We store all command line settings into a string map.

	// Iterate over all command line arguments and parse them into our string map.
	for (int i = 1; i < argc; ++i) {
		s = argv[i];
		s2 = (i < argc-1) ? argv[i+1] : 0;

		if (s[0] != '-') {
			// The argument doesn't start with a dash, so it's not an option.
			// Hence it must be the target name. We currently enforce that
			// this always comes last.
			if (i != argc - 1)
				usage("Stray argument '%s'", s);

			// We defer checking whether this is a valid target to a later point.
			return s;
		} else {

			bool isLongCmd = (s[0] == '-' && s[1] == '-');

			DO_COMMAND('h', "help")
			END_OPTION

			DO_COMMAND('v', "version")
			END_OPTION

			DO_COMMAND('t', "list-targets")
			END_OPTION

			DO_COMMAND('z', "list-games")
			END_OPTION

			// HACK FIXME TODO: This command is intentionally *not* documented!
			DO_LONG_COMMAND("test-detector")
			END_OPTION


			DO_OPTION('c', "config")
			END_OPTION

			DO_OPTION_INT('b', "boot-param")
			END_OPTION

			DO_OPTION_OPT('d', "debuglevel", "0")
			END_OPTION

			DO_LONG_OPTION("debugflags")
			END_OPTION

			DO_OPTION('e', "music-driver")
				if (MidiDriver::parseMusicDriver(option) < 0)
					usage("Unrecognized music driver '%s'", option);
			END_OPTION

			DO_LONG_OPTION_INT("output-rate")
			END_OPTION

			DO_OPTION_BOOL('f', "fullscreen")
			END_OPTION

			DO_OPTION('g', "gfx-mode")
				// Check whether 'option' specifies a valid graphics mode.
				bool isValid = false;
				if (!scumm_stricmp(option, "normal") || !scumm_stricmp(option, "default"))
					isValid = true;
				if (!isValid) {
					const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();
					while (gm->name && !isValid) {
						isValid = !scumm_stricmp(gm->name, option);
						gm++;
					}
				}
				if (!isValid)
					usage("Unrecognized graphics mode '%s'", option);
			END_OPTION

			DO_OPTION_INT('m', "music-volume")
			END_OPTION

			DO_OPTION_BOOL('n', "subtitles")
			END_OPTION

			DO_OPTION('p', "path")
				// TODO: Verify whether the path is valid
			END_OPTION

			DO_OPTION('q', "language")
				if (Common::parseLanguage(option) == Common::UNK_LANG)
					usage("Unrecognized language '%s'", option);
			END_OPTION

			DO_OPTION_INT('s', "sfx-volume")
			END_OPTION

			DO_OPTION_INT('r', "speech-volume")
			END_OPTION

			DO_LONG_OPTION_INT("midi-gain")
			END_OPTION

			DO_OPTION_BOOL('u', "dump-scripts")
			END_OPTION

			DO_OPTION_OPT('x', "save-slot", "0")
			END_OPTION

			DO_LONG_OPTION_INT("cdrom")
			END_OPTION

			DO_LONG_OPTION_OPT("joystick", "0")
				settings["joystick_num"] = option;
				settings.erase("joystick");
			END_OPTION

			DO_LONG_OPTION("platform")
				int platform = Common::parsePlatform(option);
				if (platform == Common::kPlatformUnknown)
					usage("Unrecognized platform '%s'", option);
			END_OPTION

			DO_LONG_OPTION("soundfont")
				// TODO: Verify whether the path is valid
			END_OPTION

			DO_LONG_OPTION_BOOL("disable-sdl-parachute")
			END_OPTION

			DO_LONG_OPTION_BOOL("multi-midi")
			END_OPTION

			DO_LONG_OPTION_BOOL("native-mt32")
			END_OPTION

			DO_LONG_OPTION_BOOL("enable-gs")
			END_OPTION

			DO_LONG_OPTION_BOOL("aspect-ratio")
			END_OPTION

			DO_LONG_OPTION("render-mode")
				int renderMode = Common::parseRenderMode(option);
				if (renderMode == Common::kRenderDefault)
					usage("Unrecognized render mode '%s'", option);
			END_OPTION

			DO_LONG_OPTION("savepath")
				// TODO: Verify whether the path is valid
			END_OPTION

			DO_LONG_OPTION_INT("talkspeed")
			END_OPTION

			DO_LONG_OPTION_BOOL("copy-protection")
			END_OPTION

			DO_LONG_OPTION("gui-theme")
			END_OPTION

			DO_LONG_OPTION("themepath")
				// TODO: Verify whether the path is valid
			END_OPTION

			DO_LONG_OPTION("target-md5")
			END_OPTION

#ifndef DISABLE_SCUMM
			DO_LONG_OPTION_INT("tempo")
			END_OPTION

			DO_LONG_OPTION_BOOL("demo-mode")
			END_OPTION
#endif

#if !defined(DISABLE_SKY) || !defined(DISABLE_QUEEN)
			DO_LONG_OPTION_BOOL("alt-intro")
			END_OPTION
#endif

unknownOption:
			// If we get till here, the option is unhandled and hence unknown.
			usage("Unrecognized option '%s'", argv[i]);
		}
	}


	return Common::String::emptyString;
}

/** List all supported game IDs, i.e. all games which any loaded plugin supports. */
static void listGames() {
	printf("Game ID              Full Title                                            \n"
	       "-------------------- ------------------------------------------------------\n");

	const PluginList &plugins = PluginManager::instance().getPlugins();
	PluginList::const_iterator iter = plugins.begin();
	for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
		GameList list = (*iter)->getSupportedGames();
		for (GameList::iterator v = list.begin(); v != list.end(); ++v) {
			printf("%-20s %s\n", v->gameid().c_str(), v->description().c_str());
		}
	}
}

/** List all targets which are configured in the config file. */
static void listTargets() {
	printf("Target               Description                                           \n"
	       "-------------------- ------------------------------------------------------\n");

	using namespace Common;
	const ConfigManager::DomainMap &domains = ConfMan.getGameDomains();
	ConfigManager::DomainMap::const_iterator iter = domains.begin();
	for (iter = domains.begin(); iter != domains.end(); ++iter) {
		Common::String name(iter->_key);
		Common::String description(iter->_value.get("description"));

		if (description.empty()) {
			// FIXME: At this point, we should check for a "gameid" override
			// to find the proper desc. In fact, the platform probably should
			// be taken into account, too.
			Common::String gameid(name);
			GameDescriptor g = Base::findGame(gameid);
			if (g.description().size() > 0)
				description = g.description();
		}

		printf("%-20s %s\n", name.c_str(), description.c_str());
	}
}

#ifdef DETECTOR_TESTING_HACK
static void runDetectorTest() {
	// HACK: The following code can be used to test the detection code of our
	// engines. Basically, it loops over all targets, and calls the detector
	// for the given path. It then prints out the result and also checks
	// whether the result agrees with the settings of the target.
	
	const Common::ConfigManager::DomainMap &domains = ConfMan.getGameDomains();
	Common::ConfigManager::DomainMap::const_iterator iter = domains.begin();
	int success = 0, failure = 0;
	for (iter = domains.begin(); iter != domains.end(); ++iter) {
		Common::String name(iter->_key);
		Common::String gameid(iter->_value.get("gameid"));
		Common::String path(iter->_value.get("path"));
		printf("Looking at target '%s', gameid '%s', path '%s' ...\n",
				name.c_str(), gameid.c_str(), path.c_str());
		if (path.empty()) {
			printf(" ... no path specified, skipping\n");
			continue;
		}
		if (gameid.empty()) {
			gameid = name;
		}
		
		FilesystemNode dir(path);
		FSList files;
		if (!dir.listDir(files, FilesystemNode::kListAll)) {
			printf(" ... invalid path, skipping\n");
			continue;
		}

		GameList candidates(PluginManager::instance().detectGames(files));
		bool gameidDiffers = false;
		GameList::iterator x;
		for (x = candidates.begin(); x != candidates.end(); ++x) {
			gameidDiffers |= (scumm_stricmp(gameid.c_str(), x->gameid().c_str()) != 0);
		}
		
		if (candidates.empty()) {
			printf(" FAILURE: No games detected\n");
			failure++;
		} else if (candidates.size() > 1) {
			if (gameidDiffers) {
				printf(" FAILURE: Multiple games detected, some/all with wrong gameid\n");
			} else {
				printf(" FAILURE: Multiple games detected, but all have the same gameid\n");
			}
			failure++;
		} else if (gameidDiffers) {
			printf(" FAILURE: Wrong gameid detected\n");
			failure++;
		} else {
			printf(" SUCCESS: Game was detected correctly\n");
			success++;
		}
		
		for (x = candidates.begin(); x != candidates.end(); ++x) {
			printf("    gameid '%s', desc '%s', language '%s', platform '%s'\n",
				   x->gameid().c_str(),
				   x->description().c_str(),
				   Common::getLanguageCode(x->language()),
				   Common::getPlatformCode(x->platform()));
		}
	}
	int total = domains.size();
	printf("Detector test run: %d fail, %d success, %d skipped, out of %d\n",
			failure, success, total - failure - success, total);
}
#endif

bool processSettings(Common::String &command, Common::StringMap &settings) {

	// Handle commands passed via the command line (like --list-targets and
	// --list-games). This must be done after the config file and the plugins
	// have been loaded.
	// FIXME: The way are are doing this is rather arbitrary at this time.
	// E.g. --version and --help are very similar, but are still handled
	// inside parseCommandLine. This should be unified.
	if (command == "list-targets") {
		listTargets();
		return false;
	} else if (command == "list-games") {
		listGames();
		return false;
	} else if (command == "version") {
		printf("%s\n", gScummVMFullVersion);
		printf("Features compiled in: %s\n", gScummVMFeatures);
		return false;
	} else if (command == "help") {
		printf(HELP_STRING, s_appName);
		return false;
	}
#ifdef DETECTOR_TESTING_HACK
	else if (command == "test-detector") {
		runDetectorTest();
		return false;
	}
#endif


	// If a target was specified, check whether there is either a game
	// domain (i.e. a target) matching this argument, or alternatively
	// whether there is a gameid matching that name.
	if (!command.empty()) {
		GameDescriptor gd = Base::findGame(command);
		if (ConfMan.hasGameDomain(command) || !gd.gameid().empty()) {
			bool idCameFromCommandLine = false;

			// WORKAROUND: Fix for bug #1719463: "DETECTOR: Launching
			// undefined target adds launcher entry"
			//
			// We designate gameids which come strictly from command line
			// so AdvancedDetector will not save config file with invalid
			// gameid in case target autoupgrade was performed
			if (!ConfMan.hasGameDomain(command)) {
				idCameFromCommandLine = true;
			}

			ConfMan.setActiveDomain(command);

			if (idCameFromCommandLine)
				ConfMan.set("id_came_from_command_line", "1");

		} else {
			usage("Unrecognized game target '%s'", command.c_str());
		}
	}
	

	// The user can override the savepath with the SCUMMVM_SAVEPATH
	// environment variable. This is weaker than a --savepath on the
	// command line, but overrides the default savepath, hence it is
	// handled here, just before the command line gets parsed.
#if !defined(MACOS_CARBON) && !defined(_WIN32_WCE) && !defined(PALMOS_MODE) && !defined(__GP32__)
	if (!settings.contains("savepath")) {
		const char *dir = getenv("SCUMMVM_SAVEPATH");
		if (dir && *dir && strlen(dir) < MAXPATHLEN) {
			// TODO: Verify whether the path is valid
			settings["savepath"] = dir;
		}
	}
#endif


	// Finally, store the command line settings into the config manager.
	for (Common::StringMap::const_iterator x = settings.begin(); x != settings.end(); ++x) {
		Common::String key(x->_key);
		Common::String value(x->_value);

		// Replace any "-" in the key by "_" (e.g. change "save-slot" to "save_slot").
		for (Common::String::iterator c = key.begin(); c != key.end(); ++c)
			if (*c == '-')
				*c = '_';
		
		// Store it into ConfMan.
		ConfMan.set(key, value, Common::ConfigManager::kTransientDomain);
	}

	return true;
}

} // End of namespace Base
