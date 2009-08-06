#include "mmc.h"
#include <util/delay.h>

uint8_t mmc_init()
{
    uint16_t Timeout = 0, i;
    MMC_REG |= ((1 << MMC_DO) | (1 << MMC_CS) | (1 << MMC_CLK));
    MMC_REG &= ~(1 << MMC_DI);
    MMC_WRITE |= ((1 << MMC_DO) | (1 << MMC_DI) | (1 << MMC_CS));
    _delay_ms(20);
    for (i = 0; i < 250; i++) {
        MMC_WRITE ^= (1 << MMC_CLK);
        _delay_us(4);
    }
    MMC_WRITE &= ~(1 << MMC_CLK);
    _delay_us(10);
    MMC_WRITE &= ~(1 << MMC_CS);
    _delay_us(3);

    uint8_t CMD[] = { 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };
    while (mmc_write_command(CMD) != 1) {
        if (Timeout++ > 20) {
            mmc_disable();
            return (1);
        }
    }

    Timeout = 0;
    CMD[0] = 0x41;
    CMD[5] = 0xFF;
    while (mmc_write_command(CMD) != 0) {
        if (Timeout++ > 800) {
            mmc_disable();
            return (9);
        }
    }
    return (0);
}

uint8_t mmc_write_command(uint8_t * cmd)
{
    uint8_t tmp = 0xff;
    uint16_t Timeout = 0;
    uint8_t a;

    for (a = 0; a < 0x06; a++) {
        mmc_write_byte(*cmd++);
    }

    while (tmp == 0xff) {
        tmp = mmc_read_byte();
        if (Timeout++ > 50) {
            break;
        }
    }
    return (tmp);
}


uint8_t mmc_read_byte(void)
{
    uint8_t Byte = 0, j;
    for (j = 0; j < 8; j++) {
        Byte = (Byte << 1);
        MMC_WRITE |= (1 << MMC_CLK);
        _delay_us(4);
        if (PINB & (1 << MMC_DI)) {
            Byte |= 1;
        }

        else {
            Byte &= ~1;
        }
        MMC_WRITE &= ~(1 << MMC_CLK);
        _delay_us(4);
    }
    return (Byte);
}


void mmc_write_byte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        if (Byte & 0x80) {
            MMC_WRITE |= (1 << MMC_DO);
        }

        else {
            MMC_WRITE &= ~(1 << MMC_DO);
        }
        Byte = (Byte << 1);
        MMC_WRITE |= (1 << MMC_CLK);
        _delay_us(4);
        MMC_WRITE &= ~(1 << MMC_CLK);
        _delay_us(4);
    }
    MMC_WRITE |= (1 << MMC_DO);
}

uint8_t mmc_write_sector(uint32_t addr, uint8_t * Buffer)
{
    uint8_t tmp;


    uint8_t cmd[] = { 0x58, 0x00, 0x00, 0x00, 0x00, 0xFF };
    uint8_t a;
    uint16_t i;

    addr = addr << 9;
    cmd[1] = ((addr & 0xFF000000) >> 24);
    cmd[2] = ((addr & 0x00FF0000) >> 16);
    cmd[3] = ((addr & 0x0000FF00) >> 8);


    tmp = mmc_write_command(cmd);
    if (tmp != 0) {
        return (tmp);
    }

    for (a = 0; a < 100; a++) {
        mmc_read_byte();
    }

    mmc_write_byte(0xFE);

    for (a = 0; i < 512; i++) {
        mmc_write_byte(*Buffer++);
    }

    mmc_write_byte(0xFF);
    mmc_write_byte(0xFF);

    if ((mmc_read_byte() & 0x1F) != 0x05)
        return (1);

    while (mmc_read_byte() != 0xff) {
    };
    return (0);
}


void mmc_read_block(uint8_t * cmd, uint8_t * Buffer, uint16_t Bytes)
{
    uint16_t a;

    if (mmc_write_command(cmd) != 0) {
        return;
    }

    while (mmc_read_byte() != 0xfe) {
    };


    for (a = 0; a < Bytes; a++) {
        *Buffer++ = mmc_read_byte();
    }

    mmc_read_byte();
    mmc_read_byte();
    return;
}

uint8_t mmc_read_sector(uint32_t addr, uint8_t * Buffer)
{


    uint8_t cmd[] = { 0x51, 0x00, 0x00, 0x00, 0x00, 0xFF };

    addr = addr << 9;
    cmd[1] = ((addr & 0xFF000000) >> 24);
    cmd[2] = ((addr & 0x00FF0000) >> 16);
    cmd[3] = ((addr & 0x0000FF00) >> 8);
    mmc_read_block(cmd, Buffer, 512);
    return (0);
}


uint8_t mmc_read_cid(uint8_t * Buffer)
{

    uint8_t cmd[] = { 0x4A, 0x00, 0x00, 0x00, 0x00, 0xFF };
    mmc_read_block(cmd, Buffer, 16);
    return (0);
}


uint8_t mmc_read_csd(uint8_t * Buffer)
{

    uint8_t cmd[] = { 0x49, 0x00, 0x00, 0x00, 0x00, 0xFF };
    mmc_read_block(cmd, Buffer, 16);
    return (0);
}
