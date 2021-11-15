#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QtCore>

typedef quint8 qchar;
typedef quint16 qshort;
typedef quint32 quint;
typedef quint64 qlong;
typedef QByteArray qbyte;
typedef QString qstr;

using namespace std;

#define UFS_VENDOR_MICRON_MP   0x12C
#define UFS_VENDOR_MICRON_ES   0x02C
#define UFS_VENDOR_TOSHIBA     0x198
#define UFS_VENDOR_SAMSUNG     0x1CE
#define UFS_VENDOR_SKHYNIX     0x1AD

#define MTK_BLOADER_INFO_BEGIN	"MTK_BLOADER_INFO_v"

namespace mtkPreloader {

//!MTK_BLOADER_INFO_v08_EMMC
typedef struct EMIInfoV08
{
    struct
    {
        quint m_type{0x00};
        char m_emmc_id[12]{0x00};
        quint m_dram_rank_size[4]{1024*0124}; //FIX_ME
    } emi_cfg;
public:
    unsigned int emi_len[34];
} EMIInfoV08;

//!MTK_BLOADER_INFO_v10_EMMC
typedef struct EMIInfoV10 //MT6589 , MT8135
{
    struct
    {
        quint m_sub_ver{0x00}; //# Sub_version checking for flash tool
        quint m_type{0x00}; //#type
        quint m_id_length{0x00}; // # EMMC ID checking length
        quint fw_id_length{0x00}; //# FW ID checking length
        char m_emmc_id[16]{0x00}; //#id
        char m_fw_id[8]{0x00}; //  #fw id
        union {
            quint dramc0[17];//EMI settings len = 0x44
        };
        //end to end should be 0x4f
        quint m_dram_rank_size[4]{0x00};
        int     MMD;                              //MMD info, for 6589 just has two types MMD1 and MMD2.
        int     m_reserved[8];

    } emi_cfg;
public:
    unsigned int emi_len[46]; //bc000000
} EMIInfoV10;//https://github.com/cakehonolulu/android_kernel_bq_Aquaris5HD/blob/d08666e5144f8a1de123d46a63ff9c4442e31799/mediatek/build/tools/emigen/MT6589/emigen.pl

//!MTK_BLOADER_INFO_v11_EMMC
typedef struct EMIInfoV11
{
    struct
    {
        quint m_type{0x00};
        char m_emmc_id[16]{0x00};
        quint m_id_length{0x00};
        union {
            quint dramc0[30];
        };
        quint m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[42];
} EMIInfoV11;

//!MTK_BLOADER_INFO_v12_EMMC
typedef struct EMIInfoV12
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        union {
            quint dramc0[17];
        };
        quint m_dram_rank_size[4]{0x00};
        quint m_reserved0[10]{0x00};
        union DDR
        {
            struct _LPDDR1
            {
                unsigned int m_mode_reg;
                unsigned int m_ext_mode_reg;    // dram driving strength -> customized
            } lp_ddr1;

            struct _LPDDR2
            {
                unsigned int m_mode_reg_1;
                unsigned int m_mode_reg_2;
                unsigned int m_mode_reg_3;
                unsigned int m_mode_reg_5; //vendor ID
                unsigned int m_mode_reg_10;
                unsigned int m_mode_reg_63;
            } lp_ddr2;

            struct _LPDDR3
            {
                unsigned int m_mode_reg_1;
                unsigned int m_mode_reg_2;
                unsigned int m_mode_reg_3;
                unsigned int m_mode_reg_5; //vendor ID
                unsigned int m_mode_reg_10;
                unsigned int m_mode_reg_63;
            } lp_ddr3;

            struct _PCDDR3
            {
                unsigned int m_mode_reg_0;
                unsigned int m_mode_reg_1;
                unsigned int m_mode_reg_2;
                unsigned int m_mode_reg_3;
            } pc_ddr3;
        } m_ddr;
    } emi_cfg;
public:
    unsigned int emi_len[47];
} EMIInfoV12;

//!MTK_BLOADER_INFO_v13_EMMC
typedef struct EMIInfoV13
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        union {
            quint dramc0[17];
        };
        quint m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[47];
} EMIInfoV13;

//!MTK_BLOADER_INFO_v14_EMMC
typedef struct EMIInfoV14
{
    struct
    {
        quint m_type{0x00};
        char m_emmc_id[12]{0x00};
        quint m_dram_rank_size[4]{1024*0124}; //FIX_ME
    } emi_cfg;
public:
    unsigned int emi_len[42];
} EMIInfoV14;

