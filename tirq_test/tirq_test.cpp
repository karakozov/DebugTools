//********************************************************
//
// Пример приложения, 
//   получающего информацию о модуле, 
//   программируемых устройствах на нем (ПЛИС, ППЗУ...)
//   и BARDY-службах (стримы, АЦП, ЦАП...)
//
// (C) InSys, 2007-2016
//
//********************************************************

#include	"brd.h"
#include	"extn.h"
#include	"ctrlreg.h"

#include	"gipcy.h"

#define		MAX_NAME	32		// считаем, что имя устройства/ПЛИС/службы может быть не более 32 символов 
#define		MAX_SRV		16		// считаем, что служб на одном модуле может быть не больше MAX_SRV

#pragma pack(push,1)

// информация об устройстве
typedef struct
{
	U32			size;			// sizeof(DEV_INFO)
	U16			devID;			// Device ID
	U16			rev;			// Revision
	BRDCHAR		devName[MAX_NAME];	// Device Name
	U32			pid;			// Board Phisical ID
	S32			bus;			// Bus Number
	S32			dev;			// Dev Number
	S32			slot;			// Slot Number
	U08			pwrOn;			// FMC power on/off
	U32			pwrValue;		// FMC power value
	U16			pldVer;			// ADM PLD version
	U16			pldMod;			// ADM PLD modification
	U16			pldBuild;		// ADM PLD build
	BRDCHAR		pldName[MAX_NAME];	// ADM PLD Name
	U16			subType;		// Subunit Type Code
	U16			subVer;			// Subunit version
	U32			subPID;			// Subunit serial number
	BRDCHAR		subName[MAX_NAME];	// Submodule Name
	BRDCHAR		srvName[MAX_NAME][MAX_SRV];	// massive of Service Names
} DEV_INFO, *PDEV_INFO;

#pragma pack(pop)

BRDCHAR g_SrvName[MAX_NAME] = _BRDC("REG0"); // имя службы с номером 
														
// открыть устройство
BRD_Handle DEV_open(BRDCHAR * inifile, int idev, int* numdev);
// получить информацию об открытом устройстве
int DEV_info(BRD_Handle hDEV, int idev, DEV_INFO* pdevcfg);
// закрыть устройство
int DEV_close(BRD_Handle hDEV);
// открыть службу
BRD_Handle SRV_open(BRD_Handle hDEV, BRDCHAR* srvname, U32* mode);
// закрыть службу
int SRV_close(BRD_Handle hSrv);
// запись в косвенный регистр
S32 RegWriteInd(BRD_Handle handle, S32 tetrNum, S32 regNum, U32 valNum);
// чтение косвенного регистра
S32 RegReadInd(BRD_Handle handle, S32 tetrNum, S32 regNum, U32& valNum);
// чтение из прямого регистра
S32 RegReadDir(BRD_Handle handle, S32 tetrNum, S32 regNum, ULONG& valNum);

// получить и отобразить информацию о тетрадах
void tetr_info(BRD_Handle hReg);
// прерывания от бита завершения сбора тетрады SDRAM
// необходимо предварительно запустить программу, которая бы выполнила подготовку АЦП и памяти к сбору данных
void sdram_irq(BRD_Handle hReg);
// прерывания от бита START тетрады MAIN
void main_irq(BRD_Handle hReg);

