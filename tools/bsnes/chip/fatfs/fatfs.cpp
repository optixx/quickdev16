#include <../base.hpp>
#include <../cart/cart.hpp>
#include "fatfs.hpp"


#include "diskio.h"
#include "config.h"



void FATFS::init() {
  command = CMD_NONE;
  sector = 0;
  count = 0;
  retval = -1;
  scratch_buffer = (unsigned char*)malloc(SHARED_SIZE);
}

void FATFS::enable() {
  // command
  memory::mmio.map(MMIO_CMD, *this);
  // sector
  memory::mmio.map(MMIO_SECTOR01, *this);
  memory::mmio.map(MMIO_SECTOR02, *this);
  memory::mmio.map(MMIO_SECTOR03, *this);
  memory::mmio.map(MMIO_SECTOR04, *this);
  // count
  memory::mmio.map(MMIO_COUNT, *this);
  // return value
  memory::mmio.map(MMIO_RETVAL, *this);
}

void FATFS::power() {
  reset();
}

void FATFS::reset() {
}


void FATFS::fetchMem() {
  for ( int i=0;i<SHARED_SIZE;i++){
      scratch_buffer[i] = bus.read(SHARED_ADDR + i);
  } 
}

void FATFS::pushMem() {
  for ( int i=0;i<SHARED_SIZE;i++){
      bus.write(SHARED_ADDR + i,scratch_buffer[i]);
      if ( i < 8)
        printf("0x%02x ",scratch_buffer[i]);
  }
  printf("\n");
}

uint8 FATFS::mmio_read(unsigned addr) {
  addr &= 0xffff;
  if (addr == MMIO_RETVAL){
      printf("FATFS::mmio_read retal=%i\n",retval);
      return retval;
  }
  return cpu.regs.mdr;
}

void FATFS::mmio_write(unsigned addr, uint8 data) {
  addr &= 0xffff;
  printf("FATFS::mmio_write 0x%04x 0x%02x (%i)\n",addr,data,data);
  if (addr == 0x3010){
      switch(data){
          case CMD_INIT:
              printf("FATFS::mmio_write CMD_INIT \n");
              command = CMD_INIT;
              retval = disk_initialize(0);
              break;
          case CMD_READ:
              printf("FATFS::mmio_write CMD_READ \n");
              command = CMD_READ;
              break;
          case CMD_WRITE:
              command = CMD_WRITE;
              printf("FATFS::mmio_write CMD_WRITE \n");
              break;
          default:
              command = CMD_NONE;
              printf("FATFS::mmio_write CMD_NONE \n");
              break;
      }
      fflush(stderr);
  }
  if (addr >= MMIO_SECTOR01 && addr <= MMIO_SECTOR04){
      sector = data << ( (3 - ( addr - MMIO_SECTOR01))  << 3);
      printf("FATFS::mmio_write set sector: byte=%i val=%i sector=%i \n",(3 - ( addr - MMIO_SECTOR01)),data,sector);
  }
  if (addr == MMIO_COUNT){
      count = data;
      printf("FATFS::mmio_write set count: count=%i \n",count);
      if (command == CMD_READ) {
          retval = disk_read (0, (BYTE*)scratch_buffer, sector, count);
          if (!retval)
              pushMem();
      } else if (command == CMD_WRITE) {
            fetchMem();
            retval = disk_write (0, (BYTE*)scratch_buffer, sector, count);
      } else{
            printf("FATFS::mmio_write set offset: no command to trigger \n");
      }
  }
  if (addr == MMIO_RETVAL){
      printf("FATFS::mmio_write reg 0x3016 reset\n");
      retval = STA_VOID;
  }
}

FATFS::FATFS() {
}
