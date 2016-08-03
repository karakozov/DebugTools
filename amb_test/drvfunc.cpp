
#include	<windows.h>
#include	<tchar.h>
#include	<stdio.h>

#include	"ddwambpex.h"
#include	"Streamll.h"

#ifdef _WIN64
#define AmbPlxDeviceName L"wambp"
#else
#define AmbPlxDeviceName "wambp"
#endif

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
	LONG	lastBlock;				// Number of Block which was filled last Time
	ULONG	totalCounter;			// Total Counter of all filled Block
	ULONG	offset;					// First Unfilled Byte
	ULONG	state;					// CBUF local state
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

HANDLE g_hWDM = NULL;
OVERLAPPED g_OvlStartStream[2];	// событие окончани€ всего составного буфера
HANDLE g_hBlockEndEvent[2];	// событие окончани€ блока
PAMB_MEM_DMA_CHANNEL g_pMemDescrip[2] = {NULL, NULL};
int g_DescripSize[2];

//******************** dev_open ******************
// Ёта функци€ открывает драйвер устройства
// int DeviceNumber - номер экземпл€ра нужного базового модул€
//************************************************
int dev_open(int DeviceNumber)
{
	TCHAR DeviceName[32] = _T("\\\\.\\");
	lstrcat(DeviceName, AmbDeviceName);
	TCHAR cNumber[4];
	_stprintf_s( cNumber, _T("%d"), DeviceNumber);
	lstrcat(DeviceName, cNumber);
	g_hWDM = CreateFile(DeviceName,
							GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL, 
							OPEN_EXISTING, 
							FILE_FLAG_OVERLAPPED, // 0,
							//0,
							NULL);
	if(g_hWDM == INVALID_HANDLE_VALUE)
		return -1;
	return 0;
}

int dev_openPlx(int DeviceNumber)
{
	TCHAR DeviceName[32] = _T("\\\\.\\");
	lstrcat(DeviceName, AmbPlxDeviceName);
	TCHAR cNumber[4];
	_stprintf_s( cNumber, _T("%d"), DeviceNumber);
	lstrcat(DeviceName, cNumber);
	g_hWDM = CreateFile(DeviceName,
							GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL, 
							OPEN_EXISTING, 
							FILE_FLAG_OVERLAPPED, // 0,
							//0,
							NULL);
	if(g_hWDM == INVALID_HANDLE_VALUE)
		return -1;
	return 0;
}

//************************************************
void dev_close()
{
	CloseHandle(g_hWDM);
}

//***************************************************************************************
ULONG GetVersion(char* pVerInfo)
{
    ULONG   length;
	char Buf[MAX_STRING_LEN];

    if (!DeviceIoControl(
            g_hWDM, 
			IOCTL_AMB_GET_VERSION,
            NULL,
            0,
            Buf,
            MAX_STRING_LEN,
            &length,
            NULL)) {
        return GetLastError();
    }
	lstrcpyA(pVerInfo, Buf);
	return 0;
}

//***************************************************************************************
ULONG GetLocation(ULONG& SlotNumber, ULONG& BusNumber, ULONG& DeviceNumber)
{
	AMB_LOCATION AmbLocation;

    ULONG   length;
    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_GET_LOCATION,
            NULL,
            0,
            &AmbLocation,
            sizeof(AMB_LOCATION),
            &length,
            NULL)) {
        return GetLastError();
    }
	SlotNumber = AmbLocation.SlotNumber;
	BusNumber = AmbLocation.BusNumber;
	DeviceNumber = AmbLocation.DeviceNumber;
	return 0;
}