// отобразить информацию об устройстве
void DisplayDeviceInfo(PDEV_INFO pDevInfo)
{
	if (pDevInfo->devID == 0x53B1 || pDevInfo->devID == 0x53B3) // FMC115cP or FMC117cP
	BRDC_printf(_BRDC("%s %x.%x (0x%x): Bus = %d, Dev = %d, G.adr = %d, Order = %d, PID = %d\n"),
		pDevInfo->devName, pDevInfo->rev >> 4, pDevInfo->rev & 0xf, pDevInfo->devID, pDevInfo->bus, pDevInfo->dev,
		pDevInfo->slot & 0xffff, pDevInfo->pid >> 28, pDevInfo->pid & 0xfffffff);
	else
	BRDC_printf(_BRDC("%s %x.%x (0x%x): Bus = %d, Dev = %d, Slot = %d, PID = %d\n"),
		pDevInfo->devName, pDevInfo->rev >> 4, pDevInfo->rev & 0xf, pDevInfo->devID, pDevInfo->bus, pDevInfo->dev, pDevInfo->slot, pDevInfo->pid);

	if (pDevInfo->pldVer != 0xFFFF)
	BRDC_printf(_BRDC("%s: Version %d.%d, Modification %d, Build 0x%X\n"),
		pDevInfo->pldName, pDevInfo->pldVer >> 8, pDevInfo->pldVer & 0xff, pDevInfo->pldMod, pDevInfo->pldBuild);

	if (pDevInfo->pwrOn && (pDevInfo->pwrOn != 0xFF))
	BRDC_printf(_BRDC("FMC Power: ON %.2f Volt\n"), pDevInfo->pwrValue / 100.);
	else
	BRDC_printf(_BRDC("FMC Power: OFF %.2f Volt\n"), pDevInfo->pwrValue / 100.);

	if (pDevInfo->subType != 0xFFFF)
	BRDC_printf(_BRDC("  Subm: %s %x.%x (%X), Serial Number = %d\n"),
		pDevInfo->subName, pDevInfo->subVer >> 4, pDevInfo->subVer & 0xf, pDevInfo->subType, pDevInfo->subPID);

	int j = 0;
	while (BRDC_strlen(pDevInfo->srvName[j]))
	{
		BRDC_printf(_BRDC("Service %d: %s\n"), j, pDevInfo->srvName[j]);
		j++;
	}
}

//=********************************* main ********************************************
//====================================================================================
int BRDC_main( int argc, BRDCHAR *argv[] )
{
	// чтобы все печати сразу выводились на экран
	fflush(stdout);
	setbuf(stdout, NULL);

	//====================================================================================
	// открыть устройство, описанное в указанном файле с указанным порядковым номером LID 
	// возвращает дескриптор устройства (также возвращает общее число устройств)
	int idev = 0;
	int num_dev = 0;
	BRD_Handle hDev = DEV_open(_BRDC("brd.ini"), idev, &num_dev);
	if(hDev == -1)
	{
		BRDC_printf(_BRDC("ERROR by BARDY Initialization\n"));
		BRDC_printf(_BRDC("Press any key for leaving program...\n"));
		return -1;
	}
	BRDC_printf(_BRDC("Number of devices = %d\n"), num_dev);

	//====================================================================================
	// получить информацию об открытом устройстве
	DEV_INFO dev_info;
	dev_info.size = sizeof(DEV_INFO);
	DEV_info(hDev, idev, &dev_info);

	// отобразить полученную информацию
	DisplayDeviceInfo(&dev_info);

	//====================================================================================
	// открыть службу
	U32 mode = BRDcapt_SHARED;
	BRD_Handle hReg = SRV_open(hDev, g_SrvName, &mode);
	if (hReg <= 0)
	{
		BRDC_printf(_BRDC("ERROR by REG service capture\n"));
		DEV_close(hDev); // закрыть устройство
		return -1;
	}
	else
	{
		if (mode != BRDcapt_SHARED)
		{
			BRDC_printf(_BRDC("Capture mode of REG service is NOT SHARED\n"));
		}
		else
			BRDC_printf(_BRDC("REG service capture mode is SHARED\n"));
	}

	tetr_info(hReg); // получить и отобразить информацию о тетрадах

	// прерывания от бита завершения сбора тетрады SDRAM
	// необходимо предварительно запустить программу, которая бы выполнила подготовку АЦП и памяти к сбору данных
	//sdram_irq(hReg);
		
	// прерывания от бита START тетрады MAIN
	main_irq(hReg);

	//====================================================================================
	// закрыть службу
	SRV_close(hReg);
	// закрыть устройство
	DEV_close(hDev);

	return 0;
}

//=************************* DEV_& SRV_ functions *************************

#define		MAX_DEV		12		// считаем, что модулей может быть не больше MAX_DEV
#define		MAX_PU		8		// считаем, что PU-устройств (ПЛИС, ППЗУ) на одном модуле может быть не больше MAX_PU

