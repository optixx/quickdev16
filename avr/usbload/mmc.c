#include "mmc.h"
#include <util/delay.h>
uint8_t mmc_init()
{
    uint16_t Timeout = 0, i;

    // Konfiguration des Ports an der die MMC/SD-Karte angeschlossen wurde
    DDRB |= ((1 << MMC_DO) | (1 << MMC_CS) | (1 << MMC_CLK));
    DDRB &= ~(1 << MMC_DI);
    PORTB |= ((1 << MMC_DO) | (1 << MMC_DI) | (1 << MMC_CS));

    // Wartet eine kurze Zeit
    _delay_ms(20);

    // Initialisiere MMC/SD-Karte in den SPI-Mode
    for (i = 0; i < 250; i++) {
        PORTB ^= (1 << MMC_CLK);
        _delay_us(4);
    }
    PORTB &= ~(1 << MMC_CLK);
    _delay_us(10);
    PORTB &= ~(1 << MMC_CS);
    _delay_us(3);

    // Sendet Commando CMD0 an MMC/SD-Karte
    uint8_t CMD[] = { 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };
    while (mmc_write_command(CMD) != 1) {
        if (Timeout++ > 20) {
            MMC_Disable();
            return (1);         // Abbruch bei Commando1 (Return Code1)
        }
    }

    // Sendet Commando CMD1 an MMC/SD-Karte
    Timeout = 0;
    CMD[0] = 0x41;              // Commando 1
    CMD[5] = 0xFF;
    while (mmc_write_command(CMD) != 0) {
        if (Timeout++ > 800) {
            MMC_Disable();
            return (9);         // Abbruch bei Commando2 (Return Code2)
        }
    }
    return (0);
}

    // ############################################################################
    // Sendet ein Commando an die MMC/SD-Karte
uint8_t mmc_write_command(uint8_t *cmd)
// ############################################################################
{
    uint8_t tmp = 0xff;
    uint16_t Timeout = 0;
    uint8_t a;

    // sendet 6 Byte Commando
    for (a = 0; a < 0x06; a++)  // sendet 6 Byte Commando zur MMC/SD-Karte
    {
        mmc_write_byte(*cmd++);
    }

    // Wartet auf ein gültige Antwort von der MMC/SD-Karte
    while (tmp == 0xff) {
        tmp = mmc_read_byte();
        if (Timeout++ > 50) {
            break;              // Abbruch da die MMC/SD-Karte nicht Antwortet
        }
    }
    return (tmp);
}

    // ############################################################################
    // Routine zum Empfangen eines Bytes von der MMC-Karte 
uint8_t mmc_read_byte(void)
// ############################################################################
{
    uint8_t Byte = 0, j;
    for (j = 0; j < 8; j++) {
        Byte = (Byte << 1);
        PORTB |= (1 << MMC_CLK);
        _delay_us(4);
        if (PINB & (1 << MMC_DI)) {
            Byte |= 1;
        }

        else {
            Byte &= ~1;
        }
        PORTB &= ~(1 << MMC_CLK);
        _delay_us(4);
    }
    return (Byte);
}

    // ############################################################################
    // Routine zum Senden eines Bytes zur MMC-Karte
void mmc_write_byte(uint8_t Byte)
// ############################################################################
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        if (Byte & 0x80) {
            PORTB |= (1 << MMC_DO);
        }

        else {
            PORTB &= ~(1 << MMC_DO);
        }
        Byte = (Byte << 1);
        PORTB |= (1 << MMC_CLK);
        _delay_us(4);
        PORTB &= ~(1 << MMC_CLK);
        _delay_us(4);
    }
    PORTB |= (1 << MMC_DO);
}

    // ############################################################################
    // Routine zum schreiben eines Blocks(512Byte) auf die MMC/SD-Karte
