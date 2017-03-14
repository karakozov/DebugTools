#include "reg_test.h"

#include <QtWidgets/QMessageBox>
#include <QKeyEvent>

#include	"gipcy.h"
#include	"brd.h"
#include	"extn.h"
#include	"ctrlreg.h"
#include	"ctrlstrm.h"

#define MAX_DEV 8
#define MAX_TETRAD 16

BRD_LidList g_lidList;
static BRD_Handle g_hDevice = NULL;
static BRD_Handle g_hReg = NULL;

static U32 g_tetrID[MAX_TETRAD];

U32 g_fifoSizeBytes = 65536;
U32 g_dir = 0;

reg_test::reg_test(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	S32		status;
	BRD_displayMode(BRDdm_VISIBLE);
	int NumDev;
	status = BRD_init(_BRDC("brd.ini"), &NumDev); // инициализируем устройства
												  //	status = BRD_initEx(BRDinit_FILE | BRDinit_AUTOINIT, "brd.ini", NULL, &NumDev);
	if (!NumDev)
	{
		board_info->setText("<b><font color=red>Devices not found!</font></b>");
		return;
	}

	SetDeviceList();

}

reg_test::~reg_test()
{
	S32		status;
	delete g_lidList.pLID;
	status = BRD_release(g_hReg, 0);	// освобождаем службу
	status = BRD_close(g_hDevice);	// закрываем устройство
	status = BRD_cleanup();
}

