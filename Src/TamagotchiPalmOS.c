/*
 * TamagotchiPalmOS.c
 *
 * main file for TamagotchiPalmOS
 *
 * This wizard-generated code is based on code adapted from the
 * stationery files distributed as part of the Palm OS SDK 4.0.
 *
 * Copyright (c) 1999-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 */
 
#include <PalmOS.h>
#include <PalmOSGlue.h>
#include <CWCallbackThunks.h>
#include <time.h>
#include <cstdio>
#include <stdio.h>

#include "TamagotchiPalmOS.h"
#include "TamagotchiPalmOS_Rsc.h"

#include "rom_12bit.h"
#include "tamalib.h"

/*********************************************************************
 * Entry Points
 *********************************************************************/

/*********************************************************************
 * Global variables
 *********************************************************************/
/* 
 * g_prefs
 * cache for application preferences during program execution 
 */
TamagotchiPalmOSPreferenceType g_prefs;

/* MainFormHandleEventThunk
 * holds event callback thunk for main form event handler */

static _CW_EventHandlerThunk MainFormHandleEventThunk;

//the ROM loaded into memory
static u12_t* g_program = NULL;
static u32_t g_program_size = 0;

//screen buffer
static bool_t matrix_buffer[LCD_HEIGHT][LCD_WIDTH] = {{0}};

static timestamp_t screen_ts = 0;

//icon buffer
static bool_t icon_buffer[ICON_NUM] = {0};

//audio
static u32_t current_freq = 0; // in dHz
static unsigned int sin_pos = 0;
static bool_t is_audio_playing = 0;


//HAL object

static hal_t hal = {
	NULL,
	NULL,
	&hal_halt,
	&hal_is_log_enabled,
	&hal_log,
	&hal_sleep_until,
	&hal_get_timestamp,
	&hal_update_screen,
	&hal_set_lcd_matrix,
	&hal_set_lcd_icon,
	&hal_set_frequency,
	&hal_play_frequency,
	&hal_handler,
};


/*********************************************************************
 * Internal Constants
 *********************************************************************/

/* Define the minimum OS version we support */
#define ourMinVersion    sysMakeROMVersion(3,0,0,sysROMStageDevelopment,0)
#define kPalmOS20Version sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0)

/*********************************************************************
 * Internal Functions
 *********************************************************************/

u12_t* program_load(u32_t* size)
{
	u12_t *program;
	program = (u12_t*)g_program_b12;
	
	*size = sizeof(g_program_b12) / sizeof(g_program_b12[0]);
	
	return program;
}

static void hal_halt(void)
{
	AppStop();
}

static void hal_sleep_until(timestamp_t ts)
{
	/*timestamp_t start = hal_get_timestamp();
	int remaining = (int) (ts - start);
	
	while (remaining > 0)
	{
		timestamp_t elapsed = hal_get_timestamp() - start;
		remaining = remaining - elapsed;
	}*/
}

static timestamp_t hal_get_timestamp(void)
{
	return clock() * 1000000 / CLOCKS_PER_SEC;
}



static void hal_set_lcd_icon(u8_t icon, bool_t val)
{
	icon_buffer[icon] = val;
}

static void hal_set_lcd_matrix(u8_t x, u8_t y, bool_t val)
{
	matrix_buffer[y][x] = val;
}

static void hal_update_screen(void)
{
	unsigned int y, x;
	
	for (y = 0; y < LCD_HEIGHT; y++)
	{
		for (x = 0; x < LCD_WIDTH; x++)
		{
			if (matrix_buffer[y][x])
			{
				WinDrawPixel(x + LCD_OFFSET_X, y + LCD_OFFSET_Y);

			}
			else
			{
				WinErasePixel(x + LCD_OFFSET_X, y + LCD_OFFSET_Y);

			}
		}
	}
}

static void hal_set_frequency(u32_t freq)
{
	if (current_freq != freq)
	{
		current_freq = freq;
		sin_pos = 0;
	}
}

