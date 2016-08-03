
#define MAX_STRING_LEN	255

#define MAX_TETRNUM 16 // max number of tetrades of the ADM-interface

#define INSYS_TAG	0x4953 // tag of InSys
#define ADM_VERSION 0x200 // ADM version
#define FMC_VERSION 0x300 // FMC version

#define NEXT_CAP_POINTER 0x34
#define PEXCAPID 0x10
#define DEVCAPOFFSET 0x04
#define DEVCTRLOFFSET 0x08
#define LINKOFFSET 0x10

//#define Next_Cap_Pointer 0x34
//#define PEXCapID 0x10
//#define LinkOffset 0x10

// PCI Express Capability List Register & Capabilities Register (0x00)
typedef union _PEX_CAPREG {
	ULONG AsWhole; // Register as a Whole Word
	struct { // Register as Bit Pattern
		ULONG	CapId			: 8, // Capability ID
				NextCapPointer	: 8, // Next Capability Pointer
				CapVer			: 4, // Capability version
				DevType			: 4, // Device/Port type
				Slot			: 1, // Slot Implemented
				IntMsgNum		: 5, // Interrrupt Message Number
				Res				: 2; // Reserved
	} ByBits;
} PEX_CAPREG, *PPEX_CAPREG;

// Link Control Register & Link Status Register (0x10)
typedef union _PEX_LINKREG {
	ULONG AsWhole; // Register as a Whole Word
	struct { // Register as Bit Pattern
		ULONG	ASPM			: 2, // Active State Power Management
				Res				: 1, // Reserved
				RCB				: 1, // Root Completion Boundary
				LinkDis			: 1, // Link Disable
				RetrLink		: 1, // Retrain Link
				CmnClkCfg		: 1, // Common Clock Configuration
				Sync			: 1, // Extended Synch
				Res1			: 8, // Reserved
				LinkSpeed		: 4, // Link Speed
				LinkWidth		: 6, // Negotiated Link Width
				LinkTrainEr		: 1, // Link Training Error
				LinkTrain		: 1, // Link Training
				SlotClkCfg		: 1, // Slot Clock Configuration
				Res2			: 3; // Reserved
	} ByBits;
} PEX_LINKREG, *PPEX_LINKREG;

// Numbers of Tetrad Registers
typedef enum _ADM2IF_NUM_TETR_REGS {
	ADM2IFnr_STATUS = 0,	// (0x00) Status register
	ADM2IFnr_DATA	= 1,	// (0x02) Data register
	ADM2IFnr_CMDADR	= 2,	// (0x04) Address command register
	ADM2IFnr_CMDDATA= 3,	// (0x06) Data command register
} ADM2IF_NUM_TETR_REGS;

// Numbers of PCI-Express Main block Registers
typedef enum _PE_MAIN_ADDR_REG {
    PEMAINadr_BLOCK_ID		= 0x00, // 0x00
    PEMAINadr_BLOCK_VER		= 0x08, // 0x01
    PEMAINadr_DEVICE_ID		= 0x10, // 0x02
    PEMAINadr_DEVICE_REV	= 0x18, // 0x03
    PEMAINadr_PLD_VER		= 0x20, // 0x04
    PEMAINadr_BLOCK_CNT		= 0x28, // 0x05
    PEMAINadr_CORE_MOD		= 0x30, // 0x06
    PEMAINadr_CORE_ID		= 0x38, // 0x07
    PEMAINadr_BRD_MODE		= 0x40, // 0x08
    PEMAINadr_IRQ_MASK		= 0x48, // 0x09
    PEMAINadr_IRQ_INV		= 0x50, // 0x0A
    PEMAINadr_LEDS			= 0x58, // 0x0B
    PEMAINadr_BRD_STATUS	= 0x80, // 0x10
    PEMAINadr_IRQ_MIN		= 0x90, // 0x12
    PEMAINadr_SPD_CTRL		= 0xB0, // 0x16
    PEMAINadr_SPD_ADDR		= 0xB8, // 0x17
    PEMAINadr_SPD_DATAL		= 0xC0, // 0x18
    PEMAINadr_SPD_DATAH		= 0xC8, // 0x19
    PEMAINadr_LB_DATA		= 0xD0, // 0x1A
    PEMAINadr_JTAG_CNT		= 0xE0, // 0x1C
    PEMAINadr_JTAG_TMS		= 0xE8, // 0x1D
    PEMAINadr_JTAG_TDI		= 0xF0, // 0x1E
    PEMAINadr_JTAG_TDO		= 0xF8, // 0x1F
} PE_MAIN_ADDR_REG;  

