/*
 * TamaLIB - A hardware agnostic Tamagotchi P1 emulation library
 *
 * Copyright (C) 2021 Jean-Christophe Rona <jc@rona.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef _HW_H_
#define _HW_H_

#include "hal.h"

#define LCD_WIDTH			32
#define LCD_HEIGHT			16

#define ICON_NUM			8

//btn_state_t enum replacement
#define BTN_STATE_RELEASED 0
#define BTN_STATE_PRESSED 1

//button_t enum replacement
#define BTN_LEFT 0
#define BTN_MIDDLE 1
#define BTN_RIGHT 2

bool_t hw_init(void);
void hw_release(void);

void hw_set_lcd_pin(u8_t seg, u8_t com, u8_t val);
void hw_set_button(int btn, int state);

void hw_set_buzzer_freq(u4_t freq);
void hw_enable_buzzer(bool_t en);

#endif /* _HW_H_ */
