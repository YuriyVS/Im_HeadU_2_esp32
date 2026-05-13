#ifndef DB_PARAMETERS_H
#define DB_PARAMETERS_H

#include <stdint.h>
#include <stdbool.h>


/* --- PROJECT SETTINGS STRUCTURES (Настройки/Уставки) --- */

typedef union {
    struct {
        float P1_1; //
        float P1_2; //
        float P2_1; //
        float P4_5; //
        float P4_6; //
        float P5_2; //
        float P5_3; //
        float P5_4; //
        float P5_5; //
        float P5_6; //
        float P5_7; //
        float P6_2; //
        float P6_3; //
        float P6_4; //
        float P6_5; //
        float P6_6; //
        float P6_7; //
        float P20_1; //
        float P20_2; //
        float P20_3; //
        float P20_4; //
        float P20_5; //
        float P20_6; //
        float P20_7; //
        float P20_8; //
        float P20_9; //
        float P20_10; //
        float P20_11; //
        float P20_12; //
        float P20_13; //
        float P20_14; //
        float P20_15; //
        float P20_16; //
        float P20_17; //
        float P20_18; //
        float P21_2; //
        float P21_3; //
        float P21_5; //
        float P21_6; //
        float P21_8; //
        float P21_9; //
        float P21_11; //
        float P21_12; //
        float P21_14; //
        float P21_15; //
        float P21_17; //
        float P21_18; //
        float P60_1; //
        float P60_2; //
        float P61_1; //


    };
    float as_array[50]; // Тот же объем памяти в виде массива
} DB_Parameters_Float_ID1_50;

typedef union {
    struct {
        float P31_1; //
        float P31_2; //
        float P31_3; //
        float P32_1; //
        float P32_2; //
        float P32_3; //
        float P32_4; //
        float P33_1; //
        float P33_2; //
        float P33_3; //
        float P33_4; //
        float P34_1; //
        float P34_2; //
        float P34_3; //
        float P34_4; //
        float P34_5; //
        float P34_6; //
        float P34_7; //
        float P34_8; //
        float P50_1; //
        float P50_2; //
        float P50_3; //
        float P50_4; //
        float P50_5; //
        float P50_6; //
        float P50_7; //
        float P50_8; //
        float P50_9; //
        float P50_10; //
        float P61_2; //
        float P80_1; //
        float P80_2; //
        float P80_3; //
        float P80_4; //
        float P80_5; //
        float P81_1; //
        float P81_2; //
        float P81_3; //
        float P81_4; //
        float P81_5; //
        float fP41; //
        float fP42; //
        float fP43; //
        float fP44; //
        float fP45; //
        float fP46; //
        float fP47; //
        float fP48; //
        float fP49; //
        float fP50; //

        };
        float as_array[50]; // Тот же объем памяти в виде массива
} DB_Parameters_Float_ID51_100;

typedef union {
    struct {
        int32_t iset1;
        int32_t iset2;
        int32_t iset3;
        int32_t iset4;
        int32_t iset5;
        int32_t iset6;
        int32_t iset7;
        int32_t iset8;
        int32_t iset9;
        int32_t iset10;
        int32_t iset11;
        int32_t iset12;
        int32_t iset13;
        int32_t iset14;
        int32_t iset15;
        int32_t iset16;
        int32_t iset17;
        int32_t iset18;
        int32_t iset19;
        int32_t iset20;
        int32_t iset21;
        int32_t iset22;
        int32_t iset23;
        int32_t iset24;
        int32_t iset25;
        int32_t iset26;
        int32_t iset27;
        int32_t iset28;
        int32_t iset29;
        int32_t iset30;
        int32_t iset31;
        int32_t iset32;
        int32_t iset33;
        int32_t iset34;
        int32_t iset35;
        int32_t iset36;
        int32_t iset37;
        int32_t iset38;
        int32_t iset39;
        int32_t iset40;
        int32_t iset41;
        int32_t iset42;
        int32_t iset43;
        int32_t iset44;
        int32_t iset45;
        int32_t iset46;
        int32_t iset47;
        int32_t iset48;
        int32_t iset49;
        int32_t iset50;
        // Если в названии ID1_50 подразумевается 50 параметров,
        // их стоит перечислить здесь
    };
    int32_t as_array[50]; // Размер должен точно совпадать с количеством полей в struct
} DB_Parameters_Int_ID1_50;

