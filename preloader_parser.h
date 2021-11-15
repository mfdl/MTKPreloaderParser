#ifndef PRELOADER_PARSER_H
#define PRELOADER_PARSER_H

#include "emi_structures.h"

class EMIParser
{
public:
    EMIParser(){}
    ~EMIParser(){};

    static bool PrasePreloader(QIODevice &emi_dev, QVector<mtkPreloader::MTKEMIInfo> &emis);
    static void PraseCID(qbyte raw_cid, mmcCARD::CIDInfo &cid_info, bool ufs_id = 0);
    static qstr GetEMIFlashDev(qbyte emi_buf);
private:
    static qstr get_pl_sig_type(qchar sig_type);
    static qstr get_pl_flash_dev(qchar flash_dev);
    static qstr get_dram_type(quint16 type);
    static qstr get_card_mfr_id(qchar mid);
    static qstr get_card_type(qchar type);
    static qstr get_unit(qlong bytes);
    static qstr get_hex(qlong num);
};

#endif // PRELOADER_PARSER_H
