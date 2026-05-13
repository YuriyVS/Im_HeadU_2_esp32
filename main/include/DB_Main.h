#ifndef DB_MAIN_H
#define DB_MAIN_H

#include <stdint.h>
#include <stdbool.h>

/* --- PROJECT DATA STRUCTURES (Текущие значения) --- */

typedef union {
    struct {
    	float UsetiV;
    	float Useti;
    	float IakbA;
    	float Iakb;
    	float Rezerv1U;
    	float Rezerv1;
    	float Rezerv2U;
    	float Rezerv2;
    	float Rezerv3U;
    	float Rezerv3;
    	float Rezerv4U;
    	float Rezerv4;
        float GenFreq;
        float GenFreqHz;
        float Idпорог;
        float Itint;
        float FreqGenABHz;
        float FreqGenBCHz;
        float FreqGenCAHz;
        float SpeedCar;
        float ControlPhazGenGist;
        float Alfa_ref;
        float Alfa;
        float Idref2;
        float RTout2;
        float Idelta2;
        float IntChRT2;
        float ZiTout1;
        float Idref1;
        float RTout1;
        float Idelta1;
		float IntChRT1;
        float RNref;
        float RNout;
        float Ndelta;
        float IntChRN;
        float ZiNout;
        float ZiNTiStop;
        float TimerA;
        float TimerB;
        float TimerC;
        float Ud_ref;
        float IakbRef;
        float IgenRef;
        float Iseti;
        float RefIovg;
        float fval47;
        float fval48;
        float fval49;
        float fval50;
    };
    float as_array[50];
} DB_Main_Float_ID1_50;

typedef union {
    struct {
    	float fval1;
    	float fval2;
    	float fval3;
    	float fval4;
    	float fval5;
    	float fval6;
    	float fval7;
    	float fval8;
    	float fval9;
    	float fval10;
    	float fval11;
    	float fval12;
    	float fval13;
    	float fval14;
    	float fval15;
    	float fval16;
    	float fval17;
    	float fval18;
    	float fval19;
    	float fval20;
    	float fval21;
    	float fval22;
    	float fval23;
    	float fval24;
    	float fval25;
    	float fval26;
    	float fval27;
    	float fval28;
    	float fval29;
    	float fval30;
    	float fval31;
    	float fval32;
    	float fval33;
    	float fval34;
    	float fval35;
    	float fval36;
    	float fval37;
    	float fval38;
    	float fval39;
    	float fval40;
    	float fval41;
    	float fval42;
    	float fval43;
    	float fval44;
    	float fval45;
    	float fval46;
    	float fval47;
    	float fval48;
    	float fval49;
    	float fval50;
    };
    float as_array[50];
} DB_Main_Float_ID51_100;

typedef union {
    struct {
        int32_t phaseSum;
        int32_t ival2;
        int32_t ival3;
        int32_t ival4;
        int32_t ival5;
        int32_t ival6;
        int32_t ival7;
        int32_t ival8;
        int32_t ival9;
        int32_t ival10;
        int32_t ival11;
        int32_t ival12;
        int32_t ival13;
        int32_t ival14;
        int32_t ival15;
        int32_t ival16;
        int32_t ival17;
        int32_t ival18;
        int32_t ival19;
        int32_t ival20;
        int32_t ival21;
        int32_t ival22;
        int32_t ival23;
        int32_t ival24;
        int32_t ival25;
        int32_t ival26;
        int32_t ival27;
        int32_t ival28;
        int32_t ival29;
        int32_t ival30;
        int32_t ival31;
        int32_t ival32;
        int32_t ival33;
        int32_t ival34;
        int32_t ival35;
        int32_t ival36;
        int32_t ival37;
        int32_t ival38;
        int32_t ival39;
        int32_t ival40;
        int32_t ival41;
        int32_t ival42;
        int32_t ival43;
        int32_t ival44;
        int32_t ival45;
        int32_t ival46;
        int32_t ival47;
        int32_t ival48;
        int32_t ival49;
        int32_t ival50;

    };
    int32_t as_array[50];
} DB_Main_Int_ID1_50;

typedef union {
    struct {
        uint32_t uval1;
        uint32_t uval2;
        uint32_t uval3;
        uint32_t uval4;
        uint32_t uval5;
        uint32_t uval6;
        uint32_t uval7;
        uint32_t uval8;
        uint32_t uval9;
        uint32_t uval10;
        uint32_t uval11;
        uint32_t uval12;
        uint32_t uval13;
        uint32_t uval14;
        uint32_t uval15;
        uint32_t uval16;
        uint32_t uval17;
        uint32_t uval18;
        uint32_t uval19;
        uint32_t uval20;
        uint32_t uval21;
        uint32_t uval22;
        uint32_t uval23;
        uint32_t uval24;
        uint32_t uval25;
        uint32_t uval26;
        uint32_t uval27;
        uint32_t uval28;
        uint32_t uval29;
        uint32_t uval30;
        uint32_t uval31;
        uint32_t uval32;
        uint32_t uval33;
        uint32_t uval34;
        uint32_t uval35;
        uint32_t uval36;
        uint32_t uval37;
        uint32_t uval38;
        uint32_t uval39;
        uint32_t uval40;
        uint32_t uval41;
        uint32_t uval42;
        uint32_t uval43;
        uint32_t uval44;
        uint32_t uval45;
        uint32_t uval46;
        uint32_t uval47;
        uint32_t uval48;
        uint32_t uval49;
        uint32_t uval50;
    };
    uint32_t as_array[50];
} DB_Main_UInt_ID1_50;

