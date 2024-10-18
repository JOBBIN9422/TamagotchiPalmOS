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
#include "tamalib.h"


/*********************************************************************
 * Internal Constants
 *********************************************************************/

#define appFileCreator			'JOBN'
#define appName					"TamagotchiPalmOS"
#define appVersionNum			0x01
#define appPrefID				0x00
#define appPrefVersionNum		0x01

#define LCD_OFFSET_X 64
#define LCD_OFFSET_Y 72

//#define CLOCK_FREQ 1000
//#define TARGET_FPS 30

/*********************************************************************
 * Internal Structures
 *********************************************************************/

typedef struct TamagotchiPalmOSPreferenceType
{
	UInt16 frameskip;
	
	//emulator state
	u13_t pc;
	u12_t x;
	u12_t y;
	u4_t a;
	u4_t b;
	u5_t np;
	u8_t sp;
	u4_t flags;

	u32_t tick_counter;
	u32_t clk_timer_timestamp;
	u32_t prog_timer_timestamp;
	bool_t prog_timer_enabled;
	u8_t prog_timer_data;
	u8_t prog_timer_rld;

	u32_t call_depth;

	//interrupt_t members
	/*u4_t factor_flag_reg;
	u4_t mask_reg;
	bool_t triggered;
	u8_t vector;*/
	
	interrupt_t interrupts[INT_SLOT_NUM];
	
	MEM_BUFFER_TYPE memory[MEM_BUFFER_SIZE];
	
} TamagotchiPalmOSPreferenceType;

/*********************************************************************
 * Global variables
 *********************************************************************/

//app settings
extern TamagotchiPalmOSPreferenceType g_prefs;

//Tama ROM
extern u12_t* g_program;
extern u32_t g_program_size;

//drawing
extern bool_t icon_buffer[ICON_NUM];
extern UInt16 icon_button_buffer[ICON_NUM];
extern timestamp_t screen_ts;
extern RectangleType screen_bounds;
extern BitmapType* screen_bmp;
extern void* screen_bmp_data;
extern UInt32 screen_bmp_index_table[LCD_HEIGHT][LCD_WIDTH];

//Tama audio
extern u32_t current_freq; // in dHz
extern unsigned int sin_pos;
extern bool_t is_audio_playing;

//HAL object
extern hal_t hal;

/*********************************************************************
 * Functions
 *********************************************************************/

static void AppStop(void);

u12_t* program_load(u32_t* size);

static void* hal_malloc(u32_t size);
static void hal_free(void* ptr);
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

static void clear_screen(void);
static void poll_keys(void);

static void load_state_from_prefs(void);
static void save_state_to_prefs(void);

static void calc_screen_bmp_index_table(void);

static Boolean AppHandleEvent(EventType * eventP);

static void * GetObjectPtr(UInt16 objectID);

#endif /* TAMAGOTCHIPALMOS_H_ */