//***************************************************************************************
ULONG GetDeviceID(USHORT& DeviceID)
{
    ULONG   length;     // the return length from the driver
	AMB_DATA_BUF PciConfigPtr = 
//							{NULL, sizeof(USHORT), FIELD_OFFSET(PCI_COMMON_CONFIG, DeviceID)};
							{NULL, sizeof(USHORT), 2};

	ULONG sss = sizeof(AMB_DATA_BUF);

    if (!DeviceIoControl(
            g_hWDM, 
			IOCTL_AMB_GET_BUS_CONFIG,
            &PciConfigPtr,
            sizeof(AMB_DATA_BUF),
            &DeviceID,
            sizeof(USHORT),
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
ULONG GetPciCfgReg(ULONG& regVal, ULONG offset)
{
    ULONG   length;     // the return length from the driver
	AMB_DATA_BUF PciConfigPtr = 
							{NULL, sizeof(ULONG), offset};

    if (!DeviceIoControl(
            g_hWDM, 
			IOCTL_AMB_GET_BUS_CONFIG,
            &PciConfigPtr,
            sizeof(AMB_DATA_BUF),
            &regVal,
            sizeof(ULONG),
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
ULONG SetPciCfgReg(ULONG& regVal, ULONG offset)
{
    ULONG   length;     // the return length from the driver
	AMB_DATA_BUF PciConfigPtr = 
							{&regVal, sizeof(ULONG), offset};

    if (!DeviceIoControl(
            g_hWDM, 
			IOCTL_AMB_SET_BUS_CONFIG,
            &PciConfigPtr,
            sizeof(AMB_DATA_BUF),
            &regVal,
            sizeof(ULONG),
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
// Size: 1 - UCHAR, 2 - USHORT, 4 - ULONG
ULONG WriteLocalBusReg(ULONG& RegVal, ULONG Offset, ULONG Size) 
{
    ULONG   length;  

	AMB_DATA_BUF data = { &RegVal, Size, Offset};

    if (!DeviceIoControl(
			g_hWDM, 
			IOCTL_AMB_WRITE_LOCALBUS,
			&data,
			sizeof(AMB_DATA_BUF),
            &RegVal,
            sizeof(ULONG),
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
// Size: 1 - UCHAR, 2 - USHORT, 4 - ULONG
ULONG ReadLocalBusReg(ULONG& RegVal, ULONG Offset, ULONG Size) 
{
    ULONG   length;  

	AMB_DATA_BUF data = { NULL, Size, Offset};

    if (!DeviceIoControl(
			g_hWDM, 
			IOCTL_AMB_READ_LOCALBUS,
			&data,
			sizeof(AMB_DATA_BUF),
            &RegVal,
            sizeof(ULONG),
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
ULONG WriteRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, ULONG& RegVal) 
{
    ULONG   length;  

	AMB_DATA_REG reg_data = { AdmNum, TetrNum, RegNum, RegVal};

    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_WRITE_REG_DATA,
            &reg_data,
            sizeof(AMB_DATA_REG),
            NULL,
            0,
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
ULONG WriteRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, ULONG& RegVal) 
{
    ULONG   length;  

	AMB_DATA_REG reg_data = { AdmNum, TetrNum, RegNum, RegVal};

    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_WRITE_REG_DATA_DIR,
            &reg_data,
            sizeof(AMB_DATA_REG),
            NULL,
            0,
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
ULONG ReadRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, ULONG& RegVal) 
{
    ULONG   length;  

	AMB_DATA_REG reg_data = { AdmNum, TetrNum, RegNum, 0};

//	Sleep(200);
    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_READ_REG_DATA,
            &reg_data,
            sizeof(AMB_DATA_REG),
            &reg_data,
            sizeof(AMB_DATA_REG),
            &length,
            NULL)) {
        return GetLastError();
    }
	RegVal = reg_data.Value;
	
	return 0;
}

//***************************************************************************************
ULONG ReadRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, ULONG& RegVal) 
{
    ULONG   length;  

	AMB_DATA_REG reg_data = { AdmNum, TetrNum, RegNum, 0};

//	Sleep(200);
    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_READ_REG_DATA_DIR,
            &reg_data,
            sizeof(AMB_DATA_REG),
            &reg_data,
            sizeof(AMB_DATA_REG),
            &length,
            NULL)) {
        return GetLastError();
    }
	RegVal = reg_data.Value;
	
	return 0;
}

//***************************************************************************************
ULONG WriteRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
    ULONG   length;  

	AMB_BUF_REG reg_buf = { AdmNum, TetrNum, RegNum, RegBuf, RegBufSize };

    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_WRITE_REG_BUF,
            &reg_buf,
            sizeof(AMB_BUF_REG),
            NULL,
            0,
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
ULONG WriteRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
    ULONG   length;  

	AMB_BUF_REG reg_buf = { AdmNum, TetrNum, RegNum, RegBuf, RegBufSize };

    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_WRITE_REG_BUF_DIR,
            &reg_buf,
            sizeof(AMB_BUF_REG),
            NULL,
            0,
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
ULONG ReadRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
    ULONG   length;  

	AMB_BUF_REG reg_buf = { AdmNum, TetrNum, RegNum, RegBuf, RegBufSize };

    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_READ_REG_BUF,
            &reg_buf,
            sizeof(AMB_BUF_REG),
            &reg_buf,
            sizeof(AMB_BUF_REG),
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
ULONG ReadRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
    ULONG   length;  

	AMB_BUF_REG reg_buf = { AdmNum, TetrNum, RegNum, RegBuf, RegBufSize };

    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_READ_REG_BUF_DIR,
            &reg_buf,
            sizeof(AMB_BUF_REG),
            &reg_buf,
            sizeof(AMB_BUF_REG),
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
ULONG ReadNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	ULONG   length;     // the return length from the driver
	AMB_DATA_BUF NvRAM = {NULL, BufferSize, Offset};

	if (!DeviceIoControl(
			g_hWDM,
			IOCTL_AMB_READ_NVRAM,
			&NvRAM,
			sizeof(AMB_DATA_BUF),
			pBuffer,
			BufferSize,
			&length,
			NULL)) {
		return GetLastError();
	}
	return 0;
}

//***************************************************************************************
ULONG WriteNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	ULONG   length;     // the return length from the driver
	AMB_DATA_BUF NvRAM = {pBuffer, BufferSize, Offset};

	if (!DeviceIoControl(
			g_hWDM,
			IOCTL_AMB_WRITE_NVRAM,
			&NvRAM,
			sizeof(AMB_DATA_BUF),
            NULL,
            0,
			&length,
			NULL)) {
		return GetLastError();
	}
	return 0;
}

//***************************************************************************************
ULONG ReadAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
    ULONG   length;     // the return length from the driver
	AMB_DATA_BUF admIdROM = {NULL, BufferSize, Offset};

    if (!DeviceIoControl(
            g_hWDM,
			IOCTL_AMB_READ_ADMIDROM,
            &admIdROM,
			sizeof(AMB_DATA_BUF),
			pBuffer,
            BufferSize,
            &length,
            NULL)) {
//		MessageBox(NULL, L"ReadAdmIdROM is ERROR !!!", L"ISDCLASS", MB_OK); 
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
ULONG WriteAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
    ULONG   length;     // the return length from the driver
	AMB_DATA_BUF admIdROM = {pBuffer, BufferSize, Offset};

    if (!DeviceIoControl(
            g_hWDM,
			IOCTL_AMB_WRITE_ADMIDROM,
            &admIdROM,
			sizeof(AMB_DATA_BUF),
            NULL,
            0,
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
ULONG SetPldStatus(ULONG PldStatus, ULONG PldNum)
{
	ULONG length;     // the return length from the driver
	ULONG param = (PldNum << 16) + PldStatus;

    if (!DeviceIoControl(
			g_hWDM,
            IOCTL_AMB_SET_PLD_STATUS,
            &param,
            sizeof(ULONG),
            NULL,
            0,
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
ULONG GetPldStatus(ULONG& PldStatus, ULONG PldNum)
{
	ULONG length;     // the return length from the driver
	ULONG num = PldNum;
	ULONG Status;

    if (!DeviceIoControl(
			g_hWDM,
            IOCTL_AMB_GET_PLD_STATUS,
            &num,
            sizeof(ULONG),
            &Status,
            sizeof(ULONG),
            &length,
            NULL)) {
        return GetLastError();
    }
	PldStatus = Status;

	return 0;
}

//***************************************************************************************
ULONG LoadPld(ULONG& PldStatus, PVOID pBuffer, ULONG BufferSize, ULONG PldNum)
{
	ULONG   length;     // the return length from the driver
	AMB_DATA_BUF PldData = {pBuffer, BufferSize, PldNum};
	UCHAR Status = 0;

    if (!DeviceIoControl(
			g_hWDM,
			IOCTL_AMB_SET_PLD_STATUS, //IOCTL_AMB_LOAD_PLD,
			&PldData,
			sizeof(AMB_DATA_BUF),
			&Status,
			sizeof(UCHAR),
			&length,
            NULL)) {
        return GetLastError();
    }
	PldStatus = Status;

	return 0;
}

//***************************************************************************************
ULONG GetConfiguration(PAMB_CONFIGURATION pAmbConfiguration)
{
    ULONG   length;     // the return length from the driver
    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_GET_CONFIGURATION,
            NULL,
            0,
            pAmbConfiguration,
            sizeof(AMB_CONFIGURATION),
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
ULONG GetMemoryAddress(PULONG& MemoryAddress)
{
	AMB_CONFIGURATION AmbConfiguration;
	ULONG status = GetConfiguration(&AmbConfiguration);
	if(!status)
	{
		MemoryAddress = (PULONG)AmbConfiguration.VirtAddress[0];
		//printf("addr0 = 0x%0X, addr1 = 0x%0X\n", AmbConfiguration.VirtAddress[0], AmbConfiguration.VirtAddress[1]);
		printf("addr0 = 0x%0p, addr1 = 0x%0p\n", AmbConfiguration.VirtAddress[0], AmbConfiguration.VirtAddress[1]);
	}
	return status;
}

//***************************************************************************************
ULONG ResetDmaFifo(int DmaChan)
{
	AMB_SET_DMA_CHANNEL SetDescrip;
	SetDescrip.DmaChanNum = DmaChan;

	ULONG   length;  

    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_RESET_FIFO,
            &SetDescrip,
            sizeof(AMB_SET_DMA_CHANNEL),
            NULL,
            0,
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
ULONG InfoDma(int DmaChan, ULONG& dir, ULONG& FifoSize, ULONG& MaxDmaSize)
{
	AMB_GET_DMA_INFO InfoDescrip;
	InfoDescrip.DmaChanNum = DmaChan;

    ULONG   length;  
    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_GET_INFOIO,
			&InfoDescrip,
			sizeof(AMB_GET_DMA_INFO),
			&InfoDescrip,
			sizeof(AMB_GET_DMA_INFO),
            &length,
            NULL)) {
        return GetLastError();
    }
	dir = InfoDescrip.Direction;
	FifoSize = InfoDescrip.FifoSize;
	MaxDmaSize = InfoDescrip.MaxDmaSize;
	//PciAddr = InfoDescrip.PciAddr;
	//LocalAddr = InfoDescrip.LocalAddr;

	return 0;
}

ULONG PhysAddrStub = NULL;

//***************************************************************************************
ULONG AllocMemory(PVOID* pBuf, ULONG blkSize, ULONG& blkNum, ULONG isSysMem, ULONG dir, ULONG addr, int DmaChan)
{
	g_OvlStartStream[DmaChan].hEvent = CreateEvent (NULL, TRUE, TRUE, NULL); // начальное состо€ние Signaled
	TCHAR nameEvent[MAX_PATH];
	_stprintf_s(nameEvent, _T("event_DmaChan_%d"), DmaChan);
	g_hBlockEndEvent[DmaChan] = CreateEvent(NULL, TRUE, TRUE, nameEvent); // начальное состо€ние Signaled

	g_DescripSize[DmaChan] = sizeof(AMB_MEM_DMA_CHANNEL) + (blkNum - 1) * sizeof(PVOID);
	g_pMemDescrip[DmaChan] = (PAMB_MEM_DMA_CHANNEL)new UCHAR[g_DescripSize[DmaChan]];

	g_pMemDescrip[DmaChan]->DmaChanNum = DmaChan;
	g_pMemDescrip[DmaChan]->Direction = dir;
	g_pMemDescrip[DmaChan]->LocalAddr = addr;
	g_pMemDescrip[DmaChan]->MemType = isSysMem;
	g_pMemDescrip[DmaChan]->BlockCnt = blkNum;
	g_pMemDescrip[DmaChan]->BlockSize = blkSize;
	g_pMemDescrip[DmaChan]->hBlockEndEvent = g_hBlockEndEvent[DmaChan];
#ifdef _WIN64
	g_pMemDescrip[DmaChan]->hTransEndEvent = g_OvlStartStream[DmaChan].hEvent;
#endif
	g_pMemDescrip[DmaChan]->pStub = NULL;
	for(ULONG iBlk = 0; iBlk < blkNum; iBlk++)
	{
		if(isSysMem)
			g_pMemDescrip[DmaChan]->pBlock[iBlk] = NULL;
		else
//			g_pMemDescrip[DmaChan]->pBlock[iBlk] = new UCHAR[blkSize];
			g_pMemDescrip[DmaChan]->pBlock[iBlk] = VirtualAlloc(NULL, blkSize, MEM_COMMIT, PAGE_READWRITE);
	}

	ULONG   length;  

    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_SET_MEMIO,
            g_pMemDescrip[DmaChan],
            g_DescripSize[DmaChan],
            g_pMemDescrip[DmaChan],
            g_DescripSize[DmaChan],
            &length,
            NULL)) {
        return GetLastError();
    }

	blkNum = g_pMemDescrip[DmaChan]->BlockCnt;
	g_DescripSize[DmaChan] = sizeof(AMB_MEM_DMA_CHANNEL) + (blkNum - 1) * sizeof(PVOID);
	*pBuf = &(g_pMemDescrip[DmaChan]->pBlock[0]);

	ULONG* psgtable = (ULONG*)(g_pMemDescrip[DmaChan]->pBlock[0]);
	PhysAddrStub = psgtable[3];
	//psgtable[4] = m_StubDscr.LogicalAddress.HighPart;

	return 0;
}

//***************************************************************************************
ULONG FreeMemory(int DmaChan)
{
    ULONG   length;  

    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_FREE_MEMIO,
            g_pMemDescrip[DmaChan],
            g_DescripSize[DmaChan],
            g_pMemDescrip[DmaChan],
            g_DescripSize[DmaChan],
            &length,
            NULL)) {
        return GetLastError();
    }

	if(g_pMemDescrip[DmaChan]->MemType == 0)
	{
		for(ULONG iBlk = 0; iBlk < g_pMemDescrip[DmaChan]->BlockCnt; iBlk++)
//			delete g_pMemDescrip[DmaChan]->pBlock[iBlk];
			VirtualFree(g_pMemDescrip[DmaChan]->pBlock[iBlk], 0, MEM_RELEASE);
	}
	delete g_pMemDescrip[DmaChan];
	g_pMemDescrip[DmaChan] = NULL;

	if(g_hBlockEndEvent[DmaChan])
		CloseHandle(g_hBlockEndEvent[DmaChan]);
	if(g_OvlStartStream[DmaChan].hEvent)
		CloseHandle(g_OvlStartStream[DmaChan].hEvent);

	return 0;
}

//***************************************************************************************
ULONG StartDma(int IsCycling, int DmaChan)
{
	if(g_pMemDescrip[DmaChan])
	{
		ResetEvent(g_OvlStartStream[DmaChan].hEvent); // сброс в состо€ние Non-Signaled перед стартом
		ResetEvent(g_hBlockEndEvent[DmaChan]); // сброс в состо€ние Non-Signaled перед стартом

		AMB_START_DMA_CHANNEL StartDescrip;
		StartDescrip.DmaChanNum = DmaChan;
		StartDescrip.IsCycling = IsCycling;

		ULONG   length;  

		if (!DeviceIoControl(
				g_hWDM, 
				IOCTL_AMB_START_MEMIO,
				&StartDescrip,
				sizeof(AMB_START_DMA_CHANNEL),
				NULL,
				0,
				&length,
#ifdef _WIN64
				NULL)) {
#else
				&(g_OvlStartStream[DmaChan]))){
#endif
			return GetLastError();
		}
		return 0;
	}
	return NON_MEMORY_ERROR;
}

//***************************************************************************************
ULONG StopDma(int DmaChan)
{
	if(g_pMemDescrip[DmaChan])
	{
		PBRDstrm_Stub pStub = (PBRDstrm_Stub)g_pMemDescrip[DmaChan]->pStub;
		if(pStub->state == BRDstrm_STAT_RUN)
		{
			AMB_STATE_DMA_CHANNEL StateDescrip;
			StateDescrip.DmaChanNum = DmaChan;
			StateDescrip.Timeout = 0;//pState->timeout; останавливает немедленно (в 0-кольце оставлю пока возможность ожидани€)
			ULONG   length;  

			if (!DeviceIoControl(
					g_hWDM, 
					IOCTL_AMB_STOP_MEMIO,
					&StateDescrip,
					sizeof(AMB_STATE_DMA_CHANNEL),
					&StateDescrip,
					sizeof(AMB_STATE_DMA_CHANNEL),
					&length,
					NULL)) {
				return GetLastError();
			}
			return 0;
		}
		return 0;
	}
	return NON_MEMORY_ERROR;
}

//***************************************************************************************
ULONG StateDma(ULONG msTimeout, int DmaChan, int& state, ULONG& blkNum)
{
	AMB_STATE_DMA_CHANNEL StateDescrip;
	StateDescrip.DmaChanNum = DmaChan;
	StateDescrip.Timeout = msTimeout;

    ULONG   length;  
    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_STATE_MEMIO,
			&StateDescrip,
			sizeof(AMB_STATE_DMA_CHANNEL),
			&StateDescrip,
			sizeof(AMB_STATE_DMA_CHANNEL),
            &length,
            NULL)) {
        return GetLastError();
    }
	blkNum = StateDescrip.BlockCntTotal;
	state = StateDescrip.DmaChanState;

	return 0;
}

//***************************************************************************************
ULONG WaitBuffer(ULONG msTimeout, int DmaChan)
{
	if(g_pMemDescrip[DmaChan])
	{
		int Status = WaitForSingleObject(g_OvlStartStream[DmaChan].hEvent, msTimeout);
		if(Status == WAIT_TIMEOUT) 
			return TIMEOUT_ERROR;
		else
			ResetEvent(g_OvlStartStream[DmaChan].hEvent); // сброс в состо€ние Non-Signaled после завершени€ блока
		return 0;
	}
	return NON_MEMORY_ERROR;
}

//***************************************************************************************
ULONG WaitBlock(ULONG msTimeout, int DmaChan)
{
	if(g_pMemDescrip[DmaChan])
	{
		int Status = WaitForSingleObject(g_hBlockEndEvent[DmaChan], msTimeout);
		if(Status == WAIT_TIMEOUT) 
			return TIMEOUT_ERROR;
		else
			ResetEvent(g_hBlockEndEvent); // сброс в состо€ние Non-Signaled после завершени€ блока
		return 0;
	}
	return NON_MEMORY_ERROR;
}

//***************************************************************************************
ULONG SetDmaRequest(int drqFlag, int DmaChan)
{
	AMB_SET_DMA_CHANNEL SetDescrip;
	SetDescrip.DmaChanNum = DmaChan;
	SetDescrip.Param = drqFlag;

	ULONG   length;  

    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_SET_DRQ_MEMIO,
            &SetDescrip,
            sizeof(AMB_SET_DMA_CHANNEL),
            NULL,
            0,
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}

//***************************************************************************************
ULONG SetIrqMode(int irqMode, int DmaChan)
{
	AMB_SET_IRQ_TABLE SetDescrip;
	SetDescrip.DmaChanNum = DmaChan;
	SetDescrip.Mode = irqMode;
	SetDescrip.TableNum = 4;
	SetDescrip.AddrTable[0] = PhysAddrStub;		// lastBlock
	SetDescrip.AddrTable[1] = PhysAddrStub+8;	// offset
	SetDescrip.AddrTable[2] = 0xB1B6;			// constanta
	SetDescrip.AddrTable[3] = 0xA4A5;			// reserved

	ULONG   length;  

    if (!DeviceIoControl(
            g_hWDM, 
            IOCTL_AMB_SET_IRQ_TABLE,
            &SetDescrip,
            sizeof(AMB_SET_IRQ_TABLE),
            NULL,
            0,
            &length,
            NULL)) {
        return GetLastError();
    }
	return 0;
}
