/*
 * =====================================================================================
 *
 * ________        .__        __    ________               ____  ________
 * \_____  \  __ __|__| ____ |  | __\______ \   _______  _/_   |/  _____/
 *  /  / \  \|  |  \  |/ ___\|  |/ / |    |  \_/ __ \  \/ /|   /   __  \
 * /   \_/.  \  |  /  \  \___|    <  |    `   \  ___/\   / |   \  |__\  \
 * \_____\ \_/____/|__|\___  >__|_ \/_______  /\___  >\_/  |___|\_____  /
 *        \__>             \/     \/        \/     \/                 \/
 *
 *                                  www.optixx.org
 *
 *
 *        Version:  1.0
 *        Created:  07/21/2009 03:32:16 PM
 *         Author:  david@optixx.org
 *
 * =====================================================================================
 */
#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "pwm.h"
#include "debug.h"
#include "info.h"
#include "sram.h"


#define PWM_SINE_MAX 64
#define PWM_OVERFLOW_MAX 1024

#if 0

uint8_t pwm_sine_table[]  = {
0x7f,0x8b,0x97,0xa4,0xaf,0xbb,0xc5,0xcf,0xd9,0xe1,0xe8,0xef,0xf4,0xf8,0xfb,0xfd,
0xfd,0xfd,0xfb,0xf8,0xf3,0xee,0xe7,0xe0,0xd7,0xce,0xc4,0xb9,0xae,0xa2,0x96,0x89,
0x7e,0x71,0x65,0x59,0x4d,0x42,0x37,0x2d,0x24,0x1c,0x15,0x0f,0x09,0x05,0x03,0x01,
0x01,0x01,0x03,0x07,0x0b,0x11,0x17,0x1f,0x28,0x31,0x3b,0x46,0x52,0x5e,0x6a,0x76
};

volatile uint8_t pwm_setting;                   
volatile uint16_t pwm_overflow;                    
volatile uint8_t pwm_idx;                    
volatile uint16_t pwm_overflow_max;                    
 
 ISR(TIMER2_COMPA_vect) {
    static uint8_t pwm_cnt=0;
    OCR2A += (uint16_t)T_PWM;
        
    if (pwm_setting> pwm_cnt) 
        led_pwm_on();
    else
        led_pwm_off();
    
    if (pwm_cnt==(uint8_t)(PWM_STEPS-1))
        pwm_cnt=0;
    else
        pwm_cnt++;
    if (pwm_overflow_max == pwm_overflow++ ){
        pwm_setting = pwm_sine_table[pwm_idx++];
        pwm_overflow = 0;
        if (PWM_SINE_MAX == pwm_idx)
            pwm_idx = 0;
    }
}

void pwm_speed(uint16_t val) {
    pwm_overflow_max = val; 
}

void pwm_speed_slow(uint16_t val) {
    pwm_overflow_max = PWM_OVERFLOW_MAX * 2 ; 
}

void pwm_speed_fast(uint16_t val) {
    pwm_overflow_max = PWM_OVERFLOW_MAX / 2; 
}

void pwm_speed_normal(uint16_t val) {
    pwm_overflow_max = PWM_OVERFLOW_MAX; 
}


void pwm_set(uint8_t val) {
    pwm_setting = val;
}

void pwm_stop(void) {
    while(pwm_setting!=0xfd);
    TIMSK2 = 0;
}

void pwm_init(void) {
    pwm_overflow_max = PWM_OVERFLOW_MAX;
    pwm_setting = 0x7f;
    pwm_overflow = 0;
    //cli();
    TCCR2B = 1;         
    TIMSK2 |= (1<<OCIE2A);       
    sei();                  
}
#endif