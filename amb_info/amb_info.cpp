// amb_info.cpp : Defines the entry point for the application.
//

#include	"gipcy.h"
#include	"ambll.h"

//#ifdef _WIN64
////#define _IPCSTR(x)		L ## x
////#define IPC_printf		wprintf
//#define AmbPlxDeviceName L"wambp"
//#else
////#define _IPCSTR(x)		x
////#define IPC_printf		printf
//#ifdef __linux__
//#define AmbPlxDeviceName "ambp"
//#else
//#define AmbPlxDeviceName "wambp"
//#endif
//#endif

//// C RunTime Header Files
//#include <stdlib.h>
//#include <malloc.h>
//#include <memory.h>
#include <stdio.h>
//#include <math.h>
//#include <tchar.h>
//
//#include "ambpex_cfg.h"
//#include "drvfunc.h"
//#include "devfunc.h"
//#include "icr.h"

int dev_open(IPC_str *deviceName, int DeviceNumber);
int dev_openPlx(IPC_str *deviceName, int DeviceNumber);
void dev_close();
unsigned long GetVersion(char* pVerInfo);
unsigned long GetLocation(unsigned long& SlotNumber, unsigned long& BusNumber, unsigned long& DeviceNumber);
unsigned long GetDeviceID(unsigned short& DeviceID);
unsigned long GetPldStatus(unsigned long& PldStatus, unsigned long PldNum);
unsigned long WriteRegDataDir(unsigned long AdmNum, unsigned long TetrNum, unsigned long RegNum, unsigned long& RegVal);
unsigned long WriteRegData(unsigned long AdmNum, unsigned long TetrNum, unsigned long RegNum, unsigned long& RegVal);
unsigned long ReadRegDataDir(unsigned long AdmNum, unsigned long TetrNum, unsigned long RegNum, unsigned long& RegVal);
unsigned long ReadRegData(unsigned long AdmNum, unsigned long TetrNum, unsigned long RegNum, unsigned long& RegVal);
unsigned long ResetDmaFifo(int DmaChan);
unsigned long AllocMemory(void** pBuf, unsigned long blkSize, unsigned long& blkNum, unsigned long isSysMem, unsigned long dir, unsigned long addr, int DmaChan);
unsigned long FreeMemory(int DmaChan);
unsigned long StartDma(int IsCycling, int DmaChan);
unsigned long StopDma(int DmaChan);
unsigned long StateDma(unsigned long msTimeout, int DmaChan, int& state, ULONG& blkNum);
unsigned long WaitBuffer(unsigned long msTimeout, int DmaChan);
unsigned long WaitBlock(unsigned long msTimeout, int DmaChan);

// Global Variables:
int g_dev;

typedef struct _PLD_INFO {
	ULONG version;
	ULONG modification;
	ULONG build;
	ULONG submif;
}PLD_INFO, *PPLD_INFO;

typedef struct _TRD_INFO {
	ULONG id;
	ULONG ver;
	ULONG mod;
	ULONG tres;
	ULONG fsize;
	ULONG ftype;
	ULONG status;
}TRD_INFO, *PTRD_INFO;

// Numbers of Tetrad Registers
typedef enum _ADM2IF_NUM_TETR_REGS {
	ADM2IFnr_STATUS = 0,	// (0x00) Status register
	ADM2IFnr_DATA = 1,	// (0x02) Data register
	ADM2IFnr_CMDADR = 2,	// (0x04) Address command register
	ADM2IFnr_CMDDATA = 3,	// (0x06) Data command register
} ADM2IF_NUM_TETR_REGS;

// Numbers of Common Constant Registers
typedef enum _ADM2IF_NUM_CONST_REGS {
	ADM2IFnr_ID = 0x100,
	ADM2IFnr_IDMOD = 0x101,
	ADM2IFnr_VER = 0x102,
	ADM2IFnr_TRES = 0x103,
	ADM2IFnr_FSIZE = 0x104,
	ADM2IFnr_FTYPE = 0x105,
	ADM2IFnr_PATH = 0x106,
	ADM2IFnr_IDNUM = 0x107,
} ADM2IF_NUM_CONST_REGS;

// Numbers of Main Tetrad Constant Registers
typedef enum _MAIN_NUM_CONST_REGS {
	MAINnr_SIG = 0x108,
	MAINnr_ADMVER = 0x109,
	MAINnr_FPGAVER = 0x10A,
	MAINnr_FPGAMOD = 0x10B,
	MAINnr_TMASK = 0x10C,
	MAINnr_MRES = 0x10D,
	MAINnr_BASEID = 0x110,
	MAINnr_BASEVER = 0x111,
	MAINnr_SMODID = 0x112,
	MAINnr_SMODVER = 0x113,
	MAINnr_FPGABLD = 0x114,
} MAIN_NUM_CONST_REGS;