//!MTK_BLOADER_INFO_v15_EMMC
typedef struct EMIInfoV15
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        quint m_reserved0[6]{0x00};//NO_IDEA!
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        union {
            quint dramc0[14];
        };
        quint m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[44];
} EMIInfoV15;

//!MTK_BLOADER_INFO_v16_EMMC
typedef struct EMIInfoV16
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        union {
            quint dramc0[17];
        };
        quint m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[47];
} EMIInfoV16;

//!MTK_BLOADER_INFO_v17_EMMC
typedef struct EMIInfoV17
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        union {
            quint dramc0[14];
        };
        quint m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[44];
} EMIInfoV17;

//!MTK_BLOADER_INFO_v18_EMMC
typedef struct EMIInfoV18  //for MT8590
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        union {
            quint dramc0[17];
        };
        quint m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[47];
} EMIInfoV18;

//!MTK_BLOADER_INFO_v19_EMMC
typedef struct EMIInfoV19  //for 8173
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        union {
            quint dramc0[14];
        };
        quint m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[44];
} EMIInfoV19;

//!MTK_BLOADER_INFO_v20_EMMC
typedef struct EMIInfoV20
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        union {
            quint dramc0[17];
        };
        quint m_dram_rank_size[4]{0x00};
        quint m_reserved[10]{0x00};
        union
            {
                struct
                {
                    quint LPDDR2_MODE_REG_1;
                    quint LPDDR2_MODE_REG_2;
                    quint LPDDR2_MODE_REG_3;
                    quint LPDDR2_MODE_REG_5;
                    quint LPDDR2_MODE_REG_10;
                    quint LPDDR2_MODE_REG_63;
                };
                struct
                {
                    quint DDR1_MODE_REG;
                    quint DDR1_EXT_MODE_REG;
                };
                struct
                {
                    quint PCDDR3_MODE_REG0;
                    quint PCDDR3_MODE_REG1;
                    quint PCDDR3_MODE_REG2;
                    quint PCDDR3_MODE_REG3;
                    quint PCDDR3_MODE_REG4;
                    quint PCDDR3_MODE_REG5;
                };
                struct
                {
                    quint LPDDR3_MODE_REG_1;
                    quint LPDDR3_MODE_REG_2;
                    quint LPDDR3_MODE_REG_3;
                    quint LPDDR3_MODE_REG_5;
                    quint LPDDR3_MODE_REG_10;
                    quint LPDDR3_MODE_REG_63;
                };
            };
    } emi_cfg;
public:
    unsigned int emi_len[47];
} EMIInfoV20;

//!MTK_BLOADER_INFO_v21_EMMC
typedef struct EMIInfoV21
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        union {
            quint dramc0[17];
        };
        quint m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[47];
} EMIInfoV21;

//!MTK_BLOADER_INFO_v22_EMMC
typedef struct EMIInfoV22
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        union {
            quint dramc0[14];
        };
        quint m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[44];
} EMIInfoV22;

//!MTK_BLOADER_INFO_v23_EMMC
typedef struct EMIInfoV23
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        union {
            quint dramc0[14];
        };
        quint m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[38];
} EMIInfoV23;

//!MTK_BLOADER_INFO_v24_EMMC
typedef struct EMIInfoV24 //FIX_ME
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        union {
            quint dramc0[14];
        };
        quint m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[44];
} EMIInfoV24;

//!MTK_BLOADER_INFO_v25_EMMC
typedef struct EMIInfoV25
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        union {
            quint dramc0[5]; //fix_me to detect the correct dram led fix this container.
        };
        quint m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[40];
} EMIInfoV25;

//!MTK_BLOADER_INFO_v27_EMMC
typedef struct EMIInfoV27
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        union {
            quint dramc0[17];
        };
        quint m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[47];
} EMIInfoV27;

//!MTK_BLOADER_INFO_v28_EMMC
typedef struct EMIInfoV28
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_reserved0[10]{0x00};//FIX!
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        union {
            quint dramc0[17];
        };
        quint m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[36+1];
} EMIInfoV28;

//!MTK_BLOADER_INFO_v30_EMMC
typedef struct EMIInfoV30
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        quint emi_cona_val{0x00};//@0x3000
        quint emi_conh_val{0x00};
        union {
            quint DRAMC_ACTIME_UNION[8];
            quint dramc0[8];//AC_TIMING_EXTERNAL_T AcTimeEMI;
        };
        qlong m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[40];
} EMIInfoV30;

