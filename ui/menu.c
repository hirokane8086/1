/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include <string.h>
#include <stdlib.h>  // abs()

#include "app/dtmf.h"
#include "app/menu.h"
#include "bitmaps.h"
#include "board.h"
#include "dcs.h"
#include "driver/backlight.h"
#include "driver/bk4819.h"
#include "driver/eeprom.h"   // EEPROM_ReadBuffer()
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "frequencies.h"
#include "helper/battery.h"
#include "misc.h"
#include "settings.h"
#include "ui/helper.h"
#include "ui/inputbox.h"
#include "ui/menu.h"
#include "ui/ui.h"

const t_menu_item MenuList[] =
{
//   text,     voice ID,                               menu ID
	{"步频",   VOICE_ID_FREQUENCY_STEP,                MENU_STEP          },
	{"功率",  VOICE_ID_POWER,                         MENU_TXP           }, // was "TXP"
	{"RxDCS",  VOICE_ID_DCS,                           MENU_R_DCS         }, // was "R_DCS"
	{"RxCTCS", VOICE_ID_CTCSS,                         MENU_R_CTCS        }, // was "R_CTCS"
	{"TxDCS",  VOICE_ID_DCS,                           MENU_T_DCS         }, // was "T_DCS"
	{"TxCTCS", VOICE_ID_CTCSS,                         MENU_T_CTCS        }, // was "T_CTCS"
	{"方向", VOICE_ID_TX_OFFSET_FREQUENCY_DIRECTION, MENU_SFT_D         }, // was "SFT_D"
	{"频差", VOICE_ID_TX_OFFSET_FREQUENCY,           MENU_OFFSET        }, // was "OFFSET"
	{"带宽",    VOICE_ID_CHANNEL_BANDWIDTH,             MENU_W_N           },
	{"扰频", VOICE_ID_SCRAMBLER_ON,                  MENU_SCR           }, // was "SCR"
	{"忙锁", VOICE_ID_BUSY_LOCKOUT,                  MENU_BCL           }, // was "BCL"
	{"Compnd", VOICE_ID_INVALID,                       MENU_COMPAND       },
	{"解码", VOICE_ID_INVALID,                       MENU_AM            }, // was "AM"
	{"ScAdd1", VOICE_ID_INVALID,                       MENU_S_ADD1        },
	{"ScAdd2", VOICE_ID_INVALID,                       MENU_S_ADD2        },
	{"存储", VOICE_ID_MEMORY_CHANNEL,                MENU_MEM_CH        }, // was "MEM-CH"
	{"删除", VOICE_ID_DELETE_CHANNEL,                MENU_DEL_CH        }, // was "DEL-CH"
	{"名称", VOICE_ID_INVALID,                       MENU_MEM_NAME      },	

	{"SList",  VOICE_ID_INVALID,                       MENU_S_LIST        },
	{"SList1", VOICE_ID_INVALID,                       MENU_SLIST1        },
	{"SList2", VOICE_ID_INVALID,                       MENU_SLIST2        },
	{"ScnRev", VOICE_ID_INVALID,                       MENU_SC_REV        },
#ifdef ENABLE_NOAA
	{"NOAA-S", VOICE_ID_INVALID,                       MENU_NOAA_S        },
#endif

	{"侧1短",    VOICE_ID_INVALID,                    MENU_F1SHRT        },
	{"侧1长",    VOICE_ID_INVALID,                    MENU_F1LONG        },
	{"侧2短",    VOICE_ID_INVALID,                    MENU_F2SHRT        },
	{"侧2长",    VOICE_ID_INVALID,                    MENU_F2LONG        },
	{"M长",    VOICE_ID_INVALID,                    MENU_MLONG         },

	{"键锁", VOICE_ID_INVALID,                       MENU_AUTOLK        }, // was "AUTOLk"
	{"TxTOut", VOICE_ID_TRANSMIT_OVER_TIME,            MENU_TOT           }, // was "TOT"
	{"节电", VOICE_ID_SAVE_MODE,                     MENU_SAVE          }, // was "SAVE"
	{"话筒",    VOICE_ID_INVALID,                       MENU_MIC           },
#ifdef ENABLE_AUDIO_BAR
	{"声压", VOICE_ID_INVALID,                       MENU_MIC_BAR       },
#endif		
	{"屏显", VOICE_ID_INVALID,                       MENU_MDF           }, // was "MDF"
	{"机显", VOICE_ID_INVALID,                       MENU_PONMSG        },
	{"样式", VOICE_ID_INVALID,                       MENU_BAT_TXT       },	
	{"背光", VOICE_ID_INVALID,                       MENU_ABR           }, // was "ABR"
	{"BLMin",  VOICE_ID_INVALID,                       MENU_ABR_MIN       },
	{"BLMax",  VOICE_ID_INVALID,                       MENU_ABR_MAX       },
	{"BltTRX", VOICE_ID_INVALID,                       MENU_ABR_ON_TX_RX  },
	{"Beep",   VOICE_ID_BEEP_PROMPT,                   MENU_BEEP          },
#ifdef ENABLE_VOICE
	{"语音",  VOICE_ID_VOICE_PROMPT,                  MENU_VOICE         },
#endif
	{"尾音",  VOICE_ID_INVALID,                       MENU_ROGER         },
	{"消尾",    VOICE_ID_INVALID,                       MENU_STE           },
	{"RP STE", VOICE_ID_INVALID,                       MENU_RP_STE        },
	{"1 Call", VOICE_ID_INVALID,                       MENU_1_CALL        },
#ifdef ENABLE_ALARM
	{"AlarmT", VOICE_ID_INVALID,                       MENU_AL_MOD        },
#endif
	{"ANI ID", VOICE_ID_ANI_CODE,                      MENU_ANI_ID        },
	{"UPCode", VOICE_ID_INVALID,                       MENU_UPCODE        },
	{"DWCode", VOICE_ID_INVALID,                       MENU_DWCODE        },
	{"PTT ID", VOICE_ID_INVALID,                       MENU_PTT_ID        },
	{"D ST",   VOICE_ID_INVALID,                       MENU_D_ST          },
    {"D Resp", VOICE_ID_INVALID,                       MENU_D_RSP         },
	{"D Hold", VOICE_ID_INVALID,                       MENU_D_HOLD        },
	{"D Prel", VOICE_ID_INVALID,                       MENU_D_PRE         },
	{"D 解", VOICE_ID_INVALID,                       MENU_D_DCD         },
	{"D List", VOICE_ID_INVALID,                       MENU_D_LIST        },
	{"D Live", VOICE_ID_INVALID,                       MENU_D_LIVE_DEC    }, // live DTMF decoder
#ifdef ENABLE_AM_FIX
	{"AM Fix", VOICE_ID_INVALID,                       MENU_AM_FIX        },
#endif
#ifdef ENABLE_AM_FIX_TEST1
	{"AM FT1", VOICE_ID_INVALID,                       MENU_AM_FIX_TEST1  },
#endif
#ifdef ENABLE_VOX
	{"声控",    VOICE_ID_VOX,                           MENU_VOX           },
#endif
	{"电池", VOICE_ID_INVALID,                       MENU_VOL           }, // was "VOL"
	{"模式", VOICE_ID_DUAL_STANDBY,                  MENU_TDR           },
	{"静噪",    VOICE_ID_SQUELCH,                       MENU_SQL           },

	// hidden menu items from here on
	// enabled if pressing both the PTT and upper side button at power-on
	{"键锁", VOICE_ID_INVALID,                       MENU_F_LOCK        },
	{"Tx 200", VOICE_ID_INVALID,                       MENU_200TX         }, // was "200TX"
	{"Tx 350", VOICE_ID_INVALID,                       MENU_350TX         }, // was "350TX"
	{"Tx 500", VOICE_ID_INVALID,                       MENU_500TX         }, // was "500TX"
	{"350 En", VOICE_ID_INVALID,                       MENU_350EN         }, // was "350EN"
	{"ScraEn", VOICE_ID_INVALID,                       MENU_SCREN         }, // was "SCREN"
	{"TxEnab", VOICE_ID_INVALID,                       MENU_TX_EN         }, // enable TX
#ifdef ENABLE_F_CAL_MENU
	{"FrCali", VOICE_ID_INVALID,                       MENU_F_CALI        }, // reference xtal calibration
#endif
	{"BatCal", VOICE_ID_INVALID,                       MENU_BATCAL        }, // battery voltage calibration
	{"BatTyp", VOICE_ID_INVALID,                       MENU_BATTYP        }, // battery type 1600/2200mAh
	{"重置",  VOICE_ID_INITIALISATION,                MENU_RESET         }, // might be better to move this to the hidden menu items ?

	{"",       VOICE_ID_INVALID,                       0xff               }  // end of list - DO NOT delete or move this this
};

