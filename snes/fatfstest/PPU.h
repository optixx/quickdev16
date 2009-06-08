extern byte tileMapLocation[4];
extern word characterLocation[4];

void waitForVBlank(void);
void setTileMapLocation(word vramDst, byte screenProp, byte bgNumber);
void restoreTileMapLocation(byte bgNumber);
void setCharacterLocation(word vramDst, byte bgNumber);
void restoreCharacterLocation(byte bgNumber);
void VRAMByteWrite(byte value, word vramDst);
void VRAMLoad(word src, word vramDst, word size);
void CGRAMLoad(word src, byte cgramDst, word size);
