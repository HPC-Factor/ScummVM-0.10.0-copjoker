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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/platform/gp2x/gp2x-hw.h $
 * $Id: gp2x-hw.h 27252 2007-06-09 17:29:47Z djwillis $
 *
 */

/*
 * GP2X: Hardware Stuff.
 *
 */

#ifndef GP2X_HW_H
#define GP2X_HW_H

//GP2X Main Joystick Mappings
//TODO: Use these more standard mappings over the custom ones.

#define GP2X_BUTTON_UP              (0)
#define GP2X_BUTTON_UPLEFT          (1)
#define GP2X_BUTTON_LEFT            (2)
#define GP2X_BUTTON_DOWNLEFT        (3)
#define GP2X_BUTTON_DOWN            (4)
#define GP2X_BUTTON_DOWNRIGHT       (5)
#define GP2X_BUTTON_RIGHT           (6)
#define GP2X_BUTTON_UPRIGHT         (7)
#define GP2X_BUTTON_START           (8)
#define GP2X_BUTTON_SELECT          (9)
#define GP2X_BUTTON_L               (10)
#define GP2X_BUTTON_R               (11)
#define GP2X_BUTTON_A               (12)
#define GP2X_BUTTON_B               (13)
#define GP2X_BUTTON_X               (14)
#define GP2X_BUTTON_Y               (15)
#define GP2X_BUTTON_VOLUP           (16)
#define GP2X_BUTTON_VOLDOWN         (17)
#define GP2X_BUTTON_CLICK           (18)

#define GP2X_MAXVOL 100 // Highest level permitted by GP2X's mixer
#define SYS_CLK_FREQ 7372800 // Clock Frequency

extern void	GP2X_device_init();
extern void	GP2X_device_deinit();
extern void	GP2X_mixer_set_volume(int, int);
extern int	GP2X_mixer_get_volume();
extern void	GP2X_mixer_move_volume(int);
extern void GP2X_setCpuspeed(unsigned int cpuspeed);
extern int GP2X_getBattLevel();

extern void save_system_regs(void); /* save some registers */
extern void set_display_clock_div(unsigned div);
extern void set_FCLK(unsigned MHZ); /* adjust the clock frequency (in Mhz units) */
extern void set_920_Div(unsigned short div); /* 0 to 7 divider (freq=FCLK/(1+div)) */
extern void set_DCLK_Div(unsigned short div); /* 0 to 7 divider (freq=FCLK/(1+div)) */
extern void Disable_940(void); /* 940t down */
extern void gp2x_video_wait_vsync(void);

#endif //GP2X_HW_H
