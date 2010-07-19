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
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>


#include "pwm.h"
#include "debug.h"
#include "info.h"
#include "sram.h"
#include "util.h"
#include "uart.h"
#include "dump.h"
#include "irq.h"
#include "config.h"
#include "crc.h"
#include "command.h"
#include "shared_memory.h"
#include "system.h"


extern system_t system;

const char STR_ROM[] PROGMEM = "Rom";
const char STR_RAM[] PROGMEM = "Sram";
const char STR_BAT[] PROGMEM = "Battery";
const char STR_SUPERFX[] PROGMEM = "SuperFX";
const char STR_SA[] PROGMEM = "SA-1";


uint8_t command_buf[RECEIVE_BUF_LEN];
uint8_t recv_buf[RECEIVE_BUF_LEN];

volatile uint8_t recv_counter = 0;
volatile uint8_t cr = 0;

uint8_t *token_ptr;

#if DO_SHELL

uint8_t *get_token(void)
{
    uint8_t *p = token_ptr;
    while (*p == ' ')
        p++;
    if (*p == '\0')
        return NULL;
    token_ptr = p;
    do {
        token_ptr++;
        if (*token_ptr == ' ' || *token_ptr == '\n' || *token_ptr == '\r') {
            *token_ptr++ = '\0';
            break;
        }
    } while (*token_ptr != ' ' && *token_ptr != '\n' && *token_ptr != '\r');
    return p;
}

uint8_t get_dec(uint32_t * decval)
{
    const uint8_t *t;
    t = get_token();
    if (t != NULL) {
        int x = util_sscandec(t);
        if (x < 0)
            return 0;
        *decval = x;
        return 1;
    }
    return 0;
}

uint8_t parse_hex(const uint8_t * s, uint32_t * hexval)
{
    uint32_t x = util_sscanhex(s);
    *hexval = (uint32_t) x;
    return 1;
}

uint8_t get_hex(uint32_t * hexval)
{
    const uint8_t *t;
    t = get_token();
    if (t != NULL)
        return parse_hex(t, hexval);
    return 0;
}

uint8_t get_hex_arg2(uint32_t * hexval1, uint32_t * hexval2)
{
    return get_hex(hexval1) && get_hex(hexval2);
}

uint8_t get_hex_arg3(uint32_t * hexval1, uint32_t * hexval2, uint32_t * hexval3)
{
    return get_hex(hexval1) && get_hex(hexval2) && get_hex(hexval3);
}

#if 0
static uint8_t get_int32(uint32_t * val)
{
    if (!get_hex(val)) {
        info_P(PSTR("Invalid argument!\n"));
        return 0;
    } else {
        return 1;
    }
}

static uint8_t get_int8(uint8_t * val)
{
    uint32_t ret;
    if (!get_hex(&ret) || ret > 0xff) {
        info_P(PSTR("Invalid argument!\n"));
        return 0;
    } else {
        *val = (uint8_t) ret;
        return 1;
    }
}
#endif
static int get_bool(void)
{
    const uint8_t *t;
    t = get_token();
    if (t != NULL) {
        int result = util_sscanbool(t);
        if (result >= 0)
            return result;
    }
    info_P(PSTR("Invalid argument (should be 0 or 1)!\n"));
    return -1;
}
void prompt(void)
{

    uart_putc('\r');
    uart_putc('\n');
    uart_putc('>');

}

ISR(USART0_RX_vect)
{
    UCSR0B &= (255 - (1 << RXCIE0));    // Interrupts disable for RxD
    sei();
    if (recv_counter == (sizeof(recv_buf) - 1)) {
        cr = 1;
        recv_buf[recv_counter] = '\0';
        recv_counter = 0;
        prompt();
    }
    recv_buf[recv_counter] = UDR0;
    uart_putc(recv_buf[recv_counter]);
    if (recv_buf[recv_counter] == 0x0d) {
        /*
         * recv_buf[recv_counter] = 0; 
         */
        cr = 1;
        recv_buf[++recv_counter] = '\0';
        recv_counter = 0;
        prompt();
    } else {
        // we accept backspace or delete
        if ((recv_buf[recv_counter] == 0x08 || recv_buf[recv_counter] == 0x7f)
            && recv_counter > 0) {
            recv_counter--;
        } else {
            recv_counter++;
        }
    }
    UCSR0B |= (1 << RXCIE0);
}

enum cmds {
    CMD_DUMP,
    CMD_DUMPVEC,
    CMD_DUMPHEADER,
    CMD_CRC,
    CMD_EXIT,
    CMD_RESET,
    CMD_RESETSNIFF,
    CMD_IRQ,
    CMD_AVR,
    CMD_SNES,
    CMD_LOROM,
    CMD_HIROM,
    CMD_WR,
    CMD_SHMWR,
    CMD_SHMSAVE,
    CMD_SHMRESTORE,
    CMD_LOADER,
    CMD_RECONNECT,
    CMD_STATUS,
    CMD_SYS,
    CMD_HELP
};