void reg_test::updateDeviceInfo()
{
	S32		status;
	int iDev = cbBoardList->currentIndex();

	if (g_hReg)
		status = BRD_release(g_hReg, 0);	// освобождаем службу
	if (g_hDevice)
		status = BRD_close(g_hDevice);	// закрываем устройство

	BRD_Info	info;
	info.size = sizeof(info);
	BRD_getInfo(g_lidList.pLID[iDev], &info); // получить информацию об устройстве

	ULONG* host_vadr = NULL;
	memcpy(&host_vadr, &info.base[6], sizeof(void*));
	U16 pld_ver = host_vadr[4 * 2];
	U16 core_mod = host_vadr[6 * 2];
	U16 core_id = host_vadr[7 * 2];
	U16 blk_cnt = host_vadr[5 * 2] - 1;
	U16 dma_ver = 0;
	for (U16 iBlk = 1; iBlk < blk_cnt; iBlk++)
	{
		U16 val = host_vadr[iBlk * 64];
		if (0x1018 == val)
			{	dma_ver = host_vadr[1 * 2 + iBlk * 64]; break;	}
	}

	QString strBrdInfo;
#ifdef _WIN64
	if ((info.boardType >> 16) == 0x53B1) // FMC115cP
		strBrdInfo.sprintf("<font color=green><b>%ls</b> (0x%X, 0x%X): G.adr %d, Order %d, Host PLD %d.%d CoreID 0x%X CoreMOD 0x%0X DMA %d.%02d</font>",
			info.name, info.boardType >> 16, info.boardType & 0xff, info.slot & 0xffff, pld_ver >> 8, pld_ver & 0xff, core_id, core_mod, (dma_ver & 0xFFFF) >> 8, dma_ver & 0xff);
	else
		strBrdInfo.sprintf("<font color=green><b>%ls</b> (0x%X, 0x%X): Host PLD %d.%d, CoreID 0x%X, CoreMOD 0x%0X DMA %d.%02d</font>",
			info.name, info.boardType >> 16, info.boardType & 0xff, pld_ver >> 8, pld_ver & 0xff, core_id, core_mod, (dma_ver & 0xFFFF) >> 8, dma_ver & 0xff);
#else 
	if ((info.boardType >> 16) == 0x53B1) // FMC115cP
		strBrdInfo.sprintf("<font color=green><b>%s</b> (0x%X, 0x%X): G.adr %d, Order %d, Host PLD %d.%d CoreID 0x%X CoreMOD 0x%0X DMA %d.%02d</font>",
			info.name, info.boardType >> 16, info.boardType & 0xff, info.slot & 0xffff, pld_ver >> 8, pld_ver & 0xff, core_id, core_mod, (dma_ver & 0xFFFF) >> 8, dma_ver & 0xff);
	else
		strBrdInfo.sprintf("<font color=green><b>%s</b> (0x%X, 0x%X): Host PLD %d.%d CoreID 0x%X CoreMOD 0x%0X DMA %d.%02d</font>",
			info.name, info.boardType >> 16, info.boardType & 0xff, pld_ver >> 8, pld_ver & 0xff, core_id, core_mod, (dma_ver & 0xFFFF) >> 8, dma_ver & 0xff);
#endif
	board_info->setText(strBrdInfo);

	g_hDevice = BRD_open(g_lidList.pLID[iDev], BRDopen_SHARED, NULL); // открываем первое устройство

	BRDextn_PLDINFO pld_info;
	pld_info.pldId = 0x100;
	status = BRD_extension(g_hDevice, 0, BRDextn_GET_PLDINFO, &pld_info);
	QString strPldInfo;
	strPldInfo.sprintf("<font color=blue><b>PLD</b>: Version %d.%d, Modification %d, Build %d</font>",
		pld_info.version >> 8, pld_info.version & 0xff, pld_info.modification, pld_info.build);
	pld_txt->setText(strPldInfo);

	U32 ItemReal;
	status = BRD_serviceList(g_hDevice, 0, NULL, 0, &ItemReal);
	PBRD_ServList pSrvList = new BRD_ServList[ItemReal];
	status = BRD_serviceList(g_hDevice, 0, pSrvList, ItemReal, &ItemReal);
	//U32 mode = BRDcapt_EXCLUSIVE;
	U32 mode = BRDcapt_SHARED;
	g_hReg = 0;
	for (U32 iSrv = 0; iSrv < ItemReal; iSrv++)
	{
		if (!BRDC_strcmp(pSrvList[iSrv].name, _BRDC("REG0")))
		{
			g_hReg = BRD_capture(g_hDevice, 0, &mode, _BRDC("REG0"), 10000); // захватываем службу доступа к регистрам устройства
			
			//BRD_Host host;

			//for (U32 iBlk = 0; iBlk < 8; iBlk++)
			//{
			//	host.blk = iBlk;
			//	host.reg = 0;
			//	status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READHOST, &host);
			//	U32 ofBlk = iBlk * 64;
			//	U16 val = host_vadr[0 + ofBlk];
			//	val = host.val;
			//}

			//host.blk = 0;
			//host.reg = 2;
			//status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READHOST, &host);
			//U16 dev = host.val;
			//dev = host_vadr[host.reg * 2 + host.blk * 64];

			//host.reg = 5;
			//status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READHOST, &host);
			//U16 cnt = host.val;
			//cnt = host_vadr[host.reg * 2 + host.blk * 64];

			//host.reg = 6;
			//status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READHOST, &host);
			//U16 core_mod = host.val;
			//core_mod = host_vadr[host.reg * 2 + host.blk * 64];

			//host.reg = 7;
			//status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READHOST, &host);
			//U16 core_id = host.val;
			//core_id = host_vadr[host.reg * 2 + host.blk * 64];

			//host.blk = 4;
			//host.reg = 2;
			//status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READHOST, &host);
			//U16 fifoid = host.val;
			//fifoid = host_vadr[host.reg * 2 + host.blk * 64];

			//host.reg = 3;
			//status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READHOST, &host);
			//U16 fifonum = host.val;
			//fifonum = host_vadr[host.reg * 2 + host.blk * 64];

			//host.blk = 5;
			//host.reg = 2;
			//status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READHOST, &host);
			//fifoid = host.val;

			//host.reg = 3;
			//status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READHOST, &host);
			//fifonum = host.val;

			break;
		}
	}
	delete pSrvList;
	if (!g_hReg)
	{
		QMessageBox::about(this, "Testing Registers", "<font color=red>Service REG not capture!</font>");
		return;
	}

	SetTetrList();
	int iTetr = tetridList->currentIndex();

	GetStatus(iTetr);
	FifoInfo(iTetr);
}

