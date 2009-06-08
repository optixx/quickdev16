typedef struct padStatus {
    byte right:1;
    byte left:1;
    byte down:1;
    byte up:1;
    byte start:1;               // Enter
    byte select:1;              // Space
    byte Y:1;                   // X
    byte B:1;                   // C
    // --------------------------------
    byte Dummy:4;
    byte R:1;                   // Z
    byte L:1;                   // A
    byte X:1;                   // S
    byte A:1;                   // D
} padStatus;

extern void enablePad(void);
extern void disablePad(void);
extern padStatus readPad(byte padNumber);
