
#include	"gipcy.h"
#include	"ambll.h"

#include	<stdio.h>

IPC_handle g_hWDM = NULL;
#ifdef _WIN32
OVERLAPPED g_OvlStartStream[2];	// событие окончани€ всего составного буфера
HANDLE g_hBlockEndEvent[2];	// событие окончани€ блока
#endif
PAMB_MEM_DMA_CHANNEL g_pMemDescrip[2] = { NULL, NULL };
int g_DescripSize[2];

// DMA buffer States 
enum
{
	BRDstrm_STAT_RUN = 1,
	BRDstrm_STAT_STOP = 2,
	BRDstrm_STAT_DESTROY = 3,
	BRDstrm_STAT_BREAK = 4
};

typedef struct
{
    long	lastBlock;				// Number of Block which was filled last Time
    unsigned long	totalCounter;			// Total Counter of all filled Block
    unsigned long	offset;					// First Unfilled Byte
    unsigned long	state;					// CBUF local state
} BRDstrm_Stub, *PBRDstrm_Stub, BRDctrl_StreamStub, *PBRDctrl_StreamStub;

typedef enum _AMB_Errors {
	PLD_errOK,
	PLD_errFORMAT,
	PLD_errPROGR,      // Programming error
	PLD_errRECLOST,    // PLD record is lost
	PLD_errROM,        // PLD programming from ROM only 
	PLD_errNOTRDY,      // PLD not ready for programming 
	PLD_TEST_DATA_ERROR,
	PLD_TEST_ADDRESS_ERROR,
	INSYS_TAG_ERROR,
	ADM_VERSION_ERROR,
	NON_MEMORY_ERROR,
	TIMEOUT_ERROR
} AMB_Errors;

#ifdef __linux__
unsigned long GetLastError(void)
{
	//printf("GetLastError(): %s\n", strerror(IPC_sysError()));
	//return -1;
	return errno;
}
#endif

//******************** dev_open ******************
// Ёта функци€ открывает драйвер устройства
// int DeviceNumber - номер экземпл€ра нужного базового модул€
//************************************************
int dev_open(IPC_str *deviceName, int DeviceNumber)
{
	g_hWDM = IPC_openDevice(deviceName, AmbDeviceName, DeviceNumber);
	if (!g_hWDM)
		return -1;
	return 0;
}

int dev_openPlx(IPC_str *deviceName, int DeviceNumber)
{
	g_hWDM = IPC_openDevice(deviceName, AmbPlxDeviceName, DeviceNumber);
	if (!g_hWDM)
		return -1;
	return 0;
}

//************************************************
void dev_close()
{
	IPC_closeDevice(g_hWDM);
}

//***************************************************************************************
unsigned long GetVersion(char* pVerInfo)
{
	int ret = 0;
    //	unsigned long   length;
	//char Buf[MAX_STRING_LEN];

	ret = IPC_ioctlDevice(
		g_hWDM,
		IOCTL_AMB_GET_VERSION,
		NULL,
		0,
		pVerInfo,
		MAX_STRING_LEN);

	//strcpy(pVerInfo, Buf);
	return ret;
}

//***************************************************************************************
unsigned long GetLocation(unsigned long& SlotNumber, unsigned long& BusNumber, unsigned long& DeviceNumber)
{
	int ret = 0;
	AMB_LOCATION AmbLocation;

	ret = IPC_ioctlDevice(
		g_hWDM,
		IOCTL_AMB_GET_LOCATION,
		NULL,
		0,
		&AmbLocation,
		sizeof(AMB_LOCATION));

	SlotNumber = AmbLocation.SlotNumber;
	BusNumber = AmbLocation.BusNumber;
	DeviceNumber = AmbLocation.DeviceNumber;
	return ret;
}

//***************************************************************************************
unsigned long GetDeviceID(unsigned short& DeviceID)
{
	int ret = 0;
    //	unsigned long   length;     // the return length from the driver
	AMB_DATA_BUF PciConfigPtr =
	{ NULL, sizeof(short), 2 };

    //unsigned long sss = sizeof(AMB_DATA_BUF);

	ret = IPC_ioctlDevice(
		g_hWDM,
		IOCTL_AMB_GET_BUS_CONFIG,
		&PciConfigPtr,
		sizeof(AMB_DATA_BUF),
		&DeviceID,
		sizeof(short));

	return ret;
}

//***************************************************************************************
unsigned long GetPldStatus(unsigned long& PldStatus, unsigned long PldNum)
{
	int ret = 0;

	unsigned long num = PldNum;
	unsigned long Status;

	ret = IPC_ioctlDevice(
		g_hWDM,
		IOCTL_AMB_GET_PLD_STATUS,
		&num,
		sizeof(long),
		&Status,
		sizeof(long));

	PldStatus = Status;

	return ret;
}