#define MAX_TETRNUM 16 // max number of tetrades of the ADM-interface

#define INSYS_TAG	0x4953 // tag of InSys
#define ADM_VERSION 0x200 // ADM version
#define FMC_VERSION 0x300 // FMC version

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


//***************************************************************************************
unsigned long AdmPldWorkAndCheck(PLD_INFO* pPldInfo)
{
	ULONG status = 0;
	ULONG value = 1; // ����������� ���� � ������� �����
	status = WriteRegDataDir(0, 0, ADM2IFnr_DATA, value);
	status = WriteRegDataDir(0, 0, ADM2IFnr_DATA, value);

	status = ReadRegDataDir(0, 0, ADM2IFnr_STATUS, value);
	status = ReadRegData(0, 0, 0, value);

	status = ReadRegData(0, 0, MAINnr_SIG, value);
	if (INSYS_TAG != value)
	{
		pPldInfo->version = value;
		return INSYS_TAG_ERROR;
	}
	status = ReadRegData(0, 0, MAINnr_ADMVER, value);
	if (ADM_VERSION != value && FMC_VERSION != value)
	{
		pPldInfo->version = value;
		return ADM_VERSION_ERROR;
	}
	pPldInfo->submif = value;

	status = ReadRegData(0, 0, MAINnr_FPGAVER, value);
	pPldInfo->version = value;
	status = ReadRegData(0, 0, MAINnr_FPGAMOD, value);
	pPldInfo->modification = value;

	status = ReadRegData(0, 0, MAINnr_FPGABLD, value);
	pPldInfo->build = value;

	//status = ReadRegData(0, 0, MAINnr_BASEID, value);
	//printf("Base Module ID = 0x%X\n", value);
	//status = ReadRegData(0, 0, MAINnr_BASEVER, value);
	//printf("Base Module Version = 0x%X\n", value);

	//status = ReadRegData(0, 0, MAINnr_SMODID, value);
	//printf("SubModule ID = 0x%X\n", value);
	//status = ReadRegData(0, 0, MAINnr_SMODVER, value);
	//printf("SubModule Version = 0x%X\n", value);

	return status;
}

//***************************************************************************************
void TetradList(PTRD_INFO info)
{
	ULONG status = 0;

	//	ULONG value = 1; // ����������� ���� � ������� �����
	//	status = WriteRegDataDir(0, 0, ADM2IFnr_DATA, value);

	//	ULONG valueID = 0;
	//	ULONG valueIDMOD = 0;
	//	ULONG valueVER = 0;
	int iTetr = 0;
	for (iTetr = 0; iTetr < MAX_TETRNUM; iTetr++)
	{
		status = ReadRegData(0, iTetr, ADM2IFnr_ID, info[iTetr].id);
		status = ReadRegData(0, iTetr, ADM2IFnr_IDMOD, info[iTetr].mod);
		status = ReadRegData(0, iTetr, ADM2IFnr_VER, info[iTetr].ver);
		status = ReadRegData(0, iTetr, ADM2IFnr_TRES, info[iTetr].tres);
		status = ReadRegData(0, iTetr, ADM2IFnr_FSIZE, info[iTetr].fsize);
		status = ReadRegData(0, iTetr, ADM2IFnr_FTYPE, info[iTetr].ftype);
		status = ReadRegDataDir(0, iTetr, ADM2IFnr_STATUS, info[iTetr].status);
		printf("Tetrad[%d] ID: %04X, MOD: %d, VER: %d.%d\n", iTetr, info[iTetr].id, info[iTetr].mod, info[iTetr].ver >> 8, info[iTetr].ver & 0xff);
	}
}