// открыть устройство
// inifile - (IN) файл инициализации c описанием нужного устройства
// iDev - (IN) порядковый номер LID в указанном файле c описанием нужного устройства
// pnumdev - (OUT) возвращает общее число устройств
// возвращает дескриптор устройства
BRD_Handle DEV_open(BRDCHAR* inifile, int iDev, S32* pnumdev)
{
	S32		status;

	BRD_displayMode(BRDdm_VISIBLE | BRDdm_CONSOLE); // режим вывода информационных сообщений : отображать все уровни на консоле

	//status = BRD_initEx(BRDinit_AUTOINIT, inifile, NULL, &DevNum); // инициализировать библиотеку - автоинициализация
	status = BRD_init(inifile, pnumdev); // инициализировать библиотеку
	if (!BRD_errcmp(status, BRDerr_OK))
		return -1;
	if(iDev >= *pnumdev)
		return -1;

	// получить список LID (каждая запись соответствует устройству)
	BRD_LidList lidList;
	lidList.item = MAX_DEV;
	lidList.pLID = new U32[MAX_DEV];
	status = BRD_lidList(lidList.pLID, lidList.item, &lidList.itemReal);

	BRD_Handle handle = BRD_open(lidList.pLID[iDev], BRDopen_SHARED, NULL); // открыть устройство в разделяемом режиме
//	if(handle > 0)
//	{
		// получить список служб
		//U32 ItemReal;
		//BRD_ServList srvList[MAX_SRV];
		//status = BRD_serviceList(handle, 0, srvList, MAX_SRV, &ItemReal);
		//if (ItemReal <= MAX_SRV)
		//{
		//	for (U32 j = 0; j < ItemReal; j++)
		//		BRDC_strcpy(g_srvName[j], srvList[j].name);
		//}
		//else
		//	BRDC_printf(_BRDC("BRD_serviceList: Real Items = %d (> 16 - ERROR!!!)\n"), ItemReal);
//	}
	delete lidList.pLID;
	return handle;
}

// вспомогательная функция для DEV_info
//=***************************************************************************************
void SubmodName(ULONG id, BRDCHAR * str)
{
	switch (id)
	{
	case 0x1010:    BRDC_strcpy(str, _BRDC("FM814x125M")); break;
	case 0x1012:    BRDC_strcpy(str, _BRDC("FM814x250M")); break;
	case 0x1020:    BRDC_strcpy(str, _BRDC("FM214x250M")); break;
	case 0x1030:    BRDC_strcpy(str, _BRDC("FM412x500M")); break;
	case 0x1040:    BRDC_strcpy(str, _BRDC("FM212x1G")); break;
	default: BRDC_strcpy(str, _BRDC("UNKNOW")); break;
	}
}