//***************************************************************************************
unsigned long WriteRegData(unsigned long AdmNum, unsigned long TetrNum, unsigned long RegNum, unsigned long& RegVal)
{
	int ret = 0;

	AMB_DATA_REG reg_data = { AdmNum, TetrNum, RegNum, RegVal };

	ret = IPC_ioctlDevice(
		g_hWDM,
		IOCTL_AMB_WRITE_REG_DATA,
		&reg_data,
		sizeof(AMB_DATA_REG),
		NULL,
		0);

	return ret;
}

//***************************************************************************************
unsigned long WriteRegDataDir(unsigned long AdmNum, unsigned long TetrNum, unsigned long RegNum, unsigned long& RegVal)
{
	int ret = 0;

	AMB_DATA_REG reg_data = { AdmNum, TetrNum, RegNum, RegVal };

	ret = IPC_ioctlDevice(
		g_hWDM,
		IOCTL_AMB_WRITE_REG_DATA_DIR,
		&reg_data,
		sizeof(AMB_DATA_REG),
		NULL,
		0);

	return ret;
}

//***************************************************************************************
unsigned long ReadRegData(unsigned long AdmNum, unsigned long TetrNum, unsigned long RegNum, unsigned long& RegVal)
{
	int ret = 0;

	AMB_DATA_REG reg_data = { AdmNum, TetrNum, RegNum, 0 };

	ret = IPC_ioctlDevice(
		g_hWDM,
		IOCTL_AMB_READ_REG_DATA,
		&reg_data,
		sizeof(AMB_DATA_REG),
		&reg_data,
		sizeof(AMB_DATA_REG));

	RegVal = reg_data.Value;

	return ret;
}

//***************************************************************************************
unsigned long ReadRegDataDir(unsigned long AdmNum, unsigned long TetrNum, unsigned long RegNum, unsigned long& RegVal)
{
	int ret = 0;

	AMB_DATA_REG reg_data = { AdmNum, TetrNum, RegNum, 0 };

	ret = IPC_ioctlDevice(
		g_hWDM,
		IOCTL_AMB_READ_REG_DATA_DIR,
		&reg_data,
		sizeof(AMB_DATA_REG),
		&reg_data,
		sizeof(AMB_DATA_REG));

	RegVal = reg_data.Value;

	return ret;
}

//***************************************************************************************
unsigned long ResetDmaFifo(int DmaChan)
{
	int ret = 0;

	AMB_SET_DMA_CHANNEL SetDescrip;
	SetDescrip.DmaChanNum = DmaChan;

	ret = IPC_ioctlDevice(
		g_hWDM,
		IOCTL_AMB_RESET_FIFO,
		&SetDescrip,
		sizeof(AMB_SET_DMA_CHANNEL),
		NULL,
		0);

	return ret;
}

//unsigned long PhysAddrStub = NULL;

//***************************************************************************************
unsigned long AllocMemory(void** pBuf, unsigned long blkSize, unsigned long& blkNum, unsigned long isSysMem, unsigned long dir, unsigned long addr, int DmaChan)
{
#ifdef _WIN32
	g_OvlStartStream[DmaChan].hEvent = CreateEvent(NULL, TRUE, TRUE, NULL); // начальное состо€ние Signaled
	char nameEvent[MAX_PATH];
	sprintf(nameEvent, "event_DmaChan_%d", DmaChan);
	g_hBlockEndEvent[DmaChan] = CreateEvent(NULL, TRUE, TRUE, nameEvent); // начальное состо€ние Signaled
#endif

    g_DescripSize[DmaChan] = sizeof(AMB_MEM_DMA_CHANNEL) + (blkNum - 1) * sizeof(void*);
    g_pMemDescrip[DmaChan] = (PAMB_MEM_DMA_CHANNEL)new unsigned char[g_DescripSize[DmaChan]];

	g_pMemDescrip[DmaChan]->DmaChanNum = DmaChan;
	g_pMemDescrip[DmaChan]->Direction = dir;
	g_pMemDescrip[DmaChan]->LocalAddr = addr;
	g_pMemDescrip[DmaChan]->MemType = isSysMem;
	g_pMemDescrip[DmaChan]->BlockCnt = blkNum;
	g_pMemDescrip[DmaChan]->BlockSize = blkSize;
#ifdef _WIN32
	g_pMemDescrip[DmaChan]->hBlockEndEvent = g_hBlockEndEvent[DmaChan];
#endif
#ifdef _WIN64
	g_pMemDescrip[DmaChan]->hTransEndEvent = g_OvlStartStream[DmaChan].hEvent;
#endif
	g_pMemDescrip[DmaChan]->pStub = NULL;
    for (unsigned long iBlk = 0; iBlk < blkNum; iBlk++)
	{
		if (isSysMem)
			g_pMemDescrip[DmaChan]->pBlock[iBlk] = NULL;
		else
			//			g_pMemDescrip[DmaChan]->pBlock[iBlk] = new UCHAR[blkSize];
			g_pMemDescrip[DmaChan]->pBlock[iBlk] = IPC_virtAlloc(blkSize);
			//g_pMemDescrip[DmaChan]->pBlock[iBlk] = VirtualAlloc(NULL, blkSize, MEM_COMMIT, PAGE_READWRITE);
	}

    //unsigned long   length;

	//if (!DeviceIoControl(
	//		g_hWDM,
	//		IOCTL_AMB_SET_MEMIO,
	//		g_pMemDescrip[DmaChan],
	//		g_DescripSize[DmaChan],
	//		g_pMemDescrip[DmaChan],
	//		g_DescripSize[DmaChan],
	//		&length,
	//		NULL)) {
	//	return GetLastError();
	//}
	int ret = 0;

	ret = IPC_ioctlDevice(
		g_hWDM,
		IOCTL_AMB_SET_MEMIO,
		g_pMemDescrip[DmaChan],
		g_DescripSize[DmaChan],
		g_pMemDescrip[DmaChan],
		g_DescripSize[DmaChan]);

	blkNum = g_pMemDescrip[DmaChan]->BlockCnt;
    g_DescripSize[DmaChan] = sizeof(AMB_MEM_DMA_CHANNEL) + (blkNum - 1) * sizeof(void*);
	*pBuf = &(g_pMemDescrip[DmaChan]->pBlock[0]);

//	unsigned long* psgtable = (unsigned long*)(g_pMemDescrip[DmaChan]->pBlock[0]);
//	PhysAddrStub = psgtable[3];

	return ret;
//	return 0;
}