// Numbers of PCI-Express FIFO block Registers
typedef enum _PE_FIFO_ADDR_REG {
    PEFIFOadr_BLOCK_ID		= 0x00, // 0x00
    PEFIFOadr_BLOCK_VER		= 0x08, // 0x01
    PEFIFOadr_FIFO_ID		= 0x10, // 0x02
    PEFIFOadr_FIFO_NUM		= 0x18, // 0x03
    PEFIFOadr_DMA_SIZE		= 0x20, // 0x04
    PEFIFOadr_FIFO_CTRL		= 0x40, // 0x08
    PEFIFOadr_DMA_CTRL		= 0x48, // 0x09
    PEFIFOadr_FIFO_STATUS	= 0x80, // 0x10
    PEFIFOadr_FLAG_CLR		= 0x88, // 0x11
    PEFIFOadr_ERROR_CNT		= 0x90, // 0x12
    PEFIFOadr_PCI_ADDRL		= 0xA0, // 0x14
    PEFIFOadr_PCI_ADDRH		= 0xA8, // 0x15
    PEFIFOadr_PCI_SIZE		= 0xB0, // 0x16
    PEFIFOadr_LOCAL_ADR		= 0xB8, // 0x17
	PEFIFOadr_IRQ_MODE		= 0xE0, // 0x1C
	PEFIFOadr_IRQ_TBLADR	= 0xE8, // 0x1D
	PEFIFOadr_IRQ_TBLDATA	= 0xF0, // 0x1E
	PEFIFOadr_IRQ_CNT		= 0xF8, // 0x1F
} PE_FIFO_ADDR_REG;  

// FIFO Status register 0x80 (PE_FIFO)
typedef union _FIFO_STATUS {
	ULONG AsWhole; // FIFO Status Register as a Whole Word
	struct { // FIFO Status Register as Bit Pattern
		ULONG	DmaStat		: 4, // DMA Status
				DmaEot		: 1, // DMA block Complete (End of Transfer)
				SGEot		: 1, // Scatter/Gather End of Transfer (all blocks complete)
				IntErr		: 1, // not serviced of interrrupt - ERROR!!! block leave out!!!
				IntRql		: 1, // Interrrupt request
				FifoEmpty	: 1, // Fifo Empty
				FifoPae		: 1, // Fifo full < 256 words
				FifoPaf		: 1, // Fifo empty < 256 words
				FifoFull	: 1, // Fifo Full
				NotUse		: 4; // Not Use
	} ByBits;
} FIFO_STATUS, *PFIFO_STATUS;

// Numbers of Common Constant Registers
typedef enum _ADM2IF_NUM_CONST_REGS {
	ADM2IFnr_ID		= 0x100,
	ADM2IFnr_IDMOD	= 0x101,
	ADM2IFnr_VER	= 0x102,
	ADM2IFnr_TRES	= 0x103,
	ADM2IFnr_FSIZE	= 0x104,
	ADM2IFnr_FTYPE	= 0x105,
	ADM2IFnr_PATH	= 0x106,
	ADM2IFnr_IDNUM	= 0x107,
} ADM2IF_NUM_CONST_REGS;

// Numbers of Main Tetrad Constant Registers
typedef enum _MAIN_NUM_CONST_REGS {
	MAINnr_SIG		= 0x108,
	MAINnr_ADMVER	= 0x109,
	MAINnr_FPGAVER	= 0x10A,
	MAINnr_FPGAMOD	= 0x10B,
	MAINnr_TMASK	= 0x10C,
	MAINnr_MRES		= 0x10D,
	MAINnr_BASEID	= 0x110,
	MAINnr_BASEVER	= 0x111,
	MAINnr_SMODID	= 0x112,
	MAINnr_SMODVER	= 0x113,
	MAINnr_FPGABLD	= 0x114,
} MAIN_NUM_CONST_REGS;