// получить информацию об открытом устройстве
// hDEV - (IN) дескриптор открытого устройства
// iDev - (IN) порядковый номер LID (в массиве лидов) c описанием нужного устройства
// pdevcfg - (OUT) заполняемая информацией об устройстве структура
// srv_name - (OUT) массив имен служб
//int DEV_info(BRD_Handle hDEV, int iDev, DEV_INFO* pdevcfg, BRDCHAR srv_name[][MAX_SRV])
int DEV_info(BRD_Handle hDEV, int iDev, DEV_INFO* pdevcfg)
{
	S32		status;

	BRD_Info	info;
	info.size = sizeof(info);

	// получить список LID (каждая запись соответствует устройству)
	BRD_LidList lidList;
	lidList.item = MAX_DEV;
	lidList.pLID = new U32[MAX_DEV];
	status = BRD_lidList(lidList.pLID, lidList.item, &lidList.itemReal);

	BRD_getInfo(lidList.pLID[iDev], &info); // получить информацию об устройстве
	pdevcfg->devID = info.boardType >> 16;
	pdevcfg->rev = info.boardType & 0xff;
	BRDC_strcpy(pdevcfg->devName, info.name);
	pdevcfg->pid = info.pid;
	pdevcfg->bus = info.bus;
	pdevcfg->dev = info.dev;
	pdevcfg->slot = info.slot;
	pdevcfg->subType = info.subunitType[0];

	BRDCHAR subName[MAX_NAME];
	BRDC_strcpy(pdevcfg->subName, _BRDC("NONE"));
	if(info.subunitType[0] != 0xffff)
	{
		SubmodName(info.subunitType[0], subName);
		BRDC_strcpy(pdevcfg->subName, subName);
	}

	BRDC_strcpy(pdevcfg->pldName, _BRDC("ADM PLD"));
	U32 ItemReal;
	BRD_PuList PuList[MAX_PU];
	status = BRD_puList(hDEV, PuList, MAX_PU, &ItemReal); // получить список программируемых устройств
	if (ItemReal <= MAX_PU)
	{
		for (U32 j = 0; j < ItemReal; j++)
		{
			U32	PldState;
			status = BRD_puState(hDEV, PuList[j].puId, &PldState); // получить состояние ПЛИС
			if (PuList[j].puId == 0x100 && PldState)
			{// если это ПЛИС ADM и она загружена
				BRDC_strcpy(pdevcfg->pldName, PuList[j].puDescription);
				pdevcfg->pldVer = 0xFFFF;
				BRDextn_PLDINFO pld_info;
				pld_info.pldId = 0x100;
				status = BRD_extension(hDEV, 0, BRDextn_GET_PLDINFO, &pld_info);
				if (BRD_errcmp(status, BRDerr_OK))
				{
					pdevcfg->pldVer = pld_info.version;
					pdevcfg->pldMod = pld_info.modification;
					pdevcfg->pldBuild = pld_info.build;
				}
			}
			if (PuList[j].puId == 0x03)
			{// если это ICR субмодуля
				if (PldState)
				{ // и оно прописано данными
					char subICR[14];
					status = BRD_puRead(hDEV, PuList[j].puId, 0, subICR, 14);
					U16 tagICR = *(U16*)(subICR);
					pdevcfg->subPID = *(U32*)(subICR + 7); // серийный номер субмодуля
					pdevcfg->subType = *(U16*)(subICR + 11);  // тип субмодуля
					pdevcfg->subVer = *(U08*)(subICR + 13);   // версия субмодуля
					SubmodName(pdevcfg->subType, subName);
					BRDC_strcpy(pdevcfg->subName, subName);
				}
			}
		}
	}
	delete lidList.pLID;

	// получаем состояние FMC-питания (если не FMC-модуль, то ошибка)
	pdevcfg->pwrOn = 0xFF;
	pdevcfg->pwrValue = 0;
	BRDextn_FMCPOWER power;
	power.slot = 0;
	status = BRD_extension(hDEV, 0, BRDextn_GET_FMCPOWER, &power);
	if (BRD_errcmp(status, BRDerr_OK))
	{
		pdevcfg->pwrOn = power.onOff;
		pdevcfg->pwrValue = power.value;
	}

	// получить список служб
	BRD_ServList srvList[MAX_SRV];
	status = BRD_serviceList(hDEV, 0, srvList, MAX_SRV, &ItemReal);
	if (ItemReal <= MAX_SRV)
	{
		U32 j = 0;
		for (j = 0; j < ItemReal; j++)
			BRDC_strcpy(pdevcfg->srvName[j], srvList[j].name);
		BRDC_strcpy(pdevcfg->srvName[j], _BRDC(""));
	}
	else
		BRDC_printf(_BRDC("BRD_serviceList: Real Items = %d (> 16 - ERROR!!!)\n"), ItemReal);

	return 0;
}

// закрыть устройство
int DEV_close(BRD_Handle hDEV)
{
	S32		status;
	status = BRD_close(hDEV); // закрыть устройство 
	status = BRD_cleanup();
	return 0;
}

// открыть АЦП и получить информацию о нем 
// hDEV - (IN) дескриптор открытого устройства
// adcsrv - (IN) имя службы АЦП
// adcfg - (OUT) заполняемая информацией об АЦП структура
// возвращает дескриптор службы АЦП
BRD_Handle SRV_open(BRD_Handle hDEV, BRDCHAR* srvname, U32* mode)
{
	BRD_Handle hSrv = -1;
	hSrv = BRD_capture(hDEV, 0, mode, srvname, 10000);
	return hSrv;
}
	
