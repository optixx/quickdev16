class CMMIO : public MMIO {
public:
 
  void init();
  void enable();
  void power();
  void reset();

  uint8 mmio_read (unsigned addr);
  void  mmio_write(unsigned addr, uint8 data);

  CMMIO();

};

extern CMMIO cmmio;