void reg_test::updateFifoInfo()
{
	int iTetr = tetridList->currentIndex();
	if (iTetr != -1)
	{
		GetStatus(iTetr);
		FifoInfo(iTetr);
	}
}

void reg_test::SetDeviceList()
{
	S32		status;
	cbBoardList->clear();

	// получить список LID (каждая запись соответствует устройству)
	g_lidList.item = MAX_DEV; // считаем, что устройств может быть не больше MAX_DEV
	g_lidList.pLID = new U32[MAX_DEV];
	status = BRD_lidList(g_lidList.pLID, g_lidList.item, &g_lidList.itemReal);

	BRD_Info	g_info;
	g_info.size = sizeof(g_info);
	char msgDevice[128];

	for (U32 iDev = 0; iDev < g_lidList.itemReal; iDev++)
	{
		BRD_getInfo(g_lidList.pLID[iDev], &g_info); // получить информацию об устройстве
		U32 ver = g_info.boardType & 0xff;
#ifdef _WIN64
		if ((g_info.boardType >> 16) == 0x53B1) // FMC115cP
			sprintf(msgDevice, " %ls ver%d.%d PID = %d (G.adr %d, order %d, bus %d, dev %d)",
				g_info.name, ver >> 4, ver & 0x0f, g_info.pid & 0xfffffff, g_info.slot & 0xffff, g_info.pid >> 28, g_info.bus, g_info.dev);
		else
			sprintf(msgDevice, " %ls ver%d.%d PID = %d (bus %d, dev %d)",
				g_info.name, ver >> 4, ver & 0x0f, g_info.pid, g_info.bus, g_info.dev);
#else 
		if ((g_info.boardType >> 16) == 0x53B1) // FMC115cP
			sprintf(msgDevice, " %s ver%d.%d PID = %d (G.adr %d, order %d, bus %d, dev %d)",
				g_info.name, ver >> 4, ver & 0x0f, g_info.pid & 0xfffffff, g_info.slot & 0xffff, g_info.pid >> 28, g_info.bus, g_info.dev);
		else
			sprintf(msgDevice, " %s ver%d.%d PID = %d (bus %d, dev %d)",
				g_info.name, ver >> 4, ver & 0x0f, g_info.pid, g_info.bus, g_info.dev);
#endif

		cbBoardList->addItem(msgDevice);
	}
	cbBoardList->setCurrentIndex(0);
}

