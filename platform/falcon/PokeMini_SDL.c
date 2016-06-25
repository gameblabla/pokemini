/*
  PokeMini - Pokémon-Mini Emulator
  Copyright (C) 2009-2015  JustBurn

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "SDL.h"

#include "PokeMini.h"
#include "Hardware.h"
#include "Keyboard.h"
#include "KeybMapSDL.h"

#include "Video_x1.h"
#include "Video_x2.h"
#include "Video_x3.h"
#include "PokeMini_BG2.h"
#include "PokeMini_BG3.h"
const char *AppName = "PokeMini " PokeMini_Version " SDL";

int emurunning = 1;
SDL_Surface *screen;
int PMWidth, PMHeight;
int PixPitch, PMOff, UIOff;

unsigned char only_one = 0;

unsigned short zone_update[6] = { 0, 0, 288, 192, 288, 192} ;

FILE *sdump;
void setup_screen();
void render_menu();

// Sound buffer size
#define SOUNDBUFFER	0
#define PMSNDBUFFER	0

const char *clc_zoom_txt[] = {
	"0x (Illegal)",
	"1x ( 96x 64)",
	"2x (192x128)",
	"3x (288x192)",
	"4x (384x256)",
	"5x (480x320)",
	"6x (576x384)",
};

// Custom command line (NEW IN 0.5.0)
int clc_zoom = 1, clc_bpp = 16, clc_fullscreen = 0;
char clc_dump_sound[PMTMPV] = {0};
int clc_displayfps = 0;
const TCommandLineCustom CustomArgs[] = {
	{ "-dumpsound", (int *)&clc_dump_sound, COMMANDLINE_STR, PMTMPV-1 },
	{ "-zoom", &clc_zoom, COMMANDLINE_INT, 1, 3 },
	{ "-bpp", &clc_bpp, COMMANDLINE_INT, 16, 16 },
	{ "-windowed", &clc_fullscreen, COMMANDLINE_INTSET, 0 },
	{ "-fullscreen", &clc_fullscreen, COMMANDLINE_INTSET, 1 },
	{ "-displayfps", &clc_displayfps, COMMANDLINE_INTSET, 1 },
	{ "", NULL, COMMANDLINE_EOL }
};
const TCommandLineCustom CustomConf[] = {
	{ "zoom", &clc_zoom, COMMANDLINE_INT, 1, 3 },
	{ "bpp", &clc_bpp, COMMANDLINE_INT, 16, 16 },
	{ "fullscreen", &clc_fullscreen, COMMANDLINE_BOOL },
	{ "displayfps", &clc_displayfps, COMMANDLINE_BOOL },
	{ "", NULL, COMMANDLINE_EOL }
};

// Platform menu (REQUIRED >= 0.4.4)
int UIItems_PlatformC(int index, int reason);
TUIMenu_Item UIItems_Platform[] = {
	PLATFORMDEF_GOBACK,
	{ 0,  1, "Zoom: %s", UIItems_PlatformC },
	{ 0,  9, "Define Keyboard...", UIItems_PlatformC },
	PLATFORMDEF_SAVEOPTIONS,
	PLATFORMDEF_END(UIItems_PlatformC)
};

int UIItems_PlatformC(int index, int reason)
{
	int zoomchanged = 0;
	if (reason == UIMENU_OK) {
		reason = UIMENU_RIGHT;
	}
	if (reason == UIMENU_CANCEL) {
		UIMenu_PrevMenu();
	}
	if (reason == UIMENU_LEFT) {
		switch (index) {
			case 1: // Zoom
				clc_zoom--;
				if (clc_zoom < 1) clc_zoom = 3;
				zoomchanged = 1;
				break;
			case 2: // Bits-Per-Pixel
				clc_bpp = 16;
				zoomchanged = 1;
				break;
		}
	}
	if (reason == UIMENU_RIGHT) {
		switch (index) {
			case 1: // Zoom
				clc_zoom++;
				if (clc_zoom > 3) clc_zoom = 1;
				zoomchanged = 1;
				break;
			case 2: // Bits-Per-Pixel
				clc_bpp = 16;
				zoomchanged = 1;
				break;
			case 9: // Define Keyboard...
				KeyboardEnterMenu();
				break;
		}
	}
	UIMenu_ChangeItem(UIItems_Platform, 1, "Zoom: %s", clc_zoom_txt[clc_zoom]);
	UIMenu_ChangeItem(UIItems_Platform, 2, "Depth: %dbpp", clc_bpp);
	if (zoomchanged) {
		SDL_UnlockSurface(screen);
		setup_screen();
		SDL_LockSurface(screen);
		return 0;
	}
	return 1;
}

// Setup screen
void setup_screen()
{
	TPokeMini_VideoSpec *videospec;
	int depth, PMOffX, PMOffY, UIOffX, UIOffY;

	// Calculate size based of zoom
	if (clc_zoom == 1) 
	{
		videospec = (TPokeMini_VideoSpec *)&PokeMini_Video1x1;
		PMWidth = 192; PMHeight = 128; PMOffX = 24; PMOffY = 16; UIOffX = 0; UIOffY = 0;
		UIMenu_SetDisplay(192, 128, PokeMini_BGR16, (uint8_t *)PokeMini_BG2, (uint16_t *)PokeMini_BG2_PalBGR16, (uint32_t *)PokeMini_BG2_PalBGR32);
	} 
	else if (clc_zoom == 2) 
	{
		videospec = (TPokeMini_VideoSpec *)&PokeMini_Video2x2;
		PMWidth = 192; PMHeight = 128; PMOffX = 0; PMOffY = 0; UIOffX = 0; UIOffY = 0;
		UIMenu_SetDisplay(192, 128, PokeMini_BGR16, (uint8_t *)PokeMini_BG2, (uint16_t *)PokeMini_BG2_PalBGR16, (uint32_t *)PokeMini_BG2_PalBGR32);
	} 
	else
	{
		videospec = (TPokeMini_VideoSpec *)&PokeMini_Video3x3;
		PMWidth = 288; PMHeight = 192; PMOffX = 0; PMOffY = 0; UIOffX = 0; UIOffY = 0;
		UIMenu_SetDisplay(288, 192, PokeMini_BGR16, (uint8_t *)PokeMini_BG3, (uint16_t *)PokeMini_BG3_PalBGR16, (uint32_t *)PokeMini_BG3_PalBGR32);
	} 
	
	zone_update[0] = (PMOffX*2);
	zone_update[1] = (PMOffY*2);
	zone_update[2] = PMWidth-zone_update[0];
	zone_update[3] = PMHeight-zone_update[1];
	zone_update[4] = PMWidth;
	zone_update[5] = PMHeight;
	
	// Set video spec and check if is supported
	depth = PokeMini_SetVideo(videospec, clc_bpp, CommandLine.lcdfilter, CommandLine.lcdmode);
	if (!depth) {
		fprintf(stderr, "Couldn't set video spec from %i bpp\n", clc_bpp);
		exit(1);
	}

	// Set video mode
	screen = SDL_SetVideoMode(PMWidth, PMHeight, 16, SDL_SWSURFACE | SDL_FULLSCREEN  );
	if (screen == NULL) {
		fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_ShowCursor(0);

	// Calculate pitch and offset
	PixPitch = screen->pitch / 2;
	PMOff = (PMOffY * screen->pitch) + (PMOffX * 2);
	UIOff = (UIOffY * screen->pitch) + (UIOffX * 2);

	clc_bpp = depth;
}

// Handle keyboard and quit events
void handleevents(SDL_Event *event)
{
	switch (event->type) {
	case SDL_KEYDOWN:
		if (event->key.keysym.sym == SDLK_F4) {		// Emulator Exit
			if (event->key.keysym.mod & KMOD_ALT) {
				emurunning = 0;
			}
		} else if (event->key.keysym.sym == SDLK_F10) {		// Fullscreen/Window
			clc_fullscreen = !clc_fullscreen;
			setup_screen();
			UIItems_PlatformC(0, UIMENU_LOAD);
		} else {
			KeyboardPressEvent(event->key.keysym.sym);
		}
		break;
	case SDL_KEYUP:
			KeyboardReleaseEvent(event->key.keysym.sym);
		break;
	case SDL_QUIT:
		emurunning = 0;
		break;
	};
}

// Menu loop
void menuloop()
{
	SDL_Event event;

	// Update window's title and stop sound
	SDL_WM_SetCaption(AppName, "PMEWindow");
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	// Update EEPROM
	PokeMini_SaveFromCommandLines(0);
	
	SDL_FillRect(screen, NULL, 0);

	render_menu();

	// Menu's loop
	while (emurunning && (UI_Status == UI_STATUS_MENU)) 
	{
		render_menu();
		
		// Handle events
		while (SDL_PollEvent(&event)) handleevents(&event);
	}

	// Apply configs
	PokeMini_ApplyChanges();
	if (UI_Status == UI_STATUS_EXIT) emurunning = 0;
	SDL_EnableKeyRepeat(0, 0);
	
	SDL_FillRect(screen, NULL, 0);
	SDL_Flip(screen);
}

void render_menu()
{
	// Process UI
	UIMenu_Process();

	// Screen rendering
	SDL_LockSurface(screen);
		
	// Render the menu or the game screen
	UIMenu_Display_16((uint16_t *)screen->pixels, PixPitch);

	// Unlock surface
	SDL_UnlockSurface(screen);
		
	SDL_UpdateRect(screen, 0, 0, zone_update[4], zone_update[5]);	
	SDL_Flip(screen);
	
	only_one = 0;
}

// Main function
int main(int argc, char **argv)
{
	SDL_Event event;

	// Process arguments
	printf("%s\n\n", AppName);
	PokeMini_InitDirs(argv[0], NULL);
	CommandLineInit();
	CommandLineConfFile("pokemini.cfg", "pokemini_sdl.cfg", CustomConf);
	
	if (!CommandLineArgs(argc, argv, CustomArgs)) 
	{
		PrintHelpUsage(stdout);
		printf("  -dumpsound sound.wav   Dump sound into a WAV file\n");
		printf("  -windowed              Display in window (default)\n");
		printf("  -fullscreen            Display in fullscreen\n");
		printf("  -displayfps            Display FPS counter on screen\n");
		printf("  -zoom n                Zoom display: 1 to 6 (def 4)\n");
		printf("  -bpp n                 Bits-Per-Pixel: 16 or 32 (def 16)\n");
		return 1;
	}

	// Initialize SDL
	SDL_Init(SDL_INIT_VIDEO);

	// Initialize the display
	setup_screen();

	// Set the window manager title bar
	SDL_WM_SetCaption(AppName, "PMEWindow");
	SDL_EnableKeyRepeat(0, 0);

	// Initialize the emulator
	printf("Starting emulator...\n");
	
	PokeMini_Create(0, PMSNDBUFFER);

	// Setup palette and LCD mode
	PokeMini_VideoPalette_Init(PokeMini_BGR16, 1);
	PokeMini_VideoPalette_Index(CommandLine.palette, CommandLine.custompal, CommandLine.lcdcontrast, CommandLine.lcdbright);
	PokeMini_ApplyChanges();

	// Load stuff
	PokeMini_UseDefaultCallbacks();
	if (!PokeMini_LoadFromCommandLines("Using FreeBIOS", "EEPROM data will be discarded!")) {
		UI_Status = UI_STATUS_MENU;
	}

	// Enable sound & init UI
	printf("Running emulator...\n");
	UIMenu_Init();
	KeyboardRemap(&KeybMapSDL);
	
	SDL_FillRect(screen, NULL, 0);
	SDL_Flip(screen);
	
	// Emulator's loop
	while (emurunning) 
	{
		PokeMini_EmulateFrame();

		// Screen rendering
		SDL_LockSurface(screen);
		
		// Render the menu or the game screen
		if (PokeMini_Rumbling)
			PokeMini_VideoBlit((void *)((uint8_t *)screen->pixels + PMOff + PokeMini_GenRumbleOffset(screen->pitch)), PixPitch);
		else 
			PokeMini_VideoBlit((void *)((uint8_t *)screen->pixels + PMOff), PixPitch);
		
		LCDDirty = 0;
		
		// Unlock surface
		SDL_UnlockSurface(screen);
		
		SDL_UpdateRect(screen, zone_update[0], zone_update[1], zone_update[2], zone_update[3]);	
		//SDL_Flip(screen);

		// Handle events
		while (SDL_PollEvent(&event)) handleevents(&event);
		
		// Menu
		if (UI_Status == UI_STATUS_MENU) menuloop();

	}

	UIMenu_Destroy();

	// Save Stuff
	PokeMini_SaveFromCommandLines(1);

	// Terminate...
	printf("Shutdown emulator...\n");
	PokeMini_VideoPalette_Free();
	PokeMini_Destroy();
	
	SDL_Quit();

	return 0;
}
