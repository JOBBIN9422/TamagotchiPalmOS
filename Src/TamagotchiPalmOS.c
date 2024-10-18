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
#include <stdlib.h>

#include "TamagotchiPalmOS.h"
#include "TamagotchiPalmOS_Rsc.h"

#include "rom_12bit.h"

/*********************************************************************
 * Entry Points
 *********************************************************************/

/*********************************************************************
 * Global variables
 *********************************************************************/

/* MainFormHandleEventThunk
 * holds event callback thunk for main form event handler */
static _CW_EventHandlerThunk MainFormHandleEventThunk;

/* 
 * g_prefs
 * cache for application preferences during program execution 
 */
TamagotchiPalmOSPreferenceType g_prefs;

//the ROM loaded into memory
u12_t* g_program = NULL;
u32_t g_program_size = 0;

//display
bool_t icon_buffer[ICON_NUM] = {0};
UInt16 icon_button_buffer[ICON_NUM] = { FoodButton, LightsButton, GameButton, MedicineButton, CleanButton, StatsButton, DisciplineButton, AttentionButton };
timestamp_t screen_ts = 0;
RectangleType screen_bounds = { { LCD_OFFSET_X, LCD_OFFSET_Y }, { 32, 16 } };
BitmapType* screen_bmp = NULL;
void* screen_bmp_data = NULL;

//audio
u32_t current_freq = 0; // in dHz
unsigned int sin_pos = 0;
bool_t is_audio_playing = 0;