void reg_test::SetTetrList()
{
	S32		status;
	tetridList->clear();

	BRD_Reg regdata;
	regdata.reg = 0x100;
	for (S32 iTetr = 0; iTetr < MAX_TETRAD; iTetr++)
	{
		regdata.tetr = iTetr;
		status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READIND, &regdata);
		if (!BRD_errcmp(status, BRDerr_OK))
		{
			QString msgErr;
			msgErr.sprintf("<font color=red>Error operation: Tetr = 0x%04X, Reg = 0x%04X</font>", regdata.tetr, regdata.reg);
			QMessageBox::about(this, "Testing Registers", msgErr);
		}
		if (regdata.val == 1 && iTetr) // 2-ой тетрады MAIN быть не может
			break;
		g_tetrID[iTetr] = regdata.val;
		char msgTetrID[32];
		switch (g_tetrID[iTetr])
		{
		case 0:	sprintf(msgTetrID, "0x%02X (Nothing)", g_tetrID[iTetr]); break;
		case 1: sprintf(msgTetrID, "0x%02X (MAIN)", g_tetrID[iTetr]); break;
		case 2: sprintf(msgTetrID, "0x%02X (BASE_DAC)", g_tetrID[iTetr]); break;
		case 3: sprintf(msgTetrID, "0x%02X (PIO_STD)", g_tetrID[iTetr]); break;
		case 0x12: sprintf(msgTetrID, "0x%02X (ADMDIO64_OUT)", g_tetrID[iTetr]); break;
		case 0x13: sprintf(msgTetrID, "0x%02X (ADMDIO64_IN)", g_tetrID[iTetr]); break;
		case 0x25: sprintf(msgTetrID, "0x%02X (AMBPCD_SDRAM)", g_tetrID[iTetr]); break;
		case 0x2D: sprintf(msgTetrID, "0x%02X (TEST128_OUT)", g_tetrID[iTetr]); break;
		case 0x4C: sprintf(msgTetrID, "0x%02X (TEST128_IN)", g_tetrID[iTetr]); break;
		case 0x4B: sprintf(msgTetrID, "0x%02X (AMBPCD_SDRAM_READ)", g_tetrID[iTetr]); break;
		case 0x6F: sprintf(msgTetrID, "0x%02X (AMBPEX2_SDRAM)", g_tetrID[iTetr]); break;
		case 0x77: sprintf(msgTetrID, "0x%02X (AMBPEX2_SDRAM_READ)", g_tetrID[iTetr]); break;
		case 0x70: sprintf(msgTetrID, "0x%02X (AMBPEX5_SDRAM)", g_tetrID[iTetr]); break;
		case 0x7F: sprintf(msgTetrID, "0x%02X (BASE_DAC_SDRAM)", g_tetrID[iTetr]); break;
		case 0x8F: sprintf(msgTetrID, "0x%02X (FMC106P_SDRAM)", g_tetrID[iTetr]); break;
		case 0xA5: sprintf(msgTetrID, "0x%02X (DDR3_TST)", g_tetrID[iTetr]); break;
		case 0x41: sprintf(msgTetrID, "0x%02X (BASE_DDS)", g_tetrID[iTetr]); break;
		case 0x4F: sprintf(msgTetrID, "0x%02X (TEST_CTRL)", g_tetrID[iTetr]); break;
		case 0x82: sprintf(msgTetrID, "0x%02X (FM814x125M)", g_tetrID[iTetr]); break;
		case 0x88: sprintf(msgTetrID, "0x%02X (FM214x250M)", g_tetrID[iTetr]); break;
		case 0x89: sprintf(msgTetrID, "0x%02X (FM412x500M)", g_tetrID[iTetr]); break;
		case 0x8B: sprintf(msgTetrID, "0x%02X (FM816x250M)", g_tetrID[iTetr]); break;
		case 0x93: sprintf(msgTetrID, "0x%02X (FM416x250M)", g_tetrID[iTetr]); break;
		case 0x8C: sprintf(msgTetrID, "0x%02X (TRD_FDSP)", g_tetrID[iTetr]); break;
		case 0x8E: sprintf(msgTetrID, "0x%02X (TRD_FDSP_OUT)", g_tetrID[iTetr]); break;
		case 0x92: sprintf(msgTetrID, "0x%02X (FM212x1G)", g_tetrID[iTetr]); break;
		case 0x50: sprintf(msgTetrID, "0x%02X (DAC216x400M)", g_tetrID[iTetr]); break;
		case 0x94: sprintf(msgTetrID, "0x%02X (DAC216x400M_CG)", g_tetrID[iTetr]); break;
		case 0x9B: sprintf(msgTetrID, "0x%02X (SDRAM_DDR3X)", g_tetrID[iTetr]); break;
		case 0xB3: sprintf(msgTetrID, "0x%02X (SDRAM_DDR3_RD)", g_tetrID[iTetr]); break;
		case 0x9C: sprintf(msgTetrID, "0x%02X (FM216x250MDA_ADC)", g_tetrID[iTetr]); break;
		case 0x9D: sprintf(msgTetrID, "0x%02X (FM216x250MDA_DAC)", g_tetrID[iTetr]); break;
		case 0xA7: sprintf(msgTetrID, "0x%02X (ADMDAC216x400MT)", g_tetrID[iTetr]); break;
		case 0xAA: sprintf(msgTetrID, "0x%02X (FMTEST)", g_tetrID[iTetr]); break;
		case 0xAC: sprintf(msgTetrID, "0x%02X (FM814x250M)", g_tetrID[iTetr]); break;
		case 0xAD: sprintf(msgTetrID, "0x%02X (FM416x100M)", g_tetrID[iTetr]); break;
		case 0xB2: sprintf(msgTetrID, "0x%02X (FM416x1G5D)", g_tetrID[iTetr]); break;
		case 0xBA: sprintf(msgTetrID, "0x%02X (FM214x2G5D)", g_tetrID[iTetr]); break;
		case 0xB6: sprintf(msgTetrID, "0x%02X (ADM216x160)", g_tetrID[iTetr]); break;
		case 0xB7: sprintf(msgTetrID, "0x%02X (XM416x250M)", g_tetrID[iTetr]); break;
		default: sprintf(msgTetrID, "0x%02X (Unknown)", g_tetrID[iTetr]);
		}
		tetridList->addItem(msgTetrID);
	}
	tetridList->setCurrentIndex(0);
}

