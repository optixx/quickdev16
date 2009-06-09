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
  scratch_buffer = (unsigned char*)malloc(SHARED_MAX_SIZE);
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


void FATFS::fetchMem(unsigned int len) {
  for ( int i=0;i<len;i++){
      scratch_buffer[i] = bus.read(SHARED_ADDR + i);
  } 
}

void FATFS::pushMem(unsigned int len) {
  for ( int i=0;i<len;i++){
      bus.write(SHARED_ADDR + i,scratch_buffer[i]);
      #ifdef FATFS_DEBUG
      if ( i < 4)
        printf("0x%02x ",scratch_buffer[i]);
      #endif
  }
  #ifdef FATFS_DEBUG
  printf("\n");
  #endif
}

uint8 FATFS::mmio_read(unsigned addr) {
  addr &= 0xffff;
  if (addr == MMIO_RETVAL){
      #ifdef FATFS_DEBUG
      printf("BSNES::mmio_read retal=%i\n",retval);
      #endif
      return retval;
  }
  return cpu.regs.mdr;
}

void FATFS::mmio_write(unsigned addr, uint8 data) {
  addr &= 0xffff;
  #ifdef FATFS_DEBUG
  printf("BSNES::mmio_write 0x%04x 0x%02x (%i)\n",addr,data,data);
  #endif
  if (addr == 0x3010){
      switch(data){
          case CMD_INIT:
              #ifdef FATFS_DEBUG
              printf("BSNES::mmio_write CMD_INIT \n");
              #endif
              command = CMD_INIT;
              retval = disk_initialize(0);
              break;
          case CMD_READ:
              #ifdef FATFS_DEBUG
              printf("BSNES::mmio_write CMD_READ \n");
              #endif
              command = CMD_READ;
              break;
          case CMD_WRITE:
              command = CMD_WRITE;
              #ifdef FATFS_DEBUG
              printf("BSNES::mmio_write CMD_WRITE \n");
              #endif
              break;
          default:
              command = CMD_NONE;
              #ifdef FATFS_DEBUG
              printf("BSNES::mmio_write CMD_NONE \n");
              #endif
              break;
      }
      fflush(stderr);
  }
  if (addr >= MMIO_SECTOR01 && addr <= MMIO_SECTOR04){
      if (addr == MMIO_SECTOR01)
        sector = 0;
      sector |= data << ( (3 - ( addr - MMIO_SECTOR01))  << 3);
      #ifdef FATFS_DEBUG
      printf("BSNES::mmio_write set sector: byte=%i val=%i sector=%i \n",(3 - ( addr - MMIO_SECTOR01)),data,sector);
      #endif
  }
  if (addr == MMIO_COUNT){
      count = data;
      #ifdef FATFS_DEBUG
      printf("BSNES::mmio_write set count: count=%i \n",count);
      #endif
      if (command == CMD_READ) {
          retval = disk_read (0, (BYTE*)scratch_buffer, sector, count);
          if (!retval)
              pushMem(512 * count);
      } else if (command == CMD_WRITE) {
            fetchMem(512 * count);
            retval = disk_write (0, (BYTE*)scratch_buffer, sector, count);
      } else{
            #ifdef FATFS_DEBUG
            printf("BSNES::mmio_write set offset: no command to trigger \n");
            #endif
      }
  }
  if (addr == MMIO_RETVAL){
      #ifdef FATFS_DEBUG
      printf("BSNES::mmio_write reg 0x3016 reset\n");
      #endif
      retval = STA_VOID;
  }
}

FATFS::FATFS() {
}