// закрыть службу
int SRV_close(BRD_Handle hSrv)
{
	S32		status;
	status = BRD_release(hSrv, 0);
	return status;
}

// получить и отобразить информацию о тетрадах
void tetr_info(BRD_Handle hReg)
{
	S32		status;
	BRD_Reg regdata;

	S32 iTetr = 0;
	ULONG Status = 0;
	for (iTetr = 0; iTetr < 16; iTetr++)
	{
		regdata.tetr = iTetr;
		regdata.reg = 0x100;	// получить идентитфикатор тетрады
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_READIND, &regdata);
		if (iTetr && regdata.val == 1)
			break;
		ULONG valueID = regdata.val;

		regdata.reg = 0x101;	// получить модификацию тетрады
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_READIND, &regdata);
		ULONG valueMOD = regdata.val;

		regdata.reg = 0x102;	// получить версию тетрады
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_READIND, &regdata);
		ULONG valueVER = regdata.val;

		if (!BRD_errcmp(status, BRDerr_OK))
			BRDC_printf(_BRDC("Error operation: Tetr = 0x%04X, Reg = 0x%04X\n"), iTetr, regdata.reg);
		else
		{
			status = RegReadDir(hReg, iTetr, 0, Status);
			BRDC_printf(_BRDC("%d: ID 0x%04X, mod %d, ver %d.%d, Status = 0x%04X\n"), iTetr, valueID, valueMOD, valueVER >> 8, valueVER & 0xff, Status);
		}
	}
}

// прерывания от бита завершения сбора тетрады SDRAM
// необходимо предварительно запустить программу, которая бы выполнила подготовку АЦП и памяти к сбору данных
void sdram_irq(BRD_Handle hReg)
{
	S32		status;
	BRD_Reg regdata;

	BRD_Irq irqdata;
	irqdata.tetr = 5;
	irqdata.irqMask = 0x2000;
	irqdata.irqInv = 0;
	irqdata.hEvent = NULL;
#ifdef _WIN32
	HANDLE hEvent = CreateEvent(NULL, TRUE, TRUE, _BRDC("irqevent_t5_b13")); // начальное состояние Signaled
	irqdata.hEvent = hEvent;
#endif
	status = BRD_ctrl(hReg, 0, BRDctrl_REG_SETSTATIRQ, &irqdata);

	int num = 100;
	for (int i = 0; i<num; i++)
	{
		IPC_delay(200);

		// Reset ADC FIFO
		regdata.tetr = 4; // ADC tetrada
		regdata.reg = 0; // MODE0
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_READIND, &regdata);
		regdata.val |= 2; // reset FIFO
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);
		regdata.val &= 0xfffc; // not reset FIFO
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);

		// Reset SDRAM FIFO
		regdata.tetr = 5; // SDRAM tetrada
		regdata.reg = 0; // MODE0
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_READIND, &regdata);
		regdata.val |= 2; // reset FIFO
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);
		regdata.val &= 0xfffc; // not reset FIFO
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);

		// Enable SDRAM IRQ
		regdata.val |= 4; // IRQ_EN
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);

		//regdata.tetr = 5;
		//regdata.reg = 0;
		//regdata.val |= 4; // IRQ_EN
		//status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);

		regdata.val |= 0x20; // start SDRAM
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);

		#ifdef _WIN32
		ResetEvent(hEvent); // сброс в состояние Non-Signaled
		#endif

		regdata.tetr = 4; // ADC tetrada
		regdata.reg = 0; // MODE0
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_READIND, &regdata);
		regdata.val |= 0x20; // start ADC
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);

		irqdata.irqInv = 5000; // timeout
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_WAITSTATIRQ, &irqdata);
		if (BRD_errcmp(status, BRDerr_OK))
			BRDC_printf(_BRDC("\rInterrupt %d is OK"), i);
		else
		{
			if (BRD_errcmp(status, BRDerr_WAIT_TIMEOUT))
				BRDC_printf(_BRDC("\nTimeout by waiting of interrupt\n"));
			else
				BRDC_printf(_BRDC("\nError by waiting of interrupt\n"));
			break;
		}
	}

	status = BRD_ctrl(hReg, 0, BRDctrl_REG_CLEARSTATIRQ, &irqdata);