const uint8_t FIRST_HIDDEN_MENU_ITEM = MENU_F_LOCK;

const char gSubMenu_TXP[3][5] =
{
	"低",
	"中",
	"高"
};

const char gSubMenu_SFT_D[3][4] =
{
	"关",
	"+",
	"-"
};

const char gSubMenu_W_N[2][7] =
{
	"宽",
	"窄"
};

const char gSubMenu_OFF_ON[2][4] =
{
	"关",
	"开"
};

const char gSubMenu_SAVE[5][4] =
{
	"关",
	"1:1",
	"1:2",
	"1:3",
	"1:4"
};

const char gSubMenu_TOT[11][7] =
{
	"30 秒",
	"1 分",
	"2 分",
	"3 分",
	"4 分",
	"5 分",
	"6 分",
	"7 分",
	"8 分",
	"9 分",
	"15 分"
};

const char* gSubMenu_RXMode[4] =
{
	"主信道收发", 		// TX and RX on main only
	"动态双守", // Watch both and respond
	"跨段收发", 		// TX on main, RX on secondary
	"主发双守" 	// always TX on main, but RX on both
};

#ifdef ENABLE_VOICE
	const char gSubMenu_VOICE[3][4] =
	{
		"关",
		"中文",
		"ENG"
	};
#endif