// DMA buffer Directions 
enum
{
	BRDstrm_DIR_IN = 0x1,				// To HOST
	BRDstrm_DIR_OUT = 0x2,				// From HOST
	BRDstrm_DIR_INOUT = 0x3				// Both Directions
};

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

// открывает драйвер устройства
int dev_open(int DeviceNumber);
int dev_openPlx(int DeviceNumber);
// закрывает драйвер устройства
void dev_close();
// получить версию драйвера устройства
ULONG GetVersion(char* pVerInfo);
// получить месторасположение устройства
ULONG GetLocation(ULONG& SlotNumber, ULONG& BusNumber, ULONG& DeviceNumber);
// получить идентификатор устройства
ULONG GetDeviceID(USHORT& DeviceID);
// получить значение регистра конфигурации PCI
ULONG GetPciCfgReg(ULONG& regVal, ULONG offset);
// установить значение регистра конфигурации PCI
ULONG SetPciCfgReg(ULONG& regVal, ULONG offset);
// установить значение регистра конфигурации локальной шины
ULONG WriteLocalBusReg(ULONG& RegVal, ULONG Offset, ULONG Size);
// получить значение регистра конфигурации локальной шины
ULONG ReadLocalBusReg(ULONG& RegVal, ULONG Offset, ULONG Size);
// записать значение в косвенный регистр
ULONG WriteRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, ULONG& RegVal); 
// записать значение в прямой регистр
ULONG WriteRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, ULONG& RegVal);
// прочитать значение из косвенного регистра
ULONG ReadRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, ULONG& RegVal);
// прочитать значение из прямого регистра
ULONG ReadRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, ULONG& RegVal);
// записать буфер в косвенный регистр
ULONG WriteRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize);
// записать буфер в прямой регистр
ULONG WriteRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize);
// прочитать буфер из косвенного регистра
ULONG ReadRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize);
// прочитать буфер из прямого регистра
ULONG ReadRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize);
// прочитать ППЗУ базового модуля
ULONG ReadNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
// записать ППЗУ базового модуля
ULONG WriteNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
// прочитать ППЗУ субмодуля
ULONG ReadSubICR(int submod, void* pBuffer, ULONG BufferSize, ULONG Offset);
ULONG ReadAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
// записать ППЗУ субмодуля
ULONG WriteAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
// получить состояние ПЛИС
ULONG GetPldStatus(ULONG& PldStatus, ULONG PldNum);
// загрузить файл ПЛИС
ULONG LoadPld(ULONG& PldStatus, PVOID pBuffer, ULONG BufferSize, ULONG PldNum);
// получить виртуальный адрес памяти на устройстве
ULONG GetMemoryAddress(PULONG& MemoryAddress);
// сбросить FIFO канала ПДП
ULONG ResetDmaFifo(int DmaChan);
// получить информацию о канале ПДП
ULONG InfoDma(int DmaChan, ULONG& dir, ULONG& FifoSize, ULONG& MaxDmaSize);
// размещение памяти
ULONG AllocMemory(PVOID* pBuf, ULONG blkSize, ULONG& blkNum, ULONG isSysMem, ULONG dir, ULONG addr, int DmaChan);
// освобождение памяти
ULONG FreeMemory(int DmaChan);
// начать передачу данных по каналу ПДП
ULONG StartDma(int IsCycling, int DmaChan);
// остановить передачу данных по каналу ПДП
ULONG StopDma(int DmaChan);
// получить текущее состояние канала ПДП
ULONG StateDma(ULONG msTimeout, int DmaChan, int& state, ULONG& blkNum);
// ожидать завершения передачи всего буфера данных по каналу ПДП (для режима без зацикливания)
ULONG WaitBuffer(ULONG msTimeout, int DmaChan);
// ожидать завершения передачи текущего блока данных по каналу ПДП
ULONG WaitBlock(ULONG msTimeout, int DmaChan);
// установить флаг для запроса ПДП
ULONG SetDmaRequest(int drqFlag, int DmaChan);
// установить режим прерываний
ULONG SetIrqMode(int irqMode, int DmaChan);