//***************************************************************************************
unsigned long FreeMemory(int DmaChan)
{
	int ret = 0;

	ret = IPC_ioctlDevice(
		g_hWDM,
		IOCTL_AMB_FREE_MEMIO,
		g_pMemDescrip[DmaChan],
		g_DescripSize[DmaChan],
		g_pMemDescrip[DmaChan],
		g_DescripSize[DmaChan]);

	if (g_pMemDescrip[DmaChan]->MemType == 0)
	{
        for (unsigned long iBlk = 0; iBlk < g_pMemDescrip[DmaChan]->BlockCnt; iBlk++)
			//			delete g_pMemDescrip[DmaChan]->pBlock[iBlk];
			IPC_virtFree(g_pMemDescrip[DmaChan]->pBlock[iBlk]);
			//VirtualFree(g_pMemDescrip[DmaChan]->pBlock[iBlk], 0, MEM_RELEASE);
	}
	delete g_pMemDescrip[DmaChan];
	g_pMemDescrip[DmaChan] = NULL;

#ifdef _WIN32
	if (g_hBlockEndEvent[DmaChan])
		CloseHandle(g_hBlockEndEvent[DmaChan]);
	if (g_OvlStartStream[DmaChan].hEvent)
		CloseHandle(g_OvlStartStream[DmaChan].hEvent);
#endif
	return ret;
}

//***************************************************************************************
unsigned long StartDma(int IsCycling, int DmaChan)
{
	int ret = 0;
	if (g_pMemDescrip[DmaChan])
	{
#ifdef _WIN32
		ResetEvent(g_OvlStartStream[DmaChan].hEvent); // сброс в состо€ние Non-Signaled перед стартом
		ResetEvent(g_hBlockEndEvent[DmaChan]); // сброс в состо€ние Non-Signaled перед стартом
#endif
		AMB_START_DMA_CHANNEL StartDescrip;
		StartDescrip.DmaChanNum = DmaChan;
		StartDescrip.IsCycling = IsCycling;

#ifdef _WIN32
		if (g_OvlStartStream[DmaChan].hEvent)
		{
			ret = IPC_ioctlDeviceOvl(
				g_hWDM,
				IOCTL_AMB_START_MEMIO,
				&StartDescrip,
				sizeof(AMB_START_DMA_CHANNEL),
				NULL,
				0,
#ifdef _WIN64
				NULL);
#else
				&(g_OvlStartStream[DmaChan]));
#endif
			if (ret == IPC_GENERAL_ERROR)
			{
				ret = GetLastError();
#ifdef _WIN32
				if (ret == ERROR_IO_PENDING)
					ret = IPC_OK;
#endif
			}
		}
		else
#endif
		{
			ret = IPC_ioctlDevice(
				g_hWDM,
				IOCTL_AMB_START_MEMIO,
				&StartDescrip,
				sizeof(AMB_START_DMA_CHANNEL),
				NULL,
				0);
		}
		return ret;

//		unsigned long   length;
//
//		if (!DeviceIoControl(
//			g_hWDM,
//			IOCTL_AMB_START_MEMIO,
//			&StartDescrip,
//			sizeof(AMB_START_DMA_CHANNEL),
//			NULL,
//			0,
//			&length,
//#ifdef _WIN64
//			NULL)) {
//#else
//			&(g_OvlStartStream[DmaChan]))){
//#endif
//			return GetLastError();
//		}
//		return 0;
		}
	return NON_MEMORY_ERROR;
}

