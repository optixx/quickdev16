#include <../base.hpp>
#include <../cart/cart.hpp>
#include "cmmio.hpp"


void CMMIO::init() {
}

void CMMIO::enable() {
  memory::mmio.map(0x3000, *this);
  memory::mmio.map(0x3001, *this);
  memory::mmio.map(0x3002, *this);
  memory::mmio.map(0x3004, *this);
}

void CMMIO::power() {
  reset();
}

void CMMIO::reset() {
}

uint8 CMMIO::mmio_read(unsigned addr) {
  addr &= 0xffff;
  //printf("CMMIO::mmio_read 0x%x",addr);
  return cpu.regs.mdr;
}

void CMMIO::mmio_write(unsigned addr, uint8 data) {
  addr &= 0xffff;
  //fflush(stdout);
  //printf("CMMIO::mmio_write 0x%04x 0x%02x (%i)\n",addr,data,data);
  /* debug to stderr */
  if (addr == 0x3000){
      fprintf(stderr,"%c",data);
      fflush(stderr);
      
  }

#if 0
  if (addr == 0x3001){
      fprintf(stderr,"Trigger IRQ\n");
      fflush(stderr);
      cpu.triggerIRQ();
  }
#endif
}

CMMIO::CMMIO() {
}