static void hal_play_frequency(bool_t en)
{
	if (is_audio_playing != en)
	{
		is_audio_playing = en;
	}
}

static int hal_handler(void)
{
	return 0;
}

static bool_t hal_is_log_enabled(int level)
{
	return 0;
}

static void hal_log(int level, char *buff, ...) {}

/*
 * FUNCTION: GetObjectPtr
 *
 * DESCRIPTION:
 *
 * This routine returns a pointer to an object in the current form.
 *
 * PARAMETERS:
 *
 * formId
 *     id of the form to display
 *
 * RETURNED:
 *     address of object as a void pointer
 */

static void * GetObjectPtr(UInt16 objectID)
{
	FormType * frmP;

	frmP = FrmGetActiveForm();
	return FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, objectID));
}

/*
 * FUNCTION: MainFormInit
 *
 * DESCRIPTION: This routine initializes the MainForm form.
 *
 * PARAMETERS:
 *
 * frm
 *     pointer to the MainForm form.
 */

static void MainFormInit(FormType *frmP)
{
	/*
	FieldType *field;
	const char *wizardDescription;
	UInt16 fieldIndex;

	
	fieldIndex = FrmGetObjectIndex(frmP, MainDescriptionField);
	field = (FieldType *)FrmGetObjectPtr(frmP, fieldIndex);
	FrmSetFocus(frmP, fieldIndex);

	wizardDescription =
		"C application\n"
		"Creator Code: JOBN\n"
		"\n"
		"Other SDKs:\n"
		;
				
	//dont stack FldInsert calls, since each one generates a
	//fldChangedEvent, and multiple uses can overflow the event queue 
	FldInsert(field, wizardDescription, StrLen(wizardDescription));*/
}

/*
 * FUNCTION: MainFormDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:
 *
 * command
 *     menu item id
 */

static Boolean MainFormDoCommand(UInt16 command)
{
	Boolean handled = false;

	switch (command)
	{
		case OptionsAboutTamagotchiPalmOS:
		{
			FormType * frmP;

			/* Clear the menu status from the display */
			MenuEraseStatus(0);

			/* Display the About Box. */
			frmP = FrmInitForm (AboutForm);
			FrmDoDialog (frmP);                    
			FrmDeleteForm (frmP);

			handled = true;
			break;
		}
		case OptionsPreferences:
		{
			FormType * frmP;
			ControlType * cbox = NULL;
			FieldType * field = NULL;			
			UInt16 controlID = 0;
			MemHandle handle = 0;

			/* Clear the menu status from the display */
			MenuEraseStatus(0);

			/* Initialize the preference form. */
			frmP = FrmInitForm (PrefsForm);

			/* 
			 * Set the controls in the preference dialog to reflect our 
			 * preference data structure
			 */
			cbox = (ControlType *)FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, PrefsSetting1Checkbox));
			if (cbox)
				CtlSetValue(cbox, g_prefs.pref1);
			
			field = (FieldType *)FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, PrefsSetting2Field));
			if (field && (g_prefs.pref2[0] != '\0'))
			{
				MemPtr text = NULL;
				UInt32 len = StrLen(g_prefs.pref2);
				
				handle = FldGetTextHandle(field);
				if (!handle)
					handle = MemHandleNew(len + 1);
				else
					MemHandleResize(handle, len + 1);

				text = MemHandleLock(handle);
				if (text)
					StrCopy((char *)text, g_prefs.pref2);

				MemHandleUnlock(handle);
				FldSetTextHandle(field, handle);
				
				handle = 0;
			}

			/* 
			 * Display the preferences dialog. The call will block until 
			 * the dialog is dismissed
			 */
			controlID = FrmDoDialog (frmP);
			
			/* controlID contains the ID of the button used to dismiss the dialog */
			if (controlID == PrefsOKButton)
			{
				/* 
				 * The user hit the OK button. Get the value of the controls 
				 * and store them in our pref struct 
				 */
				if (cbox)
					g_prefs.pref1 = CtlGetValue(cbox);
					
				if (field)
				{
					handle = FldGetTextHandle(field);
					if (handle)
					{				
						MemPtr text = MemHandleLock(handle);
						if (text)
						{
							/* Guard against the field text being longer than our pref's buffer */
							UInt32 len = StrLen((const char *)text);
							UInt32 count = (len > (sizeof(g_prefs.pref2) - 1)) ? (sizeof(g_prefs.pref2) - 1) : len;
							MemMove(g_prefs.pref2, text, count);
							g_prefs.pref2[count] = '\0';
						}

						MemHandleUnlock(handle);
					}
				}
			}
			
			/* Clean up */
			FrmDeleteForm (frmP);

			handled = true;
			break;
		}

	}

	return handled;
}

