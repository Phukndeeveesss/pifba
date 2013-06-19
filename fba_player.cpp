/*
 * FinalBurn Alpha for MOTO EZX Modile Phone
 * Copyright (C) 2006 OopsWare. CHINA.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: fba_player.cpp,v 0.10 2006/12/03 $
 */

#define CALC_FPS

#include <stdio.h>
#include <stdlib.h>
#include "fba_player.h"
#include "ezxaudio.h"
#include "font.h"
#include "snd.h"

#include "burnint.h"
#include "gp2xmemfuncs.h"
#include "config.h"
#include "cache.h"

extern "C"
{
#include "gp2xsdk.h"
};

void uploadfb(void);

extern unsigned int nFramesRendered;
static int frame_count = 0;
unsigned int FBA_KEYPAD[4];
unsigned char ServiceRequest = 0;
unsigned char P1P2Start = 0;
unsigned short titlefb[320][240];
extern bool bShowFPS;
void ChangeFrameskip();
extern struct usbjoy *joys[4];
extern char joyCount;
extern CFG_OPTIONS config_options;
extern volatile short *pOutput[];

int joyMap[8] = {0x0040,0x0080,0x0100,0x0200,0x0400,0x0800,0x10,0x20};
/*struct keymap_item FBA_KEYMAP[] = {
		{"-----",	0, false },
		{"START",	1, false },
		{"COIN",	2, false },
		{"A",		3, false },
		{"B", 		4, false },
		{"C",		5, false },
		{"D",		6, false },
		{"E",		7, false },
		{"F",		8, false },
		
		{"A+B",		9, false },
		{"C+D",	   10, false },
		{"A+B+C",  11, false },
		
		{"A-Turbo",	3, true  },
		{"B-Turbo",	4, true  },
		{"C-Turbo",	5, true  },
		{"D-Turbo",	6, true  },
		{"E-Turbo",	7, true  },
		{"F-Turbo",	8, true  } 	};
*/

void do_keypad()
{
	static unsigned int turbo = 0;
	unsigned long joy = gp2x_joystick_read();
	int bVert = BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL;
	turbo ++;
	
	FBA_KEYPAD[0] = 0;
	FBA_KEYPAD[1] = 0;
	FBA_KEYPAD[2] = 0;
	FBA_KEYPAD[3] = 0;
	ServiceRequest =0;
	P1P2Start = 0;
	
	if ( joy & GP2X_UP || joy & GP2X_UP_LEFT || joy & GP2X_UP_RIGHT ) FBA_KEYPAD[0] |= bVert?0x0004:0x0001;
	if ( joy & GP2X_DOWN || joy & GP2X_DOWN_LEFT || joy & GP2X_DOWN_RIGHT ) FBA_KEYPAD[0] |= bVert?0x0008:0x0002;
	if ( joy & GP2X_LEFT || joy & GP2X_UP_LEFT || joy & GP2X_DOWN_LEFT ) FBA_KEYPAD[0] |= bVert?0x0002:0x0004;
	if ( joy & GP2X_RIGHT || joy & GP2X_UP_RIGHT || joy & GP2X_DOWN_RIGHT ) FBA_KEYPAD[0] |= bVert?0x0001:0x0008;
	
	if ( joy & GP2X_SELECT )	FBA_KEYPAD[0] |= 0x0010;	
	if ( joy & GP2X_START )		FBA_KEYPAD[0] |= 0x0020;	
	
	if ( joy & GP2X_A )	FBA_KEYPAD[0] |= 0x0040;	// A
	if ( joy & GP2X_X )
		if (bVert)
			ezx_change_volume(1);
		else
			FBA_KEYPAD[0] |= 0x0080;	// B
	if ( joy & GP2X_B )	FBA_KEYPAD[0] |= 0x0100;	// C
	if ( joy & GP2X_Y )	
		if (bVert)
			ezx_change_volume(-1);
		else
			FBA_KEYPAD[0] |= 0x0200;	// D
	if ( joy & GP2X_L )							// E
		if (bVert)
			FBA_KEYPAD[0] |= 0x0100;
		else
			FBA_KEYPAD[0] |= 0x0400;
	if ( joy & GP2X_R )							// F
		if (bVert)
			FBA_KEYPAD[0] |= 0x0200;
		else
			FBA_KEYPAD[0] |= 0x0800;
	if ( joy & GP2X_VOL_UP )
		if (bVert)
			FBA_KEYPAD[0] |= 0x0040;
		else
			ezx_change_volume(1);
	if ( joy & GP2X_VOL_DOWN )
		if (bVert)
			FBA_KEYPAD[0] |= 0x0080;
		else
			ezx_change_volume(-1);
	if ( joy & GP2X_L && joy & GP2X_R)
	{
		if (joy & GP2X_Y) ChangeFrameskip();
		else
		if (joy & GP2X_START) GameLooping = false;
		else
		if ( joy & GP2X_SELECT) ServiceRequest = 1;
	}
	else
		if (joy & GP2X_START && joy & GP2X_SELECT) P1P2Start = 1;
		
	for (int i=0;i<joyCount;i++)
	{
	int numButtons = joy_buttons(joys[i]);
		if (numButtons > 8)
			numButtons = 8;
		joy_update(joys[i]);
		if(joy_getaxe(JOYUP, joys[i])) FBA_KEYPAD[i] |= bVert?0x0004:0x0001;
		if(joy_getaxe(JOYDOWN, joys[i])) FBA_KEYPAD[i] |= bVert?0x0008:0x0002;
		if(joy_getaxe(JOYLEFT, joys[i])) FBA_KEYPAD[i] |= bVert?0x0002:0x0004;
		if(joy_getaxe(JOYRIGHT, joys[i])) FBA_KEYPAD[i] |= bVert?0x0001:0x0008;

		for (int nButton = 0; nButton < numButtons; nButton++) {
			if(joy_getbutton(nButton, joys[i]))
				FBA_KEYPAD[i] |= joyMap[nButton];
		}
	}
}

