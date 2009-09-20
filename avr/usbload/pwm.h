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

#ifndef __PWM_H__
#define __PWM_H__

#define F_PWM 100                       // PWM-Frequenz in Hz
#define PWM_STEPS 256                   // PWM-Schritte pro Zyklus(1..256)
 
#define T_PWM (F_CPU/(F_PWM*PWM_STEPS)) // Systemtakte pro PWM-Takt
 
#if (T_PWM<(93+5))
    #error T_PWM zu klein, F_CPU muss vergrösst werden oder F_PWM oder PWM_STEPS verkleinert werden
#endif

 void pwm_init(void);
void pwm_stop(void);
 
 
#endif
