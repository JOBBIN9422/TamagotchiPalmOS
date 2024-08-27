/*
 * TamagotchiPalmOS.h
 *
 * header file for TamagotchiPalmOS
 *
 * This wizard-generated code is based on code adapted from the
 * stationery files distributed as part of the Palm OS SDK 4.0.
 *
 * Copyright (c) 1999-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 */
 
#ifndef TAMAGOTCHIPALMOS_H_
#define TAMAGOTCHIPALMOS_H_

#include "hal_types.h"

/*********************************************************************
 * Internal Structures
 *********************************************************************/

typedef struct TamagotchiPalmOSPreferenceType
{
	Boolean pref1;
	char pref2[256];
} TamagotchiPalmOSPreferenceType;

/*********************************************************************
 * Global variables
 *********************************************************************/

extern TamagotchiPalmOSPreferenceType g_prefs;

/*********************************************************************
 * Internal Constants
 *********************************************************************/

#define appFileCreator			'JOBN'
#define appName					"TamagotchiPalmOS"
#define appVersionNum			0x01
#define appPrefID				0x00
#define appPrefVersionNum		0x01

/*********************************************************************
 * Functions
 *********************************************************************/

static void AppStop(void);

u12_t* program_load(u32_t* size);

static void hal_halt(void);
static void hal_sleep_until(timestamp_t ts);
static timestamp_t hal_get_timestamp(void);
static void hal_set_lcd_matrix(u8_t x, u8_t y, bool_t val);
static void hal_set_lcd_icon(u8_t icon, bool_t val);
static void hal_update_screen(void);
static void hal_set_frequency(u32_t freq);
static void hal_play_frequency(bool_t en);
static int hal_handler(void);
static bool_t hal_is_log_enabled(int level);
static void hal_log(int level, char *buff, ...);

#endif /* TAMAGOTCHIPALMOS_H_ */