const char gSubMenu_SC_REV[3][12] =
{
	"超时恢复",
	"消失恢复",
	"立即停止"
};

const char* gSubMenu_MDF[4] =
{
	"频率",
	"信道\n编号",
	"信道名",
	"名称\n+\n频率"
};

#ifdef ENABLE_ALARM
	const char gSubMenu_AL_MOD[2][5] =
	{
		"SITE",
		"TONE"
	};
#endif

const char gSubMenu_D_RSP[4][11] =
{
	"无",
	"响铃",
	"回答",
	"全开"
};

const char* gSubMenu_PTT_ID[5] =
{
	"关",
	"UP CODE",
	"DOWN CODE",
	"UP+DOWN\nCODE",
	"Quindar\nTones"
};

const char gSubMenu_PONMSG[4][8] =
{
	"全屏",
	"信息",
	"电压",
	"无"
};

const char gSubMenu_ROGER[][6] =
{
	"关",
	"ROGER",
	"MDC"
};

const char gSubMenu_RESET[2][4] =
{
	"VFO",
	"ALL"
};

const char gSubMenu_F_LOCK[6][4] =
{
	"OFF",
	"FCC",
	"CE",
	"GB",
	"430",
	"438"
};

const char gSubMenu_BACKLIGHT[8][7] =
{
	"关",
	"5 秒",
	"10 秒",
	"20 秒",
	"1 分",
	"2 分",
	"4 分",
	"常开"
};

const char gSubMenu_RX_TX[4][6] =
{
	"关",
	"发",
	"收",
	"收发"
};

#ifdef ENABLE_AM_FIX_TEST1
	const char gSubMenu_AM_fix_test1[4][8] =
	{
		"LNA-S 0",
		"LNA-S 1",
		"LNA-S 2",
		"LNA-S 3"
	};
#endif

const char gSubMenu_BAT_TXT[3][8] =
{
	"无",
	"电压",
	"电量"
};

const char gSubMenu_BATTYP[2][9] =
{
	"1600mAh",
	"2200mAh"
};

const char gSubMenu_SCRAMBLER[11][7] =
{
	"关",
	"2600Hz",
	"2700Hz",
	"2800Hz",
	"2900Hz",
	"3000Hz",
	"3100Hz",
	"3200Hz",
	"3300Hz",
	"3400Hz",
	"3500Hz"
};

const t_sidefunction SIDEFUNCTIONS[] =
{
	{"无",			ACTION_OPT_NONE},
	{"手电\n灯光",	ACTION_OPT_FLASHLIGHT},
	{"功率",			ACTION_OPT_POWER},
	{"监听",			ACTION_OPT_MONITOR},
	{"扫描",			ACTION_OPT_SCAN},
#ifdef ENABLE_VOX
	{"声控",				ACTION_OPT_VOX},
#endif
#ifdef ENABLE_ALARM	
	{"警告",			ACTION_OPT_ALARM},
#endif
#ifdef ENABLE_FMRADIO
	{"收音机",		ACTION_OPT_FM},
#endif	
#ifdef ENABLE_TX1750	
	{"1750HZ",			ACTION_OPT_1750},
#endif
	{"键盘\n锁定",	ACTION_OPT_KEYLOCK},
	{"切换\n上下信道",		ACTION_OPT_A_B},
	{"VFO/MR",			ACTION_OPT_VFO_MR},
	{"切换\n模式",	ACTION_OPT_SWITCH_DEMODUL},
#ifdef ENABLE_BLMIN_TMP_OFF
	{"BLMIN\nTMP OFF",  ACTION_OPT_BLMIN_TMP_OFF}, 		//BackLight Minimum Temporay OFF
#endif
};
const t_sidefunction* gSubMenu_SIDEFUNCTIONS = SIDEFUNCTIONS;
const uint8_t gSubMenu_SIDEFUNCTIONS_size = ARRAY_SIZE(SIDEFUNCTIONS);

