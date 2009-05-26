class FATFS : public MMIO {
public:
 
  void init();
  void enable();
  void power();
  void reset();


  void fetchMem();
  void pushMem();


  uint8 mmio_read (unsigned addr);
  void  mmio_write(unsigned addr, uint8 data);

  FATFS();
private:
  char command;
  int  sector;
  char count;
  char retval;
  char *scratch_buffer;
};

extern FATFS fatfs;