typedef union {
    struct {
        uint32_t P10_1;
        uint32_t P10_2;
        uint32_t P10_3;
        uint32_t P10_4;
        uint32_t P10_5;
        uint32_t P10_6;
        uint32_t P10_7;
        uint32_t P10_8;
        uint32_t P10_9;
        uint32_t P10_10;
        uint32_t P10_11;
        uint32_t P10_12;
        uint32_t P10_13;
        uint32_t uset14;
        uint32_t uset15;
        uint32_t uset16;
        uint32_t uset17;
        uint32_t uset18;
        uint32_t uset19;
        uint32_t uset20;
        uint32_t uset21;
        uint32_t uset22;
        uint32_t uset23;
        uint32_t uset24;
        uint32_t uset25;
        uint32_t uset26;
        uint32_t uset27;
        uint32_t uset28;
        uint32_t uset29;
        uint32_t uset30;
        uint32_t uset31;
        uint32_t uset32;
        uint32_t uset33;
        uint32_t uset34;
        uint32_t uset35;
        uint32_t uset36;
        uint32_t uset37;
        uint32_t uset38;
        uint32_t uset39;
        uint32_t uset40;
        uint32_t uset41;
        uint32_t uset42;
        uint32_t uset43;
        uint32_t uset44;
        uint32_t uset45;
        uint32_t uset46;
        uint32_t uset47;
        uint32_t uset48;
        uint32_t uset49;
        uint32_t uset50;
        // Здесь можно продолжить список до 50 параметров
    };
    uint32_t as_array[50]; // Размер массива должен соответствовать количеству полей в struct
} DB_Parameters_UInt_ID1_50;

typedef union {
    uint32_t all;
    struct {
    	bool P3_1 : 1;  bool P3_2 : 1;  bool P3_3 : 1;  bool P4_1 : 1;
    	        bool P4_2 : 1;  bool P4_3 : 1;  bool P4_4 : 1;  bool P5_1 : 1;
    	        bool P5_8 : 1;  bool P6_1 : 1;  bool P11_1: 1;  bool P11_2: 1;
    	        bool P11_3: 1;  bool P11_4: 1;  bool P11_5: 1;  bool P11_6: 1;
    	        bool P11_7: 1;  bool P11_8: 1;  bool P11_9: 1;  bool P11_10: 1;
    	        bool P11_11: 1;  bool P11_12: 1;  bool P11_13: 1;  bool P11_14: 1;
    	        bool P11_15: 1;  bool P11_16: 1;  bool P30_1: 1;  bool P30_2: 1;
    	        bool P30_4: 1;  bool P30_5: 1;  bool P30_6: 1;  bool P30_7: 1;
    };
} DB_Parameters_Bool_ID1_32;

typedef union {
    uint32_t all;
    struct {
    	bool P70_1 : 1;  bool P70_2 : 1;  bool P70_3 : 1;  bool P70_4 : 1;
    	        bool P70_5 : 1;  bool P70_6 : 1;  bool P70_7 : 1;  bool P70_8 : 1;
    	        bool P70_9 : 1;  bool P70_10 : 1;  bool P70_11: 1;  bool P70_12: 1;
    	        bool P70_13: 1;  bool P70_14: 1;  bool P21_1: 1;  bool P21_4: 1;
    	        bool P21_7: 1;  bool P21_10: 1;  bool P21_13: 1;  bool P21_16: 1;
    	        bool P30_3: 1;  bool bit22: 1;  bool bit23: 1;  bool bit24: 1;
    	        bool bit25: 1;  bool bit26: 1;  bool bit27: 1;  bool bit28: 1;
    	        bool bit29: 1;  bool bit30: 1;  bool bit31: 1;  bool bit32: 1;
    };
} DB_Parameters_Bool_ID33_64;

typedef union {
    uint32_t all;
    struct {
    	bool bit1 : 1;  bool bit2 : 1;  bool bit3 : 1;  bool bit4 : 1;
    	        bool bit5 : 1;  bool bit6 : 1;  bool bit7 : 1;  bool bit8 : 1;
    	        bool bit9 : 1;  bool bit10 : 1;  bool bit11: 1;  bool bit12: 1;
    	        bool bit13: 1;  bool bit14: 1;  bool bit15: 1;  bool bit16: 1;
    	        bool bit17: 1;  bool bit18: 1;  bool bit19: 1;  bool bit20: 1;
    	        bool bit21: 1;  bool bit22: 1;  bool bit23: 1;  bool bit24: 1;
    	        bool bit25: 1;  bool bit26: 1;  bool bit27: 1;  bool bit28: 1;
    	        bool bit29: 1;  bool bit30: 1;  bool bit31: 1;  bool bit32: 1;
    };
} DB_Parameters_Bool_ID65_96;

typedef struct __attribute__((packed)){
	DB_Parameters_Float_ID1_50 f50;
	DB_Parameters_Float_ID51_100 f100;
	DB_Parameters_Int_ID1_50   i50;
	DB_Parameters_UInt_ID1_50  u50;
	DB_Parameters_Bool_ID1_32  b32;
	DB_Parameters_Bool_ID33_64 b64;
	DB_Parameters_Bool_ID65_96 b96;
	uint32_t CRC32; // Поле для контрольной суммы (4 байта)
} DB_Parameters_Main;
// Это говорит компилятору: "Эти переменные есть, но их значения лежат в другом месте"
extern volatile DB_Parameters_Main DBParameters;
extern const DB_Parameters_Main DBParametersFactory;


#endif