void reg_test::GetStatus(int iTetr)
{
	S32		status;
	BRD_Reg regdata;
	regdata.reg = 0;
	regdata.tetr = iTetr;
	status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READDIR, &regdata);
	if (!BRD_errcmp(status, BRDerr_OK))
	{
		QString msgErr;
		msgErr.sprintf("<font color=red>Error getting status: Tetr = 0x%04X</font>", regdata.tetr);
		QMessageBox::about(this, "Testing Registers", msgErr);
	}
	else
	{
		UINT reg_val = regdata.val & 0xFFFF;
		QString msgHex;
		msgHex.sprintf("%04X", reg_val);
		hex_status->setText(msgHex);
		char msgBin[32];
		U32 mask = 0x8000;
		for (int i = 0; i < 16; i++)
		{
			msgBin[i] = (reg_val & mask) ? '1' : '0';
			mask >>= 1;
		}
		msgBin[16] = 0;
		bin_status->setText(msgBin);
	}
}

void reg_test::FifoInfo(int iTetr)
{
	S32		status;
	QString msgFifoInfo;
	msgFifoInfo = "FIFO: ";
	//lstrcpy(msgFifoInfo, _BRDC("FIFO: "));

	BRD_Reg regdata;
	regdata.tetr = iTetr;
	regdata.reg = 0x103;
	status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READIND, &regdata);
	if (!BRD_errcmp(status, BRDerr_OK))
	{
		QString msgErr;
		msgErr.sprintf("<font color=red>Error operation: Tetr = 0x%04X, Reg = 0x%04X</font>", regdata.tetr, regdata.reg);
		QMessageBox::about(this, "Testing Registers", msgErr);
	}
	if ((regdata.val & 0x30) == 0)
	{ // двунаправленное FIFO
		msgFifoInfo += "none";
		//lstrcat(msgFifoInfo, _BRDC("none"));
		//CWnd* pWnd = (CWnd*)GetDlgItem(IDC_GETFIFO);
		//pWnd->EnableWindow(FALSE);
		//pWnd = (CWnd*)GetDlgItem(IDC_PUTFIFO);
		//pWnd->EnableWindow(FALSE);
	}
	else
	{
		if (regdata.val & 0x10)
		{ // FIFO ввода
			g_dir = BRDstrm_DIR_IN;
			msgFifoInfo += "input, ";
			//lstrcat(msgFifoInfo, _BRDC("input, "));
			//CWnd* pWnd = (CWnd*)GetDlgItem(IDC_GETFIFO);
			//pWnd->EnableWindow(TRUE);
			//pWnd = (CWnd*)GetDlgItem(IDC_PUTFIFO);
			//pWnd->EnableWindow(FALSE);
		}
		else
		{
			if (regdata.val & 0x20)
			{ // FIFO вывода
				g_dir = BRDstrm_DIR_OUT;
				msgFifoInfo += "output, ";
				//lstrcat(msgFifoInfo, _BRDC("output, "));
				//CWnd* pWnd = (CWnd*)GetDlgItem(IDC_GETFIFO);
				//pWnd->EnableWindow(FALSE);
				//pWnd = (CWnd*)GetDlgItem(IDC_PUTFIFO);
				//pWnd->EnableWindow(TRUE);
			}
		}
		int exFifo = ((regdata.val & 0x200) >> 9);
		if (exFifo)
		{ // внешнее FIFO
			msgFifoInfo += "external, size = unknown";
			//lstrcat(msgFifoInfo, _BRDC("external, size = unknown"));
			g_fifoSizeBytes = 65536;
		}
		else
		{ // внутреннее FIFO
			regdata.reg = 0x105;
			status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READIND, &regdata);
			int widthFifo = regdata.val >> 3;
			regdata.reg = 0x104;
			status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READIND, &regdata);
			g_fifoSizeBytes = regdata.val * widthFifo;
			char msgFifoType[64];
			sprintf(msgFifoType, "internal, size = %d kBytes", g_fifoSizeBytes / 1024);
			msgFifoInfo += msgFifoType;
			//lstrcat(msgFifoInfo, msgFifoType);
		}
	}
	//QString strInfo; 
	//strInfo.sprintf("<font color=green><b>%s</b>, DevID = 0x%x, RevID = 0x%x, PID = %lu, Bus = %d, Dev = %d.</font>", 
	//		info.name, info.boardType >> 16, info.boardType & 0xff, info.pid, info.bus, info.dev);
	fifo_info->setText(msgFifoInfo);

	//CStatic* pFifoInfo = (CStatic*)GetDlgItem(IDC_FINFO);
	//pFifoInfo->SetWindowText(msgFifoInfo);
}

