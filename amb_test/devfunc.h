
#pragma pack(push,1)

typedef struct _PCIEC_INFO {
	ULONG pldver;
	ULONG dmaver;
	ULONG coreid;
	ULONG coremod;
	ULONG errcrc;
	ULONG errcto;
	ULONG irqmin;
}PCIEC_INFO,*PPCIEC_INFO;

typedef struct _PLD_INFO {
	ULONG version;
	ULONG modification;
	ULONG build;
	ULONG submif;
}PLD_INFO,*PPLD_INFO;

typedef struct _TRD_INFO {
	ULONG id;
	ULONG ver;
	ULONG mod;
	ULONG tres;
	ULONG fsize;
	ULONG ftype;
	ULONG status;
}TRD_INFO,*PTRD_INFO;

typedef struct _FMC_POWER
{
	ULONG onOff;		// 1 - on, 0 - off
	ULONG value;		// voltage
	ULONG slot;		// slot number (submodule number)
} FMC_POWER, *PFMC_POWER;

#pragma pack(pop)

ULONG PCIE_info(ULONG* params);
ULONG LinkWidth(ULONG& LinkSpeed);
ULONG HostReg(PPCIEC_INFO pInfo);
ULONG GetBlockFidAddress(PULONG& BlockMem);
void FM_Present(PPCIEC_INFO pInfo, int* fm_present);
ULONG GetPower(PFMC_POWER pow);
ULONG PowerOnOff(PFMC_POWER pow, int force);
ULONG AdmPldWorkAndCheck(PPLD_INFO pPldInfo);
void TetradList(PTRD_INFO info);
void TetradName(ULONG id, TCHAR* str);
void BasemodName(ULONG id, TCHAR* str);
void SubmodName(ULONG id, TCHAR* str);
ULONG GetPldDescription(TCHAR* Description, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);
ULONG DmaChannelTest(int tetrNum, int width);
ULONG GetMemorySize(TCHAR* strMemType, UCHAR* pSpdData);
ULONG isSysMon(ULONG& smon_stat);
double getTemp(double& maxv, double& minv);
double getVccint(double& maxv, double& minv);
double getVccaux(double& maxv, double& minv);
void getVref(double& refp, double& refn);
int ReadPldFile(PCTSTR PldFileName, PVOID& fileBuffer, ULONG& fileSize);
ULONG BrdTest(int in_tetr, int out_tetr);