//***************************************************************************************
unsigned long StopDma(int DmaChan)
{
	int ret = 0;
	if (g_pMemDescrip[DmaChan])
	{
		PBRDstrm_Stub pStub = (PBRDstrm_Stub)g_pMemDescrip[DmaChan]->pStub;
		if (pStub->state == BRDstrm_STAT_RUN)
		{
			AMB_STATE_DMA_CHANNEL StateDescrip;
			StateDescrip.DmaChanNum = DmaChan;
			StateDescrip.Timeout = 0;//pState->timeout; останавливает немедленно (в 0-кольце оставлю пока возможность ожидани€)

			ret = IPC_ioctlDevice(
				g_hWDM,
				IOCTL_AMB_STOP_MEMIO,
				&StateDescrip,
				sizeof(AMB_STATE_DMA_CHANNEL),
				&StateDescrip,
				sizeof(AMB_STATE_DMA_CHANNEL));

			return ret;
		}
		return ret;
	}
	return NON_MEMORY_ERROR;
}

//***************************************************************************************
unsigned long StateDma(unsigned long msTimeout, int DmaChan, int& state, unsigned long& blkNum)
{
	int ret = 0;

	AMB_STATE_DMA_CHANNEL StateDescrip;
	StateDescrip.DmaChanNum = DmaChan;
	StateDescrip.Timeout = msTimeout;

	ret = IPC_ioctlDevice(
		g_hWDM,
		IOCTL_AMB_STATE_MEMIO,
		&StateDescrip,
		sizeof(AMB_STATE_DMA_CHANNEL),
		&StateDescrip,
		sizeof(AMB_STATE_DMA_CHANNEL));

	blkNum = StateDescrip.BlockCntTotal;
	state = StateDescrip.DmaChanState;

	return ret;
}

//***************************************************************************************
unsigned long WaitBuffer(unsigned long msTimeout, int DmaChan)
{
	if (g_pMemDescrip[DmaChan])
	{
#ifdef __linux__
		int ret = 0;
		AMB_STATE_DMA_CHANNEL StateDescrip;
		StateDescrip.DmaChanNum = DmaChan;
		StateDescrip.Timeout = msTimeout;

		ret = IPC_ioctlDevice(
			g_hWDM,
			IOCTL_AMB_WAIT_DMA_BUFFER,
			&StateDescrip,
			sizeof(AMB_STATE_DMA_CHANNEL),
			&StateDescrip,
			sizeof(AMB_STATE_DMA_CHANNEL));
		if (ret == (int)ETIMEDOUT)
			return TIMEOUT_ERROR;
#else
		int Status = WaitForSingleObject(g_OvlStartStream[DmaChan].hEvent, msTimeout);
		if (Status == WAIT_TIMEOUT)
			return TIMEOUT_ERROR;
		else
			ResetEvent(g_OvlStartStream[DmaChan].hEvent); // сброс в состо€ние Non-Signaled после завершени€ блока
#endif
		return 0;
	}
	return NON_MEMORY_ERROR;
}

//***************************************************************************************
unsigned long WaitBlock(unsigned long msTimeout, int DmaChan)
{
	if (g_pMemDescrip[DmaChan])
	{
#ifdef __linux__
		int ret = 0;
		AMB_STATE_DMA_CHANNEL StateDescrip;
		StateDescrip.DmaChanNum = DmaChan;
		StateDescrip.Timeout = msTimeout;

		ret = IPC_ioctlDevice(
			g_hWDM,
			IOCTL_AMB_WAIT_DMA_BLOCK,
			&StateDescrip,
			sizeof(AMB_STATE_DMA_CHANNEL),
			&StateDescrip,
			sizeof(AMB_STATE_DMA_CHANNEL));
		if (ret == (int)ETIMEDOUT)
			return TIMEOUT_ERROR;
#else
		int Status = WaitForSingleObject(g_hBlockEndEvent[DmaChan], msTimeout);
		if (Status == WAIT_TIMEOUT)
			return TIMEOUT_ERROR;
		else
			ResetEvent(g_hBlockEndEvent); // сброс в состо€ние Non-Signaled после завершени€ блока
#endif
		return 0;
	}
	return NON_MEMORY_ERROR;
}