// tetrNum : ����� ������� 0 - MAIN, 6 - DIO_IN
// width : 0 - 64-������ ���� ������, 1 - 128-������ ���� ������
//***************************************************************************************
ULONG DmaChannelTest(int tetrNum, int width)
{
	ULONG status = 0;
	int dmaChan = 0; // ����� ������ DMA
	int fl_err = 0;

	int chan_state;
	ULONG blockNum;
	status = StateDma(0, dmaChan, chan_state, blockNum);
	if (status)
	{
		printf("StateDma (channel%d status = 0x%0X): ERROR !!!", dmaChan, status);
		return status;
	}
	if (3 != chan_state)
	{
		printf("StateDma: DMA Channel %d is busy already!!!\n", dmaChan);
		dmaChan = 1; // ����� ������ DMA

		status = StateDma(0, dmaChan, chan_state, blockNum);
		if (status)
		{
			printf("StateDma (channel%d status = 0x%0X): ERROR !!!", dmaChan, status);
			return status;
		}
		if (3 != chan_state)
		{
			printf("All DMA Channels are busy already!!!");
			return status;
		}
	}
	//printf("Buffer size is %d kBytes (%d block * %d kBytes)\n", (BlockSize / 1024) * BlockNum, BlockNum, BlockSize / 1024);

	//	HANDLE hThisThread = GetCurrentThread();
	//	SetThreadAffinityMask(hThisThread, 0x1);

	ULONG regVal = 1;
	if (tetrNum == 6) // DIO_IN
	{
		status = WriteRegData(0, 1, 0x0f, regVal); // TEST_CTLR:MUX = 1 - ���������� ���� ������������ �������� ������������������
		regVal = 1;
		status = WriteRegData(0, 1, 0x1f, regVal); // TEST_CTLR:GEN_SIZE - ������ ����� �������� ������������������ = 1 - 4 ������
		regVal = 0x1;
		status = WriteRegData(0, 1, 0x1e, regVal); // TEST_CTLR:GEN_CTRL - ����� ���� ������������ �������� ������������������
		regVal = 0x680;
		status = WriteRegData(0, 1, 0x1e, regVal); // TEST_CTLR:GEN_CTRL - ���� ��������� ������������� ���� (6 - 64-��������� ������� � ���������)
	}
	else
	{ // MAIN
		status = WriteRegData(0, 0, 0x0c, regVal); // ������� TEST_MODE[0]=1 - ����� ������������ ��������������� ������������������
	}
	//printf("Reseting DmaFifo...!!!\r"));
	//status = ResetDmaFifo(dmaChan);
	//if(status)
	//	printf("ResetDmaFifo: ERROR !!!\n"));
	//printf("DmaFifo is RESET!!!    \n"));

	//FIFO_STATUS fifo_stat;
	//fifo_stat.AsWhole = 0;
	//status = ReadLocalBusReg(fifo_stat.AsWhole, PEFIFOadr_FIFO_STATUS+4*0x100, 2);
	//printf("FIFO Status = 0x%04X\n", fifo_stat.AsWhole);

	ULONG dir = BRDstrm_DIR_IN; // ���� // BRDstrm_DIR_OUT; // �����
								//	ULONG blkSize = 4096*16;// * 1024; // ������ ������� �����
								//	ULONG blkSize = 1024 * 1024 * 8; // ������ ������� �����
								//ULONG blkSize = 65536;
								//ULONG blkSize = 1024 * 1024;
	ULONG blkSize = 4096 * 1024;
	ULONG blkNum = 1; // ����� ������
	ULONG isSysMem = 1; // ��������� ������ = 1, ���������������� ������ = 0
	ULONG addr = tetrNum; // ������� 0 (������� 16 ���), ADM-�������� 0 (������� 16 ���)
	PVOID pBuf = NULL; // ��������� �� ������ ���������� �� ����� (������������ �������� AllocMemory)

	printf("Allocating Memory...!!!\r");
	for (int i = 0; i < 4; i++)
	{
		status = AllocMemory(&pBuf, blkSize, blkNum, isSysMem, dir, addr, dmaChan);
		if (status)
			blkSize >>= 1;
		else
			break;
	}
	if (status)
	{
		printf("Allocate memory is ERROR!!!\n Block size = %d bytes", blkSize << 1);
	}
	else
	{
		printf("Memory is Allocated!!!  (%d block, Block size = %d kBytes)\n", blkNum, blkSize / 1024);
		PULONG* pBufferZ = (PULONG*)pBuf;

		//		if(extdma_sup)
		//			SetIrqMode(1, dmaChan);

		// �������� ���������� ������ ������
		//printf("Zeroing Memory...!!!\r"));
		for (ULONG iBlock = 0; iBlock < blkNum; iBlock++)
		{
			//for(ULONG i = 0; i < blkSize/4; i++)
			for (ULONG i = 0; i < 32; i++)
				pBufferZ[iBlock][i] = i;
		}

		if (tetrNum == 6) // DIO_IN
		{
			regVal = 0x1;
			status = WriteRegData(0, 1, 0x1e, regVal); // TEST_CTLR:GEN_CTRL - ����� ���� ������������ �������� ������������������
			regVal = 0x680;
			status = WriteRegData(0, 1, 0x1e, regVal); // TEST_CTLR:GEN_CTRL - ���� ��������� ������������� ���� (6 - 64-��������� ������� � ���������)
		}
		regVal = 2; // FIFO_RESET
		status = WriteRegData(0, tetrNum, 0, regVal); // MODE0 (DIO64_IN | MAIN tetrad)
		IPC_delay(1);
		regVal = 0;
		status = WriteRegData(0, tetrNum, 0, regVal); // MODE0 (DIO64_IN | MAIN tetrad)
		IPC_delay(1);
		status = ResetDmaFifo(dmaChan);
		status = StartDma(0, dmaChan); // ��� ������������
		if(status == IPC_OK)
			printf("StartDma: Success (DMA channel %d)!!!\n", dmaChan);
		else
			printf("StartDma: ERROR (DMA channel %d)!!!\n", dmaChan);

		//regVal = 0x2010; // HF, MASTER
		//status = WriteRegData(0, tetrNum, 0, regVal); // MODE0 (DIO64_IN | MAIN tetrad)
		regVal = 0x2038; // HF, START, MASTER, DRQ_EN
		status = WriteRegData(0, tetrNum, 0, regVal); // MODE0 (DIO64_IN | MAIN tetrad)
		if (tetrNum == 6) // DIO_IN
		{
			regVal = 0x6A0;
			status = WriteRegData(0, 1, 0x1e, regVal); // TEST_CTLR:GEN_CTRL - ����� ���� ������������ �������� ������������������
		}
		// ������� ��������� ���������� ����� ������

		//int i = 0;
		//if(extdma_sup)
		//{
		//	for(i = 0; i < 200; i++)
		//	{
		//		status = ReadLocalBusReg(regVal, PEFIFOadr_IRQ_CNT + extdma_offset[dmaChan], 2);
		//		if(regVal)
		//			break;
		//		IPC_delay(50);
		//	}
		//}
		//else		
		status = WaitBuffer(10000, dmaChan); // time-out 10 ��� (���� 0xFFFFFFFF - ����������� ��������)
		if (status == TIMEOUT_ERROR)
		{
			printf("TIMEOUT ERROR (DMA channel %d)!!!\n", dmaChan);
			fl_err = 1;
		}
		//printf("WaitBuffer: Success !!!\n"));
		status = StopDma(dmaChan);
		regVal = 0;
		status = WriteRegData(0, tetrNum, 0, regVal); // MODE0 (DIO64_IN | MAIN tetrad)

													  //		if(extdma_sup)
													  //			SetIrqMode(0, dmaChan);

		if (!fl_err)
		{
			// �������� �������� ������������������
			__int64 cnt_err = 0;
			__int64** pBuffer = (__int64**)pBuf;
			__int64 data_rd;
			if (tetrNum == 6) // DIO_IN
			{
				__int64 data_wr = 0x00000000A5A50123;
				for (ULONG iBlock = 0; iBlock < blkNum; iBlock++)
				{
					data_rd = pBuffer[iBlock][0];
					if (data_wr != data_rd)
					{
						cnt_err++;
						if (cnt_err < 10)
							printf("Error (%d): wr %016I64X : rd %016I64X\n", iBlock, data_wr, data_rd);
					}
					data_wr = ((__int64)iBlock + 1) << 40 | 0xA5A50123;
				}
			}
			else
			{
				__int64 data_wr = 2;
				__int64 data_wrh = 0xAA55;
				for (ULONG iBlock = 0; iBlock < blkNum; iBlock++)
				{
					for (ULONG i = 0; i < blkSize / 8; i++)
					{
						data_rd = pBuffer[iBlock][i];
						if (data_wr != data_rd)
						{
							cnt_err++;
							if (cnt_err < 10)
								printf("Error (%d, %d): wr %016I64X: rd %016I64X\n", i, iBlock, data_wr, data_rd);
							data_wr = data_rd;
						}
						ULONG data_h = ULONG(data_wr >> 32);
						ULONG f63 = data_h >> 31;
						ULONG f62 = data_h >> 30;
						ULONG f60 = data_h >> 28;
						ULONG f59 = data_h >> 27;
						ULONG f0 = (f63 ^ f62 ^ f60 ^ f59) & 1;

						data_wr <<= 1;
						data_wr &= ~1;
						data_wr |= f0;

						if (width)
						{
							data_rd = pBuffer[iBlock][i + 1];
							if (data_wrh != data_rd)
							{
								cnt_err++;
								if (cnt_err < 10)
									printf("Error (%d, %d): wr %016I64X: rd %016I64X\n", i, iBlock, data_wr, data_rd);
								data_wrh = data_rd;
							}
							ULONG data_h = ULONG(data_wrh >> 32);
							ULONG f63 = data_h >> 31;
							ULONG f62 = data_h >> 30;
							ULONG f60 = data_h >> 28;
							ULONG f59 = data_h >> 27;
							ULONG f0 = (f63 ^ f62 ^ f60 ^ f59) & 1;

							data_wrh <<= 1;
							data_wrh &= ~1;
							data_wrh |= f0;
							i++;
						}
					}
				}
			}

			if (cnt_err)
				printf("MAIN tetrada test is ERROR!!!\n");
			else
				printf("MAIN tetrada test is SUCCESS!!!\n");
			status = FreeMemory(dmaChan);
		}
	}
	return status;
}