//!MTK_BLOADER_INFO_v31_EMMC
typedef struct EMIInfoV31
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        quint emi_cona_val{0x00};//@0x3000
        quint emi_conh_val{0x00};
        union {
            quint DRAMC_ACTIME_UNION[8];
            quint AcTimeEMI[8];//AC_TIMING_EXTERNAL_T AcTimeEMI;
        };
        qlong m_dram_rank_size[4]{0x00};
    } emi_cfg;//FOR_DV_SIMULATION_USED
public:
    unsigned int emi_len[40];
} EMIInfoV31;

//!MTK_BLOADER_INFO_v32_EMMC
typedef struct EMIInfoV32
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        union {
            quint dramc0[17];
        };
        quint m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[47];
} EMIInfoV32;

//!MTK_BLOADER_INFO_v35_EMMC
typedef struct EMIInfoV35
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        quint emi_cona_val{0x00};//@0x3000
        quint emi_conh_val{0x00};
        union {
            quint DRAMC_ACTIME_UNION[8];
            quint AcTimeEMI[8];//AC_TIMING_EXTERNAL_T AcTimeEMI;
        };
        qlong m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[40];
} EMIInfoV35;

//!MTK_BLOADER_INFO_v36_EMMC
typedef struct EMIInfoV36
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        quint emi_cona_val{0x00};//@0x3000
        quint emi_conh_val{0x00};
        union {
            quint DRAMC_ACTIME_UNION[8];
            quint AcTimeEMI[8];//AC_TIMING_EXTERNAL_T AcTimeEMI;
        };
        qlong m_dram_rank_size[4]{0x00}; //!# combo rule in preloader must support same emmc id with different rank size
    } emi_cfg;
public:
    unsigned int emi_len[40];
} EMIInfoV36;

//!MTK_BLOADER_INFO_v38_EMMC
typedef struct EMIInfoV38
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        quint emi_cona_val{0x00};//@0x3000
        quint emi_conh_val{0x00};
        union {
            quint DRAMC_ACTIME_UNION[8];
            quint AcTimeEMI[8];//AC_TIMING_EXTERNAL_T AcTimeEMI;
        };
        qlong m_dram_rank_size[4]{0x00};
    } emi_cfg;//!custom/evb6763_64_emmc/inc/custom_MemoryDevice.h
public:
    unsigned int emi_len[40];
} EMIInfoV38;

//!MTK_BLOADER_INFO_v39_eMMC + UFS //COMBO_BEGIN
typedef struct EMIInfoV39
{
    //!H9HQ16AFAMMDAR / H9HCNNNFAMMLXR-NEE / K4UCE3Q4AA-MGCR - 8GB (4+4) Byte Mode
    struct //MT29VZZZAD8GQFSL-046 - 4GB -Normal mode (4+0), //MT53E2G32D4 - 8GB (4+4) Normal Mode
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_emmc_id[16]{0x00};
        char m_fw_id[8]{0x00};
        quint emi_cona_val{0x00};/* EMI_CONA_VAL */
        quint emi_conh_val{0x00}; /* EMI_CONH_VAL */
        union {
            quint DRAMC_ACTIME_UNION[8];
            //0x00000000,/* U 00 */
            //0x00000000,/* U 01 */
            //0x00000000,/* U 02 */
            //0x00000000,/* U 03 */
            //0x00000000,/* U 04 */
            //0x00000000,/* U 05 */
            //0x00000000,/* U 06 */
            //0x00000000,/* U 07 */
        };
        qlong m_dram_rank_size[4]{0x00};
    } emi_cfg;//!custom/evb6763_64_emmc/inc/custom_MemoryDevice.h
public:
    unsigned int emi_len[40];
} EMIInfoV39;