int DrvInit(int nDrvNum, bool bRestore);
int DrvExit();

int RunReset();
int RunOneFrame(bool bDraw, int fps);

int VideoInit();
void VideoExit();
 
int InpInit();
int InpExit();
void InpDIP();

extern char szAppRomPath[];
extern int nBurnFPS;
int fps=0;

void show_rom_loading_text(char * szText, int nSize, int nTotalSize)
{
	static long long size = 0;
	//printf("!!! %s, %d / %d\n", szText, size + nSize, nTotalSize);

	DrawRect((uint16 *) titlefb, 20, 120, 300, 20, 0, 320);
	
	if (szText)
		DrawString (szText, (uint16 *) titlefb, 20, 120, 320);
	
	if (nTotalSize == 0) {
		size = 0;
		DrawRect((uint16 *) titlefb, 20, 140, 280, 12, 0x00FFFFFF, 320);
		DrawRect((uint16 *) titlefb, 21, 141, 278, 10, 0x00808080, 320);
	} else {
		size += nSize;
		if (size > nTotalSize) size = nTotalSize;
		DrawRect((uint16 *) titlefb, 21, 141, size * 278 / nTotalSize, 10, 0x00FFFF00, 320);
	}

	gp2x_memcpy (VideoBuffer, titlefb, 320*240*2); gp2x_video_flip();
}

