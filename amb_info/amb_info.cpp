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
#include <stdlib.h>
#include <string.h>
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
unsigned int GetVersion(char* pVerInfo);
unsigned int GetLocation(unsigned int& SlotNumber, unsigned int& BusNumber, unsigned int& DeviceNumber);
unsigned int GetDeviceID(unsigned short& DeviceID);
unsigned int GetPciCfgReg(unsigned int& RegVal, unsigned int Offset);
unsigned int ReadLocalBusReg(unsigned int& RegVal, unsigned int Offset, unsigned int Size);
unsigned int GetPldStatus(unsigned int& PldStatus, unsigned int PldNum);
unsigned int WriteRegDataDir(unsigned int AdmNum, unsigned int TetrNum, unsigned int RegNum, unsigned int& RegVal);
unsigned int WriteRegData(unsigned int AdmNum, unsigned int TetrNum, unsigned int RegNum, unsigned int& RegVal);
unsigned int ReadRegDataDir(unsigned int AdmNum, unsigned int TetrNum, unsigned int RegNum, unsigned int& RegVal);
unsigned int ReadRegData(unsigned int AdmNum, unsigned int TetrNum, unsigned int RegNum, unsigned int& RegVal);
unsigned int ReadNvRAM(void* pBuffer, unsigned int BufferSize, unsigned int Offset);
unsigned int ResetDmaFifo(int DmaChan);
unsigned int AllocMemory(void** pBuf, unsigned int blkSize, unsigned int& blkNum, unsigned int isSysMem, unsigned int dir, unsigned int addr, int DmaChan);
unsigned int FreeMemory(int DmaChan);
unsigned int StartDma(int IsCycling, int DmaChan);
unsigned int StopDma(int DmaChan);
unsigned int StateDma(unsigned int msTimeout, int DmaChan, int& state, unsigned int& blkNum);
unsigned int WaitBuffer(unsigned int msTimeout, int DmaChan);
unsigned int WaitBlock(unsigned int msTimeout, int DmaChan);

// Global Variables:
int g_dev;

typedef struct _PLD_INFO {
    unsigned int version;
    unsigned int modification;
    unsigned int build;
    unsigned int submif;
}PLD_INFO, *PPLD_INFO;