void reg_test::keyPressEvent(QKeyEvent *pEvent)
{
	if (pEvent->key() == Qt::Key_Escape)
		return;
	if (pEvent->key() == Qt::Key_Enter)
		return;
	if (pEvent->key() == Qt::Key_Return)
		return;

	QDialog::keyPressEvent(pEvent);
}

typedef struct _THREAD_PARAM {
	UINT reg;
	UINT val;
	int tetr;
	int flagRW;
	void* pDlg;
} THREAD_PARAM, *PTHREAD_PARAM;

//HANDLE g_hThread = NULL;
IPC_handle  g_hThread = NULL;
THREAD_PARAM param;
volatile int g_isRealCycling = FALSE;

//unsigned __stdcall ThreadReadWrite(void* pParams)
thread_value __IPC_API ThreadReadWrite(void* pParams)
{
	S32		status;
	PTHREAD_PARAM pThreadParam = (PTHREAD_PARAM)pParams;
	BRD_Reg regdata;
	regdata.tetr = pThreadParam->tetr;
	regdata.reg = pThreadParam->reg;
	if (pThreadParam->flagRW)
	{
		do
		{
			status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READIND, &regdata);
			if (!BRD_errcmp(status, BRDerr_OK))
			{
				QString msgErr;
				msgErr.sprintf("<font color=red>Error reading: Tetr = 0x%04X, Reg = 0x%04X</font>", regdata.tetr, regdata.reg);
				QMessageBox::about((QDialog*)pThreadParam->pDlg, "Testing Registers", msgErr);
				break;
			}
			if (g_isRealCycling == FALSE)
				break;
			IPC_delay(10);
		} while (g_isRealCycling);
	}
	else
	{
		do
		{
			regdata.val = pThreadParam->val;
			status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_WRITEIND, &regdata);
			if (!BRD_errcmp(status, BRDerr_OK))
			{
				QString msgErr;
				msgErr.sprintf("<font color=red>Error writing: Tetr = 0x%04X, Reg = 0x%04X</font>", regdata.tetr, regdata.reg);
				QMessageBox::about((QDialog*)pThreadParam->pDlg, "Testing Registers", msgErr);
				break;
			}
			if (g_isRealCycling == FALSE)
				break;
			IPC_delay(10);
		} while (1);
	}
	return 0;
}

