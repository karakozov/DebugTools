
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "drvfunc.h"
#include "devfunc.h"
#include "icr.h"

//***************************************************************************************
ULONG PCIE_info(ULONG* params)
{
	ULONG status = 0;
	ULONG regVal = 0;
	ULONG iaddr = 0;
	ULONG next_addr = 0;
	status = GetPciCfgReg(next_addr, NEXT_CAP_POINTER);
	next_addr &= 0xff;

	for(int i = 0; i < 5; i++)
	{
		status = GetPciCfgReg(regVal, next_addr);
		PPEX_CAPREG pCapReg = (PPEX_CAPREG)&regVal;
		if(pCapReg->ByBits.CapId == PEXCAPID)
			break;
		next_addr = pCapReg->ByBits.NextCapPointer;
	}
	PPEX_CAPREG pCapReg = (PPEX_CAPREG)&regVal;
	if(pCapReg->ByBits.CapId != PEXCAPID)
		return 0;
		
	ULONG devcap_addr = next_addr + DEVCAPOFFSET;
	status = GetPciCfgReg(regVal, devcap_addr);
	ULONG Max_Payload_Size_Support = regVal & 0x7;

	ULONG devctrl_addr = next_addr + DEVCTRLOFFSET;
	status = GetPciCfgReg(regVal, devctrl_addr);
	ULONG Max_Payload_Size = (regVal >> 5) & 0x7;

	//printf("Max Payload Size (Supported) = %d(%d) \n", Max_Payload_Size, Max_Payload_Size_Support);
//	printf("Capability version = %d\n", pCapReg->ByBits.CapVer);
//	printf("Device/Port type = %d\n", pCapReg->ByBits.DevType);

	ULONG link_addr = next_addr + LINKOFFSET;
	status = GetPciCfgReg(regVal, link_addr);
	PPEX_LINKREG pLinkReg = (PPEX_LINKREG)&regVal;

//	printf("Negotiated Link Width = %d\n", pLinkReg->ByBits.LinkWidth);
	params[0] = pLinkReg->ByBits.LinkSpeed;
	params[1] = pLinkReg->ByBits.LinkWidth;
	switch(Max_Payload_Size)
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

	switch(Max_Payload_Size_Support)
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

//***************************************************************************************
ULONG LinkWidth(ULONG& LinkSpeed)
{
	ULONG status = 0;
	ULONG regVal = 0;
	ULONG iaddr = 0;
	ULONG next_addr = 0;
	status = GetPciCfgReg(next_addr, NEXT_CAP_POINTER);
	next_addr &= 0xff;

	for(int i = 0; i < 5; i++)
	{
		status = GetPciCfgReg(regVal, next_addr);
		PPEX_CAPREG pCapReg = (PPEX_CAPREG)&regVal;
		if(pCapReg->ByBits.CapId == PEXCAPID)
			break;
		next_addr = pCapReg->ByBits.NextCapPointer;
	}
	PPEX_CAPREG pCapReg = (PPEX_CAPREG)&regVal;
	if(pCapReg->ByBits.CapId != PEXCAPID)
		return 0;
		
//	printf("Capability version = %d\n", pCapReg->ByBits.CapVer);
//	printf("Device/Port type = %d\n", pCapReg->ByBits.DevType);

	ULONG link_addr = next_addr + LINKOFFSET;
	status = GetPciCfgReg(regVal, link_addr);
	PPEX_LINKREG pLinkReg = (PPEX_LINKREG)&regVal;

//	printf("Negotiated Link Width = %d\n", pLinkReg->ByBits.LinkWidth);
	LinkSpeed = pLinkReg->ByBits.LinkSpeed;
	return pLinkReg->ByBits.LinkWidth;
}

static int extdma_sup = 0;
static ULONG extdma_offset[2] = {0, 0};

//***************************************************************************************
ULONG HostReg(PPCIEC_INFO pInfo)
{
	ULONG status = 0;
	ULONG regVal = 0;

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

	int blk_cnt = regVal-1;
	if(blk_cnt > 0xFF)
		return status;
	int idxDma = 0;
	pInfo->errcrc = (ULONG)-1;
	pInfo->errcto = (ULONG)-1;
	for(int i = 0; i < blk_cnt; i++)
	{
		ULONG offset = (i+1) * 0x100;
		status = ReadLocalBusReg(regVal, PEFIFOadr_BLOCK_ID + offset, 2);
		if(regVal != 0X1018) continue;
		//printf("FIFO_BLOCK_ID[%d] = 0x%X.\n", i, regVal);
		status = ReadLocalBusReg(regVal, PEFIFOadr_BLOCK_VER + offset, 2);
		pInfo->dmaver = regVal;
		status = ReadLocalBusReg(regVal, PEFIFOadr_ERROR_CNT + offset, 2);
		UCHAR sig = UCHAR(regVal >> 8);
		if(sig == 0xC1)
			pInfo->errcrc = regVal & 0xFF;
		if(sig == 0xC2)
			pInfo->errcto = regVal & 0xFF;
		status = ReadLocalBusReg(regVal, PEFIFOadr_IRQ_MODE + offset, 4);
		sig = UCHAR(regVal >> 24);
		//if(sig == 0xA2) // 0xA2 - старая сигнатура
		if(sig == 0xA4) // 0xA4 - новая сигнатура
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
ULONG GetBlockFidAddress(PULONG& BlockMem)
{
	ULONG errorCode = GetMemoryAddress(BlockMem);
	ULONG BlockCnt = BlockMem[10];
	ULONG BlockFidAddr = 0;
	if(BlockCnt >= 0xFF)
		return BlockFidAddr;
	for(ULONG iBlock = 1; iBlock < BlockCnt; iBlock++)
	{
		ULONG FidAddr = iBlock * 64;
		ULONG temp = BlockMem[FidAddr];
		//temp = ReadOperationWordReg(PEFIDadr_BLOCK_ID + FidAddr);
		ULONG block_id = (temp & 0x0FFF);
		if(block_id == 0x0019)
		{
			BlockFidAddr = FidAddr;
			break;
		}
	}
	return BlockFidAddr;
}

//***************************************************************************************
void FM_Present(PPCIEC_INFO pInfo, int* fm_present)
{
	ULONG status = 0;
	// значения по-умолчанию
	fm_present[0] = 1;	// субмодуль FMC0 присутствует
	fm_present[1] = 0;	// субмодуль FMC1 отсутствует
	fm_present[2] = 0;	// субмодуль FMC2 отсутствует
	if(pInfo->coreid != 0x22 || (pInfo->coreid == 0x22 && pInfo->pldver > 0x104))
	{
		PULONG addrBlocks;
		ULONG addrFID = GetBlockFidAddress(addrBlocks);
		if(addrFID)
		{ // FMC субмодули
			PULONG BlockMem = addrBlocks + addrFID;
			ULONG PowerStat = BlockMem[32]; // POWER_STATUS

			if (!(PowerStat & 0x100))
				fm_present[0] = 0;	// субмодуль FMC0 отсутствует
			if (PowerStat & 0x400)
				fm_present[1] = 1;	// субмодуль FMC1 присутствует
			if (PowerStat & 0x1000)
				fm_present[2] = 1;	// субмодуль FMC2 присутствует
		}
	}
}

//***************************************************************************************
void PauseEx(unsigned int mctime_out)
{
#ifdef _WIN32
	static LARGE_INTEGER Frequency, zero_time, cur_time;
	int bHighRes = QueryPerformanceFrequency (&Frequency);
	QueryPerformanceCounter(&zero_time);
	unsigned int wait_time;
	do {
		QueryPerformanceCounter(&cur_time);
		wait_time = (unsigned int)((cur_time.QuadPart - zero_time.QuadPart) * 1.E6 / Frequency.QuadPart);
	} while(wait_time < mctime_out);
#else
    //fprintf(stderr, "%s, %d: - %s not implemented\n", __FILE__, __LINE__, __FUNCTION__);
    IPC_delay(1);
#endif
}

//***************************************************************************************
ULONG WaitFidReady(ULONG* BlockMem)
{
	ULONG SpdStatus;
	for(int j = 0; j < 1000; j++)
	{
		SpdStatus = BlockMem[44]; // SPD_CTRL
		if(SpdStatus & 0x8000)// Ready
			break;
		PauseEx(10);
	}
	if(!(SpdStatus & 0x8000)) // Ready
		return 1;

	return 0;
}

//***************************************************************************************
ULONG ReadFidSpd(ULONG* BlockMem, int devnum, int devadr, UCHAR* buf, USHORT Offset, int Length)
{
	ULONG Status = 0;
	//ULONG* BlockMem = m_pMemBuf[0] + m_BlockFidAddr;
	//ULONG blk_id = BlockMem[0];
	//ULONG blk_ver = BlockMem[2];

	BlockMem[42] = devnum; // SPD_DEVICE
	BlockMem[44] = 0; // SPD_CTRL
	PauseEx(1000);
	if(WaitFidReady(BlockMem) != 0)
		return 1;
	
	for(int i = 0; i < Length; i++)
	{
		int sector = Offset / 256;
		BlockMem[46] = Offset; // SPD_ADR
		BlockMem[44] = ((devadr + sector) << 4) + 1 ; // SPD_CTRL, type operation - read
		if(WaitFidReady(BlockMem) != 0)
		{
			buf[i] = (UCHAR)BlockMem[48]; // read data
			Status = 1;
			break;
		}
		buf[i] = (UCHAR)BlockMem[48]; // read data
		Offset++;
	}

	BlockMem[44] = 0; // SPD_CTRL
		
	return Status;
}

//***************************************************************************************
ULONG ReadSubICR(int submod, void* pBuffer, ULONG BufferSize, ULONG Offset)
{
	ULONG Status = 0;
	PULONG addrBlocks;
	ULONG addrFID = GetBlockFidAddress(addrBlocks);
	if(addrFID)
	{
		ULONG* BlockMem = addrBlocks + addrFID;
		//memset(EepromSubmod, 0xaa, 256);
		if(submod)
			Status = ReadFidSpd(BlockMem, 2, 0x50, (UCHAR*)pBuffer, 0x400, BufferSize); // FMC2
		else
			Status = ReadFidSpd(BlockMem, 0, 0x50, (UCHAR*)pBuffer, 0x400, BufferSize); // FMC1

		//memcpy(pBuffer, EepromSubmod + Offset, BufferSize);
	}
	else
	{
		if(!submod)
			Status = ReadAdmIdROM(pBuffer, BufferSize, Offset);
		else
			Status = 1;
	}
	return Status;
}

//***************************************************************************************
ULONG GetPower(PFMC_POWER pow)
{
	ULONG Status = 0;
	PULONG addrBlocks;
	ULONG addrFID = GetBlockFidAddress(addrBlocks);
	if(addrFID)
	{
		PULONG BlockMem = addrBlocks + addrFID;
		ULONG blk_id = BlockMem[0];
		ULONG blk_ver = BlockMem[2];
		ULONG blk_v0 = BlockMem[4];
		ULONG blk_v1 = BlockMem[6];
		ULONG blk_v2 = BlockMem[8];
		ULONG blk_v3 = BlockMem[10];

		ULONG PowerStat;
		PowerStat = BlockMem[32]; // POWER_STATUS
		ULONG PowerCtrl;
		PowerCtrl = BlockMem[22]; // POWER_CTRL
		PowerCtrl &= 0x8000; // mask POWER_CTRL[power_en]
		if(PowerCtrl)
			pow->onOff = 1;
		else
			pow->onOff = 0;
		pow->value = blk_v0;
		pow->slot = PowerStat;
	}
	else
	{
		pow->value = 0;
		pow->slot = 0;
		Status = 1;
	}
	return Status;
}

//***************************************************************************************
ULONG PowerOnOff(PFMC_POWER pow, int force)
{
	ULONG Status = 0;
	ULONG PowerStat;
	PULONG addrBlocks;
	ULONG addrFID = GetBlockFidAddress(addrBlocks);
	if(addrFID)
	{// программируем источник питания для FMC-субмодуля
		PULONG BlockMem = addrBlocks + addrFID;
		ULONG blk_id = BlockMem[0];
		ULONG blk_ver = BlockMem[2];
		ULONG blk_v0 = BlockMem[4];
		ULONG blk_v1 = BlockMem[6];
		ULONG blk_v2 = BlockMem[8];
		ULONG blk_v3 = BlockMem[10];
		if(pow->onOff ==0)
		{
			BlockMem[22] = 0; // POWER_CTRL[power_en] = 0
			Sleep(100);
			PowerStat = BlockMem[32]; // POWER_STATUS
		}
		else
		{
			ULONG PowerStat0;
			PowerStat0 = BlockMem[32]; // POWER_STATUS
			ULONG PowerCtrl;
			PowerCtrl = BlockMem[22]; // POWER_CTRL
			PowerStat0 = PowerCtrl & 0x8000; // mask POWER_CTRL[power_en]
			if(!PowerStat0 || force)
			{
				ULONG PowerNum = 0;
				BlockMem[42] = 1; // SPD_DEVICE = 1
				for(int i = 0; i < 3; i++)
				{
					BlockMem[24] = 0; // POWER_ROM[num] = 0
					BlockMem[22] = 9; // POWER_CTRL[key] = 9
					BlockMem[24] = 0x100; // POWER_ROM[start] = 1
					Sleep(1);
					for(int j = 0; j < 50; j++)
					{
						Sleep(100);
						PowerStat = BlockMem[32]; // POWER_STATUS
						if(!(PowerStat & 0x10)) // POWER_RUN
							break;
					}
					if(PowerStat & 1) // POWER_OK
					{ 
						BlockMem[22] = 0x8000; // POWER_CTRL[power_en] = 1
						for(int j = 0; j < 50; j++)
						{
							Sleep(100);
							PowerStat = BlockMem[32]; // POWER_STATUS
							if(PowerStat & 0x20) // POWER_GOOD
								break;
						}
						if(PowerStat & 0x20) // POWER_GOOD
						{
							if(PowerStat & 0x100)
							{	//FMC0
								for(int j = 0; j < 30; j++)
								{
									Sleep(100);
									PowerStat = BlockMem[32]; // POWER_STATUS
									ULONG flag_on = (PowerStat & 0x100) << 1; // FMCx_PRESENT -> FMCx_POWER_ON
									if((PowerStat & 0x200) == flag_on)
										break;
								}
							}
							if(PowerStat & 0x400)
							{	//FMC1
								for(int j = 0; j < 30; j++)
								{
									Sleep(100);
									PowerStat = BlockMem[32]; // POWER_STATUS
									ULONG flag_on = (PowerStat & 0x400) << 1; // FMCx_PRESENT -> FMCx_POWER_ON
									if((PowerStat & 0x800) == flag_on)
										break;
								}
							}
							if(PowerStat & 0x1000)
							{	//FMC2
								for(int j = 0; j < 30; j++)
								{
									Sleep(100);
									PowerStat = BlockMem[32]; // POWER_STATUS
									ULONG flag_on = (PowerStat & 0x1000) << 1; // FMCx_PRESENT -> FMCx_POWER_ON
									if((PowerStat & 0x2000) == flag_on)
										break;
								}
							}
							//if(PowerStat & 0x1500)
							//{
							//	for(int j = 0; j < 30; j++)
							//	{
							//		Sleep(100);
							//		PowerStat = BlockMem[32]; // POWER_STATUS
							//		ULONG flag_on = (PowerStat & 0x1500) << 1; // FMCx_PRESENT -> FMCx_POWER_ON
							//		if((PowerStat & 0x2A00) == flag_on)
							//			break;
							//	}
							//}
						}
					}
					if((PowerStat & 2) ||		// POWER_ERROR
					   (!(PowerStat & 0x20))	// NO POWER_GOOD
					   )
					{
						BlockMem[22] = 0; // POWER_CTRL[key] = 0
						Sleep(1);
					}
					else
						break;
				}
			}
		}
		pow->value = blk_v0;
		pow->slot = PowerStat;
	}
	else
	{
		pow->value = 0;
		pow->slot = 0;
		Status = 1;
	}

	return Status;
}

//***************************************************************************************
ULONG AdmPldWorkAndCheck(PPLD_INFO pPldInfo)
{
	ULONG status = 0;
	ULONG value = 1; // переключить ПЛИС в рабочий режим
	status = WriteRegDataDir(0, 0, ADM2IFnr_DATA, value);
	status = WriteRegDataDir(0, 0, ADM2IFnr_DATA, value);

	status = ReadRegDataDir(0, 0, ADM2IFnr_STATUS, value);
//	status = WriteRegDataDir(0, 0, ADM2IFnr_STATUS, value);
//	status = WriteRegDataDir(0, 0, ADM2IFnr_STATUS, value);
	//Sleep(5);
	//Sleep(5);
	status = ReadRegData(0, 0, 0, value);

	status = ReadRegData(0, 0, MAINnr_SIG, value);
	if(INSYS_TAG != value)
	{
		pPldInfo->version = value;
		return INSYS_TAG_ERROR;
	}
	status = ReadRegData(0, 0, MAINnr_ADMVER, value);
	if(ADM_VERSION != value && FMC_VERSION != value)
	{
		pPldInfo->version = value;
		return ADM_VERSION_ERROR;
	}
	pPldInfo->submif = value;
	//status = ReadRegData(0, 0, ADM2IFnr_ID, value);
	//printf("Tetrad ID = 0x%X\n", value);
	//status = ReadRegData(0, 0, ADM2IFnr_IDMOD, value);
	//printf("Tetrad Modification = 0x%X\n", value);
	//status = ReadRegData(0, 0, ADM2IFnr_VER, value);
	//printf("Tetrad Version = 0x%X\n", value);
//	status = ReadRegData(0, 0, ADM2IFnr_TRES, value);
//	printf("Tetrad ID = %d\n", value);
	//status = ReadRegData(0, 0, ADM2IFnr_FSIZE, value);
	//printf("FIFO size = %d\n", value);
	//status = ReadRegData(0, 0, ADM2IFnr_FTYPE, value);
	//printf("FIFO type = %d\n", value);
//	status = ReadRegData(0, 0, ADM2IFnr_PATH, value);
//	printf("Tetrad ID = %d\n", value);
//	status = ReadRegData(0, 0, ADM2IFnr_IDNUM, value);
//	printf("Tetrad ID = %d\n", value);

	status = ReadRegData(0, 0, MAINnr_FPGAVER, value);
	pPldInfo->version = value;
	status = ReadRegData(0, 0, MAINnr_FPGAMOD, value);
	pPldInfo->modification = value;

//	status = ReadRegData(0, 0, MAINnr_TMASK, value);
//	printf("Tetrad ID = %d\n", value);
//	status = ReadRegData(0, 0, MAINnr_MRES, value);
//	printf("Tetrad ID = %d\n", value);

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

//	ULONG value = 1; // переключить ПЛИС в рабочий режим
//	status = WriteRegDataDir(0, 0, ADM2IFnr_DATA, value);

//	ULONG valueID = 0;
//	ULONG valueIDMOD = 0;
//	ULONG valueVER = 0;
	int iTetr = 0;
	for(iTetr = 0; iTetr < MAX_TETRNUM; iTetr++)
	{
		status = ReadRegData(0, iTetr, ADM2IFnr_ID, info[iTetr].id);
		status = ReadRegData(0, iTetr, ADM2IFnr_IDMOD, info[iTetr].mod);
		status = ReadRegData(0, iTetr, ADM2IFnr_VER, info[iTetr].ver);
		status = ReadRegData(0, iTetr, ADM2IFnr_TRES, info[iTetr].tres);
		status = ReadRegData(0, iTetr, ADM2IFnr_FSIZE, info[iTetr].fsize);
		status = ReadRegData(0, iTetr, ADM2IFnr_FTYPE, info[iTetr].ftype);
		status = ReadRegDataDir(0, iTetr, ADM2IFnr_STATUS, info[iTetr].status);
		//printf("Tetrad[%d] ID = 0x%04X, MOD = 0x%04X, VER = 0x%04X\n", iTetr, valueID, valueIDMOD, valueVER);
	}
}

//***************************************************************************************
void TetradName(ULONG id, TCHAR* str)
{
	switch(id)
	{
		case 0:    lstrcpy(str, _T(" - ")); break;
		case 0xFFFF: lstrcpy(str, _T(" - ")); break;
		case 1:    lstrcpy(str, _T("MAIN")); break;
		case 2:    lstrcpy(str, _T("BASE_DAC")); break;
		case 0x7F: lstrcpy(str, _T("BASE_DAC_SDRAM")); break;
		case 3:    lstrcpy(str, _T("PIO_STD")); break;
        case 0x47: lstrcpy(str, _T("SBSRAM_IN")); break;
        case 0x48: lstrcpy(str, _T("SBSRAM_OUT")); break;
        case 0x12: lstrcpy(str, _T("DIO64_OUT")); break;
        case 0x13: lstrcpy(str, _T("DIO64_IN")); break;
        case 0x14: lstrcpy(str, _T("ADM212x200M")); break;
        case 0x5D: lstrcpy(str, _T("ADM212x500M")); break;
        case 0x19: lstrcpy(str, _T("ADM414x65M")); break;
        case 0x07: lstrcpy(str, _T("ADMDDC4x16_DDC")); break;
        case 0x16: lstrcpy(str, _T("ADMDDC4x16_ADC")); break;
        case 0x20: lstrcpy(str, _T("ADMDDC4x16_FPGA")); break;
        case 0x53: lstrcpy(str, _T("ADMDDC416x100M")); break;
        case 0x41: lstrcpy(str, _T("DDS9956")); break;
        case 0x4F: lstrcpy(str, _T("TEST_CTRL")); break;
        case 0x3F: lstrcpy(str, _T("ADM214x200M")); break;
        case 0x40: lstrcpy(str, _T("ADM216x100")); break;
        case 0x2F: lstrcpy(str, _T("ADM28x1G")); break;
        case 0x69: lstrcpy(str, _T("ADM210x1G")); break;
        case 0x78: lstrcpy(str, _T("ADM212x1G")); break;
        case 0x2D: lstrcpy(str, _T("TEST128_OUT")); break;
        case 0x4C: lstrcpy(str, _T("TEST128_IN")); break;
        case 0x30: lstrcpy(str, _T("ADMDDC5016")); break;
        case 0x3D: lstrcpy(str, _T("ADC1624x192")); break;
        case 0x3E: lstrcpy(str, _T("DAC1624x192")); break;
        case 0x2E: lstrcpy(str, _T("ADMFOTR2G")); break;
        case 0x49: lstrcpy(str, _T("ADMFOTR3G")); break;
        case 0x64: lstrcpy(str, _T("SYNC8CH")); break;
        case 0x68: lstrcpy(str, _T("DDS9912")); break;
        case 0x6F: lstrcpy(str, _T("SDRAM_PEX2")); break;
        case 0x77: lstrcpy(str, _T("SDRAM_PEX2_OUT")); break;
        case 0x8F: lstrcpy(str, _T("SDRAM_106P")); break;
        case 0x70: lstrcpy(str, _T("SDRAM_PEX5")); break;
        case 0x76: lstrcpy(str, _T("ADMTEST2")); break;
        case 0x71: lstrcpy(str, _T("TRD_MSG")); break;
        case 0x72: lstrcpy(str, _T("TRD_TS201")); break;
        case 0x73: lstrcpy(str, _T("TRD_STREAM_IN")); break;
        case 0x74: lstrcpy(str, _T("TRD_STREAM_OUT")); break;
        case 0x7B: lstrcpy(str, _T("ADMQM9957")); break;
        case 0x7C: lstrcpy(str, _T("ADMSUBPLD")); break;
        case 0x82: lstrcpy(str, _T("FM814x125M")); break;
        case 0x88: lstrcpy(str, _T("FM214x250M")); break;
        case 0x89: lstrcpy(str, _T("FM412x500M")); break;
        case 0x8B: lstrcpy(str, _T("FM816x250M")); break;
        case 0x91: lstrcpy(str, _T("TRD_EMAC")); break;
        case 0x93: lstrcpy(str, _T("FM416x250M")); break;
        case 0x8C: lstrcpy(str, _T("TRD_FDSP")); break;
        case 0x8E: lstrcpy(str, _T("TRD_FDSP_OUT")); break;
        case 0x92: lstrcpy(str, _T("FM212x1G")); break;
        case 0x50: lstrcpy(str, _T("DAC216x400M")); break;
        case 0x94: lstrcpy(str, _T("DAC216x400M_CG")); break;
        case 0x95: lstrcpy(str, _T("ADC1624x128")); break;
        case 0x96: lstrcpy(str, _T("DAC1624x128")); break;
        case 0x9B: lstrcpy(str, _T("SDRAM_DDR3X")); break;
        case 0xA5: lstrcpy(str, _T("DDR3_TST")); break;
        case 0xB3: lstrcpy(str, _T("DDR3X_SDRAM_RD")); break;
        case 0x9C: lstrcpy(str, _T("FM216x250MDA_ADC")); break;
        case 0x9D: lstrcpy(str, _T("FM216x250MDA_DAC")); break;
        case 0xA7: lstrcpy(str, _T("ADMDAC216x400MT")); break;
        case 0xAA: lstrcpy(str, _T("FMTEST")); break;
        case 0xAC: lstrcpy(str, _T("FM814x250M")); break;
        case 0xAD: lstrcpy(str, _T("FM416x100M")); break;
        case 0xB2: lstrcpy(str, _T("FM416x1G5D")); break;
        case 0xBA: lstrcpy(str, _T("FM214x2G5D")); break;
        case 0xB6: lstrcpy(str, _T("ADM216x160")); break;
        case 0xB7: lstrcpy(str, _T("XM416x250M")); break;
        case 0xBE: lstrcpy(str, _T("ADC_RFDR4")); break;
        case 0xBF: lstrcpy(str, _T("DDC_RFDR4")); break;
        case 0x8A: lstrcpy(str, _T("FM401S")); break;
        case 0xA8: lstrcpy(str, _T("FM402S")); break;
        case 0xB5: lstrcpy(str, _T("BPI_FLASH")); break;
        case 0xA4: lstrcpy(str, _T("GTX_TST")); break;
        case 0x85: lstrcpy(str, _T("TRD_M25P128")); break;
        case 0xBC: lstrcpy(str, _T("PSD_CHECK")); break;
        case 0xBD: lstrcpy(str, _T("PSD_GEN")); break;
        case 0xCA: lstrcpy(str, _T("ADC_1620x1M")); break;
        case 0xCB: lstrcpy(str, _T("DAC_1620x1M")); break;
        case 0xCD: lstrcpy(str, _T("FM816x250M1")); break;
        case 0xB0: lstrcpy(str, _T("PE_CHN_CHECK")); break;
        case 0xC9: lstrcpy(str, _T("PRQ_IN")); break;
        case 0xCF: lstrcpy(str, _T("DDS9956_V2")); break;

		case 0xD0: lstrcpy(str, _T("DDC6010_LX130")); break;
        case 0xD1: lstrcpy(str, _T("DDC6010_SX315")); break;
        case 0xD2: lstrcpy(str, _T("FM412P")); break;
        case 0xD3: lstrcpy(str, _T("ADC_416x125M")); break;
        case 0xD4: lstrcpy(str, _T("DDC_416x125M")); break;
        case 0xD5: lstrcpy(str, _T("ADC_216x370MDA")); break;
        case 0xD6: lstrcpy(str, _T("CLK_216x370MDA")); break;
        case 0xD7: lstrcpy(str, _T("DDC_216x370MDA")); break;
        case 0xD8: lstrcpy(str, _T("DAC_216x370MDA")); break;
        case 0xD9: lstrcpy(str, _T("CLK_212x4GDA")); break;
        case 0xDA: lstrcpy(str, _T("FM416x500MD")); break;
        case 0xDB: lstrcpy(str, _T("ADC_212x4GDA")); break;
        case 0xDC: lstrcpy(str, _T("DAC_212x4GDA")); break;

		default: lstrcpy(str, _T("UNKNOW")); break;
	}
}

//***************************************************************************************
void BasemodName(ULONG id, TCHAR* str)
{
	switch(id)
	{
		case 0x4D58:    lstrcpy(str, _T("AMBPCX")); break;
		case 0x4D44:    lstrcpy(str, _T("AMBPCD")); break;
		case 0x1020:    lstrcpy(str, _T("ADS10X2G")); break;
		case 0x5502:    lstrcpy(str, _T("AMBPEX1")); break;
		case 0x5508:    lstrcpy(str, _T("AMBPEX2")); break;
		case 0x53A0:    lstrcpy(str, _T("SyncDAC")); break;
		case 0x53A1:    lstrcpy(str, _T("SyncBCO")); break;
		case 0x53A2:    lstrcpy(str, _T("Sync-cP6")); break;
		case 0x53B4:    lstrcpy(str, _T("RFDR4")); break;
		case 0x551B:    lstrcpy(str, _T("Sync8P")); break;

		case 0x5503:    lstrcpy(str, _T("AMBPEX8")); break;
		case 0x5504:    lstrcpy(str, _T("ADP201X1AMB")); break;
		case 0x5505:    lstrcpy(str, _T("ADP201X1DSP")); break;
		case 0x5507:    lstrcpy(str, _T("AMBPEX5")); break;
		case 0x5509:    lstrcpy(str, _T("FMC105P")); break;
		case 0x550A:    lstrcpy(str, _T("FMC106P")); break;
		case 0x5511:    lstrcpy(str, _T("FMC107P")); break;
		case 0x550B:    lstrcpy(str, _T("FMC103E2")); break;
		case 0x550C:    lstrcpy(str, _T("FMC114V")); break;
		case 0x550D:    lstrcpy(str, _T("FMC110P")); break;
		case 0x550E:    lstrcpy(str, _T("FMC113E")); break;
		case 0x550F:    lstrcpy(str, _T("FMC108V")); break;

		case 0x53B1:    lstrcpy(str, _T("FMC115cP")); break;
		case 0x53B2:    lstrcpy(str, _T("FMC112cP")); break;
		case 0x53B3:    lstrcpy(str, _T("FMC117cP")); break;
		case 0x53B5:    lstrcpy(str, _T("FMC121cP")); break;
		case 0x53B6:    lstrcpy(str, _T("FMC125cP")); break;
		case 0x53B7:    lstrcpy(str, _T("Sync-cP6R")); break;
		case 0x53B8:    lstrcpy(str, _T("Panorama")); break;

		case 0x5516:    lstrcpy(str, _T("XM216X250M")); break;
		case 0x551C:    lstrcpy(str, _T("FMC122P")); break;
		case 0x551D:    lstrcpy(str, _T("FMC123E")); break;
		case 0x551E:    lstrcpy(str, _T("FMC124P")); break;
		case 0x551F:    lstrcpy(str, _T("FMC127P")); break;
		case 0x5520:    lstrcpy(str, _T("FMC122P-SLAVE")); break;
		case 0x5521:    lstrcpy(str, _T("FMC111P")); break;

		case 0x5514:    lstrcpy(str, _T("AC_SYNC")); break;
		case 0x5518:    lstrcpy(str, _T("PS_DSP")); break;
		case 0x5519:    lstrcpy(str, _T("PS_ADC")); break;
		case 0x551A:    lstrcpy(str, _T("PS_SYNC")); break;

		default: lstrcpy(str, _T("UNKNOWN")); break;
	}
}

//***************************************************************************************
void SubmodName(ULONG id, TCHAR* str)
{
	switch(id)
	{
		case 0x0031:    lstrcpy(str, _T("ADM28x1G")); break;
		case 0x0100:    lstrcpy(str, _T("ADM210x200M")); break;
		case 0x0110:    lstrcpy(str, _T("ADM210x1G")); break;

		case 0x0210:    lstrcpy(str, _T("ADM212x10M")); break;
		case 0x0211:    lstrcpy(str, _T("ADM212x25M")); break;
		case 0x0212:    lstrcpy(str, _T("ADM212x40M")); break;
		case 0x0220:    lstrcpy(str, _T("ADM212x50M")); break;
		case 0x0221:    lstrcpy(str, _T("ADM212x60M")); break;

		case 0x0230:    lstrcpy(str, _T("ADM212x100M")); break;
		case 0x0240:    lstrcpy(str, _T("ADM1612x1M")); break;
		case 0x0250:    lstrcpy(str, _T("ADM212x200M")); break;
		case 0x0258:    lstrcpy(str, _T("ADM212x500M")); break;
		case 0x0259:    lstrcpy(str, _T("ADM212x500MWB")); break;
		case 0x0260:    lstrcpy(str, _T("ADM1612x65M")); break;

		case 0x0400:    lstrcpy(str, _T("ADM214x1M")); break;
		case 0x0401:    lstrcpy(str, _T("ADM214x3M")); break;
		case 0x0402:    lstrcpy(str, _T("ADM214x10M")); break;
		case 0x0410:    lstrcpy(str, _T("ADM214x10MX")); break;
		case 0x0420:    lstrcpy(str, _T("ADM214x60M")); break;
		case 0x0430:    lstrcpy(str, _T("ADM1614x160")); break;
		case 0x0440:    lstrcpy(str, _T("ADM214x100M")); break;
		case 0x0450:    lstrcpy(str, _T("ADM414x65M")); break;
		case 0x0460:    lstrcpy(str, _T("ADM214x200M")); break;
		case 0x0468:    lstrcpy(str, _T("ADM214x400M")); break;
		case 0x0469:    lstrcpy(str, _T("ADM214x400MWB")); break;

		case 0x0601:    lstrcpy(str, _T("ADM216x250")); break;
		case 0x0610:    lstrcpy(str, _T("ADM216x2M5")); break;
		case 0x0620:    lstrcpy(str, _T("ADM416x200")); break;
		case 0x0630:    lstrcpy(str, _T("ADM816x48")); break;
		case 0x0640:    lstrcpy(str, _T("ADM216x100M")); break;
		case 0x0800:    lstrcpy(str, _T("ADM818x800")); break;
		case 0x0801:    lstrcpy(str, _T("ADM1624x192")); break;
		case 0x0802:    lstrcpy(str, _T("ADM1624x128")); break;

		case 0x0A00:    lstrcpy(str, _T("ADMDDC2NB")); break;
		case 0x0A01:    lstrcpy(str, _T("ADMDDC2NBH")); break;
		case 0x0A02:    lstrcpy(str, _T("ADMDDC2NB_T(")); break;
		case 0x0A03:    lstrcpy(str, _T("ADMDDC4NB")); break;
		case 0x0A04:    lstrcpy(str, _T("ADMDDC2WB")); break;
		case 0x0A05:    lstrcpy(str, _T("ADMDDC2WB_T(")); break;
		case 0x0A06:    lstrcpy(str, _T("ADMDDC2WBH")); break;
		case 0x0A07:    lstrcpy(str, _T("ADMDDC8WB_T(")); break;
		case 0x0A08:    lstrcpy(str, _T("ADMDDC4x16")); break;
		case 0x0A09:    lstrcpy(str, _T("ADMDDC8WB")); break;
		case 0x0A0A:    lstrcpy(str, _T("ADMDDC2WBL16")); break;
		case 0x0A0B:    lstrcpy(str, _T("ADMDDC5016")); break;
		case 0x0A0C:    lstrcpy(str, _T("ADMDDC416x100M")); break;
		case 0x0A0D:    lstrcpy(str, _T("ADMDDC214x400M")); break;
		case 0x0A0E:    lstrcpy(str, _T("ADMDDC216x250M")); break;

		case 0x0B00:    lstrcpy(str, _T("ADMDDS9854")); break;
		case 0x0B01:    lstrcpy(str, _T("ADMDDS9852A")); break;
		case 0x0B10:    lstrcpy(str, _T("ADMQM9854")); break;
		case 0x0C00:    lstrcpy(str, _T("ADMDAC214x125M")); break;
		case 0x0C10:    lstrcpy(str, _T("ADMDAC214x160M")); break;
		case 0x0C20:    lstrcpy(str, _T("ADMDAC3224x192")); break;
		case 0x0C30:    lstrcpy(str, _T("ADMDAC216x400M")); break;
		case 0x0C31:    lstrcpy(str, _T("ADMDAC216x400MV2")); break;
		case 0x0C38:    lstrcpy(str, _T("ADMDAC216x400MT")); break;

		case 0x0D00:    lstrcpy(str, _T("ADME1T1")); break;
		case 0x0D10:    lstrcpy(str, _T("ADMFOTR")); break;
		case 0x0F00:    lstrcpy(str, _T("ADMDIO32")); break;
		case 0x0F10:    lstrcpy(str, _T("ADMDIOV")); break;
		case 0x0F11:    lstrcpy(str, _T("ADMDIO2")); break;
		case 0x0F12:    lstrcpy(str, _T("ADMDIOV6")); break;
		case 0x0F13:    lstrcpy(str, _T("ADMTEST2")); break;

		case 0x1010:    lstrcpy(str, _T("FM814x125M")); break;
		case 0x1012:    lstrcpy(str, _T("FM814x250M")); break;
		case 0x1020:    lstrcpy(str, _T("FM214x250M")); break;
		case 0x1030:    lstrcpy(str, _T("FM412x500M")); break;
		case 0x1040:    lstrcpy(str, _T("FM212x1G")); break;
		case 0x1050:    lstrcpy(str, _T("FM816x250M")); break;
		case 0x1051:    lstrcpy(str, _T("FM416x250M")); break;
		case 0x1052:    lstrcpy(str, _T("FM216x250MDA")); break;
		case 0x1053:    lstrcpy(str, _T("FM816x250M1")); break;
		case 0x1060:    lstrcpy(str, _T("FM416x100M")); break;
		case 0x1070:    lstrcpy(str, _T("FM416x1G5D")); break;
		case 0x1078:    lstrcpy(str, _T("FM214x2G5D")); break;
		case 0x1090:    lstrcpy(str, _T("FM1620x1M")); break;
		case 0x10A0:    lstrcpy(str, _T("FM216x370MDA")); break;
		case 0x1F10:    lstrcpy(str, _T("FM416x125M")); break;
			
		case 0x1F00:    lstrcpy(str, _T("FM404V")); break;
		case 0x1F01:    lstrcpy(str, _T("FM404V-TEST")); break;
		case 0x1F08:    lstrcpy(str, _T("FM405P")); break;

		default: lstrcpy(str, _T("UNKNOW")); break;
	}
}

//***************************************************************************************
void GetPldCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAdmPld pCfgPld)
{
	PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
	int end_flag = 0;
	do
	{
		USHORT sign = *((USHORT*)pCfgMem);
		USHORT size = 0;
		if(sign == PLD_CFG_TAG)
		{
			PICR_CfgAdmPld pPldCfg = (PICR_CfgAdmPld)pCfgMem;
			memcpy(pCfgPld, pPldCfg, sizeof(ICR_CfgAdmPld));
			size += sizeof(ICR_CfgAdmPld);
			end_flag = 1;
		}
		else
		{
			size = *((USHORT*)pCfgMem + 1);
			size += 4;
//			break;
		}
		pCfgMem = (PUCHAR)pCfgMem + size;
	} while(!end_flag && pCfgMem < pEndCfgMem);
}

UCHAR g_pldType = 0;

//***************************************************************************************
ULONG GetPldDescription(TCHAR* Description, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
{
	ICR_CfgAdmPld cfgPld;
	cfgPld.bType = 0;
	cfgPld.wVolume = 1;
	cfgPld.wPins = 2;
	cfgPld.bSpeedGrade = 3;
	//MessageBox(NULL, _T("1", _T("ISDCLASS", MB_OK); 
	GetPldCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &cfgPld);
	//MessageBox(NULL, _T("2", _T("ISDCLASS", MB_OK); 
	//BRDCHAR buf[256];
	//_stprintf_s(Description, MAX_STRING_LEN, _BRDC("%d %d %d"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
	g_pldType = cfgPld.bType;
	_stprintf_s(Description, MAX_STRING_LEN, _T("Chip: %d %d %d %d"), cfgPld.bType, cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
	switch(cfgPld.bType)
	{
	case 3:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XCVE%d-%dFF-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
		break;
	case 4:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC2V%d-%dFF-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
		break;
	case 5:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC2S%d-FG%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
	case 6:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC2S%dE-FG%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
		break;
	case 7:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC3S%d-FG%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
		break;
	case 8:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC4VLX%d-%dFF%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
		break;
	case 9:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC4VSX%d-%dFF%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
		break;
	case 10:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC4VFX%d-%dFF%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
		break;
	case 11:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC3S%dE-FG%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
		break;
	case 12:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC3S%dA-FG%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
		break;
	case 15:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC5VLX%d-%dFFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
		break;
	case 16:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC5VSX%d-%dFFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
		break;
	case 17:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC5VFX%d-%dFFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
		break;
	case 18:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC5VLX%dT-%dFFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
		break;
	case 19:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC5VSX%dT-%dFFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
		break;
	case 20:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC5VFX%dT-%dFFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
		break;
	case 21:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC6VSX%dT-%dFFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
		break;
	case 22:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC6VLX%dT-%dFFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
		break;
	case 23:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC7K%dT-%dFFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
		break;
	case 24:
		_stprintf_s(Description, MAX_STRING_LEN, _BRDC("Chip: XC7A%dT-%dFFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
		break;
	}
	return 1;
}

// tetrNum : номер тетрады 0 - MAIN, 6 - DIO_IN
// width : 0 - 64-битная шина данных, 1 - 128-битная шина данных
//***************************************************************************************
ULONG DmaChannelTest(int tetrNum, int width)
{	
	ULONG status = 0;
	int dmaChan = 0; // номер канала DMA

	int chan_state;
	ULONG blockNum;
	status = StateDma( 0, dmaChan, chan_state, blockNum);
	if(status)
	{
		printf("StateDma: ERROR !!!\n");
		return status;
	}
	if(3 != chan_state)
	{
		printf("StateDma: DMA Channel %d is busy already!!!\n", dmaChan);
		dmaChan = 1; // номер канала DMA

		status = StateDma( 0, dmaChan, chan_state, blockNum);
		if(status)
		{
			printf("StateDma: ERROR !!!\n");
			return status;
		}
		if(3 != chan_state)
		{
			MessageBox(NULL, _BRDC("All DMA Channels are busy already!!!"), _BRDC("ISDCLASS"), MB_OK); 
			//printf("StateDma: DMA Channel %d is busy already!!!\n", dmaChan);
			return status;
		}
	}
	//printf("Buffer size is %d kBytes (%d block * %d kBytes)\n", (BlockSize / 1024) * BlockNum, BlockNum, BlockSize / 1024);

//	HANDLE hThisThread = GetCurrentThread();
//	SetThreadAffinityMask(hThisThread, 0x1);

	ULONG regVal = 1;
	if(tetrNum == 6) // DIO_IN
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

	ULONG dir = BRDstrm_DIR_IN; // ввод // BRDstrm_DIR_OUT; // вывод
//	ULONG blkSize = 4096*16;// * 1024; // размер каждого блока
//	ULONG blkSize = 1024 * 1024 * 8; // размер каждого блока
	//ULONG blkSize = 65536;
	//ULONG blkSize = 1024 * 1024;
	ULONG blkSize = 4096 * 1024;
	ULONG blkNum = 1; // число блоков
	ULONG isSysMem = 1; // системная память = 1, пользовательская память = 0
	ULONG addr = tetrNum; // тетрада 0 (младшие 16 бит), ADM-интерфес 0 (старшие 16 бит)
	PVOID pBuf = NULL; // указатель на массив указателей на блоки (возвращается функцией AllocMemory)

	printf("Allocating Memory...!!!\r");
	status = AllocMemory(&pBuf, blkSize, blkNum, isSysMem, dir, addr, dmaChan);
	if(status)
		//printf("AllocMemory: ERROR !!!\n");
		MessageBox(NULL, _BRDC("Allocate memory is ERROR!!!"), _BRDC("ISDCLASS"), MB_OK); 
	else
	{
		printf("Memory is Allocated!!!  (%d block)\n", blkNum);
		PULONG* pBufferZ = (PULONG*)pBuf;

//		if(extdma_sup)
//			SetIrqMode(1, dmaChan);

		// заполняю выделенные буфера нулями
		//printf("Zeroing Memory...!!!\r"));
		for(ULONG iBlock = 0; iBlock < blkNum; iBlock++)
		{
			//for(ULONG i = 0; i < blkSize/4; i++)
			for(ULONG i = 0; i < 32; i++)
				pBufferZ[iBlock][i] = i;
		}

		if(tetrNum == 6) // DIO_IN
		{
			regVal = 0x1;
			status = WriteRegData(0, 1, 0x1e, regVal); // TEST_CTLR:GEN_CTRL - сброс узла формирования тестовой последовательности
			regVal = 0x680;
			status = WriteRegData(0, 1, 0x1e, regVal); // TEST_CTLR:GEN_CTRL - узел формирует фиксированный блок (6 - 64-разрядный счетчик с инверсией)
		}
		regVal = 2; // FIFO_RESET
		status = WriteRegData(0, tetrNum, 0, regVal); // MODE0 (DIO64_IN | MAIN tetrad)
		Sleep( 1 );
		regVal = 0;
		status = WriteRegData(0, tetrNum, 0, regVal); // MODE0 (DIO64_IN | MAIN tetrad)
		Sleep( 1 );
		status = ResetDmaFifo(dmaChan);
		status = StartDma(0, dmaChan); // без зацикливания
		//if(status && (status != ERROR_IO_PENDING))
		//	printf("StartDma: ERROR !!!\n"));
		//printf("StartDma: Success !!!\n"));
		//regVal = 0x2010; // HF, MASTER
		//status = WriteRegData(0, tetrNum, 0, regVal); // MODE0 (DIO64_IN | MAIN tetrad)
		regVal = 0x2038; // HF, START, MASTER, DRQ_EN
		status = WriteRegData(0, tetrNum, 0, regVal); // MODE0 (DIO64_IN | MAIN tetrad)
		if(tetrNum == 6) // DIO_IN
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
		//		Sleep(50);
		//	}
		//}
		//else		
			status = WaitBuffer(10000, dmaChan); // time-out 10 сек (если 0xFFFFFFFF - бесконечное ожидание)
		if(status == TIMEOUT_ERROR) 
		{
			printf("TIMEOUT ERROR !!!\n");
		}
		//printf("WaitBuffer: Success !!!\n"));
		status = StopDma(dmaChan);
		regVal = 0;
		status = WriteRegData(0, tetrNum, 0, regVal); // MODE0 (DIO64_IN | MAIN tetrad)

//		if(extdma_sup)
//			SetIrqMode(0, dmaChan);

		// контроль тестовой последовательности
		__int64 cnt_err = 0;
		__int64** pBuffer = (__int64**)pBuf;
		__int64 data_rd;
		if(tetrNum == 6) // DIO_IN
		{
			__int64 data_wr = 0x00000000A5A50123;
			for(ULONG iBlock = 0; iBlock < blkNum; iBlock++)
			{
				data_rd = pBuffer[iBlock][0];
				if(data_wr != data_rd)
				{
					cnt_err++;
					if(cnt_err < 10)
						_tprintf(_BRDC("Error (%d): wr %016I64X : rd %016I64X\n"),
											iBlock, data_wr, data_rd);
				}
				data_wr = ((__int64)iBlock+1) << 40 | 0xA5A50123;
			}
		}
		else
		{
			__int64 data_wr = 2;
			__int64 data_wrh = 0xAA55;
			for(ULONG iBlock = 0; iBlock < blkNum; iBlock++)
			{
				for(ULONG i = 0; i < blkSize/8; i++)
				{
					data_rd = pBuffer[iBlock][i];
					if(data_wr != data_rd)
					{
						cnt_err++;
						if(cnt_err < 10)
							_tprintf(_BRDC("Error (%d, %d): wr %016I64X: rd %016I64X\n"),
												i, iBlock, data_wr, data_rd);
						data_wr = data_rd;
					}
					ULONG data_h = ULONG(data_wr>>32);
					ULONG f63 = data_h >> 31;
					ULONG f62 = data_h >> 30;
					ULONG f60 = data_h >> 28;
					ULONG f59 = data_h >> 27;
					ULONG f0 = (f63 ^ f62 ^ f60 ^ f59)&1;

					data_wr <<= 1;
					data_wr &= ~1;
					data_wr |=f0;
					
					if(width)
					{
						data_rd = pBuffer[iBlock][i+1];
						if(data_wrh != data_rd)
						{
							cnt_err++;
							if(cnt_err < 10)
								_tprintf(_BRDC("Error (%d, %d): wr %016I64X: rd %016I64X\n"),
													i, iBlock, data_wrh, data_rd);
							data_wrh = data_rd;
						}
						ULONG data_h = ULONG(data_wrh>>32);
						ULONG f63 = data_h >> 31;
						ULONG f62 = data_h >> 30;
						ULONG f60 = data_h >> 28;
						ULONG f59 = data_h >> 27;
						ULONG f0 = (f63 ^ f62 ^ f60 ^ f59)&1;

						data_wrh <<= 1;
						data_wrh &= ~1;
						data_wrh |=f0;
						i++;
					}
				}
			}
		}
		if(cnt_err)
			MessageBox(NULL, _BRDC("MAIN tetrad test is ERROR!!!"), _BRDC("ISDCLASS"), MB_OK); 
		else
			MessageBox(NULL, _BRDC("MAIN tetrad test is SUCCESS!!!"), _BRDC("ISDCLASS"), MB_OK); 
		status = FreeMemory(dmaChan);
	}
	
	return status;
}

// SPD (Serial Presence-Detect) control register (direct - 0x204)
typedef union _SDRAM_SPDCTRL {
  ULONG AsWhole; // SPD control Register as a Whole Word
  struct { // SPD control Register as Bit Pattern
   ULONG Write	: 1,  // Read Memory Mode : 0 - auto, 1 - random
	     Read	: 1,  // Random read memory enable
		 PgMode	: 1,  // 1 - page mode flag
		 Res	: 5,  // Reserved
		 Slot	: 3,  // 000 - slot 0, 001 - slot 1
		 Res1	: 5; // Reserved
  } ByBits;
} SDRAM_SPDCTRL, *PSDRAM_SPDCTRL;

#define SDRAM_MAXSLOTS 2

// Fundamental memory type (into SPD, byte offset = 2)
typedef enum _SDRAM_MEMTYPE {
	SDRAMmt_FPM =	0x01,
	SDRAMmt_EDO =	0x02,
	SDRAMmt_SDR =	0x04,
	SDRAMmt_DDR =	0x07,
	SDRAMmt_DDR2 =	0x08,
	SDRAMmt_DDR3 =	0x0B,
} SDRAM_MEMTYPE;

// Numbers of Command Registers
typedef enum _SDRAM_NUM_SPD_BYTES {
	SDRAMspd_MEMTYPE	= 2,	// Memory Type: 01 - Fast Page Mode, 02 - EDO, 04 - SDRAM, 07 - DDR SDRAM, 08 - DDR2 SDRAM
	SDRAMspd_ROWADDR	= 3,	// Number of row addresses (11-14)
	SDRAMspd_COLADDR	= 4,	// Number of column addresses (8-13)
	SDRAMspd_MODBANKS	= 5,	// Number of banks on module
	SDRAMspd_CYCLETIME	= 9,	// SDRAM Cycle time at Maximum Supported CAS Latency
	SDRAMspd_WIDTH		= 13,	// Primary DDR SDRAM Width
	SDRAMspd_CHIPBANKS	= 17,	// Number of banks in each DDR SDRAM device
	SDRAMspd_CASLAT		= 18,	// CAS# Latencies Supported
	SDRAMspd_ATTRIB		= 21,	// Various module attributes
} SDRAM_NUM_SPD_BYTES;

// Numbers of Command Registers
typedef enum _DDR3_NUM_SPD_BYTES {
	DDR3spd_MEMTYPE		= 2,	// DRAM Device Type: 0B - DDR3 SDRAM
	DDR3spd_CHIPBANKS	= 4,	// SDRAM Density and Banks: Bits 6~4 - Bank Address, Bits 3~0 - Total SDRAM capacity, in megabits
	DDR3spd_ROWCOLADDR	= 5,	// SDRAM Addressing: Bits 5~3 - Row Address (12-16), Bits 2~0 - Column Address (9-12)
	DDR3spd_MODBANKS	= 7,	// Module Organization: Bits 5~3 - Number of Ranks, Bits 2~0 - SDRAM Device width
	DDR3spd_WIDTH		= 8,	// Module Memory Bus Width: Bits 2~0 - Primary bus width
	DDR3spd_MTBDIVIDEND	= 10,	// Medium Timebase (MTB) Dividend
	DDR3spd_MTBDIVIDER	= 11,	// Medium Timebase (MTB) Divisor
	DDR3spd_CYCLETIME	= 12,	// SDRAM Minimum Cycle Time (tCKmin)
	DDR3spd_CASLATL		= 14,	// CAS Latencies Supported, Least Significant Byte
	DDR3spd_CASLATH		= 15,	// CAS Latencies Supported, Most Significant Byte
	DDR3spd_CASLATMIN	= 16,	// Minimum CAS Latency Time
} DDR3_NUM_SPD_BYTES;

//***************************************************************************************
long GetMemTetrNum(ULONG& valueID)
{
	ULONG status = 0;

	valueID = 0;
	int iTetr = 0;
	for(iTetr = 0; iTetr < MAX_TETRNUM; iTetr++)
	{
		status = ReadRegData(0, iTetr, ADM2IFnr_ID, valueID);
		if((valueID == 0x70) || (valueID == 0x6F) || (valueID == 0x77) || (valueID == 0x8F) || (valueID == 0x9B) || (valueID == 0xA5))
			break;
	}
	return iTetr;
}

//***************************************************************************************
UCHAR ReadSpdByte(long TetrNum, ULONG OffsetSPD, ULONG CtrlSPD)
{
	ULONG status = 0;
	status = WriteRegData(0, TetrNum, 0x205, OffsetSPD); //SDRAMnr_SPDADDR;
	status = WriteRegData(0, TetrNum, 0x204, CtrlSPD);//SDRAMnr_SPDCTRL;
	ULONG val = 0;
	status = ReadRegData(0, TetrNum, 0x206, val);//SDRAMnr_SPDDATAL;
	return (UCHAR)val;
}

//***************************************************************************************
void MemoryManufacturer(unsigned __int64 id, TCHAR* str)
{
	switch(id)
	{
		case 0x000000000000002C:    lstrcpy(str, _T("Micron")); break;
		case 0x00000000000000AD:    lstrcpy(str, _T("Hynix")); break;
		case 0x00000000000000C1:    lstrcpy(str, _T("Infineon")); break;
		case 0x00000000000000CE:    lstrcpy(str, _T("Samsung")); break;
		case 0x000000000000987F:    lstrcpy(str, _T("Kingston")); break;
		case 0x0000517F7F7F7F7F:    lstrcpy(str, _T("Qimonda")); break;
		case 0x00000000009E7F7F:    lstrcpy(str, _T("Corsair")); break;
		case 0x000000B07F7F7F7F:    lstrcpy(str, _T("OCZ")); break;
		case 0x000000000B7F7F7F:    lstrcpy(str, _T("Nanya")); break;
		case 0x000000000000000B:    lstrcpy(str, _T("Nanya")); break;
		case 0x0000000000004F7F:    lstrcpy(str, _T("Transcend")); break;

		case 0x0000000000000098:    lstrcpy(str, _T("Kingston")); break;
		case 0x000000000000004F:    lstrcpy(str, _T("Transcend")); break;
		case 0x0000000000000051:    lstrcpy(str, _T("Qimonda")); break;
		default: lstrcpy(str, _T("InSys")); break;
	}
}

//***************************************************************************************
ULONG GetMemorySize(TCHAR* strMemType)
{
	ULONG memTetrID = 0;
	long MemTetrNum = GetMemTetrNum(memTetrID);
	if(MAX_TETRNUM == MemTetrNum)
		return 0;
	
	SDRAM_SPDCTRL SpdCtrl;
	SpdCtrl.AsWhole = 0;
	SpdCtrl.ByBits.Read = 1;

	UCHAR mem_type[SDRAM_MAXSLOTS];
	SpdCtrl.ByBits.Slot = 0;
	mem_type[0] = ReadSpdByte(MemTetrNum, SDRAMspd_MEMTYPE, SpdCtrl.AsWhole);
	if(memTetrID != 0x70 && memTetrID != 0x9B && memTetrID != 0xA5)
	{
		SpdCtrl.ByBits.Slot = 1;
		mem_type[1] = ReadSpdByte(MemTetrNum, SDRAMspd_MEMTYPE, SpdCtrl.AsWhole);
	}
	else
	{
		UCHAR val = ReadSpdByte(MemTetrNum, DDR3spd_ROWCOLADDR, SpdCtrl.AsWhole);
		if(mem_type[0] == 0 && val != 0)
			mem_type[0] = SDRAMmt_DDR3;
		mem_type[1] = 0;
	}
	int ModuleCnt = 0;
	for(int i = 0; i < SDRAM_MAXSLOTS; i++)
		if(mem_type[i] == SDRAMmt_DDR || mem_type[i] == SDRAMmt_DDR2 || mem_type[i] == SDRAMmt_DDR3)
			ModuleCnt++;
	if(!ModuleCnt)
	    return 0;

	ULONG Status = 0;
	UCHAR val;
	UCHAR row_addr[SDRAM_MAXSLOTS], col_addr[SDRAM_MAXSLOTS], module_banks[SDRAM_MAXSLOTS], module_width[SDRAM_MAXSLOTS],
		  chip_width[SDRAM_MAXSLOTS], chip_banks[SDRAM_MAXSLOTS], cycle_time[SDRAM_MAXSLOTS], capacity[SDRAM_MAXSLOTS];
	UCHAR mtb_dividend[SDRAM_MAXSLOTS], mtb_divider[SDRAM_MAXSLOTS];
	unsigned __int64 mem_name[SDRAM_MAXSLOTS] = {0, 0};
	USHORT cas_lat[SDRAM_MAXSLOTS];
	for(int i = 0; i < ModuleCnt; i++)
	{
		SpdCtrl.ByBits.Slot = i;
		if(mem_type[i] == SDRAMmt_DDR3)
		{
			val = ReadSpdByte(MemTetrNum, DDR3spd_ROWCOLADDR, SpdCtrl.AsWhole);
			row_addr[i] = (val >> 3) & 0x7;
			col_addr[i] = val & 0x7;
			val = ReadSpdByte(MemTetrNum, DDR3spd_MODBANKS, SpdCtrl.AsWhole);
			module_banks[i] = (val >> 3) & 0x7;
			chip_width[i] = val & 0x7;
			val = ReadSpdByte(MemTetrNum, DDR3spd_CHIPBANKS, SpdCtrl.AsWhole);
			chip_banks[i] = (val >> 4) & 0x7;
			capacity[i] = val & 0xF;
			val = ReadSpdByte(MemTetrNum, DDR3spd_WIDTH, SpdCtrl.AsWhole);
			module_width[i] = val & 0x7;
			mtb_dividend[i] = ReadSpdByte(MemTetrNum, DDR3spd_MTBDIVIDEND, SpdCtrl.AsWhole);
			mtb_divider[i] = ReadSpdByte(MemTetrNum, DDR3spd_MTBDIVIDER, SpdCtrl.AsWhole);
			cas_lat[i] = ReadSpdByte(MemTetrNum, DDR3spd_CASLATL, SpdCtrl.AsWhole);
			val = ReadSpdByte(MemTetrNum, DDR3spd_CASLATH, SpdCtrl.AsWhole);
			cas_lat[i] |= (val << 8);
			cycle_time[i] = ReadSpdByte(MemTetrNum, DDR3spd_CYCLETIME, SpdCtrl.AsWhole);
			mem_name[i] = ReadSpdByte(MemTetrNum, 118, SpdCtrl.AsWhole);
			//for(int j = 0; j < 8; j++)
			//{
			//	val = ReadSpdByte(MemTetrNum, 117+j, SpdCtrl.AsWhole);
			//	mem_name[i] |= val << (8*j);
			//}
		}
		else
		{
			row_addr[i] = ReadSpdByte(MemTetrNum, SDRAMspd_ROWADDR, SpdCtrl.AsWhole);
			col_addr[i] = ReadSpdByte(MemTetrNum, SDRAMspd_COLADDR, SpdCtrl.AsWhole);
			module_banks[i] = ReadSpdByte(MemTetrNum, SDRAMspd_MODBANKS, SpdCtrl.AsWhole);
			if(mem_type[i] == SDRAMmt_DDR2)
				module_banks[i] = (module_banks[i] & 0x07) + 1;
			chip_width[i] = ReadSpdByte(MemTetrNum, SDRAMspd_WIDTH, SpdCtrl.AsWhole);
			chip_banks[i] = ReadSpdByte(MemTetrNum, SDRAMspd_CHIPBANKS, SpdCtrl.AsWhole);
			cycle_time[i] = ReadSpdByte(MemTetrNum, SDRAMspd_CYCLETIME, SpdCtrl.AsWhole);
			cas_lat[i] = ReadSpdByte(MemTetrNum, SDRAMspd_CASLAT, SpdCtrl.AsWhole);
			for(int j = 0; j < 8; j++)
			{
				unsigned __int64 val64 = (unsigned __int64)ReadSpdByte(MemTetrNum, 64+j, SpdCtrl.AsWhole);
				mem_name[i] |= val64 << (8*j);
				//if(val != 0x7F)
				//{
				//	mem_name[i] = val;
				//	break;
				//}
			}
			if(i && ((row_addr[i] != row_addr[i-1]) ||
					 (col_addr[i] != col_addr[i-1]) || 
					 (module_banks[i] != module_banks[i-1]) ||
					 (chip_width[i] != chip_width[i-1]) || 
					 (chip_banks[i] != chip_banks[i-1]) || 
					 (cas_lat[i] != cas_lat[i-1]) || 
					 (cycle_time[i] != cycle_time[i-1])
					))
			{
				Status = -1;
			}			
		}
	}
	if(-1 == Status)
		ModuleCnt--;

	TCHAR strMem[64];
	MemoryManufacturer(mem_name[0], strMem);
	if(mem_type[0] == SDRAMmt_DDR)
	{
		_stprintf_s(strMemType, MAX_STRING_LEN, _BRDC("%s  DDR "), strMem);
	}
	if(mem_type[0] == SDRAMmt_DDR2)
	{
		ULONG cycletns = (cycle_time[0] >> 4) & 0xf;
		ULONG cyclet = cycle_time[0] & 0xf;
		double cTime;
		switch(cyclet)
		{
		case 0xA:
			cTime = cycletns + 0.25;
			break;
		case 0xB:
			cTime = cycletns + 0.33;
			break;
		case 0xC:
			cTime = cycletns + 0.66;
			break;
		case 0xD:
			cTime = cycletns + 0.75;
		break;
		default:
			cTime = cycletns + double(cyclet)/10;
		}
		UCHAR mask = 0x80;
		int i = 7;
		for(; i > 0; i--)
		{
			if(cas_lat[0] & mask)
				break;
			mask >>= 1;
		}
		_stprintf_s(strMemType, MAX_STRING_LEN, _BRDC("%s  DDR2-%d  CL%d  "), strMem, ULONG((1000/cTime)*2+0.5), i);
	    //printf( "DDR2 memory type (%02X) - %d MHz\n", buf0[2], FreqMHz);
	}
	ULONG PhysMemSize =	(1 << row_addr[0]) *
						(1 << col_addr[0]) * 
						module_banks[0] * 
						chip_banks[0] *
						ModuleCnt * 2; // in 32-bit Words

	if(mem_type[0] == SDRAMmt_DDR3)
	{
		ULONG FreqMHz = ULONG(2*(1000 / (cycle_time[0] * mtb_dividend[0]/(double)mtb_divider[0]))+0.5);
		USHORT mask = 0x8000;
		int i = 15;
		for(; i > 0; i--)
		{
			if(cas_lat[0] & mask)
				break;
			mask >>= 1;
		}
		_stprintf_s(strMemType, MAX_STRING_LEN, _BRDC("%s  DDR3-%d  CL%d  "), strMem, FreqMHz, i+4);
		//printf( "DDR3 memory type (%02X) - %d MHz\n", buf0[2], FreqMHz);
		ULONG ModuleBanks = module_banks[0] + 1;
		ULONG PrimWidth = 8 << module_width[0];
		ULONG ChipWidth = 4 << chip_width[0];
		__int64 CapacityMbits = __int64(1 << capacity[0]) * 256 * 1024 * 1024;
		PhysMemSize = ((CapacityMbits >> 3) * (__int64)PrimWidth / ChipWidth * ModuleBanks *
						ModuleCnt) >> 2; // in 32-bit Words
	}

	return PhysMemSize;
}

//***************************************************************************************
ULONG isSysMon(ULONG& smon_stat)
{
	ULONG val = 0;
	ULONG status = ReadRegData(0, 0, 0x10D, val);// MAINnr_MRES
	if(val & 0x10)
	{
		status = ReadRegData(0, 0, 0x212, smon_stat);// SMON_STATUS
		return 1;
	}
	else
		return 0;
}

//***************************************************************************************
ULONG readSysMon(ULONG reg)
{
	ULONG status = 0;
	ULONG val = 0;
	do {
		status = ReadRegData(0, 0, 0x212, val);// SMON_STATUS
	} while(!(val&1));
	status = WriteRegData(0, 0, 0x210, reg); // SMON_ADDRESS
	status = ReadRegData(0, 0, 0x211, val);//SMON_DATA
//	if(g_pldType >= 15 && g_pldType <= 22) // заготовка на будущее (vertex7)
		val >>= 6;
//	else
//		val >>= 4;
	return val;
}

//***************************************************************************************
double getTemp(double& maxv, double& minv)
{
	U32 val = readSysMon(0);
	double curv = ((val * 503.975) / 1024 - 273.15);
	val = readSysMon(0x20);
	maxv = ((val * 503.975) / 1024 - 273.15);
	val = readSysMon(0x24);
	minv = ((val * 503.975) / 1024 - 273.15);
	return curv;
}

//***************************************************************************************
double getVccint(double& maxv, double& minv)
{
	U32 val = readSysMon(1);
	double curv = ((val * 3.) / 1024);
	val = readSysMon(0x21);
	maxv = ((val * 3.) / 1024);
	val = readSysMon(0x25);
	minv = ((val * 3.) / 1024);
	return curv;
}

//***************************************************************************************
double getVccaux(double& maxv, double& minv)
{
	U32 val = readSysMon(2);
	double curv = ((val * 3.) / 1024);
	val = readSysMon(0x22);
	maxv = ((val * 3.) / 1024);
	val = readSysMon(0x26);
	minv = ((val * 3.) / 1024);
	return curv;
}

//***************************************************************************************
void getVref(double& refp, double& refn)
{
	U32 val = readSysMon(4);
	refp = ((val * 3.) / 1024);
	val = readSysMon(5);
	if(val == 0x3FF)
		val = 0;
	refn = ((val * 3.) / 1024);
}

//***************************************************************************************
int ReadPldFile(PCTSTR PldFileName, PVOID& fileBuffer, ULONG& fileSize)
{
	BRDCHAR *FirstChar;
	BRDCHAR FullFileName[MAX_PATH];
	GetFullPathName(PldFileName, MAX_PATH, FullFileName, &FirstChar);
	HANDLE hFile = CreateFile(FullFileName,
								GENERIC_READ, 
								FILE_SHARE_READ, NULL, 
								OPEN_EXISTING, 
								FILE_ATTRIBUTE_NORMAL,
								NULL);
    if(hFile == INVALID_HANDLE_VALUE)
		return -1;
	fileSize = SetFilePointer(hFile, 0, NULL, FILE_END);
	if(fileSize == 0xffffffff)
		return -1;
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	fileBuffer = VirtualAlloc(NULL, fileSize, MEM_COMMIT, PAGE_READWRITE);
	if(fileBuffer == NULL)
		return -2;
	DWORD dwNumBytesRead;
	if(ReadFile(hFile, fileBuffer, fileSize, &dwNumBytesRead, NULL) == 0)
	{
		VirtualFree(fileBuffer, 0, MEM_RELEASE);
		return -3;
	}
	if(CloseHandle(hFile) == 0)
	{
		VirtualFree(fileBuffer, 0, MEM_RELEASE);
		return -4;
	}
	return 0;
}

ULONG BrdTest(int in_tetr, int out_tetr)
{
	ULONG status = 0;
	ULONG FifoStatus = 0;
	int dmaChan = 0; // номер канала DMA

	int chan_state0;
	int chan_state1;
	ULONG blockNum;
	status = StateDma( 0, dmaChan, chan_state0, blockNum);
	if(status)
	{
		printf("StateDma: ERROR !!!\n");
		return status;
	}
	dmaChan = 1;
	status = StateDma( 0, dmaChan, chan_state1, blockNum);
	if(status)
	{
		printf("StateDma: ERROR !!!\n");
		return status;
	}
	if((3 != chan_state0) || (3 != chan_state1))
	{
		MessageBox(NULL, _BRDC("DMA Channels are busy already!!!"), _BRDC("ISDCLASS"), MB_OK); 
		return status;
	}

	ULONG cnt_err = 0;

	ULONG dir = BRDstrm_DIR_OUT; // вывод
//	ULONG blkSize = 4096*16;// * 1024; // размер каждого блока
//	ULONG blkSize = 1024 * 1024 * 8; // размер каждого блока
	//ULONG blkSize = 65536;
	ULONG blkSize = 1024 * 1024;
	ULONG blkNum = 1; // число блоков
	ULONG isSysMem = 1; // системная память = 1, пользовательская память = 0
	ULONG addr = out_tetr; // тетрада 0 (младшие 16 бит), ADM-интерфес 0 (старшие 16 бит)
	PVOID pOutBuf = NULL; // указатель на массив указателей на блоки (возвращается функцией AllocMemory)

	printf("Allocating Memory for output...!!!\r");
	status = AllocMemory(&pOutBuf, blkSize, blkNum, isSysMem, dir, addr, 0); // канал 0
	if(status)
		//printf("AllocMemory: ERROR !!!\n");
		MessageBox(NULL, _BRDC("Allocate memory for output is ERROR!!!"), _BRDC("ISDCLASS"), MB_OK); 
	else
	{
		printf("Memory for output is Allocated!!!  (%d block)\n", blkNum);
	}

	// заполняю выходные буфера
	BRDC_printf(_BRDC("Counter test\n"));
	PULONG* pBufferZ = (PULONG*)pOutBuf;
	for(ULONG iBlock = 0; iBlock < blkNum; iBlock++)
		for(ULONG i = 0; i < blkSize / sizeof(ULONG); i++)
			pBufferZ[iBlock][i] = i;
	PULONG PTstBuf = pBufferZ[0];
	
	dir = BRDstrm_DIR_IN; // ввод
	blkSize = 1024 * 1024;
	blkNum = 1; // число блоков
	isSysMem = 1; // системная память = 1, пользовательская память = 0
	addr = in_tetr; // тетрада 0 (младшие 16 бит), ADM-интерфес 0 (старшие 16 бит)
	PVOID pInBuf = NULL; // указатель на массив указателей на блоки (возвращается функцией AllocMemory)

	printf("Allocating Memory for input...!!!\r");
	status = AllocMemory(&pInBuf, blkSize, blkNum, isSysMem, dir, addr, 1); // канал 1
	if(status)
		//printf("AllocMemory: ERROR !!!\n");
		MessageBox(NULL, _BRDC("Allocate memory for input is ERROR!!!"), _BRDC("ISDCLASS"), MB_OK); 
	else
	{
		printf("Memory for input is Allocated!!!  (%d block)\n", blkNum);
	}
	pBufferZ = (PULONG*)pInBuf;
	for(ULONG iBlock = 0; iBlock < blkNum; iBlock++)
		for(ULONG i = 0; i < blkSize / sizeof(ULONG); i++)
			pBufferZ[iBlock][i] = 0;
	PULONG PAdcBuf = pBufferZ[0];

	ULONG regVal = 0x80; // TEST_MODE
	status = WriteRegData(0, in_tetr, 9, regVal); // MODE1 (in_tetr)

//	regVal = 1; // TETRADA RESET
//	status = WriteRegData(0, out_tetr, 0, regVal); // MODE0 (out_tetr)

	regVal = 2; // FIFO_RESET
	status = WriteRegData(0, in_tetr, 0, regVal); // MODE0 (in_tetr)
	status = WriteRegData(0, out_tetr, 0, regVal); // MODE0 (out_tetr)
	Sleep( 1 );
	regVal = 0;
	status = WriteRegData(0, in_tetr, 0, regVal); // MODE0 (in_tetr)
	status = WriteRegData(0, out_tetr, 0, regVal); // MODE0 (out_tetr)
	Sleep( 1 );

	regVal = 0x2018; // HF, MASTER, DRQ_EN
	status = WriteRegData(0, in_tetr, 0, regVal); // MODE0 (in_tetr)
	regVal = 0x2018; // HF, MASTER, DRQ_EN
	status = WriteRegData(0, out_tetr, 0, regVal); // MODE0 (out_tetr)

	status = ReadRegDataDir(0, in_tetr, 0, FifoStatus);

	status = StartDma(0, 1); // без зацикливания, канал 1 - ввод
	regVal = 0x2038; // HF, START, MASTER, DRQ_EN
	status = WriteRegData(0, in_tetr, 0, regVal); // MODE0 (in_tetr)
	//regVal = 0x2010; // HF, MASTER
	//status = WriteRegData(0, tetrNum, 0, regVal); // MODE0 (DIO64_IN | MAIN tetrad)

	status = StartDma(1, 0); // с зацикливанием, канал 0 - вывод
	status = WriteRegData(0, out_tetr, 0, regVal); // MODE0 (out_tetr)

	// ожидаем окончания заполнения всего буфера ввода 
	status = WaitBuffer(10000, 1); // time-out 10 сек (если 0xFFFFFFFF - бесконечное ожидание)
	if(status == TIMEOUT_ERROR) 
	{
		status = ReadRegDataDir(0, in_tetr, 0, FifoStatus);
		printf("INPUT TIMEOUT ERROR: ADC FIFO Status = %X!!!\n", FifoStatus);
	}
	//printf("WaitBuffer: Success !!!\n"));
	status = StopDma(0); // останавливаем вывод
	regVal = 0; // input disable
	status = WriteRegData(0, in_tetr, 0, regVal); // MODE0 (out_tetr)

	status = StopDma(1); // останавливаем ввод

	for(U32 i = 0; i < blkSize / sizeof(ULONG); i++)
	{
		if(PAdcBuf[i] != PTstBuf[i])
			cnt_err++;
	}
	if(cnt_err)
		MessageBox(NULL, _BRDC("COUNTER test is ERROR!!!"), _BRDC("ISDCLASS"), MB_OK); 
	else
		MessageBox(NULL, _BRDC("COUNTER test is SUCCESS!!!"), _BRDC("ISDCLASS"), MB_OK); 

	status = FreeMemory(0);
	status = FreeMemory(1);

	return status;
}
