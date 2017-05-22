//****************** File ddamb.h *********************************
//  AMB user application interface
//
//	Copyright (c) 2017, Instrumental Systems,Corp.
//  Written by Dorokhin Andrey
//
//  History:
//  03-03-17 - builded
//
//*******************************************************************

#ifndef _DDAMB_H_
 #define _DDAMB_H_

#ifdef __linux__
#define AmbDeviceName "ambpex"
#define AmbPlxDeviceName "ambp"
#else
#define AmbDeviceName "wambpex"
#define AmbPlxDeviceName "wambp"
#endif

#define MAX_STRING_LEN	255


#ifdef __linux__
#include <linux/types.h>
#ifndef __KERNEL__
#include <stdint.h>
#include <sys/ioctl.h>
#endif

#define AMB_DEVICE_TYPE             'm'
#define AMB_MAKE_IOCTL(c) _IO(AMB_DEVICE_TYPE, (c))

#else

#define AMB_DEVICE_TYPE             0x8000
#define AMB_MAKE_IOCTL(c)\
		(ULONG)CTL_CODE(AMB_DEVICE_TYPE, 0x800+(c), METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif

#define IOCTL_AMB_GET_VERSION			AMB_MAKE_IOCTL(0)
#define IOCTL_AMB_WRITE_REG_DATA		AMB_MAKE_IOCTL(1)
#define IOCTL_AMB_WRITE_REG_DATA_DIR            AMB_MAKE_IOCTL(2)
#define IOCTL_AMB_READ_REG_DATA			AMB_MAKE_IOCTL(3)
#define IOCTL_AMB_READ_REG_DATA_DIR		AMB_MAKE_IOCTL(4)
#define IOCTL_AMB_WRITE_REG_BUF			AMB_MAKE_IOCTL(5)
#define IOCTL_AMB_WRITE_REG_BUF_DIR		AMB_MAKE_IOCTL(6)
#define IOCTL_AMB_READ_REG_BUF			AMB_MAKE_IOCTL(7)
#define IOCTL_AMB_READ_REG_BUF_DIR		AMB_MAKE_IOCTL(8)
#define IOCTL_AMB_SET_BUS_CONFIG		AMB_MAKE_IOCTL(9)
#define IOCTL_AMB_GET_BUS_CONFIG		AMB_MAKE_IOCTL(10)
#define IOCTL_AMB_GET_LOCATION			AMB_MAKE_IOCTL(11)
#define IOCTL_AMB_WRITE_NVRAM			AMB_MAKE_IOCTL(12)
#define IOCTL_AMB_READ_NVRAM			AMB_MAKE_IOCTL(13)
#define IOCTL_AMB_WRITE_ADMIDROM		AMB_MAKE_IOCTL(14)
#define IOCTL_AMB_READ_ADMIDROM			AMB_MAKE_IOCTL(15)
#define IOCTL_AMB_SET_PLD_STATUS		AMB_MAKE_IOCTL(16)
#define IOCTL_AMB_GET_PLD_STATUS		AMB_MAKE_IOCTL(17)
#define IOCTL_AMB_SET_MEMIO				AMB_MAKE_IOCTL(18)
#define IOCTL_AMB_FREE_MEMIO			AMB_MAKE_IOCTL(19)
#define IOCTL_AMB_START_MEMIO			AMB_MAKE_IOCTL(20)
#define IOCTL_AMB_STOP_MEMIO			AMB_MAKE_IOCTL(21)
#define IOCTL_AMB_STATE_MEMIO			AMB_MAKE_IOCTL(22)
#define IOCTL_AMB_SET_DIR_MEMIO			AMB_MAKE_IOCTL(23)
#define IOCTL_AMB_SET_SRC_MEMIO			AMB_MAKE_IOCTL(24)
#define IOCTL_AMB_SET_DRQ_MEMIO			AMB_MAKE_IOCTL(25)
#define IOCTL_AMB_WRITE_LOCALBUS		AMB_MAKE_IOCTL(26)
#define IOCTL_AMB_READ_LOCALBUS			AMB_MAKE_IOCTL(27)
#define IOCTL_AMB_GET_CONFIGURATION		AMB_MAKE_IOCTL(28)
#define IOCTL_AMB_SET_TETRIRQ			AMB_MAKE_IOCTL(29)
#define IOCTL_AMB_CLEAR_TETRIRQ			AMB_MAKE_IOCTL(30)
#define IOCTL_AMB_RESET_FIFO			AMB_MAKE_IOCTL(31)
#define IOCTL_AMB_GET_INFOIO			AMB_MAKE_IOCTL(32)
#define IOCTL_AMB_ADJUST				AMB_MAKE_IOCTL(33)
#define IOCTL_AMB_DONE					AMB_MAKE_IOCTL(34)
#define IOCTL_AMB_WAIT_DMA_BUFFER		AMB_MAKE_IOCTL(35) // LINUX only
#define IOCTL_AMB_WAIT_DMA_BLOCK		AMB_MAKE_IOCTL(36) // LINUX only
#define IOCTL_AMB_WAIT_TETRIRQ          AMB_MAKE_IOCTL(37) // LINUX only
#define IOCTL_AMB_SET_IRQ_TABLE			AMB_MAKE_IOCTL(38)


#ifndef __linux__
#define __packed
#pragma pack(push,1)
#else
#define __packed __attribute__((packed))
#endif

// data structure for read/write value from/to register of board
typedef struct _AMB_DATA_REG {
    unsigned int	AdmNumber;		// IN
    unsigned int	TetrNumber;		// IN
    unsigned int	RegNumber;		// IN
    unsigned int	Value;			// INOUT
} __packed AMB_DATA_REG, *PAMB_DATA_REG;

// data structure for read/write buffer from/to register of board
typedef struct _AMB_BUF_REG {
    unsigned int	AdmNumber;		// IN
    unsigned int	TetrNumber;		// IN
    unsigned int	RegNumber;		// IN
    void*           pBuffer;		// IN
    unsigned int	BufferSize;		// IN
} __packed AMB_BUF_REG, *PAMB_BUF_REG;

typedef struct _AMB_DATA_BUF {
    void*           pBuffer;
    unsigned int	BufferSize;
    unsigned int	Offset;
} __packed AMB_DATA_BUF, *PAMB_DATA_BUF;

// board location data structure
typedef struct _AMB_LOCATION {
    unsigned int	BusNumber;		// OUT
    unsigned int	DeviceNumber;	// OUT
    unsigned int	SlotNumber;		// OUT
} __packed AMB_LOCATION, *PAMB_LOCATION;

// board configuration data structure
typedef struct _AMB_CONFIGURATION {
#ifdef _WIN64
	__int64	PhysAddress[3];	// OUT
#else
        size_t	PhysAddress[3];	// OUT
#endif
	//ULONG	VirtAddress[3];	// OUT
	void*	VirtAddress[3];	// OUT
    unsigned int	Size[3];		// OUT
    unsigned int	InterruptLevel;	// OUT
    unsigned int	InterruptVector;// OUT
} __packed AMB_CONFIGURATION, *PAMB_CONFIGURATION;

// tetrad interrupt request data structure
typedef struct _AMB_TETR_IRQ {
    unsigned int	AdmNumber;		// IN
    unsigned int	TetrNumber;		// IN
    unsigned int	IrqMask;		// IN
    unsigned int	IrqInv;			// IN
	IPC_handle	hTetrEvent;		// IN
} __packed AMB_TETR_IRQ, *PAMB_TETR_IRQ;

typedef struct _AMB_MEM_DMA_CHANNEL {
    unsigned int	DmaChanNum;		// IN
    unsigned int	Direction;
    unsigned int	LocalAddr;
    unsigned int	MemType;
    unsigned int	BlockCnt;
    unsigned int	BlockSize;
	void*	pStub;
	void*	hBlockEndEvent; // HANDLE of block end event
#ifdef _WIN64
	HANDLE	hTransEndEvent; // HANDLE of transfer (buffer) end event
#endif
	void*	pBlock[1];
} __packed AMB_MEM_DMA_CHANNEL, *PAMB_MEM_DMA_CHANNEL;

typedef struct _AMB_START_DMA_CHANNEL {
    unsigned int	DmaChanNum;		// IN
    unsigned int	IsCycling;
} __packed AMB_START_DMA_CHANNEL, *PAMB_START_DMA_CHANNEL;

typedef struct _AMB_STATE_DMA_CHANNEL {
    unsigned int	DmaChanNum;		// IN
    int             BlockNum;		// OUT
    unsigned int	BlockCntTotal;	// OUT
    unsigned int	OffsetInBlock;	// OUT
    unsigned int	DmaChanState;	// OUT
    int             Timeout;		// IN
} __packed AMB_STATE_DMA_CHANNEL, *PAMB_STATE_DMA_CHANNEL;

typedef struct _AMB_SET_DMA_CHANNEL {
    unsigned int	DmaChanNum;		// IN
    unsigned int	Param;
} __packed AMB_SET_DMA_CHANNEL, *PAMB_SET_DMA_CHANNEL;

// DMA buffer Directions 
enum
{
	BRDstrm_DIR_IN = 0x1,				// To HOST
	BRDstrm_DIR_OUT = 0x2,				// From HOST
	BRDstrm_DIR_INOUT = 0x3				// Both Directions
};

#ifndef __linux__
#pragma pack(pop)
#endif

#endif // _DDAMB_H_

// ****************** End of file ddamb.h **********************
