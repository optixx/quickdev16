class FATFS : public MMIO {
public:
 
  void init();
  void enable();
  void power();
  void reset();


  void fetchMem(unsigned int len);
  void pushMem(unsigned int len);


  uint8 mmio_read (unsigned addr);
  void  mmio_write(unsigned addr, uint8 data);

  FATFS();
private:
  char command;
  int  sector;
  char count;
  char retval;
  unsigned char *scratch_buffer;
};

extern FATFS fatfs;