/*
 * FUNCTION: MainFormHandleEvent
 *
 * DESCRIPTION:
 *
 * This routine is the event handler for the "MainForm" of this 
 * application.
 *
 * PARAMETERS:
 *
 * eventP
 *     a pointer to an EventType structure
 *
 * RETURNED:
 *     true if the event was handled and should not be passed to
 *     FrmHandleEvent
 */

static Boolean MainFormHandleEvent(EventType * eventP)
{
	Boolean handled = false;
	FormType * frmP;

	switch (eventP->eType) 
	{
		case menuEvent:
			return MainFormDoCommand(eventP->data.menu.itemID);

		case frmOpenEvent:
			frmP = FrmGetActiveForm();
			FrmDrawForm(frmP);
			MainFormInit(frmP);
			
			handled = true;
			break;
            
        case frmUpdateEvent:
			/* 
			 * To do any custom drawing here, first call
			 * FrmDrawForm(), then do your drawing, and
			 * then set handled to true. 
			 */
			break;
			
		case ctlSelectEvent:
		{
			if (eventP->data.ctlSelect.controlID == MainClearTextButton)
			{
				break;
			}

			break;
		}
		
		case penDownEvent:
			//int screenX = eventP->screenX;
			//int screenY = eventP->screenY;
			WinDrawPixel(eventP->screenX, eventP->screenY);
			handled = true;
			break;
	}
    
	return handled;
}

/*
 * FUNCTION: AppHandleEvent
 *
 * DESCRIPTION: 
 *
 * This routine loads form resources and set the event handler for
 * the form loaded.
 *
 * PARAMETERS:
 *
 * event
 *     a pointer to an EventType structure
 *
 * RETURNED:
 *     true if the event was handled and should not be passed
 *     to a higher level handler.
 */

static Boolean AppHandleEvent(EventType * eventP)
{
	UInt16 formId;
	FormType * frmP;

	if (eventP->eType == frmLoadEvent)
	{
		/* Load the form resource. */
		formId = eventP->data.frmLoad.formID;
		frmP = FrmInitForm(formId);
		FrmSetActiveForm(frmP);

		/* 
		 * Set the event handler for the form.  The handler of the
		 * currently active form is called by FrmHandleEvent each
		 * time is receives an event. 
		 */
		switch (formId)
		{
			case MainForm:
				FrmSetEventHandler(frmP, (FormEventHandlerType *)&MainFormHandleEventThunk);
				break;

		}
		return true;
	}

	return false;
}

/*
 * FUNCTION: AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the application.
 */

static void AppEventLoop(void)
{
	UInt16 error;
	EventType event;
	timestamp_t ts;

	do 
	{	
		//tamalib_mainloop();
		if (!hal_handler())
		{
			tamalib_step();
			ts = hal_get_timestamp();
			if (ts - screen_ts >= 1000000 / 30)
			{
				screen_ts = ts;
				hal_update_screen();
			}
		}
		
		
		/* change timeout if you need periodic nilEvents */
		//EvtGetEvent(&event, evtWaitForever);
		EvtGetEvent(&event, evtNoWait);

		if (! SysHandleEvent(&event))
		{
			if (! MenuHandleEvent(0, &event, &error))
			{
				if (! AppHandleEvent(&event))
				{
					FrmDispatchEvent(&event);
				}
			}
		}
	} while (event.eType != appStopEvent);
}

