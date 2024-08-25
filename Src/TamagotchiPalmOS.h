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

#endif /* TAMAGOTCHIPALMOS_H_ */