//***************************************************************************************
unsigned long MainTest()
{
	printf("\n");
	ULONG valueVER = 0;
	ULONG status = ReadRegData(0, 0, ADM2IFnr_VER, valueVER);
	ULONG valueMOD = 0;
	status = ReadRegData(0, 0, ADM2IFnr_IDMOD, valueMOD);
	if ((valueMOD == 8 && valueVER >= 0x104) || (valueMOD == 17))
	{
		//MessageBox(NULL, L"Test MAIN tetrad!!!", L"ISDCLASS", MB_OK); 
		status = DmaChannelTest(0, 0);
	}
	else
	{
		if (valueMOD == 18)
			status = DmaChannelTest(0, 1);
		else
		{
			//TRD_INFO info[MAX_TETRNUM];
			//TetradList(info);
			//int iTetr = 0;
			//for (iTetr = 0; iTetr < MAX_TETRNUM; iTetr++)
			//	if (info[iTetr].id == 0x2D)
			//		break;
			//if (iTetr != MAX_TETRNUM)
			//{
			//	int tetr_out = iTetr;
			//	for (iTetr = 1; iTetr < MAX_TETRNUM; iTetr++)
			//		if (info[iTetr].tres & 0x10)
			//			break;
			//	if (iTetr != MAX_TETRNUM)
			//		BrdTest(iTetr, tetr_out);
			//}
			//else
			//	MessageBox(NULL, _T("TEST tetrada (test128_out) is NOT present !!!"), _T("AMBPEX Config"), MB_OK);
			printf("Test MAIN tetrada is not available\n");
		}
	}
	return status;
}

