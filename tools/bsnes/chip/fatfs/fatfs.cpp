#include <../base.hpp>
#include <../cart/cart.hpp>
#include "fatfs.hpp"


#include "diskio.h"

#define CMD_INIT        0x00
#define CMD_READ        0x01
#define CMD_WRITE       0x02
#define CMD_NONE        0xff

#define SHARED_SIZE     512
#define SHARED_ADDR     0x3f0000

void FATFS::init() {
  command = CMD_NONE;
  sector = 0;
  count = 0;
  retval = -1;
  scratch_buffer = (char*)malloc(SHARED_SIZE);
}

void FATFS::enable() {
  // command
  memory::mmio.map(0x3010, *this);
  // sector
  memory::mmio.map(0x3011, *this);
  memory::mmio.map(0x3012, *this);
  memory::mmio.map(0x3013, *this);
  memory::mmio.map(0x3014, *this);
  // offset
  memory::mmio.map(0x3015, *this);
  // return value
  memory::mmio.map(0x3016, *this);
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
  }
}

uint8 FATFS::mmio_read(unsigned addr) {
  addr &= 0xffff;
  if (addr == 0x3016){
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
              disk_initialize(0);
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
  if (addr >= 0x3011 && addr <= 0x3014){
      sector = data << ( (3 - ( addr - 0x3011))  << 3);
      printf("FATFS::mmio_write set sector: byte=%i val=%i sector=%i \n",(3 - ( addr - 0x3011)),data,sector);
  }
  if (addr == 0x3015){
      count = data;
      printf("FATFS::mmio_write set offset: countr=%i \n",count);
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
  if (addr == 0x3016){
      printf("FATFS::mmio_write reg 0x3016 retav is RW\n");
  }
}

FATFS::FATFS() {
}