#ifdef _WIN32
	CloseHandle(hEvent);
#endif
}

// прерывания от бита START тетрады MAIN
void main_irq(BRD_Handle hReg)
{
	S32		status;
	BRD_Reg regdata;
	regdata.tetr = 0; // MAIN tetrada
	regdata.reg = 0; // MODE0
	regdata.val = 0x11; // MASTER
	status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);
	regdata.val = 0x10; // MASTER
	status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);

	regdata.reg = 3; // FMODE
	regdata.val = 0; // internal clock
	status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);

	regdata.reg = 4; // FDIV
	regdata.val = 1; // clock divider
	status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);

	regdata.reg = 5; // STMODE
	regdata.val = 0; // program start
	status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);

	BRD_Irq irqdata;
	irqdata.tetr = 0;
	irqdata.irqMask = 0x800; // bit 11 - START
	irqdata.irqInv = 0;
	irqdata.hEvent = NULL;
#ifdef _WIN32
	HANDLE hEvent = CreateEvent(NULL, TRUE, TRUE, _BRDC("irqevent_t0_b11")); // начальное состояние Signaled
	irqdata.hEvent = hEvent;
#endif
	status = BRD_ctrl(hReg, 0, BRDctrl_REG_SETSTATIRQ, &irqdata);

	int num = 100;
	for (int i = 0; i<num; i++)
	{
		IPC_delay(500);

		// Enable IRQ 
		regdata.reg = 0; // MODE0
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_READIND, &regdata);
		regdata.val |= 4; // IRQ_EN
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);

#ifdef _WIN32
		ResetEvent(hEvent); // сброс в состояние Non-Signaled
#endif

		regdata.reg = 0; // MODE0
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_READIND, &regdata);
		regdata.val |= 0x20; // start on
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);

		irqdata.irqInv = 5000; // timeout
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_WAITSTATIRQ, &irqdata);
		if (BRD_errcmp(status, BRDerr_OK))
			BRDC_printf(_BRDC("\rInterrupt %d is OK"), i);
		else
		{
			if (BRD_errcmp(status, BRDerr_WAIT_TIMEOUT))
				BRDC_printf(_BRDC("\nTimeout by waiting of interrupt\n"));
			else
				BRDC_printf(_BRDC("\nError by waiting of interrupt\n"));
			break;
		}

		regdata.reg = 0; // MODE0
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_READIND, &regdata);
		regdata.val &= 0xffdf; // start off
		status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);

	}
	BRDC_printf(_BRDC("\n"));

	status = BRD_ctrl(hReg, 0, BRDctrl_REG_CLEARSTATIRQ, &irqdata.tetr);

#ifdef _WIN32
	CloseHandle(hEvent);
#endif
}

S32 RegWriteInd(BRD_Handle hReg, S32 tetrNum, S32 regNum, U32 valNum)
{
	S32 status;
	BRD_Reg regdata;
	regdata.reg = regNum;
	regdata.tetr = tetrNum;
	regdata.val = valNum;
	status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);
	return status;
}

S32 RegReadInd(BRD_Handle hReg, S32 tetrNum, S32 regNum, U32& valNum)
{
	S32 status;
	BRD_Reg regdata;
	regdata.reg = regNum;
	regdata.tetr = tetrNum;
	status = BRD_ctrl(hReg, 0, BRDctrl_REG_READIND, &regdata);
	valNum = regdata.val;
	return status;
}

S32 RegReadDir(BRD_Handle hReg, S32 tetrNum, S32 regNum, ULONG& valNum)
{
	S32 status;
	BRD_Reg regdata;
	regdata.reg = regNum;
	regdata.tetr = tetrNum;
	status = BRD_ctrl(hReg, 0, BRDctrl_REG_READDIR, &regdata);
	valNum = regdata.val;
	return status;
}

//
// End of file
//
