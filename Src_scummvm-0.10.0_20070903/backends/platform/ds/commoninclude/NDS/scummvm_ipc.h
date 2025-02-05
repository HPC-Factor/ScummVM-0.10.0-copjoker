
#ifndef SCUMMVM_IPC_INCLUDE
#define SCUMMVM_IPC_INCLUDE

//////////////////////////////////////////////////////////////////////

#include <nds/jtypes.h>
#include <nds/ipc.h>

//////////////////////////////////////////////////////////////////////

 
typedef struct _adpcmBuffer {
	u8* buffer[8];
	bool filled[8];
	u8* arm7Buffer[8];
	bool arm7Dirty[8];
	bool semaphore;
} adpcmBuffer;
 
//////////////////////////////////////////////////////////////////////

typedef struct scummvmTransferRegion {
  uint32 heartbeat;          // counts frames
 
   int16 touchX,   touchY;   // TSC X, Y
   int16 touchXpx, touchYpx; // TSC X, Y pixel values
   int16 touchZ1,  touchZ2;  // TSC x-panel measurements
  uint16 tdiode1,  tdiode2;  // TSC temperature diodes
  uint32 temperature;        // TSC computed temperature
 
  uint16 buttons;            // X, Y, /PENIRQ buttons
 
  union {
    uint8 curtime[8];        // current time response from RTC
 
    struct {
      u8 rtc_command;
      u8 rtc_year;           //add 2000 to get 4 digit year
      u8 rtc_month;          //1 to 12
      u8 rtc_day;            //1 to (days in month)
 
      u8 rtc_incr;
      u8 rtc_hours;          //0 to 11 for AM, 52 to 63 for PM
      u8 rtc_minutes;        //0 to 59
      u8 rtc_seconds;        //0 to 59
    };
  };
 
  uint16 battery;            // battery life ??  hopefully.  :)
  uint16 aux;                // i have no idea...
 
  pTransferSound soundData;
  
  adpcmBuffer adpcm;
  
 
  // Don't rely on these below, will change or be removed in the future
  vuint32 mailAddr;
  vuint32 mailData;
  vuint8 mailRead;
  vuint8 mailBusy;
  vuint32 mailSize;
  
  bool performArm9SleepMode;
  
  u32 test;
  int tweak;
  bool tweakChanged;
  
//  bool fillSoundFirstHalf;
//  bool fillSoundSecondHalf;

  // These are used for ScummVMs sound output
  bool fillNeeded[4];
  int playingSection;
  
  bool reset;
  
  // Streaming sound
  bool streamFillNeeded[4];
  int streamPlayingSection;
} scummTransferRegion, * pscummTransferRegion;

//////////////////////////////////////////////////////////////////////

#undef IPC
#define IPC ((scummTransferRegion volatile *)(0x027FF000))


#endif



#ifndef SCUMMVM_IPC_INCLUDE
#define SCUMMVM_IPC_INCLUDE

//////////////////////////////////////////////////////////////////////

#include <nds/jtypes.h>
#include <nds/ipc.h>

//////////////////////////////////////////////////////////////////////

 
typedef struct _adpcmBuffer {
	u8* buffer[8];
	bool filled[8];
	u8* arm7Buffer[8];
	bool arm7Dirty[8];
	bool semaphore;
} adpcmBuffer;
 
//////////////////////////////////////////////////////////////////////

typedef struct scummvmTransferRegion {
  uint32 heartbeat;          // counts frames
 
   int16 touchX,   touchY;   // TSC X, Y
   int16 touchXpx, touchYpx; // TSC X, Y pixel values
   int16 touchZ1,  touchZ2;  // TSC x-panel measurements
  uint16 tdiode1,  tdiode2;  // TSC temperature diodes
  uint32 temperature;        // TSC computed temperature
 
  uint16 buttons;            // X, Y, /PENIRQ buttons
 
  union {
    uint8 curtime[8];        // current time response from RTC
 
    struct {
      u8 rtc_command;
      u8 rtc_year;           //add 2000 to get 4 digit year
      u8 rtc_month;          //1 to 12
      u8 rtc_day;            //1 to (days in month)
 
      u8 rtc_incr;
      u8 rtc_hours;          //0 to 11 for AM, 52 to 63 for PM
      u8 rtc_minutes;        //0 to 59
      u8 rtc_seconds;        //0 to 59
    };
  };
 
  uint16 battery;            // battery life ??  hopefully.  :)
  uint16 aux;                // i have no idea...
 
  pTransferSound soundData;
  
  adpcmBuffer adpcm;
  
 
  // Don't rely on these below, will change or be removed in the future
  vuint32 mailAddr;
  vuint32 mailData;
  vuint8 mailRead;
  vuint8 mailBusy;
  vuint32 mailSize;
  
  bool performArm9SleepMode;
  
  u32 test;
  int tweak;
  bool tweakChanged;
  
//  bool fillSoundFirstHalf;
//  bool fillSoundSecondHalf;

  // These are used for ScummVMs sound output
  bool fillNeeded[4];
  int playingSection;
  
  bool reset;
  
  // Streaming sound
  bool streamFillNeeded[4];
  int streamPlayingSection;
} scummTransferRegion, * pscummTransferRegion;

//////////////////////////////////////////////////////////////////////

#undef IPC
#define IPC ((scummTransferRegion volatile *)(0x027FF000))


#endif