void reg_test::ClickedRead()
{
	// TODO: Add your control notification handler code here
	S32		status;
	bool ok;
	QString str_addr = lineAddr->text();
	ULONG reg_addr = str_addr.toULong(&ok, 16);
	int iTetr = tetridList->currentIndex();
	int isCyclingRW = checkRdWr->isChecked();
	if (!isCyclingRW)
	{
		BRD_Reg regdata;
		regdata.reg = reg_addr;
		regdata.tetr = iTetr;
		status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READIND, &regdata);
		if (!BRD_errcmp(status, BRDerr_OK))
		{
			QString msgErr;
			msgErr.sprintf("<font color=red>Error reading: Tetr = 0x%04X, Reg = 0x%04X</font>", regdata.tetr, regdata.reg);
			QMessageBox::about(this, "Testing Registers", msgErr);
		}
		UINT reg_val = regdata.val & 0xFFFF;
		QString str_val;
		str_val.sprintf("%04X", reg_val);
		lineVal->setText(str_val);
	}
	else
	{
		if (g_isRealCycling == FALSE)
		{
			g_isRealCycling = TRUE;
			checkRdWr->setEnabled(false);
			writeButton->setEnabled(false);
			readButton->setText("Reading...");
			unsigned int threadID;
			param.flagRW = 1;
			//			param.reg = reg_addr;
			param.tetr = iTetr;
			param.pDlg = this;
			//g_hThread = (HANDLE)_beginthreadex( NULL, 0, &ThreadReadWrite, &param, 0, &threadID );
			g_hThread = IPC_createThread(_BRDC("thread_rw"), ThreadReadWrite, &param);

		}
		else
		{
			g_isRealCycling = FALSE;
			checkRdWr->setEnabled(true);
			writeButton->setEnabled(true);
			readButton->setText("Read");
		}
	}
}

void reg_test::ClickedWrite()
{
	// TODO: Add your control notification handler code here
	S32		status;
	bool ok;
	QString str_addr = lineAddr->text();
	ULONG reg_addr = str_addr.toULong(&ok, 16);
	QString str_val = lineVal->text();
	ULONG reg_val = str_val.toULong(&ok, 16);
	int iTetr = tetridList->currentIndex();
	BRD_Reg regdata;
	regdata.reg = reg_addr;
	regdata.tetr = iTetr;
	int isCyclingRW = checkRdWr->isChecked();
	if (!isCyclingRW)
	{
		regdata.val = reg_val;
		status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_WRITEIND, &regdata);
		if (!BRD_errcmp(status, BRDerr_OK))
		{
			QString msgErr;
			msgErr.sprintf("<font color=red>Error writing: Tetr = 0x%04X, Reg = 0x%04X</font>", regdata.tetr, regdata.reg);
			QMessageBox::about(this, "Testing Registers", msgErr);
		}
	}
	else
	{
		if (g_isRealCycling == FALSE)
		{
			g_isRealCycling = TRUE;
			checkRdWr->setEnabled(false);
			readButton->setEnabled(false);
			writeButton->setText("Writing...");
			unsigned int threadID;
			param.flagRW = 0;
			param.reg = reg_addr;
			param.tetr = iTetr;
			param.val = reg_val;
			param.pDlg = this;
			//g_hThread = (HANDLE)_beginthreadex( NULL, 0, &ThreadReadWrite, &param, 0, &threadID );
			g_hThread = IPC_createThread(_BRDC("thread_rw"), ThreadReadWrite, &param);

		}
		else
		{
			g_isRealCycling = FALSE;
			checkRdWr->setEnabled(true);
			readButton->setEnabled(true);
			writeButton->setText("Write");
		}
	}
}