//!MTK_BLOADER_INFO_v49_UFS + V52
typedef struct EMIInfoV49 // H9HCNNNFAMMLXR-NEE / K4UCE3Q4AA-MGCR - 8GB (4+4) Byte Mode
{
    struct
    {
        quint m_type{0x00};
        quint m_id_length{0x00};
        char m_ufs_id[16]{0x00};
        qlong m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[21];
} EMIInfoV49;

//!MTK_BLOADER_INFO_v46_UFS
typedef struct EMIInfoV46
{
    struct
    {
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint m_reserved0[8]{0x00};//FIX!
        char m_ufs_id[16]{0x00};
        qlong m_dram_rank_size[4]{0x00};
    } emi_cfg;
public:
    unsigned int emi_len[21];
} EMIInfoV46;

//!MTK_BLOADER_INFO_v51_UFS
typedef struct EMIInfoV51 //FIX_ME
{
    struct
    {
        quint m_sub_ver{0x00};
        quint m_type{0x00};
        quint m_id_length{0x00};
        quint fw_id_length{0x00};
        char m_ufs_id[16]{0x00};
        char m_fw_id[8]{0x00};
        quint emi_cona_val{0x00};/* EMI_CONA_VAL */
        quint emi_conh_val{0x00}; /* EMI_CONH_VAL */
        union {
            quint DRAMC_ACTIME_UNION[8];
            //0x00000000,/* U 00 */
            //0x00000000,/* U 01 */
            //0x00000000,/* U 02 */
            //0x00000000,/* U 03 */
            //0x00000000,/* U 04 */
            //0x00000000,/* U 05 */
            //0x00000000,/* U 06 */
            //0x00000000,/* U 07 */
        };
        qlong m_dram_rank_size[4]{0x00};
    } emi_cfg;//!custom/evb6763_64_emmc/inc/custom_MemoryDevice.h
public:
    unsigned int emi_len[40];
} EMIInfoV51;

typedef struct MTKEMIInfo
{
    union
    {
        EMIInfoV08 emi_v08;
        EMIInfoV10 emi_v10;
        EMIInfoV11 emi_v11;
        EMIInfoV12 emi_v12;
        EMIInfoV13 emi_v13;
        EMIInfoV14 emi_v14;
        EMIInfoV15 emi_v15;
        EMIInfoV16 emi_v16;
        EMIInfoV17 emi_v17;
        EMIInfoV18 emi_v18;
        EMIInfoV19 emi_v19;

        EMIInfoV20 emi_v20;
        EMIInfoV21 emi_v21;
        EMIInfoV22 emi_v22;
        EMIInfoV23 emi_v23;

        EMIInfoV24 emi_v24;
        EMIInfoV25 emi_v25;
        EMIInfoV27 emi_v27;
        EMIInfoV28 emi_v28;

        EMIInfoV30 emi_v30;
        EMIInfoV31 emi_v31;
        EMIInfoV32 emi_v32;
        EMIInfoV35 emi_v35;
        EMIInfoV36 emi_v36;
        EMIInfoV38 emi_v38;
        EMIInfoV39 emi_v39;

        EMIInfoV46 emi_v46;
        EMIInfoV49 emi_v49;
        EMIInfoV51 emi_v51;

    } emi_cfg = {};
public:
    qstr index{};
    qstr flash_id{};
    qstr manufacturer_id{};
    qstr manufacturer{};
    qstr ProductName{};
    qstr OEMApplicationId{};
    qstr CardBGA{};
    qstr dram_type{};
    qstr dram_size{};
    qbyte m_emi_info{};
    qsizetype m_emi_ver{};
}MTKEMIInfo;

typedef struct
{
    quint magic;
    qshort size;
    qshort type;
    qint8 id[12];
    quint file_version;
    qshort file_type;
    qint8 flash_dev;
    qint8 sig_type;
    quint load_addr;
    quint length;
    quint max_size;
    quint content_offset;
    quint sig_length;
    quint jump_offset;
    quint addr;
} gfh_info_t;
}

namespace mmcCARD {

typedef struct emmc_card_info_cid_t
{
public:
    qchar mid{0x00}; /* Manufacturer ID */
    qchar cbx{0x00}; /* Reserved(6)+Card/BGA(2) */ //- Only lower 2 bit valid
    qchar oid{0x00}; /* OEM/Application ID */
    qchar pnm0{0x00}; /* Product name [0] */
    qchar pnm1{0x00}; /* Product name [1] */
    qchar pnm2{0x00}; /* Product name [2] */
    qchar pnm3{0x00}; /* Product name [3] */
    qchar pnm4{0x00}; /* Product name [4] */
    qchar pnm5{0x00}; /* Product name [5] */
    qchar pdrv{0x00}; /* Product revision */
    qchar psn0{0x00}; /* Serial Number [0] */
    qchar psn1{0x00}; /* Serial Number [1] */
    qchar psn2{0x00}; /* Serial Number [2] */
    qchar psn3{0x00}; /* Serial Number [3] */
    qchar mdt{0x00}; /* Manufacturer date */
    qchar crc7{0x00}; /* CRC7 + stuff bit*/  //--Only top 7 bit
}mmc_cid_t;

typedef struct CIDInfo
{
    qstr ManufacturerId{};
    qstr Manufacturer{};
    qstr CardBGA{};
    qstr OEMApplicationId{};
    qstr ProductName{};
    qstr ProductRevision{};
    qstr ProductSerialNumber{};
    qstr ManufacturingDate{};
}CIDInfo;
}
#endif // STRUCTURES_H