uint8_t mmc_write_sector(uint32_t addr, uint8_t *Buffer)
// ############################################################################
{
    uint8_t tmp;

    // Commando 24 zum schreiben eines Blocks auf die MMC/SD - Karte
    uint8_t cmd[] = { 0x58, 0x00, 0x00, 0x00, 0x00, 0xFF };
    uint8_t a;
    uint16_t i;

    /*
     * Die Adressierung der MMC/SD-Karte wird in Bytes angegeben, addr wird von Blocks zu Bytes umgerechnet danach werden  diese in
     * das Commando eingefügt
     */
    addr = addr << 9;           // addr = addr * 512
    cmd[1] = ((addr & 0xFF000000) >> 24);
    cmd[2] = ((addr & 0x00FF0000) >> 16);
    cmd[3] = ((addr & 0x0000FF00) >> 8);

    // Sendet Commando cmd24 an MMC/SD-Karte (Write 1 Block/512 Bytes)
    tmp = mmc_write_command(cmd);
    if (tmp != 0) {
        return (tmp);
    }
    // Wartet einen Moment und sendet einen Clock an die MMC/SD-Karte
    for (a = 0; a < 100; a++) {
        mmc_read_byte();
    }

    // Sendet Start Byte an MMC/SD-Karte
    mmc_write_byte(0xFE);

    // Schreiben des Bolcks (512Bytes) auf MMC/SD-Karte
    for (a = 0; i < 512; i++) {
        mmc_write_byte(*Buffer++);
    }

    // CRC-Byte schreiben
    mmc_write_byte(0xFF);       // Schreibt Dummy CRC
    mmc_write_byte(0xFF);       // CRC Code wird nicht benutzt

    // Fehler beim schreiben? (Data Response XXX00101 = OK)
    if ((mmc_read_byte() & 0x1F) != 0x05)
        return (1);

    // Wartet auf MMC/SD-Karte Bussy
    while (mmc_read_byte() != 0xff) {
    };
    return (0);
}

    // ############################################################################
    // Routine zum lesen des CID Registers von der MMC/SD-Karte (16Bytes)
void mmc_read_block(uint8_t *cmd, uint8_t *Buffer,
                    uint16_t Bytes)
// ############################################################################
{
    uint16_t a;

    // Sendet Commando cmd an MMC/SD-Karte
    if (mmc_write_command(cmd) != 0) {
        return;
    }
    // Wartet auf Start Byte von der MMC/SD-Karte (FEh/Start Byte)
    while (mmc_read_byte() != 0xfe) {
    };

    // Lesen des Bolcks (normal 512Bytes) von MMC/SD-Karte
    for (a = 0; a < Bytes; a++) {
        *Buffer++ = mmc_read_byte();
    }

    // CRC-Byte auslesen
    mmc_read_byte();            // CRC - Byte wird nicht ausgewertet
    mmc_read_byte();            // CRC - Byte wird nicht ausgewertet
    return;
}

    // ############################################################################
    // Routine zum lesen eines Blocks(512Byte) von der MMC/SD-Karte
uint8_t mmc_read_sector(uint32_t addr, uint8_t *Buffer)
// ############################################################################
{

    // Commando 16 zum lesen eines Blocks von der MMC/SD - Karte
    uint8_t cmd[] = { 0x51, 0x00, 0x00, 0x00, 0x00, 0xFF };

    /*
     * Die Adressierung der MMC/SD-Karte wird in Bytes angegeben, addr wird von Blocks zu Bytes umgerechnet danach werden  diese in
     * das Commando eingefügt
     */
    addr = addr << 9;           // addr = addr * 512
    cmd[1] = ((addr & 0xFF000000) >> 24);
    cmd[2] = ((addr & 0x00FF0000) >> 16);
    cmd[3] = ((addr & 0x0000FF00) >> 8);
    mmc_read_block(cmd, Buffer, 512);
    return (0);
}

    // ############################################################################
    // Routine zum lesen des CID Registers von der MMC/SD-Karte (16Bytes)
uint8_t mmc_read_cid(uint8_t *Buffer)
// ############################################################################
{

    // Commando zum lesen des CID Registers
    uint8_t cmd[] = { 0x4A, 0x00, 0x00, 0x00, 0x00, 0xFF };
    mmc_read_block(cmd, Buffer, 16);
    return (0);
}

    // ############################################################################
    // Routine zum lesen des CSD Registers von der MMC/SD-Karte (16Bytes)
uint8_t mmc_read_csd(uint8_t *Buffer)
// ############################################################################
{

    // Commando zum lesen des CSD Registers
    uint8_t cmd[] = { 0x49, 0x00, 0x00, 0x00, 0x00, 0xFF };
    mmc_read_block(cmd, Buffer, 16);
    return (0);
}