/*
 * FUNCTION: AppStart
 *
 * DESCRIPTION:  Get the current application's preferences.
 *
 * RETURNED:
 *     errNone - if nothing went wrong
 */

static Err AppStart(void)
{
	UInt16 prefsSize;
	/* Read the saved preferences / saved-state information. */
	prefsSize = sizeof(g_prefs);
	if (PrefGetAppPreferences(
		appFileCreator, appPrefID, &g_prefs, &prefsSize, true) == 
		noPreferenceFound)
	{
		/* no prefs; initialize pref struct with default values */
		g_prefs.pref1 = false;
		g_prefs.pref2[0] = '\0';
	}
	/* Setup main form event handler callback thunk (needed for "expanded" mode) */
	_CW_GenerateEventThunk(MainFormHandleEvent, &MainFormHandleEventThunk);

	return errNone;
}

/*
 * FUNCTION: AppStop
 *
 * DESCRIPTION: Save the current state of the application.
 */

static void AppStop(void)
{
	/* 
	 * Write the saved preferences / saved-state information.  This
	 * data will be saved during a HotSync backup. 
	 */
	PrefSetAppPreferences(
		appFileCreator, appPrefID, appPrefVersionNum, 
		&g_prefs, sizeof(g_prefs), true);
        
	/* Close all the open forms. */
	FrmCloseAllForms();

}

/*
 * FUNCTION: RomVersionCompatible
 *
 * DESCRIPTION: 
 *
 * This routine checks that a ROM version is meet your minimum 
 * requirement.
 *
 * PARAMETERS:
 *
 * requiredVersion
 *     minimum rom version required
 *     (see sysFtrNumROMVersion in SystemMgr.h for format)
 *
 * launchFlags
 *     flags that indicate if the application UI is initialized
 *     These flags are one of the parameters to your app's PilotMain
 *
 * RETURNED:
 *     error code or zero if ROM version is compatible
 */

static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
	UInt32 romVersion;

	/* See if we're on in minimum required version of the ROM or later. */
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	if (romVersion < requiredVersion)
	{
		if ((launchFlags & 
			(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
			(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
		{
			FrmAlert (RomIncompatibleAlert);

			/* Palm OS versions before 2.0 will continuously relaunch this
			 * app unless we switch to another safe one. */
			if (romVersion < kPalmOS20Version)
			{
				AppLaunchWithCommand(
					sysFileCDefaultApp, 
					sysAppLaunchCmdNormalLaunch, NULL);
			}
		}

		return sysErrRomIncompatible;
	}

	return errNone;
}

/*
 * FUNCTION: PilotMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 * 
 * PARAMETERS:
 *
 * cmd
 *     word value specifying the launch code. 
 *
 * cmdPB
 *     pointer to a structure that is associated with the launch code
 *
 * launchFlags
 *     word value providing extra information about the launch.
 *
 * RETURNED:
 *     Result of launch, errNone if all went OK
 */

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	Err error;

	error = RomVersionCompatible (ourMinVersion, launchFlags);
	if (error) return (error);
			
	g_program = program_load(&g_program_size);	
	tamalib_register_hal(&hal);
	tamalib_init(g_program, NULL, 1000000);

	switch (cmd)
	{
		case sysAppLaunchCmdNormalLaunch:
			error = AppStart();
			if (error) 
				return error;

			/* 
			 * start application by opening the main form
			 * and then entering the main event loop 
			 */
			FrmGotoForm(MainForm);
			AppEventLoop();

			AppStop();
			break;
	}

	return errNone;
}