typedef union {
    uint32_t all;
    struct {
        bool Kn380V_Vtkl_DIxor: 1;  bool Kn380V_Otkl_DIxor: 1;  bool KrishkaVidkrita_DIxor: 1;  bool DistanVidkl380V_DIxor: 1;
        bool QFVvod_DIxor: 1;  bool QFVvodAvarStan_DIxor: 1;  bool Rezerv1_DIxor: 1;  bool Rezerv2_DIxor: 1;
        bool Rezerv3_DIxor: 1;  bool Rezerv4_DIxor: 1;  bool Rezerv5_DIxor: 1;  bool Rezerv6_DIxor: 1;
        bool Rezerv7_DIxor: 1;  bool i380V_Podano_DOxor: 1;  bool i380V_Znyato_DOxor: 1;  bool Merezha380vFault_DOxor: 1;
        bool GeneratorFault_DOxor: 1;  bool i380V_Vvod_DOxor: 1;  bool BlockDoor_DOxor: 1;  bool Rezerv1_DOxor: 1;
        bool Rezerv2_DOxor: 1;  bool Rezerv3_DOxor: 1;  bool Rezerv4_DOxor: 1;  bool Rezerv5_DOxor: 1;
        bool Rezerv6_DOxor: 1;  bool Rezerv7_DOxor: 1;  bool Rezerv8_DOxor: 1;  bool Reset: 1;
        bool DistanVidkl380V: 1;  bool QFVvodAvarStan: 1;  bool i380V_Vvod: 1;  bool FazaA1Loss: 1;
    } ;
} DB_Main_Bool_ID1_32;

typedef union {
    uint32_t all;
    struct {
        bool FazaB1Loss: 1;  bool FazaC1Loss: 1;  bool ABC380ok: 1;  bool FazaLossInWork: 1;
        bool Iakb_TimemaxMem: 1;  bool IakbmaxMem: 1;  bool ReadyVkl: 1;  bool ReadyRun: 1;
        bool ReadyRef: 1;  bool Run: 1;  bool Fault: 1;  bool Gotov: 1;
        bool Alarm: 1;  bool RectVkluchen: 1;  bool BlockRect: 1;  bool GenAloss: 1;
        bool GenBloss: 1;  bool GenCloss: 1;  bool GenAsboi: 1;  bool GenBsboi: 1;
        bool GenCsboi: 1;  bool FaultGenerator: 1;  bool BlockCarDoors: 1;  bool BlockSifu: 1;
        bool TM: 1;  bool EnableSifu: 1;  bool BlokDeleyTimeSifu: 1;  bool ShuntReg: 1;
        bool ShuntRT2: 1;  bool ShuntZiT1: 1;  bool ShuntRT1: 1;  bool ShuntZiT2: 1;
    } ;
} DB_Main_Bool_ID33_64;

typedef union {
    uint32_t all;
    struct {
        bool ShuntRN: 1;  bool ShuntZiN: 1;  bool UdmaxMem: 1;  bool Sled: 1;
        bool EStopMem: 1;  bool EnableSifuOld: 1;  bool PulseStage1: 1;  bool PulseStage2: 1;
        bool PulseStage3: 1;  bool PulseStage4: 1;  bool PulseStage5: 1;  bool PulseStage6: 1;
        bool Save_to_Flash: 1;  bool Read_from_Flash: 1;  bool bit79: 1;  bool bit80: 1;
        bool bit81: 1;  bool bit82: 1;  bool bit83: 1;  bool bit84: 1;
        bool bit85: 1;  bool bit86: 1;  bool bit87: 1;  bool bit88: 1;
        bool bit89: 1;  bool bit90: 1;  bool bit91: 1;  bool bit92: 1;
        bool bit93: 1;  bool bit94: 1;  bool bit95: 1;  bool bit96: 1;
    } ;
} DB_Main_Bool_ID65_96;

/* --- MASTER CONTAINERS --- */

typedef struct __attribute__((packed)) {
	DB_Main_Float_ID1_50  f50;
	DB_Main_Float_ID51_100 f100;
	DB_Main_Int_ID1_50    i50;
	DB_Main_UInt_ID1_50   u50;
	DB_Main_Bool_ID1_32   b32;
	DB_Main_Bool_ID33_64  b64;
	DB_Main_Bool_ID65_96  b96;
} DB_Main;

extern volatile DB_Main DBMain;
extern const DB_Main DBMainInit;

#endif