void run_fba_emulator(const char *fn)
{
	// process rom path and name
	char romname[MAX_PATH];
	if (BurnCacheInit(fn, romname))
		goto finish;
/*	
	strcpy(szAppRomPath, fn);
	char * p = strrchr(szAppRomPath, '/');
	if (p) {
		p++;
		strcpy(romname, p);
		
		*p = 0;
		p = strrchr(romname, '.');
		if (p) *p = 0;
		else {
			// error
			goto finish;
		}
	} else {
		// error
		goto finish;
	}
*/
	BurnLibInit();
	
	// find rom by name
	for (nBurnDrvSelect=0; nBurnDrvSelect<nBurnDrvCount; nBurnDrvSelect++)
		if ( strcasecmp(romname, BurnDrvGetTextA(DRV_NAME)) == 0 )
			break;
	if (nBurnDrvSelect >= nBurnDrvCount) {
		// unsupport rom ...
		nBurnDrvSelect = ~0U;
		printf ("rom not supported!\n");
		goto finish;
	}
	
	printf("Attempt to initialise '%s'\n", BurnDrvGetTextA(DRV_FULLNAME));
	
	gp2x_memset (titlefb, 0, 320*240*2);
	DrawString ("Finalburn Alpha Plus for Pi (Ver.090308/7.3)", (uint16*)&titlefb, 10, 20, 320);
	DrawString ("Based on FinalBurnAlpha", (uint16*)&titlefb, 10, 35, 320);
	DrawString ("Now loading ... ", (uint16 *)&titlefb, 10, 105, 320);	
	show_rom_loading_text("Open Zip", 0, 0);
	gp2x_memcpy (VideoBuffer, titlefb, 320*240*2); 
	gp2x_video_flip();
	 	
	InpInit();
	InpDIP();
	
	VideoInit();

	if (DrvInit(nBurnDrvSelect, false) != 0) {
		printf ("Driver initialisation failed! Likely causes are:\n- Corrupt/Missing ROM(s)\n- I/O Error\n- Memory error\n\n");
		goto finish;
	}

	RunReset();

	frame_count = 0;
	GameLooping = true;

	EZX_StartTicks();

	printf ("Lets go!\n");

	gp2x_clear_framebuffers();
	if (config_options.option_sound_enable)
	{
	int aim=0, done=0, timer = 0, tick=0, i=0, fps = 0;
	unsigned int frame_limit = nBurnFPS/100, frametime = 100000000/nBurnFPS;
	bool bRenderFrame;

		if (SndOpen() == 0)
		{		
			while (GameLooping)
			{
				for (i=10;i;i--)
				{
					if (bShowFPS)
					{
						timer = EZX_GetTicks();
						if(timer-tick>1000000)
						{
							fps = nFramesRendered;
							nFramesRendered = 0;
							tick = timer;
						}
					}
					aim=SegAim();
					if (done!=aim)
					{
						//We need to render more audio:  
						pBurnSoundOut=(short *)pOutput[done];
						done++; if (done>=8) done=0;
		
						if ((done==aim)) 
							bRenderFrame=true; // Render last frame
						else
							bRenderFrame=false; // Render last frame
						RunOneFrame(bRenderFrame,fps);	
					}
		
					if (done==aim) break; // Up to date now
				}
				done=aim; // Make sure up to date
			}
		}
	}
	else
	{
	int now, done=0, timer = 0, ticks=0, tick=0, i=0, fps = 0;
	unsigned int frame_limit = nBurnFPS/100, frametime = 100000000/nBurnFPS;
		
		while (GameLooping)
		{
			timer = EZX_GetTicks()/frametime;;
			if(timer-tick>frame_limit && bShowFPS)
			{
				fps = nFramesRendered;
				nFramesRendered = 0;
				tick = timer;
			}
			now = timer;
			ticks=now-done;
			if(ticks<1) continue;
			if(ticks>10) ticks=10;
			for (i=0; i<ticks-1; i++)
			{
				RunOneFrame(false,fps);	
			} 
			if(ticks>=1)
			{
				RunOneFrame(true,fps);	
			}
			
			done = now;
		}
	}
	
	printf ("Finished emulating\n");
	
finish:
	printf("---- Shutdown Finalburn Alpha plus ----\n\n");
	DrvExit();
	BurnLibExit();

	if (config_options.option_sound_enable)
		SndExit();
	VideoExit();
	InpExit();
	BurnCacheExit();
}

int BurnStateLoad(const char * szName, int bAll, int (*pLoadGame)());
int BurnStateSave(const char * szName, int bAll);

int DrvInitCallback()
{
	return DrvInit(nBurnDrvSelect, false);
}