//HAL object
hal_t hal = {
	&hal_malloc,
	&hal_free,
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

static void* hal_malloc(u32_t size)
{
	return malloc(size);
}

static void hal_free(void* ptr)
{
	free(ptr);
}

static void hal_halt(void)
{
}

static void hal_sleep_until(timestamp_t ts)
{
	/*timestamp_t start = hal_get_timestamp();
	long long remaining = (long long)(ts - start);
	if (remaining > 0)
	{
		Int32 ticks_to_sleep = (Int32)(remaining * SysTicksPerSecond() / CLOCK_FREQ);
		SysTaskDelay(ticks_to_sleep);
	}*/
	
	//while ((long long)(ts - hal_get_timestamp()) > 0) { }
}

static timestamp_t hal_get_timestamp(void)
{
	return (timestamp_t)(TimGetTicks());
}



static inline void hal_set_lcd_icon(u8_t icon, bool_t val)
{
	icon_buffer[icon] = val;
}

static inline void hal_set_lcd_matrix(u8_t x, u8_t y, bool_t val)
{
	bool_t* pix_location = ((bool_t*)screen_bmp_data) + (y << 5) + x;
	*pix_location = val ? 0b11111111 : 0b0;
}

static inline void clear_screen(void)
{
	WinEraseRectangle(&screen_bounds, 0);
}

static inline void hal_update_screen(void)
{
	int i;
	ControlType* icon_button_ptr;
	
	WinDrawBitmap((BitmapPtr)screen_bmp, LCD_OFFSET_X, LCD_OFFSET_Y);
	
	for (i = 0; i < ICON_NUM; i++)
	{
		icon_button_ptr = (ControlType*)GetObjectPtr(icon_button_buffer[i]);
		if (icon_button_ptr)
		{
			if (icon_buffer[i] && !CtlEnabled(icon_button_ptr))
			{
				CtlShowControl(icon_button_ptr);
				CtlSetEnabled(icon_button_ptr, 1);
			}
			else if (!icon_buffer[i] && CtlEnabled(icon_button_ptr))
			{
				CtlHideControl(icon_button_ptr);
				CtlSetEnabled(icon_button_ptr, 0);
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

static inline int hal_handler(void)
{
	UInt16 error;
	EventType event;
	
	EvtGetEvent(&event, evtNoWait);
	
	if (event.eType == nilEvent || event.eType == sysNotifySleepRequestEvent)
	{
		return 0;
	}

	if ((event.eType == keyDownEvent)
	&& (TxtCharIsHardKey(event.data.keyDown.modifiers, event.data.keyDown.chr))
	&& (event.data.keyDown.chr >= vchrHard1)
	&& (event.data.keyDown.chr <= vchrHard4)
	&& (!(event.data.keyDown.modifiers & poweredOnKeyMask)))
	{
		return 0;
	}

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
	return event.eType == appStopEvent;
}

static bool_t hal_is_log_enabled(int level)
{
	return 0;
}

static void hal_log(int level, char *buff, ...) {}

static inline void poll_keys(void)
{
	UInt32 keyState = KeyCurrentState();
		
	if (keyState & keyBitPageDown)
	{
		tamalib_set_button(BTN_MIDDLE, BTN_STATE_PRESSED);
	}
	else
	{
		tamalib_set_button(BTN_MIDDLE, BTN_STATE_RELEASED);
	}
	if (keyState & keyBitHard2)
	{
		tamalib_set_button(BTN_LEFT, BTN_STATE_PRESSED);
	}
	else
	{
		tamalib_set_button(BTN_LEFT, BTN_STATE_RELEASED);
	}
	if (keyState & keyBitHard3)
	{
		tamalib_set_button(BTN_RIGHT, BTN_STATE_PRESSED);
	}
	else
	{
		tamalib_set_button(BTN_RIGHT, BTN_STATE_RELEASED);
	}
}

static void load_state_from_prefs(void)
{
	int i;
	state_t* load_state;
	
	tamalib_set_exec_mode(EXEC_MODE_PAUSE);
	
	//fetch current state from emulator
	load_state = tamalib_get_state();
	
	//load state primitives from preferences struct
	*(load_state->pc) = g_prefs.pc;
	*(load_state->x) = g_prefs.x;
	*(load_state->y) = g_prefs.y;
	*(load_state->a) = g_prefs.a;
	*(load_state->b) = g_prefs.b;
	*(load_state->np) = g_prefs.np;
	*(load_state->sp) = g_prefs.sp;
	*(load_state->flags) = g_prefs.flags;
	*(load_state->tick_counter) = g_prefs.tick_counter;
	*(load_state->clk_timer_timestamp) = g_prefs.clk_timer_timestamp;
	*(load_state->prog_timer_timestamp) = g_prefs.prog_timer_timestamp;
	*(load_state->prog_timer_enabled) = g_prefs.prog_timer_enabled;
	*(load_state->prog_timer_data) = g_prefs.prog_timer_data;
	*(load_state->prog_timer_rld) = g_prefs.prog_timer_rld;
	*(load_state->call_depth) = g_prefs.call_depth;
	
	//load interrupts from preference struct
	for (i = 0; i < INT_SLOT_NUM; i++)
	{
		load_state->interrupts[i].factor_flag_reg = g_prefs.interrupts[i].factor_flag_reg & 0xF;
		load_state->interrupts[i].mask_reg = g_prefs.interrupts[i].mask_reg & 0xF;
		load_state->interrupts[i].triggered = g_prefs.interrupts[i].triggered & 0x1;
	}

	//load memory from preference struct
	for (i = 0; i < MEM_BUFFER_SIZE; i++)
	{
		load_state->memory[i] = g_prefs.memory[i] & 0xF;
	}
	
	//read memory and unpause
	tamalib_refresh_hw();
	tamalib_set_exec_mode(EXEC_MODE_RUN);
}

static void save_state_to_prefs(void)
{
	UInt32 i;
	state_t* save_state;
	
	tamalib_set_exec_mode(EXEC_MODE_PAUSE);
	
	//fetch current state from emulator
	save_state = tamalib_get_state();
	
	//save state primitives to prefs struct
	g_prefs.pc = *(save_state->pc);
	g_prefs.x = *(save_state->x);
	g_prefs.y = *(save_state->y);
	g_prefs.a = *(save_state->a);
	g_prefs.b = *(save_state->b);
	g_prefs.np = *(save_state->np);
	g_prefs.sp = *(save_state->sp);
	g_prefs.flags = *(save_state->flags);
	g_prefs.tick_counter = *(save_state->tick_counter);
	g_prefs.clk_timer_timestamp = *(save_state->clk_timer_timestamp);
	g_prefs.prog_timer_timestamp = *(save_state->prog_timer_timestamp);
	g_prefs.prog_timer_enabled = *(save_state->prog_timer_enabled);
	g_prefs.prog_timer_data = *(save_state->prog_timer_data);
	g_prefs.prog_timer_rld = *(save_state->prog_timer_rld);
	g_prefs.call_depth = *(save_state->call_depth);
	
	//save interrupts to prefs struct
	for (i = 0; i < INT_SLOT_NUM; i++)
	{
		g_prefs.interrupts[i].factor_flag_reg = save_state->interrupts[i].factor_flag_reg & 0xF;
		g_prefs.interrupts[i].mask_reg  = save_state->interrupts[i].mask_reg & 0xF;
		g_prefs.interrupts[i].triggered = save_state->interrupts[i].triggered & 0x1;
	}
	
	//save memory to prefs struct
	
	for (i = 0; i < MEM_BUFFER_SIZE; i++)
	{
		g_prefs.memory[i] = save_state->memory[i] & 0xF;
	}
}

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
			ControlType * slider = NULL;
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
			slider = (ControlType *)FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, FrameSkipSlider));
			if (slider)
			{
				CtlSetSliderValues(slider, NULL, NULL, NULL, &g_prefs.frameskip);
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
				if (slider)
				{
					UInt16 slider_value;
					CtlGetSliderValues(slider, NULL, NULL, NULL, &slider_value);
					g_prefs.frameskip = slider_value;
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
	//UInt32 keyState;
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
	UInt16 render_steps_slept = 0;

	//event loop: read buttons, step the CPU, and draw at the specified interval
	while (!hal_handler())
	{
		poll_keys();
		tamalib_step();
		if (render_steps_slept < g_prefs.frameskip)
		{
			render_steps_slept++;
		}
		else
		{
			render_steps_slept = 0;
			hal_update_screen();
		}
	}
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
	UInt16 bmp_error;
	
	//load ROM and init tama emu
	g_program = program_load(&g_program_size);	
	tamalib_register_hal(&hal);
	tamalib_init(g_program, NULL, (u32_t)SysTicksPerSecond());
	tamalib_set_speed(0);
	
	//try to allocate the BMP for drawing the screen
	screen_bmp = BmpCreate(LCD_WIDTH, LCD_HEIGHT, 8, NULL, &bmp_error);
	screen_bmp_data = BmpGetBits(screen_bmp); 
	
	/* Read the saved preferences / saved-state information. */
	prefsSize = sizeof(g_prefs);
	if (PrefGetAppPreferences(
		appFileCreator, appPrefID, &g_prefs, &prefsSize, true) == 
		noPreferenceFound)
	{
		/* no prefs; initialize pref struct with default values */
		g_prefs.frameskip = 20;
	}
	else
	{
		load_state_from_prefs();
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
	BmpDelete(screen_bmp);
	
	save_state_to_prefs();
		
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