//***************************************************************************************
int main(int argc, char *argv[])
{
	unsigned long status = 0;
	// ����� ��� ������ ����� ���������� �� �����
	fflush(stdout);
	setbuf(stdout, NULL);

	g_dev = 0;
	//if (lstrlen(lpCmdLine))
	//{
	//	g_dev = _ttoi(lpCmdLine);
	//	_tcscpy(g_szDir, lpCmdLine + 2 * sizeof(TCHAR));
	//	SetCurrentDirectory(g_szDir);
	//}

	//IPC_str deviceName[256] = _IPCSTR("");
	IPC_str deviceName[256] = "";
	if (dev_open(deviceName, g_dev) < 0)
	{
		if (dev_openPlx(deviceName, g_dev) < 0)
		{
			printf("Open error device %s!", deviceName);
			return -1;
		}
	}

	char buf[MAX_STRING_LEN];
	status = GetVersion(buf);

	printf("Driver %s\n", buf);

	unsigned short DeviceID;
	status = GetDeviceID(DeviceID);

	printf("DeviceID 0x%X\n", DeviceID);

	unsigned long SlotNumber;
	unsigned long BusNumber;
	unsigned long DeviceNumber;
	status = GetLocation(SlotNumber, BusNumber, DeviceNumber);
	printf("Board location: slot %ld, bus %ld, device %ld\n", SlotNumber, BusNumber, DeviceNumber);

	ULONG PldStatus;
	GetPldStatus(PldStatus, 0);

	if (PldStatus)
	{
		PLD_INFO pld_info;
		status = AdmPldWorkAndCheck(&pld_info);
		if (!status)
		{
			printf("\nPLD: Ver: %d.%d, Mod: %d, Build: %d\n",
				pld_info.version >> 8, pld_info.version & 0xff, pld_info.modification, pld_info.build);

			TRD_INFO info[MAX_TETRNUM];
			TetradList(info);

			//DisplayTrdList(hWndTrdList, info);
		}
		else
		{
			printf("Error by AdmPldWorkAndCheck() = %d (0x%4X)\n", status, pld_info.version);
		}
	}
	else
		printf("ADM PLD is NOT loaded!!!\n");

	MainTest();

	dev_close();

	return 0;
}