typedef struct _TRD_INFO {
    unsigned int id;
    unsigned int ver;
    unsigned int mod;
    unsigned int tres;
    unsigned int fsize;
    unsigned int ftype;
    unsigned int status;
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

#define NEXT_CAP_POINTER 0x34
#define PEXCAPID 0x10
#define DEVCAPOFFSET 0x04
#define DEVCTRLOFFSET 0x08
#define LINKOFFSET 0x10

// PCI Express Capability List Register & Capabilities Register (0x00)
typedef union _PEX_CAPREG {
	unsigned int AsWhole; // Register as a Whole Word
	struct { // Register as Bit Pattern
		unsigned int	CapId : 8, // Capability ID
			NextCapPointer : 8, // Next Capability Pointer
			CapVer : 4, // Capability version
			DevType : 4, // Device/Port type
			Slot : 1, // Slot Implemented
			IntMsgNum : 5, // Interrrupt Message Number
			Res : 2; // Reserved
	} ByBits;
} PEX_CAPREG, *PPEX_CAPREG;

// Link Control Register & Link Status Register (0x10)
typedef union _PEX_LINKREG {
	unsigned int AsWhole; // Register as a Whole Word
	struct { // Register as Bit Pattern
		unsigned int	ASPM : 2, // Active State Power Management
			Res : 1, // Reserved
			RCB : 1, // Root Completion Boundary
			LinkDis : 1, // Link Disable
			RetrLink : 1, // Retrain Link
			CmnClkCfg : 1, // Common Clock Configuration
			Sync : 1, // Extended Synch
			Res1 : 8, // Reserved
			LinkSpeed : 4, // Link Speed
			LinkWidth : 6, // Negotiated Link Width
			LinkTrainEr : 1, // Link Training Error
			LinkTrain : 1, // Link Training
			SlotClkCfg : 1, // Slot Clock Configuration
			Res2 : 3; // Reserved
	} ByBits;
} PEX_LINKREG, *PPEX_LINKREG;

// Numbers of PCI-Express Main block Registers
typedef enum _PE_MAIN_ADDR_REG {
	PEMAINadr_BLOCK_ID = 0x00, // 0x00
	PEMAINadr_BLOCK_VER = 0x08, // 0x01
	PEMAINadr_DEVICE_ID = 0x10, // 0x02
	PEMAINadr_DEVICE_REV = 0x18, // 0x03
	PEMAINadr_PLD_VER = 0x20, // 0x04
	PEMAINadr_BLOCK_CNT = 0x28, // 0x05
	PEMAINadr_CORE_MOD = 0x30, // 0x06
	PEMAINadr_CORE_ID = 0x38, // 0x07
	PEMAINadr_BRD_MODE = 0x40, // 0x08
	PEMAINadr_IRQ_MASK = 0x48, // 0x09
	PEMAINadr_IRQ_INV = 0x50, // 0x0A
	PEMAINadr_LEDS = 0x58, // 0x0B
	PEMAINadr_BRD_STATUS = 0x80, // 0x10
	PEMAINadr_IRQ_MIN = 0x90, // 0x12
	PEMAINadr_SPD_CTRL = 0xB0, // 0x16
	PEMAINadr_SPD_ADDR = 0xB8, // 0x17
	PEMAINadr_SPD_DATAL = 0xC0, // 0x18
	PEMAINadr_SPD_DATAH = 0xC8, // 0x19
	PEMAINadr_LB_DATA = 0xD0, // 0x1A
	PEMAINadr_JTAG_CNT = 0xE0, // 0x1C
	PEMAINadr_JTAG_TMS = 0xE8, // 0x1D
	PEMAINadr_JTAG_TDI = 0xF0, // 0x1E
	PEMAINadr_JTAG_TDO = 0xF8, // 0x1F
} PE_MAIN_ADDR_REG;

// Numbers of PCI-Express FIFO block Registers
typedef enum _PE_FIFO_ADDR_REG {
	PEFIFOadr_BLOCK_ID = 0x00, // 0x00
	PEFIFOadr_BLOCK_VER = 0x08, // 0x01
	PEFIFOadr_FIFO_ID = 0x10, // 0x02
	PEFIFOadr_FIFO_NUM = 0x18, // 0x03
	PEFIFOadr_DMA_SIZE = 0x20, // 0x04
	PEFIFOadr_FIFO_CTRL = 0x40, // 0x08
	PEFIFOadr_DMA_CTRL = 0x48, // 0x09
	PEFIFOadr_FIFO_STATUS = 0x80, // 0x10
	PEFIFOadr_FLAG_CLR = 0x88, // 0x11
	PEFIFOadr_ERROR_CNT = 0x90, // 0x12
	PEFIFOadr_PCI_ADDRL = 0xA0, // 0x14
	PEFIFOadr_PCI_ADDRH = 0xA8, // 0x15
	PEFIFOadr_PCI_SIZE = 0xB0, // 0x16
	PEFIFOadr_LOCAL_ADR = 0xB8, // 0x17
	PEFIFOadr_IRQ_MODE = 0xE0, // 0x1C
	PEFIFOadr_IRQ_TBLADR = 0xE8, // 0x1D
	PEFIFOadr_IRQ_TBLDATA = 0xF0, // 0x1E
	PEFIFOadr_IRQ_CNT = 0xF8, // 0x1F
} PE_FIFO_ADDR_REG;

typedef struct _PCIEC_INFO {
	unsigned int pldver;
	unsigned int dmaver;
	unsigned int coreid;
	unsigned int coremod;
	unsigned int errcrc;
	unsigned int errcto;
	unsigned int irqmin;
}PCIEC_INFO, *PPCIEC_INFO;

//***************************************************************************************
unsigned int PCIE_info(unsigned int* params)
{
	unsigned int status = 0;
	unsigned int regVal = 0;
	//unsigned int iaddr = 0;
	unsigned int next_addr = 0;
	status = GetPciCfgReg(next_addr, NEXT_CAP_POINTER);
	next_addr &= 0xff;

	for (int i = 0; i < 5; i++)
	{
		status = GetPciCfgReg(regVal, next_addr);
		PPEX_CAPREG pCapReg = (PPEX_CAPREG)&regVal;
		if (pCapReg->ByBits.CapId == PEXCAPID)
			break;
		next_addr = pCapReg->ByBits.NextCapPointer;
	}
	PPEX_CAPREG pCapReg = (PPEX_CAPREG)&regVal;
	if (pCapReg->ByBits.CapId != PEXCAPID)
		return 0;

	unsigned int devcap_addr = next_addr + DEVCAPOFFSET;
	status = GetPciCfgReg(regVal, devcap_addr);
	unsigned int Max_Payload_Size_Support = regVal & 0x7;

	unsigned int devctrl_addr = next_addr + DEVCTRLOFFSET;
	status = GetPciCfgReg(regVal, devctrl_addr);
	unsigned int Max_Payload_Size = (regVal >> 5) & 0x7;

	//printf("Max Payload Size (Supported) = %d(%d) \n", Max_Payload_Size, Max_Payload_Size_Support);
	//	printf("Capability version = %d\n", pCapReg->ByBits.CapVer);
	//	printf("Device/Port type = %d\n", pCapReg->ByBits.DevType);

	unsigned int link_addr = next_addr + LINKOFFSET;
	status = GetPciCfgReg(regVal, link_addr);
	PPEX_LINKREG pLinkReg = (PPEX_LINKREG)&regVal;

	//	printf("Negotiated Link Width = %d\n", pLinkReg->ByBits.LinkWidth);
	params[0] = pLinkReg->ByBits.LinkSpeed;
	params[1] = pLinkReg->ByBits.LinkWidth;
	switch (Max_Payload_Size)
	{
	case 0:
		params[2] = 128;
		break;
	case 1:
		params[2] = 256;
		break;
	case 2:
		params[2] = 512;
		break;
	case 3:
		params[2] = 1024;
		break;
	case 4:
		params[2] = 2048;
		break;
	case 5:
		params[2] = 4096;
		break;
	}

	switch (Max_Payload_Size_Support)
	{
	case 0:
		params[3] = 128;
		break;
	case 1:
		params[3] = 256;
		break;
	case 2:
		params[3] = 512;
		break;
	case 3:
		params[3] = 1024;
		break;
	case 4:
		params[3] = 2048;
		break;
	case 5:
		params[3] = 4096;
		break;
	}

	return status;
}

static int extdma_sup = 0;
static unsigned int extdma_offset[2] = { 0, 0 };

//***************************************************************************************
unsigned int HostReg(PPCIEC_INFO pInfo)
{
	unsigned int status = 0;
	unsigned int regVal = 0;

	pInfo->pldver = 0;
	pInfo->dmaver = 0;
	//status = ReadLocalBusReg(regVal, PEMAINadr_BLOCK_ID, 2);
	//printf("MAIN_BLOCK_ID = 0x%X.\n", regVal);
	//status = ReadLocalBusReg(regVal, PEMAINadr_BLOCK_VER, 2);
	//printf("MAIN_BLOCK_VER = 0x%X.\n", regVal);
	//status = ReadLocalBusReg(regVal, PEMAINadr_DEVICE_ID, 2);
	//printf("DeviceID = 0x%X.\n", regVal);
	//status = ReadLocalBusReg(regVal, PEMAINadr_DEVICE_REV, 2);
	//printf("REV_MOD = 0x%X.\n", regVal);
	status = ReadLocalBusReg(regVal, PEMAINadr_PLD_VER, 2);
	pInfo->pldver = regVal;
	//printf("PLDVER = 0x%X.\n", regVal);
	status = ReadLocalBusReg(regVal, PEMAINadr_CORE_ID, 2);
	pInfo->coreid = regVal;
	status = ReadLocalBusReg(regVal, PEMAINadr_CORE_MOD, 2);
	pInfo->coremod = regVal;
	status = ReadLocalBusReg(regVal, PEMAINadr_IRQ_MIN, 2);
	pInfo->irqmin = regVal;
	status = ReadLocalBusReg(regVal, PEMAINadr_BLOCK_CNT, 2);
	//printf("BLOCK_CNT = 0x%X.\n", regVal);

	int blk_cnt = regVal - 1;
	if (blk_cnt > 0xFF)
		return status;
	int idxDma = 0;
	pInfo->errcrc = (unsigned int)-1;
	pInfo->errcto = (unsigned int)-1;
	for (int i = 0; i < blk_cnt; i++)
	{
		unsigned int offset = (i + 1) * 0x100;
		status = ReadLocalBusReg(regVal, PEFIFOadr_BLOCK_ID + offset, 2);
		if (regVal != 0X1018) continue;
		//printf("FIFO_BLOCK_ID[%d] = 0x%X.\n", i, regVal);
		status = ReadLocalBusReg(regVal, PEFIFOadr_BLOCK_VER + offset, 2);
		pInfo->dmaver = regVal;
		status = ReadLocalBusReg(regVal, PEFIFOadr_ERROR_CNT + offset, 2);
		unsigned char sig = (unsigned char)(regVal >> 8);
		if (sig == 0xC1)
			pInfo->errcrc = regVal & 0xFF;
		if (sig == 0xC2)
			pInfo->errcto = regVal & 0xFF;
		status = ReadLocalBusReg(regVal, PEFIFOadr_IRQ_MODE + offset, 4);
		sig = (unsigned char)(regVal >> 24);
		//if(sig == 0xA2) // 0xA2 - старая сигнатура
		if (sig == 0xA4) // 0xA4 - новая сигнатура
		{
			extdma_sup = 1;
			extdma_offset[idxDma++] = offset;
			pInfo->dmaver |= (1 << 28);
		}
		status = ReadLocalBusReg(regVal, PEFIFOadr_IRQ_TBLADR + offset, 4);
		status = ReadLocalBusReg(regVal, PEFIFOadr_IRQ_TBLDATA + offset, 4);
		status = ReadLocalBusReg(regVal, PEFIFOadr_IRQ_CNT + offset, 4);
		//printf("FIFO_BLOCK_VER[%d] = 0x%X.\n", i, regVal);
		//status = ReadLocalBusReg(regVal, PEFIFOadr_FIFO_ID + offset, 2);
		//printf("FIFO_ID[%d] = 0x%X.\n", i, regVal);
		//status = ReadLocalBusReg(regVal, PEFIFOadr_FIFO_NUM + offset, 2);
		//printf("FIFO_NUM[%d] = 0x%X.\n", i, regVal);
		//status = ReadLocalBusReg(regVal, PEFIFOadr_DMA_SIZE + offset, 2);
		//printf("DMA_SIZE[%d] = 0x%X.\n", i, regVal);
	}

	return status;
}

//***************************************************************************************
void BasemodName(unsigned int id, char* str)
{
	switch (id)
	{
	case 0x4D58:    strcpy(str, "AMBPCX"); break;
	case 0x4D44:    strcpy(str, "AMBPCD"); break;
	case 0x1020:    strcpy(str, "ADS10X2G"); break;
	case 0x5502:    strcpy(str, "AMBPEX1"); break;
	case 0x5508:    strcpy(str, "AMBPEX2"); break;
	case 0x53A0:    strcpy(str, "SyncDAC"); break;
	case 0x53A1:    strcpy(str, "SyncBCO"); break;
	case 0x53A2:    strcpy(str, "Sync-cP6"); break;
	case 0x53B4:    strcpy(str, "RFDR4"); break;
	case 0x551B:    strcpy(str, "Sync8P"); break;

	case 0x5503:    strcpy(str, "AMBPEX8"); break;
	case 0x5504:    strcpy(str, "ADP201X1AMB"); break;
	case 0x5505:    strcpy(str, "ADP201X1DSP"); break;
	case 0x5507:    strcpy(str, "AMBPEX5"); break;
	case 0x5509:    strcpy(str, "FMC105P"); break;
	case 0x550A:    strcpy(str, "FMC106P"); break;
	case 0x5511:    strcpy(str, "FMC107P"); break;
	case 0x550B:    strcpy(str, "FMC103E2"); break;
	case 0x550C:    strcpy(str, "FMC114V"); break;
	case 0x550D:    strcpy(str, "FMC110P"); break;
	case 0x550E:    strcpy(str, "FMC113E"); break;
	case 0x550F:    strcpy(str, "FMC108V"); break;

	case 0x53B1:    strcpy(str, "FMC115cP"); break;
	case 0x53B2:    strcpy(str, "FMC112cP"); break;
	case 0x53B3:    strcpy(str, "FMC117cP"); break;
	case 0x53B5:    strcpy(str, "FMC121cP"); break;
	case 0x53B6:    strcpy(str, "FMC125cP"); break;
	case 0x53B7:    strcpy(str, "Sync-cP6R"); break;
	case 0x53B8:    strcpy(str, "Panorama"); break;

	case 0x5516:    strcpy(str, "XM216X250M"); break;
	case 0x551C:    strcpy(str, "FMC122P"); break;
	case 0x551D:    strcpy(str, "FMC123E"); break;
	case 0x551E:    strcpy(str, "FMC124P"); break;
	case 0x551F:    strcpy(str, "FMC127P"); break;
	case 0x5520:    strcpy(str, "FMC122P-SLAVE"); break;
	case 0x5521:    strcpy(str, "FMC111P"); break;
	case 0x5522:    strcpy(str, "FMC126P"); break;

	case 0x5514:    strcpy(str, "AC_SYNC"); break;
	case 0x5518:    strcpy(str, "PS_DSP"); break;
	case 0x5519:    strcpy(str, "PS_ADC"); break;
	case 0x551A:    strcpy(str, "PS_SYNC"); break;

	default: strcpy(str, "UNKNOWN"); break;
	}
}

//unsigned char g_pldType = 0;
//
////***************************************************************************************
//unsigned int GetPldDescription(char* Description, unsigned char* pBaseEEPROMMem, unsigned int BaseEEPROMSize)
//{
//	ICR_CfgAdmPld cfgPld;
//	cfgPld.bType = 0;
//	cfgPld.wVolume = 1;
//	cfgPld.wPins = 2;
//	cfgPld.bSpeedGrade = 3;
//	GetPldCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &cfgPld);
//	g_pldType = cfgPld.bType;
//	sprintf(Description, "Chip: %d %d %d %d\n", cfgPld.bType, cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//	switch (cfgPld.bType)
//	{
//	case 3:
//		sprintf(Description, "Chip: XCVE%d-%dFF-%dC\n", cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
//		break;
//	case 4:
//		sprintf(Description, "Chip: XC2V%d-%dFF-%dC\n", cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
//		break;
//	case 5:
//		sprintf(Description, "Chip: XC2S%d-FG%d-%dC\n", cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
//	case 6:
//		sprintf(Description, "Chip: XC2S%dE-FG%d-%dC\n", cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
//		break;
//	case 7:
//		sprintf(Description, "Chip: XC3S%d-FG%d-%dC\n", cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
//		break;
//	case 8:
//		sprintf(Description, "Chip: XC4VLX%d-%dFF%dC\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	case 9:
//		sprintf(Description, "Chip: XC4VSX%d-%dFF%dC\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	case 10:
//		sprintf(Description, "Chip: XC4VFX%d-%dFF%dC\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	case 11:
//		sprintf(Description, "Chip: XC3S%dE-FG%d-%dC\n", cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
//		break;
//	case 12:
//		sprintf(Description, "Chip: XC3S%dA-FG%d-%dC\n", cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
//		break;
//	case 15:
//		sprintf(Description, "Chip: XC5VLX%d-%dFFG%dC\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	case 16:
//		sprintf(Description, "Chip: XC5VSX%d-%dFFG%dC\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	case 17:
//		sprintf(Description, "Chip: XC5VFX%d-%dFFG%dC\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	case 18:
//		sprintf(Description, "Chip: XC5VLX%dT-%dFFG%dC\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	case 19:
//		sprintf(Description, "Chip: XC5VSX%dT-%dFFG%dC\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	case 20:
//		sprintf(Description, "Chip: XC5VFX%dT-%dFFG%dC\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	case 21:
//		sprintf(Description, "Chip: XC6VSX%dT-%dFFG%dC\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	case 22:
//		sprintf(Description, "Chip: XC6VLX%dT-%dFFG%dC\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	case 23:
//		sprintf(Description, "Chip: XC7K%dT-%dFFG%dC\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	case 24:
//		sprintf(Description, "Chip: XC7A%dT-%dFFG%dC\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	case 25:
//		sprintf(Description, "Chip: XCKU%d-%dFFVA%dE\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	case 26:
//		sprintf(Description, "Chip: XCKU%dP-%dFFVA%dE\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	case 27:
//		sprintf(Description, "Chip: XCVU%d-%dFFVA%dE\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	case 28:
//		sprintf(Description, "Chip: XCVU%dP-%dFFVA%dE\n", cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
//		break;
//	}
//	return 1;
//}

//***************************************************************************************
unsigned int AdmPldWorkAndCheck(PLD_INFO* pPldInfo)
{
    unsigned int status = 0;
    unsigned int value = 1; // переключить ПЛИС в рабочий режим
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
    //unsigned int status = 0;

    //	unsigned int value = 1; // переключить ПЛИС в рабочий режим
	//	status = WriteRegDataDir(0, 0, ADM2IFnr_DATA, value);

    //	unsigned int valueID = 0;
    //	unsigned int valueIDMOD = 0;
    //	unsigned int valueVER = 0;
	int iTetr = 0;
	for (iTetr = 0; iTetr < MAX_TETRNUM; iTetr++)
	{
		ReadRegData(0, iTetr, ADM2IFnr_ID, info[iTetr].id);
		ReadRegData(0, iTetr, ADM2IFnr_IDMOD, info[iTetr].mod);
		ReadRegData(0, iTetr, ADM2IFnr_VER, info[iTetr].ver);
		ReadRegData(0, iTetr, ADM2IFnr_TRES, info[iTetr].tres);
		ReadRegData(0, iTetr, ADM2IFnr_FSIZE, info[iTetr].fsize);
		ReadRegData(0, iTetr, ADM2IFnr_FTYPE, info[iTetr].ftype);
		ReadRegDataDir(0, iTetr, ADM2IFnr_STATUS, info[iTetr].status);
		printf("Tetrad[%d] ID: %04X, MOD: %d, VER: %d.%d\n", iTetr, info[iTetr].id, info[iTetr].mod, info[iTetr].ver >> 8, info[iTetr].ver & 0xff);
	}
}

//***************************************************************************************
void ErrorDetect()
{
	//unsigned int status = 0;

	unsigned int valueVER = 0;
	ReadRegData(0, 0, ADM2IFnr_VER, valueVER);
	unsigned int valueMOD = 0;
	ReadRegData(0, 0, ADM2IFnr_IDMOD, valueMOD);
	PCIEC_INFO pcic_info;
	HostReg(&pcic_info);
	if (pcic_info.coreid == 0x22 && pcic_info.pldver <= 0x104)
	{
		printf("\n>>>PLD consists error<<<:\nBug#1455 - FM-module PRESENT is not correct.\nUpdate that PLD!!!\n");
	}

	if ((pcic_info.coreid == 0x12 && pcic_info.pldver < 0x101) || (pcic_info.coreid == 0x11 && pcic_info.pldver < 0x102))
	{
		printf("\n>>>PLD consists error<<<:\nBug#466 - FMC Power on is not correct.\nUpdate that PLD!!!\n");
	}

	if (pcic_info.dmaver < 0x102)
	{
		printf("\n>>>PLD (DMA channel version < 1.2) consists error<<<:\nBug#182 - error by 64-bit DMA addressing (above 4GB memory).\nUpdate that PLD!!!\n");
	}
	else
	{
		if (pcic_info.dmaver < 0x103)
		{
			printf("\n>>>PLD (DMA channel version < 1.3) consists error<<<:\nBug#147 - there is a chance the DMA channel is stopped.\nUpdate that PLD!!!\n");
		}
		if (pcic_info.dmaver == 0x107)
		{
			printf("\n>>>PLD (DMA channel version = 1.7) consists error<<<:\nBug#2362 - error by DMA restart.\nUpdate that PLD!!!\n");
		}
		if (pcic_info.errcrc != (unsigned int)-1 && pcic_info.errcrc)
		{
			printf("\n>>>DMA descriptor CRC error counter = %d !!!<<<\n", pcic_info.errcrc);
		}
		if (pcic_info.errcto != (unsigned int)-1 && pcic_info.errcto)
		{
			printf("\n>>>DMA Completion timeout error counter = %d !!!<<<\n", pcic_info.errcto);
		}

	}
}

// tetrNum : номер тетрады 0 - MAIN, 6 - DIO_IN
// width : 0 - 64-битная шина данных, 1 - 128-битная шина данных
//***************************************************************************************
unsigned int DmaChannelTest(int tetrNum, int width)
{
    unsigned int status = 0;
	int dmaChan = 0; // номер канала DMA
	int fl_err = 0;

	int chan_state;
    unsigned int blockNum;
	status = StateDma(0, dmaChan, chan_state, blockNum);
	if (status)
	{
		printf("StateDma (channel%d status = 0x%0X): ERROR !!!", dmaChan, status);
		return status;
	}
	if (3 != chan_state)
	{
		printf("StateDma: DMA Channel %d is busy already!!!\n", dmaChan);
		dmaChan = 1; // номер канала DMA

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

    unsigned int regVal = 1;
	if (tetrNum == 6) // DIO_IN
	{
		status = WriteRegData(0, 1, 0x0f, regVal); // TEST_CTLR:MUX = 1 - подключить узел формирования тестовой последовательности
		regVal = 1;
		status = WriteRegData(0, 1, 0x1f, regVal); // TEST_CTLR:GEN_SIZE - размер блока тестовой последовательности = 1 - 4 кбайта
		regVal = 0x1;
		status = WriteRegData(0, 1, 0x1e, regVal); // TEST_CTLR:GEN_CTRL - сброс узла формирования тестовой последовательности
		regVal = 0x680;
		status = WriteRegData(0, 1, 0x1e, regVal); // TEST_CTLR:GEN_CTRL - узел формирует фиксированный блок (6 - 64-разрядный счетчик с инверсией)
	}
	else
	{ // MAIN
		status = WriteRegData(0, 0, 0x0c, regVal); // Регистр TEST_MODE[0]=1 - режим формирования псевдослучайной последовательности
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

    unsigned int dir = BRDstrm_DIR_IN; // ввод // BRDstrm_DIR_OUT; // вывод
                                //	unsigned int blkSize = 4096*16;// * 1024; // размер каждого блока
                                //	unsigned int blkSize = 1024 * 1024 * 8; // размер каждого блока
                                //unsigned int blkSize = 65536;
                                //unsigned int blkSize = 1024 * 1024;
    unsigned int blkSize = 4096 * 1024;
    unsigned int blkNum = 1; // число блоков
    unsigned int isSysMem = 1; // системная память = 1, пользовательская память = 0
    unsigned int addr = tetrNum; // тетрада 0 (младшие 16 бит), ADM-интерфес 0 (старшие 16 бит)
    void* pBuf = NULL; // указатель на массив указателей на блоки (возвращается функцией AllocMemory)

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
        //unsigned int* pBufferZ = (unsigned int*)pBuf;

		//		if(extdma_sup)
		//			SetIrqMode(1, dmaChan);

		// заполняю выделенные буфера нулями
		//printf("Zeroing Memory...!!!\r"));
        //for (unsigned int iBlock = 0; iBlock < blkNum; iBlock++)
        //{
            //for(unsigned int i = 0; i < blkSize/4; i++)
            //for (unsigned int i = 0; i < 32; i++)
            //	pBufferZ[iBlock][i] = i;
        //}

		if (tetrNum == 6) // DIO_IN
		{
			regVal = 0x1;
			status = WriteRegData(0, 1, 0x1e, regVal); // TEST_CTLR:GEN_CTRL - сброс узла формирования тестовой последовательности
			regVal = 0x680;
			status = WriteRegData(0, 1, 0x1e, regVal); // TEST_CTLR:GEN_CTRL - узел формирует фиксированный блок (6 - 64-разрядный счетчик с инверсией)
		}
		regVal = 2; // FIFO_RESET
		status = WriteRegData(0, tetrNum, 0, regVal); // MODE0 (DIO64_IN | MAIN tetrad)
		IPC_delay(1);
		regVal = 0;
		status = WriteRegData(0, tetrNum, 0, regVal); // MODE0 (DIO64_IN | MAIN tetrad)
		IPC_delay(1);
		status = ResetDmaFifo(dmaChan);
		status = StartDma(0, dmaChan); // без зацикливания
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
			status = WriteRegData(0, 1, 0x1e, regVal); // TEST_CTLR:GEN_CTRL - старт узла формирования тестовой последовательности
		}
		// ожидаем окончания заполнения всего буфера

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
		status = WaitBuffer(10000, dmaChan); // time-out 10 сек (если 0xFFFFFFFF - бесконечное ожидание)
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
			// контроль тестовой последовательности
            long long cnt_err = 0;
            long long** pBuffer = (long long**)pBuf;
            long long data_rd;
			if (tetrNum == 6) // DIO_IN
			{
                long long data_wr = 0x00000000A5A50123;
                for (unsigned int iBlock = 0; iBlock < blkNum; iBlock++)
				{
					data_rd = pBuffer[iBlock][0];
					if (data_wr != data_rd)
					{
						cnt_err++;
						if (cnt_err < 10)
							printf("Error (%d): wr %016llX : rd %016llX\n", iBlock, data_wr, data_rd);
					}
                    data_wr = ((long long)iBlock + 1) << 40 | 0xA5A50123;
				}
			}
			else
			{
                long long data_wr = 2;
                long long data_wrh = 0xAA55;
                for (unsigned int iBlock = 0; iBlock < blkNum; iBlock++)
				{
                    for (unsigned int i = 0; i < blkSize / 8; i++)
					{
						data_rd = pBuffer[iBlock][i];
						if (data_wr != data_rd)
						{
							cnt_err++;
							if (cnt_err < 10)
								printf("Error (%d, %d): wr %016llX: rd %016llX\n", i, iBlock, data_wr, data_rd);
							data_wr = data_rd;
						}
                        unsigned int data_h = (unsigned int)(data_wr >> 32);
                        unsigned int f63 = data_h >> 31;
                        unsigned int f62 = data_h >> 30;
                        unsigned int f60 = data_h >> 28;
                        unsigned int f59 = data_h >> 27;
                        unsigned int f0 = (f63 ^ f62 ^ f60 ^ f59) & 1;

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
									printf("Error (%d, %d): wr %016llX: rd %016llX\n", i, iBlock, data_wr, data_rd);
								data_wrh = data_rd;
							}
                            unsigned int data_h = (unsigned int)(data_wrh >> 32);
                            unsigned int f63 = data_h >> 31;
                            unsigned int f62 = data_h >> 30;
                            unsigned int f60 = data_h >> 28;
                            unsigned int f59 = data_h >> 27;
                            unsigned int f0 = (f63 ^ f62 ^ f60 ^ f59) & 1;

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
unsigned int MainTest()
{
	printf("\n");
    unsigned int valueVER = 0;
    unsigned int status = ReadRegData(0, 0, ADM2IFnr_VER, valueVER);
    unsigned int valueMOD = 0;
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
    unsigned int status = 0;
	// чтобы все печати сразу выводились на экран
	fflush(stdout);
	setbuf(stdout, NULL);

    //printf("sizeof(unsigned int) = %d\n", sizeof(unsigned int));
    //printf("sizeof(unsigned int) = %d\n", sizeof(unsigned int));
    //printf("sizeof(long long) = %d\n", sizeof(long long));

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

	//int g_basemFlg = 0;
	unsigned char g_bicrData[256];
	for (int i = 0; i < 256; i++)
		g_bicrData[i] = 0;

	ReadNvRAM(g_bicrData, 256, 0x80);
	unsigned short btag = *(unsigned short*)g_bicrData;
	char pldDescrBuf[MAX_STRING_LEN];
	if (btag == 0x4953)
	{
		char typestr[MAX_STRING_LEN];
		unsigned int ser_num = *(unsigned int*)(g_bicrData + 6);
		unsigned int base_type = *(unsigned short*)(g_bicrData + 10);
		unsigned int base_ver = *(g_bicrData + 12);
		BasemodName(base_type, typestr);
		printf("%s (0x%04X) : s/n = %d, version = %d.%d\n", typestr, base_type, ser_num, base_ver >> 4, base_ver & 0xF);
		//GetPldDescription(pldDescrBuf, g_bicrData, 256);
		//SetDlgItemText(hdlg, IDC_PLDDESCR, pldDescrBuf);
		//g_basemFlg = 1;
	}
	else
	{
		printf("Base ICR error!!!\n");
		printf(pldDescrBuf, "Base ICR error!!!");
		//g_basemFlg = 0;
	}
	
	unsigned int SlotNumber;
    unsigned int BusNumber;
    unsigned int DeviceNumber;
	status = GetLocation(SlotNumber, BusNumber, DeviceNumber);
	printf("Board location: slot %d, bus %d, device %d\n", SlotNumber, BusNumber, DeviceNumber);

	unsigned int pciex_info[4];
	PCIE_info(pciex_info);
	if (pciex_info[0] == 1)
		printf("PCI Express 1.1, lanes: %d\n", pciex_info[1]);
	else
		if (pciex_info[0] == 2)
			printf("PCI Express 2.0, lanes: %d\n", pciex_info[1]);
		else
			if (pciex_info[0] == 3)
				printf("PCI Express 3.0, lanes: %d\n", pciex_info[1]);

	printf("Max Payload (Supported) = %d (%d) bytes\n", pciex_info[2], pciex_info[3]);

	unsigned int PldStatus;
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

	ErrorDetect();
		
	MainTest();

	dev_close();

	return 0;
}