bool    gIsInSubMenu;
uint8_t gMenuCursor;
int UI_MENU_GetCurrentMenuId() {
	if(gMenuCursor < ARRAY_SIZE(MenuList))
		return MenuList[gMenuCursor].menu_id;
	else
		return MenuList[ARRAY_SIZE(MenuList)-1].menu_id;
}

uint8_t UI_MENU_GetMenuIdx(uint8_t id)
{
	for(uint8_t i = 0; i < ARRAY_SIZE(MenuList); i++)
		if(MenuList[i].menu_id == id)
			return i;
	return 0;
}

int32_t gSubMenuSelection;

// edit box
char    edit_original[17]; // a copy of the text before editing so that we can easily test for changes/difference
char    edit[17];
int     edit_index;

void UI_DisplayMenu(void)
{
	const unsigned int menu_list_width = 6; // max no. of characters on the menu list (left side)
	const unsigned int menu_item_x1    = (8 * menu_list_width) + 2;
	const unsigned int menu_item_x2    = LCD_WIDTH - 1;
	unsigned int       i;
	char               String[128];  // bigger cuz we can now do multi-line in one string (use '\n' char)
	char               Contact[16];

	// clear the screen buffer
	memset(gFrameBuffer, 0, sizeof(gFrameBuffer));

	#if 0
		// original menu layout

		for (i = 0; i < 3; i++)
			if (gMenuCursor > 0 || i > 0)
				if ((gMenuListCount - 1) != gMenuCursor || i != 2)
					UI_PrintString(MenuList[gMenuCursor + i - 1].name, 0, 0, i * 2, 8);

		// invert the current menu list item pixels
		for (i = 0; i < (8 * menu_list_width); i++)
		{
			gFrameBuffer[2][i] ^= 0xFF;
			gFrameBuffer[3][i] ^= 0xFF;
		}

		// draw vertical separating dotted line
		for (i = 0; i < 7; i++)
			gFrameBuffer[i][(8 * menu_list_width) + 1] = 0xAA;

		// draw the little sub-menu triangle marker
		if (gIsInSubMenu)
			memmove(gFrameBuffer[0] + (8 * menu_list_width) + 1, BITMAP_CurrentIndicator, sizeof(BITMAP_CurrentIndicator));

		// draw the menu index number/count
		sprintf(String, "%2u.%u", 1 + gMenuCursor, gMenuListCount);
		UI_PrintStringSmall(String, 2, 0, 6);

	#else
	{	// new menu layout .. experimental & unfinished

		const int menu_index = gMenuCursor;  // current selected menu item
		i = 1;

		if (!gIsInSubMenu)
		{
			while (i < 2)
			{	// leading menu items - small text
				const int k = menu_index + i - 2;
				if (k < 0)
					UI_PrintStringSmall(MenuList[gMenuListCount + k].name, 0, 0, i);  // wrap-a-round
				else
				if (k >= 0 && k < (int)gMenuListCount)
					UI_PrintStringSmall(MenuList[k].name, 0, 0, i);
				i++;
			}

			// current menu item - keep big n fat
			if (menu_index >= 0 && menu_index < (int)gMenuListCount)
				UI_PrintString(MenuList[menu_index].name, 0, 0, 2, 8);
			i++;

			while (i < 4)
			{	// trailing menu item - small text
				const int k = menu_index + i - 2;
				if (k >= 0 && k < (int)gMenuListCount)
					UI_PrintStringSmall(MenuList[k].name, 0, 0, 1 + i);
				else
				if (k >= (int)gMenuListCount)
					UI_PrintStringSmall(MenuList[gMenuListCount - k].name, 0, 0, 1 + i);  // wrap-a-round
				i++;
			}

			// draw the menu index number/count
			sprintf(String, "%2u.%u", 1 + gMenuCursor, gMenuListCount);
			UI_PrintStringSmall(String, 2, 0, 6);
		}
		else
		if (menu_index >= 0 && menu_index < (int)gMenuListCount)
		{	// current menu item
			strcpy(String, MenuList[menu_index].name);
//			strcat(String, ":");
			UI_PrintString(String, 0, 0, 0, 8);
//			UI_PrintStringSmall(String, 0, 0, 0);
		}
	}
	#endif

	// **************

	memset(String, 0, sizeof(String));

	bool already_printed = false;

	switch (UI_MENU_GetCurrentMenuId())
	{
		case MENU_SQL:
			sprintf(String, "%d", gSubMenuSelection);
			break;

		case MENU_MIC:
			{	// display the mic gain in actual dB rather than just an index number
				const uint8_t mic = gMicGain_dB2[gSubMenuSelection];
				sprintf(String, "+%u.%01udB", mic / 2, mic % 2);
			}
			break;

		#ifdef ENABLE_AUDIO_BAR
			case MENU_MIC_BAR:
				strcpy(String, gSubMenu_OFF_ON[gSubMenuSelection]);
				break;
		#endif

		case MENU_STEP: {
			uint16_t step = gStepFrequencyTable[FREQUENCY_GetStepIdxFromSortedIdx(gSubMenuSelection)];
			sprintf(String, "%d.%02ukHz", step / 100, step % 100);
			break;
		}

		case MENU_TXP:
			strcpy(String, gSubMenu_TXP[gSubMenuSelection]);
			break;

		case MENU_R_DCS:
		case MENU_T_DCS:
			if (gSubMenuSelection == 0)
				strcpy(String, "OFF");
			else if (gSubMenuSelection < 105)
				sprintf(String, "D%03oN", DCS_Options[gSubMenuSelection -   1]);
			else
				sprintf(String, "D%03oI", DCS_Options[gSubMenuSelection - 105]);
			break;

		case MENU_R_CTCS:
		case MENU_T_CTCS:
		{
			if (gSubMenuSelection == 0)
				strcpy(String, "OFF");
			else
				sprintf(String, "%u.%uHz", CTCSS_Options[gSubMenuSelection - 1] / 10, CTCSS_Options[gSubMenuSelection - 1] % 10);
			break;
		}

		case MENU_SFT_D:
			strcpy(String, gSubMenu_SFT_D[gSubMenuSelection]);
			break;

		case MENU_OFFSET:
			if (!gIsInSubMenu || gInputBoxIndex == 0)
			{
				sprintf(String, "%3d.%05u", gSubMenuSelection / 100000, abs(gSubMenuSelection) % 100000);
				UI_PrintString(String, menu_item_x1, menu_item_x2, 1, 8);
			}
			else
			{
				const char * ascii = INPUTBOX_GetAscii();
				sprintf(String, "%.3s.%.3s  ",ascii, ascii + 3);
				UI_PrintString(String, menu_item_x1, menu_item_x2, 1, 8);
			}

			UI_PrintString("MHz",  menu_item_x1, menu_item_x2, 3, 8);

			already_printed = true;
			break;

		case MENU_W_N:
			strcpy(String, gSubMenu_W_N[gSubMenuSelection]);
			break;

		case MENU_SCR:
			strcpy(String, gSubMenu_SCRAMBLER[gSubMenuSelection]);
			#if 1
				if (gSubMenuSelection > 0 && gSetting_ScrambleEnable)
					BK4819_EnableScramble(gSubMenuSelection - 1);
				else
					BK4819_DisableScramble();
			#endif
			break;

		#ifdef ENABLE_VOX
			case MENU_VOX:
				if (gSubMenuSelection == 0)
					strcpy(String, "OFF");
				else
					sprintf(String, "%d", gSubMenuSelection);
				break;
		#endif

		case MENU_ABR:
			strcpy(String, gSubMenu_BACKLIGHT[gSubMenuSelection]);
			BACKLIGHT_SetBrightness(-1);
			break;

		case MENU_ABR_MIN:
		case MENU_ABR_MAX:
			sprintf(String, "%d", gSubMenuSelection);
			if(gIsInSubMenu)
				BACKLIGHT_SetBrightness(gSubMenuSelection);
			else
				BACKLIGHT_SetBrightness(-1);
			break;	

		case MENU_AM:
			strcpy(String, gModulationStr[gSubMenuSelection]);
			break;

		#ifdef ENABLE_AM_FIX_TEST1
			case MENU_AM_FIX_TEST1:
				strcpy(String, gSubMenu_AM_fix_test1[gSubMenuSelection]);
//				gSetting_AM_fix = gSubMenuSelection;
				break;
		#endif

		case MENU_AUTOLK:
			strcpy(String, (gSubMenuSelection == 0) ? "OFF" : "AUTO");
			break;

		case MENU_COMPAND:
		case MENU_ABR_ON_TX_RX:
			strcpy(String, gSubMenu_RX_TX[gSubMenuSelection]);
			break;

		#ifdef ENABLE_AM_FIX
			case MENU_AM_FIX:
		#endif
		case MENU_BCL:
		case MENU_BEEP:
		case MENU_S_ADD1:
		case MENU_S_ADD2:
		case MENU_STE:
		case MENU_D_ST:
		case MENU_D_DCD:
		case MENU_D_LIVE_DEC:
		#ifdef ENABLE_NOAA
			case MENU_NOAA_S:
		#endif
		case MENU_350TX:
		case MENU_200TX:
		case MENU_500TX:
		case MENU_350EN:
		case MENU_SCREN:
		case MENU_TX_EN:
			strcpy(String, gSubMenu_OFF_ON[gSubMenuSelection]);
			break;

		case MENU_MEM_CH:
		case MENU_1_CALL:
		case MENU_DEL_CH:
		{
			const bool valid = RADIO_CheckValidChannel(gSubMenuSelection, false, 0);

			UI_GenerateChannelStringEx(String, valid, gSubMenuSelection);
			UI_PrintString(String, menu_item_x1, menu_item_x2, 0, 8);

			if (valid && !gAskForConfirmation)
			{	// show the frequency so that the user knows the channels frequency
				const uint32_t frequency = BOARD_fetchChannelFrequency(gSubMenuSelection);
				sprintf(String, "%u.%05u", frequency / 100000, frequency % 100000);
				UI_PrintString(String, menu_item_x1, menu_item_x2, 4, 8);
			}

			already_printed = true;
			break;
		}

		case MENU_MEM_NAME:
		{
			const bool valid = RADIO_CheckValidChannel(gSubMenuSelection, false, 0);

			UI_GenerateChannelStringEx(String, valid, gSubMenuSelection);
			UI_PrintString(String, menu_item_x1, menu_item_x2, 0, 8);

			if (valid)
			{
				const uint32_t frequency = BOARD_fetchChannelFrequency(gSubMenuSelection);

				if (!gIsInSubMenu || edit_index < 0)
				{	// show the channel name
					BOARD_fetchChannelName(String, gSubMenuSelection);
					if (String[0] == 0)
						strcpy(String, "--");
					UI_PrintString(String, menu_item_x1, menu_item_x2, 2, 8);
				}
				else
				{	// show the channel name being edited
					UI_PrintString(edit, menu_item_x1, 0, 2, 8);
					if (edit_index < 10)
						UI_PrintString(     "^", menu_item_x1 + (8 * edit_index), 0, 4, 8);  // show the cursor
				}

				if (!gAskForConfirmation)
				{	// show the frequency so that the user knows the channels frequency
					sprintf(String, "%u.%05u", frequency / 100000, frequency % 100000);
					if (!gIsInSubMenu || edit_index < 0)
						UI_PrintString(String, menu_item_x1, menu_item_x2, 4, 8);
					else
						UI_PrintString(String, menu_item_x1, menu_item_x2, 5, 8);
				}
			}

			already_printed = true;
			break;
		}

		case MENU_SAVE:
			strcpy(String, gSubMenu_SAVE[gSubMenuSelection]);
			break;

		case MENU_TDR:
			strcpy(String, gSubMenu_RXMode[gSubMenuSelection]);
			break;

		case MENU_TOT:
			strcpy(String, gSubMenu_TOT[gSubMenuSelection]);
			break;

		#ifdef ENABLE_VOICE
			case MENU_VOICE:
				strcpy(String, gSubMenu_VOICE[gSubMenuSelection]);
				break;
		#endif

		case MENU_SC_REV:
			strcpy(String, gSubMenu_SC_REV[gSubMenuSelection]);
			break;

		case MENU_MDF:
			strcpy(String, gSubMenu_MDF[gSubMenuSelection]);
			break;

		case MENU_RP_STE:
			if (gSubMenuSelection == 0)
				strcpy(String, "OFF");
			else
				sprintf(String, "%d*100ms", gSubMenuSelection);
			break;

		case MENU_S_LIST:
			if (gSubMenuSelection < 2)
				sprintf(String, "LIST%u", 1 + gSubMenuSelection);
			else
				strcpy(String, "ALL");
			break;

		#ifdef ENABLE_ALARM
			case MENU_AL_MOD:
				sprintf(String, gSubMenu_AL_MOD[gSubMenuSelection]);
				break;
		#endif

		case MENU_ANI_ID:
			strcpy(String, gEeprom.ANI_DTMF_ID);
			break;

		case MENU_UPCODE:
			strcpy(String, gEeprom.DTMF_UP_CODE);
			break;

		case MENU_DWCODE:
			strcpy(String, gEeprom.DTMF_DOWN_CODE);
			break;

		case MENU_D_RSP:
			strcpy(String, gSubMenu_D_RSP[gSubMenuSelection]);
			break;

		case MENU_D_HOLD:
			sprintf(String, "%ds", gSubMenuSelection);
			break;

		case MENU_D_PRE:
			sprintf(String, "%d*10ms", gSubMenuSelection);
			break;

		case MENU_PTT_ID:
			strcpy(String, gSubMenu_PTT_ID[gSubMenuSelection]);
			break;

		case MENU_BAT_TXT:
			strcpy(String, gSubMenu_BAT_TXT[gSubMenuSelection]);
			break;

		case MENU_D_LIST:
			gIsDtmfContactValid = DTMF_GetContact((int)gSubMenuSelection - 1, Contact);
			if (!gIsDtmfContactValid)
				strcpy(String, "NULL");
			else
				memmove(String, Contact, 8);
			break;

		case MENU_PONMSG:
			strcpy(String, gSubMenu_PONMSG[gSubMenuSelection]);
			break;

		case MENU_ROGER:
			strcpy(String, gSubMenu_ROGER[gSubMenuSelection]);
			break;

		case MENU_VOL:
			sprintf(String, "%u.%02uV\n%u%%",
				gBatteryVoltageAverage / 100, gBatteryVoltageAverage % 100,
				BATTERY_VoltsToPercent(gBatteryVoltageAverage));
			break;

		case MENU_RESET:
			strcpy(String, gSubMenu_RESET[gSubMenuSelection]);
			break;

		case MENU_F_LOCK:
			strcpy(String, gSubMenu_F_LOCK[gSubMenuSelection]);
			break;

		#ifdef ENABLE_F_CAL_MENU
			case MENU_F_CALI:
				{
					const uint32_t value   = 22656 + gSubMenuSelection;
					const uint32_t xtal_Hz = (0x4f0000u + value) * 5;

					writeXtalFreqCal(gSubMenuSelection, false);

					sprintf(String, "%d\n%u.%06u\nMHz",
						gSubMenuSelection,
						xtal_Hz / 1000000, xtal_Hz % 1000000);
				}
				break;
		#endif

		case MENU_BATCAL:
		{
			const uint16_t vol = (uint32_t)gBatteryVoltageAverage * gBatteryCalibration[3] / gSubMenuSelection;
			sprintf(String, "%u.%02uV\n%u", vol / 100, vol % 100, gSubMenuSelection);
			break;
		}

		case MENU_BATTYP:
			strcpy(String, gSubMenu_BATTYP[gSubMenuSelection]);
			break;	

		case MENU_F1SHRT:
		case MENU_F1LONG:
		case MENU_F2SHRT:
		case MENU_F2LONG:
		case MENU_MLONG:
			strcpy(String, gSubMenu_SIDEFUNCTIONS[gSubMenuSelection].name);
			break;

	}

	if (!already_printed)
	{	// we now do multi-line text in a single string

		unsigned int y;
		unsigned int lines = 1;
		unsigned int len   = strlen(String);
		bool         small = false;

		if (len > 0)
		{
			// count number of lines
			for (i = 0; i < len; i++)
			{
				if (String[i] == '\n' && i < (len - 1))
				{	// found new line char
					lines++;
					String[i] = 0;  // null terminate the line
				}
			}

			if (lines > 3)
			{	// use small text
				small = true;
				if (lines > 7)
					lines = 7;
			}

			// center vertically'ish
			if (small)
				y = 3 - ((lines + 0) / 2);  // untested
			else
				y = 2 - ((lines + 0) / 2);

			// draw the text lines
			for (i = 0; i < len && lines > 0; lines--)
			{
				if (small)
					UI_PrintStringSmall(String + i, menu_item_x1, menu_item_x2, y);
				else
					UI_PrintString(String + i, menu_item_x1, menu_item_x2, y, 8);

				// look for start of next line
				while (i < len && String[i] >= 32)
					i++;

				// hop over the null term char(s)
				while (i < len && String[i] < 32)
					i++;

				y += small ? 1 : 2;
			}
		}
	}

	if (UI_MENU_GetCurrentMenuId() == MENU_SLIST1 || UI_MENU_GetCurrentMenuId() == MENU_SLIST2)
	{
		i = (UI_MENU_GetCurrentMenuId() == MENU_SLIST1) ? 0 : 1;

//		if (gSubMenuSelection == 0xFF)
		if (gSubMenuSelection < 0)
			strcpy(String, "NULL");
		else
			UI_GenerateChannelStringEx(String, true, gSubMenuSelection);

//		if (gSubMenuSelection == 0xFF || !gEeprom.SCAN_LIST_ENABLED[i])
		if (gSubMenuSelection < 0 || !gEeprom.SCAN_LIST_ENABLED[i])
		{
			// channel number
			UI_PrintString(String, menu_item_x1, menu_item_x2, 0, 8);

			// channel name
			BOARD_fetchChannelName(String, gSubMenuSelection);
			if (String[0] == 0)
				strcpy(String, "--");
			UI_PrintString(String, menu_item_x1, menu_item_x2, 2, 8);
		}
		else
		{
			// channel number
			UI_PrintString(String, menu_item_x1, menu_item_x2, 0, 8);

			// channel name
			BOARD_fetchChannelName(String, gSubMenuSelection);
			if (String[0] == 0)
				strcpy(String, "--");
			UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 2);

			if (IS_MR_CHANNEL(gEeprom.SCANLIST_PRIORITY_CH1[i]))
			{
				sprintf(String, "PRI1:%u", gEeprom.SCANLIST_PRIORITY_CH1[i] + 1);
				UI_PrintString(String, menu_item_x1, menu_item_x2, 3, 8);
			}

			if (IS_MR_CHANNEL(gEeprom.SCANLIST_PRIORITY_CH2[i]))
			{
				sprintf(String, "PRI2:%u", gEeprom.SCANLIST_PRIORITY_CH2[i] + 1);
				UI_PrintString(String, menu_item_x1, menu_item_x2, 5, 8);
			}
		}
	}

	if (UI_MENU_GetCurrentMenuId() == MENU_MEM_CH   ||
	    UI_MENU_GetCurrentMenuId() == MENU_DEL_CH   ||
	    UI_MENU_GetCurrentMenuId() == MENU_1_CALL)
	{	// display the channel name
		char s[11];
		BOARD_fetchChannelName(s, gSubMenuSelection);
		if (s[0] == 0)
			strcpy(s, "--");
		UI_PrintString(s, menu_item_x1, menu_item_x2, 2, 8);
	}

	if ((UI_MENU_GetCurrentMenuId() == MENU_R_CTCS || UI_MENU_GetCurrentMenuId() == MENU_R_DCS) && gCssBackgroundScan)
		UI_PrintString("SCAN", menu_item_x1, menu_item_x2, 4, 8);
		

	if (UI_MENU_GetCurrentMenuId() == MENU_UPCODE)
		if (strlen(gEeprom.DTMF_UP_CODE) > 8)
			UI_PrintString(gEeprom.DTMF_UP_CODE + 8, menu_item_x1, menu_item_x2, 4, 8);

	if (UI_MENU_GetCurrentMenuId() == MENU_DWCODE)
		if (strlen(gEeprom.DTMF_DOWN_CODE) > 8)
			UI_PrintString(gEeprom.DTMF_DOWN_CODE + 8, menu_item_x1, menu_item_x2, 4, 8);

	if (UI_MENU_GetCurrentMenuId() == MENU_D_LIST && gIsDtmfContactValid)
	{
		Contact[11] = 0;
		memmove(&gDTMF_ID, Contact + 8, 4);
		sprintf(String, "ID:%s", Contact + 8);
		UI_PrintString(String, menu_item_x1, menu_item_x2, 4, 8);
	}

	if (UI_MENU_GetCurrentMenuId() == MENU_R_CTCS ||
	    UI_MENU_GetCurrentMenuId() == MENU_T_CTCS ||
	    UI_MENU_GetCurrentMenuId() == MENU_R_DCS  ||
	    UI_MENU_GetCurrentMenuId() == MENU_T_DCS  ||
	    UI_MENU_GetCurrentMenuId() == MENU_D_LIST)
	{
		sprintf(String, "%2d", gSubMenuSelection);
		UI_PrintStringSmall(String, 105, 0, 0);
	}

	if ((UI_MENU_GetCurrentMenuId() == MENU_RESET    ||
	     UI_MENU_GetCurrentMenuId() == MENU_MEM_CH   ||
	     UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME ||
	     UI_MENU_GetCurrentMenuId() == MENU_DEL_CH) && gAskForConfirmation)
	{	// display confirmation
		strcpy(String, (gAskForConfirmation == 1) ? "SURE?" : "WAIT!");
		UI_PrintString(String, menu_item_x1, menu_item_x2, 5, 8);
	}

	ST7565_BlitFullScreen();
}