void reg_test::ClickedReadSpd()
{
	// TODO: Add your control notification handler code here
	S32		status;
	bool ok;
	QString str_dev = lineDev->text();
	ULONG dev = str_dev.toULong(&ok, 16);
	QString str_num = lineNum->text();
	ULONG num = str_num.toULong(&ok, 16);
	QString str_reg = lineReg->text();
	ULONG reg = str_reg.toULong(&ok, 16);
	int iTetr = tetridList->currentIndex();
	//int isCyclingRW = checkRdWr->isChecked();
	//if (!isCyclingRW)
	{
		BRD_Spd spd_data;
		spd_data.dev = dev;
		spd_data.num = num;
		spd_data.reg = reg;
		spd_data.tetr = iTetr;
		spd_data.val = 0;
		status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_READSPD, &spd_data);
		if (!BRD_errcmp(status, BRDerr_OK))
		{
			QString msgErr;
			msgErr.sprintf("<font color=red>Error reading: Tetr = 0x%04X, Dev = 0x%04X, Num = 0x%04X, Reg = 0x%04X</font>", spd_data.tetr, spd_data.dev, spd_data.num, spd_data.reg);
			QMessageBox::about(this, "Testing Registers", msgErr);
		}
		UINT reg_val = spd_data.val & 0xFFFF;
		QString str_val;
		str_val.sprintf("%04X", reg_val);
		lineValSpd->setText(str_val);
	}
	//else
	//{
	//	if (g_isRealCycling == FALSE)
	//	{
	//		g_isRealCycling = TRUE;
	//		checkRdWr->setEnabled(false);
	//		writeButton->setEnabled(false);
	//		readButton->setText("Reading...");
	//		unsigned int threadID;
	//		param.flagRW = 1;
	//		//			param.reg = reg_addr;
	//		param.tetr = iTetr;
	//		param.pDlg = this;
	//		//g_hThread = (HANDLE)_beginthreadex( NULL, 0, &ThreadReadWrite, &param, 0, &threadID );
	//		g_hThread = IPC_createThread(_BRDC("thread_rw"), ThreadReadWrite, &param);

	//	}
	//	else
	//	{
	//		g_isRealCycling = FALSE;
	//		checkRdWr->setEnabled(true);
	//		writeButton->setEnabled(true);
	//		readButton->setText("Read");
	//	}
	//}
}

void reg_test::ClickedWriteSpd()
{
	// TODO: Add your control notification handler code here
	S32		status;
	bool ok;
	QString str_dev = lineDev->text();
	ULONG dev = str_dev.toULong(&ok, 16);
	QString str_num = lineNum->text();
	ULONG num = str_num.toULong(&ok, 16);
	QString str_reg = lineReg->text();
	ULONG reg = str_reg.toULong(&ok, 16);
	QString str_val = lineValSpd->text();
	ULONG reg_val = str_val.toULong(&ok, 16);
	int iTetr = tetridList->currentIndex();
	BRD_Spd spd_data;
	spd_data.dev = dev;
	spd_data.num = num;
	spd_data.reg = reg;
	spd_data.tetr = iTetr;
	int isCyclingRW = checkRdWr->isChecked();
	if (!isCyclingRW)
	{
		spd_data.val = reg_val;
		status = BRD_ctrl(g_hReg, 0, BRDctrl_REG_WRITESPD, &spd_data);
		if (!BRD_errcmp(status, BRDerr_OK))
		{
			QString msgErr;
			msgErr.sprintf("<font color=red>Error writing: Tetr = 0x%04X, Dev = 0x%04X, Num = 0x%04X, Reg = 0x%04X</font>", spd_data.tetr, spd_data.dev, spd_data.num, spd_data.reg);
			QMessageBox::about(this, "Testing Registers", msgErr);
		}
	}
	//else
	//{
	//	if (g_isRealCycling == FALSE)
	//	{
	//		g_isRealCycling = TRUE;
	//		checkRdWr->setEnabled(false);
	//		readButton->setEnabled(false);
	//		writeButton->setText("Writing...");
	//		unsigned int threadID;
	//		param.flagRW = 0;
	//		param.reg = reg_addr;
	//		param.tetr = iTetr;
	//		param.val = reg_val;
	//		param.pDlg = this;
	//		//g_hThread = (HANDLE)_beginthreadex( NULL, 0, &ThreadReadWrite, &param, 0, &threadID );
	//		g_hThread = IPC_createThread(_BRDC("thread_rw"), ThreadReadWrite, &param);

	//	}
	//	else
	//	{
	//		g_isRealCycling = FALSE;
	//		checkRdWr->setEnabled(true);
	//		readButton->setEnabled(true);
	//		writeButton->setText("Write");
	//	}
	//}
}