uint8_t cmdlist[][CMD_HELP] PROGMEM = {
    {"DUMP"},
    {"DUMPVEC"},
    {"DUMPHEADER"},
    {"CRC"},
    {"EXIT"},
    {"RESET"},
    {"RESETSNIFF"},
    {"IRQ"},
    {"AVR"},
    {"SNES"},
    {"LOROM"},
    {"HIROM"},
    {"WR"},
    {"SHMWR"},
    {"SHMSAVE"},
    {"SHMRESTORE"},
    {"LOADER"},
    {"RECONNECT"},
    {"STATUS"},
    {"SYS"},
    {"HELP"},
};


void shell_help(void)
{
    uint8_t i;
    info_P(PSTR("\n"));
    for (i = CMD_DUMP; i < CMD_HELP; i++) {
        info_P((PGM_P) cmdlist[i]);
        info_P(PSTR("\n"));

    }
}


void shell_run(void)
{
    uint8_t *t;
    uint32_t arg1;
    uint32_t arg2;
    uint16_t crc;
    uint16_t offset;
    uint8_t c;

    if (!cr)
        return;
    cr = 0;
    strcpy((char *) command_buf, (char *) recv_buf);

    token_ptr = command_buf;
    t = get_token();

    if (t == NULL)
        shell_help();

    util_strupper(t);

    if (strcmp_P((const char *) t, (PGM_P) cmdlist[CMD_DUMP]) == 0) {
        if (get_hex_arg2(&arg1, &arg2))
            dump_memory(arg1, arg2);
        else
            info_P(PSTR("DUMP <start addr> <end addr>\n"));

    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_CRC]) == 0) {
        if (get_hex_arg2(&arg1, &arg2)) {
            crc = crc_check_bulk_memory(arg1, arg2, 0x8000);
            info_P(PSTR("0x%06lx - 0x%06lx crc=0x%04x\n"), arg1, arg2, crc);
        } else
            info_P(PSTR("CRC <start addr> <end addr>\n"));
    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_EXIT]) == 0) {
        leave_application();
    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_RESET]) == 0) {
        system_send_snes_reset();
    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_IRQ]) == 0) {
        info_P(PSTR("Send IRQ\n"));
        snes_irq_on();
        snes_irq_lo();
        _delay_us(20);
        snes_irq_hi();
        snes_irq_off();
    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_AVR]) == 0) {
        system_set_bus_avr();
        snes_irq_lo();
        system_snes_irq_off();

    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_SNES]) == 0) {
        snes_irq_lo();
        system_snes_irq_off();
        system_set_wr_disable();
        system_set_bus_snes();

    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_LOROM]) == 0) {
        system_set_rom_lorom();
        system_set_wr_disable();

    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_HIROM]) == 0) {
        system_set_rom_hirom();
        system_set_wr_disable();
    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_WR]) == 0) {
        arg1 = get_bool();
        if (arg1 == 1) {
            info_P(PSTR("Set WR enable"));
            snes_wr_enable();
        } else if (arg1 == 0) {
            info_P(PSTR("Set WR disable"));
            snes_wr_disable();
        }
    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_RESETSNIFF]) == 0) {
        arg1 = get_bool();
        if (arg1 == 1) {
            info_P(PSTR("Start Reset sniffer"));
            irq_init();
        } else if (arg1 == 0) {
            info_P(PSTR("Stop Reset sniffer"));
            irq_stop();
        }
    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_DUMPVEC]) == 0) {
        uint16_t offset;
        if (system.rom_mode == LOROM)
            offset = 0x8000;
        else
            offset = 0x0000;

        info_P(PSTR("ABORT	0x%04x 0x%04x\n"), (0xFFE8 - offset),
               sram_read16_be(0xFFE8 - offset));
        info_P(PSTR("BRK	0x%04x 0x%04x\n"), (0xFFE6 - offset),
               sram_read16_be(0xFFE6 - offset));
        info_P(PSTR("COP	0x%04x 0x%04x\n"), (0xFFE4 - offset),
               sram_read16_be(0xFFE4 - offset));
        info_P(PSTR("IRQ	0x%04x 0x%04x\n"), (0xFFEE - offset),
               sram_read16_be(0xFFEE - offset));
        info_P(PSTR("NMI	0x%04x 0x%04x\n"), (0xFFEA - offset),
               sram_read16_be(0xFFEA - offset));
        info_P(PSTR("RES	0x%04x 0x%04x\n"), (0xFFFC - offset),
               sram_read16_be(0xFFFC - offset));

    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_DUMPHEADER]) == 0) {

        if (system.rom_mode == LOROM)
            offset = 0x8000;
        else
            offset = 0x0000;
        /*
         * # $ffc0..$ffd4 => Name of the ROM, typically in ASCII, using spaces to pad the name to 21 bytes. # $ffd5 => ROM layout,
         * typically $20 for LoROM, or $21 for HiROM. Add $10 for FastROM. # $ffd6 => Cartridge type, typically $00 for ROM only, or $02
         * for ROM with save-RAM. # $ffd7 => ROM size byte. # $ffd8 => RAM size byte. # $ffd9 => Country code, which selects the video in
         * the emulator. Values $00, $01, $0d use NTSC. Values in range $02..$0c use PAL. Other values are invalid. # $ffda => Licensee
         * code. If this value is $33, then the ROM has an extended header with ID at $ffb2..$ffb5. # $ffdb => Version number, typically
         * $00. # $ffdc..$ffdd => Checksum complement, which is the bitwise-xor of the checksum and $ffff. # $ffde..$ffdf => SNES checksum, 
         * an unsigned 16-bit checksum of bytes. # $ffe0..$ffe3 => Unknown. 
         */
        info_P(PSTR("NAME	0x%04x "), (0xffc0 - offset));
        for (arg1 = (0xffc0 - offset); arg1 < (0xffc0 - offset + 21); arg1++) {
            c = sram_read(arg1);
            if (c > 0x1f && c < 0x7f)
                printf("%c", c);
        }
        printf("\n");
        c = sram_read(0xffd5 - offset);
        info_P(PSTR("LAYOUT	0x%04x "), (0xffd5 - offset));

        switch (c) {
            case 0x20:
                info_P(PSTR("LoROM, not fast\n"));
                break;
            case 0x21:
                info_P(PSTR("HiRom, not fast\n"));
                break;
            case 0x30:
                info_P(PSTR("LoROM, fast\n"));
                break;
            case 0x31:
                info_P(PSTR("HiRom, fast\n"));
                break;
            default:
                info_P(PSTR("Unkown 0x%02x\n"), c);
                break;
        }

        c = sram_read(0xffd6 - offset);
        info_P(PSTR("TYPE	0x%04xc"), (0xffd6 - offset), c);
        switch (c) {
            case 0x00:
                info_P(PSTR("Rom\n"));
                break;
            case 0x01:
                info_P(PSTR("Rom + Sram\n"));
                break;
            case 0x02:
                info_P(PSTR("Rom + Sram + Battery\n"));
                break;
            case 0x13:
                info_P(PSTR("SuperFX\n"));
                break;
            case 0x14:
                info_P(PSTR("SuperFX\n"));
                break;
            case 0x15:
                info_P(PSTR("SuperFX + Sram\n"));
                break;
            case 0x1a:
                info_P(PSTR("SuperFX + Sram\n"));
                break;
            case 0x34:
                info_P(PSTR("SA-1"));
                break;
            case 0x35:
                info_P(PSTR("SA-1"));
                break;
            default:
                info_P(PSTR("Unkown 0x%02x\n"), c);
                break;
        }
        arg1 = (2 << (sram_read(0xffd7 - offset) - 1));
        info_P(PSTR("ROM	0x%04x %li MBit ( %li KiB)\n"),
               (0xffd7 - offset), (arg1 / 128), arg1);
        arg1 = (2 << (sram_read(0xffd8 - offset) - 1));
        info_P(PSTR("RAM	0x%04x %li KiB\n"), (0xffd8 - offset), arg1);

        info_P(PSTR("CCODE	0x%04x "), (0xffd9 - offset));
        c = sram_read(0xffd9 - offset);
        if (c == 0x00 || c == 0x01 || 0x0d)
            info_P(PSTR("NTSC\n"));
        else if (c >= 0x02 || c <= 0x0c)
            info_P(PSTR("PAL\n"));
        else
            info_P(PSTR("Unkown 0x%02x\n"), c);

        info_P(PSTR("LIC	0x%04x 0x%02x\n"), (0xffda - offset),
               sram_read(0xffda - offset));
        info_P(PSTR("VER	0x%04x 0x%02x\n"), (0xffdb - offset),
               sram_read(0xffdb - offset));
        info_P(PSTR("SUM1	0x%04x 0x%04x\n"), (0xffdc - offset),
               sram_read16_be(0xffdc - offset));
        info_P(PSTR("SUM2	0x%04x 0x%04x\n"), (0xffde - offset),
               sram_read16_be(0xffde - offset));


    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_SHMWR]) == 0) {
        if (get_hex_arg2(&arg1, &arg2))
            shared_memory_write((uint8_t) arg1, (uint8_t) arg1);
        else
            info_P(PSTR("SHMWR <command> <value>\n"));
    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_SHMSAVE]) == 0) {
        shared_memory_scratchpad_region_tx_save();
        shared_memory_scratchpad_region_rx_save();
        info_P(PSTR("Save scratchpad\n"));
    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_SHMRESTORE]) == 0) {
        shared_memory_scratchpad_region_tx_restore();
        shared_memory_scratchpad_region_rx_restore();
        info_P(PSTR("Restore scratchpad\n"));
    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_LOADER]) == 0) {
        boot_startup_rom(500);
    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_RECONNECT]) == 0) {
        usb_connect();
    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_STATUS]) == 0) {
        transaction_status();
    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_SYS]) == 0) {
        system_status();
    } else if (strcmp_P((char *) t, (PGM_P) cmdlist[CMD_HELP]) == 0) {
        shell_help();
    }
    prompt();
}

#endif
