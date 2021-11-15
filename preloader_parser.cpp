#include "preloader_parser.h"

bool EMIParser::PrasePreloader(QIODevice &emi_dev, QVector<mtkPreloader::MTKEMIInfo> &emis)
{
    if (!emi_dev.seek(0x00))
        return 0;

    mtkPreloader::gfh_info_t gfh_info = {};
    if (!emi_dev.read((char*)&gfh_info, sizeof(gfh_info)))
        return 0;

    if (gfh_info.length == 0
            || (gfh_info.magic != 0x14d4d4d //!PRELOADER
                && gfh_info.magic != 0x5f4b544d //!MTK_BLOADER_INFO
                && gfh_info.magic != 0x434d4d45 //!EMMC_BOOT0
                && gfh_info.magic != 0x5f534655)) //!UFS_LUN0
    {
        qInfo().noquote() << qstr("invalid/unsupported mtk_boot_region file format{%0}").arg(get_hex(gfh_info.magic));
        return 0;
    }

    qint64 emi_idx = 0x00;
    if (gfh_info.magic == 0x434d4d45
            || gfh_info.magic == 0x5f534655) //!MTK_BOOT_REGION!
    {
        qsizetype seek_off = (gfh_info.magic == 0x5f534655)?0x1000: 0x800; //UFS_LUN & EMMC_BOOT
        if (!emi_dev.seek(seek_off))
            return 0;

        memset(&gfh_info, 0x00, sizeof(gfh_info));
        if (!emi_dev.read((char*)&gfh_info, sizeof(gfh_info)))
            return 0;

        if (gfh_info.length == 0
                || gfh_info.magic != 0x14d4d4d) //!MTK_PRELOADER_MAGIC!
        {
            qInfo().noquote() << qstr("invalid/unsupported mtk_boot_region data{%0}").arg(get_hex(gfh_info.magic));
            return 0;
        }

        if (!emi_dev.seek(0x00))
            return 0;

        emi_idx = emi_dev.readAll().indexOf(MTK_BLOADER_INFO_BEGIN);
        if (emi_idx == -1)
        {
            qInfo().noquote() << qstr("invalid/unsupported mtk_boot_region data{%0}").arg(get_hex(gfh_info.magic));
            return 0;
        }

        if (!emi_dev.seek(0x00))
            return 0;
    }

    qbyte BldrInfo = {};
    qstr platform = {"MT6752"};
    qstr flash_dev = get_pl_flash_dev(gfh_info.flash_dev);

    if (gfh_info.magic == 0x5f4b544d) //!MTK_BLOADER_INFO!
    {
        BldrInfo.resize(emi_dev.size());

        if (!emi_dev.seek(0x00))
            return 0;
        if (!emi_dev.read(BldrInfo.data(), BldrInfo.size()))
            return 0;

        platform = GetEMIFlashDev(BldrInfo);
    }
    else
    {
        if (gfh_info.length == 0
                || gfh_info.magic != 0x14d4d4d) //!MTK_PRELOADER_MAGIC!
        {
            qInfo().noquote() << qstr("invalid/unsupported mtk_boot_region data{%0}").arg(get_hex(gfh_info.magic));
            return 0;
        }

        if (!emi_dev.seek(0x00))
            return 0;

        qbyte prl_info = emi_dev.readAll();
        platform = GetEMIFlashDev(prl_info);

        if (!emi_dev.seek(0x00))
            return 0;

        quint emilength = 0x1000; //!MAX_EMI_LEN
        quint emi_loc = gfh_info.length - gfh_info.sig_length - sizeof(quint);

        if (emi_idx == 0x00)
        {
            if (!emi_dev.seek(emi_loc))
                return 0;

            if (!emi_dev.read((char*)&emilength, sizeof(quint)))
                return 0;

            if (emilength == 0)
            {
                qInfo().noquote() << qstr("invalid/unsupported mtk_bloader_info data{%0}").arg(get_hex(emi_loc));
                return 0;
            }

            emi_idx = emi_loc - emilength;
        }

        BldrInfo.resize(emilength);
        if (!emi_dev.seek(emi_idx))
            return 0;
        if (!emi_dev.read(BldrInfo.data(), BldrInfo.size()))
            return 0;
    }

    struct MTKBLOADERINFO
    {
        char m_identifier[0x1b]{0x00};
        char m_filename[0x3d]{0x00};
        quint m_version{0x00}; //V116
        quint m_chksum_seed{0x00}; //22884433
        quint m_start_addr{0x00}; //90007000
        char m_bin_identifier[8]{0x00}; //MTK_BIN
        quint m_num_emi_settings{0x00}; //!# number of emi settings.
    } bldr = {};
    memcpy(&bldr, BldrInfo.data(), sizeof(bldr));
    qbyte emi_hdr((char*)bldr.m_identifier , sizeof(bldr.m_identifier ));
    qbyte project_id((char*)bldr.m_filename, sizeof(bldr.m_filename));

    qInfo().noquote() << qstr("EMIInfo{%0}:%1:%2:%3:num_records[%4]").arg(emi_hdr.data(),
                                                                          platform,
                                                                          flash_dev,
                                                                          project_id,
                                                                          get_hex(bldr.m_num_emi_settings));

    if (!emi_hdr.startsWith(MTK_BLOADER_INFO_BEGIN))
    {
        qInfo().noquote() << qstr("invalid/unsupported mtk_emi_info{%0}").arg(emi_hdr.data());
        return 0;
    }

    QFile BLDRINFO(emi_hdr);
    BLDRINFO.open(QIODevice::WriteOnly);
    BLDRINFO.write(BldrInfo);
    BLDRINFO.close();

    emi_hdr.remove(0, 0x12);
    quint8 emi_ver = emi_hdr.toInt(nullptr, 0xa);

    qInfo(".....................................................");

    qsizetype idx = sizeof(bldr);
    for (uint i = 0; i < bldr.m_num_emi_settings; i++)
    {
        mtkPreloader::MTKEMIInfo emi = {};
        emi.m_emi_ver = emi_ver;

        if (emi_ver == 0x08)
        {
            memcpy(&emi.emi_cfg.emi_v08, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v08.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v08.emi_len);
            if (!emi.emi_cfg.emi_v08.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v08.emi_cfg, sizeof(emi.emi_cfg.emi_v08.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)emi.emi_cfg.emi_v08.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v08.emi_cfg.m_emmc_id));
//            dev_id.resize(emi.emi_cfg.emi_v08.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v08.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v08.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v08.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v08.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v08.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0xa)
        {
            memcpy(&emi.emi_cfg.emi_v10, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v10.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v10.emi_len);
            if (!emi.emi_cfg.emi_v10.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v10.emi_cfg, sizeof(emi.emi_cfg.emi_v10.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v10.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v10.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v10.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v10.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v10.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v10.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v10.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v10.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0xb)
        {
            memcpy(&emi.emi_cfg.emi_v11, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v11.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v11.emi_len);
            if (!emi.emi_cfg.emi_v11.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v11.emi_cfg, sizeof(emi.emi_cfg.emi_v11.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v11.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v11.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v11.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v11.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v11.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v11.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v11.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v11.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0xc)
        {
            memcpy(&emi.emi_cfg.emi_v12, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v12.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v12.emi_len);
            if (!emi.emi_cfg.emi_v12.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v12.emi_cfg, sizeof(emi.emi_cfg.emi_v12.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v12.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v12.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v12.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v12.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v12.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v12.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v12.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v12.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0xd)
        {
            memcpy(&emi.emi_cfg.emi_v13, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v13.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v13.emi_len);
            if (!emi.emi_cfg.emi_v13.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v13.emi_cfg, sizeof(emi.emi_cfg.emi_v13.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v13.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v13.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v13.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v13.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v13.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v13.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v13.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v13.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0xe) //combo => (TODO) for NAND type. //gfh_info.flash_dev != 0x5
        {
            memcpy(&emi.emi_cfg.emi_v14, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v14.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v14.emi_len);
            if (!emi.emi_cfg.emi_v14.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v14.emi_cfg, sizeof(emi.emi_cfg.emi_v14.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v14.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v14.emi_cfg.m_emmc_id));
            //dev_id.resize(emi.emi_cfg.emi_v14.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v14.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v14.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v14.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v14.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v14.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0xf) //FIX_ME . wired flash id's =>4B 47 FD 77 00 00 00 11 03 84 04 00 B1 53 00 00
        {
            memcpy(&emi.emi_cfg.emi_v15, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v15.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v15.emi_len);
            if (!emi.emi_cfg.emi_v15.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v15.emi_cfg, sizeof(emi.emi_cfg.emi_v15.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v15.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v15.emi_cfg.m_emmc_id));
            //dev_id.resize(emi.emi_cfg.emi_v15.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v15.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v15.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v15.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v15.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v15.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x10)
        {
            memcpy(&emi.emi_cfg.emi_v16, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v16.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v16.emi_len);
            if (!emi.emi_cfg.emi_v16.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v16.emi_cfg, sizeof(emi.emi_cfg.emi_v16.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v16.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v16.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v16.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v16.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v16.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v16.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v16.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v16.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x11)
        {
            memcpy(&emi.emi_cfg.emi_v17, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v17.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v17.emi_len);
            if (!emi.emi_cfg.emi_v17.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v17.emi_cfg, sizeof(emi.emi_cfg.emi_v17.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v17.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v17.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v17.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v17.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v17.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v17.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v17.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v17.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x12)
        {
            memcpy(&emi.emi_cfg.emi_v18, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v18.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v18.emi_len);
            if (!emi.emi_cfg.emi_v18.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v18.emi_cfg, sizeof(emi.emi_cfg.emi_v18.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v18.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v18.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v18.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v18.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v18.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v18.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v18.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v18.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x13)
        {
            memcpy(&emi.emi_cfg.emi_v19, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v19.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v19.emi_len);
            if (!emi.emi_cfg.emi_v19.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v19.emi_cfg, sizeof(emi.emi_cfg.emi_v19.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v19.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v19.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v19.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v19.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v19.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v19.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v19.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v19.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x14)
        {
            memcpy(&emi.emi_cfg.emi_v20, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v20.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v20.emi_len);
            if (!emi.emi_cfg.emi_v20.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v20.emi_cfg, sizeof(emi.emi_cfg.emi_v20.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v20.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v20.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v20.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v20.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v20.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v20.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v20.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v20.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x15)
        {
            memcpy(&emi.emi_cfg.emi_v21, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v21.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v21.emi_len);
            if (!emi.emi_cfg.emi_v21.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v21.emi_cfg, sizeof(emi.emi_cfg.emi_v21.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v21.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v21.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v21.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v21.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v21.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v21.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v21.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v21.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x16)
        {
            memcpy(&emi.emi_cfg.emi_v22, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v22.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v22.emi_len);
            if (!emi.emi_cfg.emi_v22.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v22.emi_cfg, sizeof(emi.emi_cfg.emi_v22.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v22.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v22.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v22.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v22.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v22.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v22.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v22.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v22.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x17)
        {
            memcpy(&emi.emi_cfg.emi_v23, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v23.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v23.emi_len);
            if (!emi.emi_cfg.emi_v23.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v23.emi_cfg, sizeof(emi.emi_cfg.emi_v23.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v23.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v23.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v23.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v23.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v23.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v23.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v23.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v23.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x18)
        {
            memcpy(&emi.emi_cfg.emi_v24, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v24.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v24.emi_len);
            if (!emi.emi_cfg.emi_v24.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v24.emi_cfg, sizeof(emi.emi_cfg.emi_v24.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v24.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v24.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v24.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v24.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v24.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v24.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v24.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v24.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x19)
        {
            memcpy(&emi.emi_cfg.emi_v25, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v25.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v25.emi_len);
            if (!emi.emi_cfg.emi_v25.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v25.emi_cfg, sizeof(emi.emi_cfg.emi_v25.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v25.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v25.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v25.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v25.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v25.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v25.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v25.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v25.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x1b)
        {
            memcpy(&emi.emi_cfg.emi_v27, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v27.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v27.emi_len);
            if (!emi.emi_cfg.emi_v27.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v27.emi_cfg, sizeof(emi.emi_cfg.emi_v27.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v27.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v27.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v27.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v27.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v27.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v27.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v27.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v27.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x1c)
        {
            memcpy(&emi.emi_cfg.emi_v28, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v28.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v28.emi_len);
            if (!emi.emi_cfg.emi_v28.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v28.emi_cfg, sizeof(emi.emi_cfg.emi_v28.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v28.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v28.emi_cfg.m_emmc_id));
            //dev_id.resize(emi.emi_cfg.emi_v28.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v28.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v28.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v28.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v28.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v28.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x1e)
        {
            memcpy(&emi.emi_cfg.emi_v30, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v30.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v30.emi_len);
            if (!emi.emi_cfg.emi_v30.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v30.emi_cfg, sizeof(emi.emi_cfg.emi_v30.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v30.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v30.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v30.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v30.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v30.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v30.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v30.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v30.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x1f)
        {
            memcpy(&emi.emi_cfg.emi_v31, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v31.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v31.emi_len);
            if (!emi.emi_cfg.emi_v31.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v31.emi_cfg, sizeof(emi.emi_cfg.emi_v31.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v31.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v31.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v31.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v31.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v31.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v31.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v31.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v31.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x20)
        {
            memcpy(&emi.emi_cfg.emi_v32, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v32.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v32.emi_len);
            if (!emi.emi_cfg.emi_v32.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v32.emi_cfg, sizeof(emi.emi_cfg.emi_v32.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v32.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v32.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v32.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v32.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v32.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v32.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v32.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v32.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x23)
        {
            memcpy(&emi.emi_cfg.emi_v35, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v35.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v35.emi_len);
            if (!emi.emi_cfg.emi_v35.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v35.emi_cfg, sizeof(emi.emi_cfg.emi_v35.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v35.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v35.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v35.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v35.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v35.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v35.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v35.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v35.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x24)
        {
            memcpy(&emi.emi_cfg.emi_v36, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v36.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v36.emi_len);
            if (!emi.emi_cfg.emi_v36.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v36.emi_cfg, sizeof(emi.emi_cfg.emi_v36.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v36.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v36.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v36.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v36.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v36.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v36.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v36.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v36.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x26)
        {
            memcpy(&emi.emi_cfg.emi_v38, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v38.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v38.emi_len);
            if (!emi.emi_cfg.emi_v38.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v38.emi_cfg, sizeof(emi.emi_cfg.emi_v38.emi_cfg))); //fixed_len

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v38.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v38.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v38.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v38.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v38.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v38.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v38.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v38.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x27
                || emi_ver == 0x28
                || emi_ver == 0x2d
                || emi_ver == 0x2f) //MTK_BLOADER_INFO_v39 => MTK EMI V2 combo mode. !common.
        {
            memcpy(&emi.emi_cfg.emi_v39, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v39.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v39.emi_len);
            if (!emi.emi_cfg.emi_v39.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v39.emi_cfg, sizeof(emi.emi_cfg.emi_v39.emi_cfg))); //fixed_len

            bool is_ufs(emi.emi_cfg.emi_v39.emi_cfg.m_id_length != 0x9);//len = 0x9 = eMMC & 0xe, 0xf = eUFS

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v39.emi_cfg.m_emmc_id, sizeof(emi.emi_cfg.emi_v39.emi_cfg.m_emmc_id));
            dev_id.resize(emi.emi_cfg.emi_v39.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid, is_ufs);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v39.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v39.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v39.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v39.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v39.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x2e) //MTK_BLOADER_INFO_v46
        {
            memcpy(&emi.emi_cfg.emi_v46, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v46.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v46.emi_len);
            if (!emi.emi_cfg.emi_v46.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v46.emi_cfg, sizeof(emi.emi_cfg.emi_v46.emi_cfg))); //fixed_len

            bool is_ufs(emi.emi_cfg.emi_v46.emi_cfg.m_id_length != 0x9);//len = 0x9 = eMMC & 0xe, 0xf = eUFS

            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v46.emi_cfg.m_ufs_id, sizeof(emi.emi_cfg.emi_v46.emi_cfg.m_ufs_id));
            dev_id.resize(emi.emi_cfg.emi_v46.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid, is_ufs);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v46.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v46.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v46.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v46.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v46.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x31 || emi_ver == 0x34 || emi_ver == 0x36) //MTK_BLOADER_INFO_v49 - MTK_BLOADER_INFO_v52 - MTK_BLOADER_INFO_v54
        {
            memcpy(&emi.emi_cfg.emi_v49, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v49.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v49.emi_len);
            if (!emi.emi_cfg.emi_v49.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v49.emi_cfg, sizeof(emi.emi_cfg.emi_v49.emi_cfg))); //fixed_len

            bool is_ufs(emi.emi_cfg.emi_v49.emi_cfg.m_id_length != 0x9);//len = 0x9 = eMMC & 0xe, 0xf = eUFS
            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v49.emi_cfg.m_ufs_id, sizeof(emi.emi_cfg.emi_v49.emi_cfg.m_ufs_id));
            dev_id.resize(emi.emi_cfg.emi_v49.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid, is_ufs);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v49.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v49.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v49.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v49.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v49.emi_cfg.m_dram_rank_size[3]);
        }
        else if(emi_ver == 0x33) //MTK_BLOADER_INFO_v51
        {
            memcpy(&emi.emi_cfg.emi_v51, BldrInfo.mid(idx).data(), sizeof(emi.emi_cfg.emi_v51.emi_cfg));
            idx += sizeof(emi.emi_cfg.emi_v51.emi_len);
            if (!emi.emi_cfg.emi_v51.emi_cfg.m_type)
                continue;

            emi.m_emi_info = (qbyte((char*)&emi.emi_cfg.emi_v51.emi_cfg, sizeof(emi.emi_cfg.emi_v51.emi_cfg))); //fixed_len

            bool is_ufs(emi.emi_cfg.emi_v51.emi_cfg.m_id_length != 0x9);//len = 0x9 = eMMC & 0xe, 0xf = eUFS
            qbyte dev_id = qbyte((char*)&emi.emi_cfg.emi_v51.emi_cfg.m_ufs_id, sizeof(emi.emi_cfg.emi_v51.emi_cfg.m_ufs_id));
            dev_id.resize(emi.emi_cfg.emi_v51.emi_cfg.m_id_length);

            mmcCARD::CIDInfo m_cid = {};
            PraseCID(dev_id, m_cid, is_ufs);

            emi.index = get_hex(i);
            emi.flash_id = dev_id.toHex().data();
            emi.manufacturer_id = m_cid.ManufacturerId;
            emi.manufacturer = m_cid.Manufacturer;
            emi.ProductName = m_cid.ProductName;
            emi.OEMApplicationId = m_cid.OEMApplicationId;
            emi.CardBGA = m_cid.CardBGA;
            emi.dram_type = get_dram_type(emi.emi_cfg.emi_v51.emi_cfg.m_type);
            emi.dram_size = get_unit(emi.emi_cfg.emi_v51.emi_cfg.m_dram_rank_size[0] +
                    emi.emi_cfg.emi_v51.emi_cfg.m_dram_rank_size[1] +
                    emi.emi_cfg.emi_v51.emi_cfg.m_dram_rank_size[2] +
                    emi.emi_cfg.emi_v51.emi_cfg.m_dram_rank_size[3]);
        }
        else
        {
            qInfo().noquote() << qstr("EMI version not supported{%0}").arg(get_hex(emi_ver));
            return 0;
        }

        if (!emi.flash_id.size())
            continue;

        emis.push_back(emi);
    }

    return 0;
}

qstr EMIParser::GetEMIFlashDev(qbyte emi_buf)
{
    qstr emi_dev = {"MT6752"};
    qbyte serach0("AND_ROMINFO_v");
    qsizetype idx0 = emi_buf.indexOf(serach0);
    if (idx0 != -1)
        emi_dev = emi_buf.mid(idx0 + 20, 6);

    if (emi_dev == "MT6752")
    {
        qbyte serach1("bootable/bootloader/preloader/platform/mt");
        qsizetype idx1 = emi_buf.indexOf(serach1);
        if (idx1 != -1)
            emi_dev = emi_buf.mid(idx1 + serach1.length() - 2, 6);

        qbyte serach2("preloader_");
        qsizetype idx2 = emi_buf.indexOf(serach2);
        if (idx2 != -1)
        {
            qstr emi_dev = emi_buf.mid(idx2 + serach2.length(), sizeof(quint));
            QRegExp regex0("67(\\d+)");
            if (regex0.indexIn(emi_dev)!= -1)
            {
                qsizetype dev_id = regex0.cap(1).toInt();
                emi_dev = qstr("MT67%0").arg(dev_id);
            }

            QRegExp regex1("65(\\d+)");
            if (regex1.indexIn(emi_dev)!= -1)
            {
                qsizetype dev_id = regex1.cap(1).toInt();
                emi_dev = qstr("MT65%0").arg(dev_id);
            }
        }
    }

    if (emi_dev == "MT6752" && emi_buf.contains(MTK_BLOADER_INFO_BEGIN))
    {
        qbyte emi_tag = emi_buf.mid(emi_buf.indexOf(MTK_BLOADER_INFO_BEGIN), 0x14);

        if (emi_tag == "MTK_BLOADER_INFO_v00")
            emi_dev = "MT6595/MT6797";
        if (emi_tag == "MTK_BLOADER_INFO_v04")
            emi_dev = "MT6516";
        if (emi_tag == "MTK_BLOADER_INFO_v07")
            emi_dev = "MT6573";
        if (emi_tag == "MTK_BLOADER_INFO_v08")
            emi_dev = "MT6575/MT6577";
        if (emi_tag == "MTK_BLOADER_INFO_v10")
            emi_dev = "MT6589/MT8135";
        if(emi_tag == "MTK_BLOADER_INFO_v11")
            emi_dev = "MT6572";
        if (emi_tag == "MTK_BLOADER_INFO_v12")
            emi_dev = "MT6582";
        if (emi_tag == "MTK_BLOADER_INFO_v13")
            emi_dev = "MT6592/MT8127";
        if (emi_tag == "MTK_BLOADER_INFO_v20")
            emi_dev = "MT6735";
        if (emi_tag == "MTK_BLOADER_INFO_v21")
            emi_dev = "MT6580";
        if (emi_tag == "MTK_BLOADER_INFO_v22")
            emi_dev = "MT6755";
        if (emi_tag == "MTK_BLOADER_INFO_v25")
            emi_dev = "MT6757";
        if (emi_tag == "MTK_BLOADER_INFO_v27")
            emi_dev = "MT6570";
        if (emi_tag == "MTK_BLOADER_INFO_v28")
            emi_dev = "MT8167";
        if (emi_tag == "MTK_BLOADER_INFO_v30")
            emi_dev = "MT6763";
        if (emi_tag == "MTK_BLOADER_INFO_v31")
            emi_dev = "MT6758";
        if (emi_tag == "MTK_BLOADER_INFO_v32")
            emi_dev = "MT6739";
        if (emi_tag == "MTK_BLOADER_INFO_v35")
            emi_dev = "MT6765";
        if (emi_tag == "MTK_BLOADER_INFO_v36")
            emi_dev = "MT6771";
        if (emi_tag == "MTK_BLOADER_INFO_v38")
            emi_dev = "MT6761";
        if (emi_tag == "MTK_BLOADER_INFO_v39")
            emi_dev = "MT6779";
        if (emi_tag == "MTK_BLOADER_INFO_v40")
            emi_dev = "MT6768";
        if (emi_tag == "MTK_BLOADER_INFO_v45")
            emi_dev = "MT6785";
        if (emi_tag == "MTK_BLOADER_INFO_v46")
            emi_dev = "MT6883/MT6885/MT6889";
        if (emi_tag == "MTK_BLOADER_INFO_v47")
            emi_dev = "MT6873/MT6875";
        if (emi_tag == "MTK_BLOADER_INFO_v49")
            emi_dev = "MT6853";
        if (emi_tag == "MTK_BLOADER_INFO_v51")
            emi_dev = "MT6893";
        if (emi_tag == "MTK_BLOADER_INFO_v52")
            emi_dev = "MT6833";
        if (emi_tag == "MTK_BLOADER_INFO_v54")
            emi_dev = "MT6877";
    }

    return emi_dev.toUpper();
}

void EMIParser::PraseCID(qbyte raw_cid, mmcCARD::CIDInfo &cid_info, bool ufs_id)
{
    if (ufs_id)
    {
        if (raw_cid.startsWith("KM"))
        {
            cid_info.Manufacturer = "Samsung";
            cid_info.ManufacturerId = "0x1CE"; //wmanufacturerid
        }
        else if (raw_cid.startsWith("H9"))
        {
            cid_info.Manufacturer = "SkHynix";
            cid_info.ManufacturerId = "0x1AD";
        }
        else if (raw_cid.startsWith("MT"))
        {
            cid_info.Manufacturer = "Micron";
            cid_info.ManufacturerId = "0x12C";
        }
        else if (raw_cid.startsWith("Z"))
        {
            cid_info.Manufacturer = "Micron";
            cid_info.ManufacturerId = "0x02C";
        }
        else if (raw_cid.startsWith("TH"))
        {
            cid_info.Manufacturer = "TOSHIBA";
            cid_info.ManufacturerId = "0x198";
        }

        cid_info.ProductName = raw_cid.data();
        cid_info.OEMApplicationId = get_hex(raw_cid.toHex().mid(0, 4).toUShort(0, 0x10));//0000;
        cid_info.CardBGA = "eUFS";
    }
    else
    {
        mmcCARD::emmc_card_info_cid_t m_cid = {};
        memset(&m_cid, 0x00, sizeof(m_cid));
        memcpy(&m_cid, raw_cid.data(), sizeof(m_cid));

        cid_info = {};
        struct {
            char pnm[6]{0x00};
        } _pnm = {};
        memcpy(&_pnm.pnm[0], &m_cid.pnm0, sizeof(_pnm.pnm[0]));
        memcpy(&_pnm.pnm[1], &m_cid.pnm1, sizeof(_pnm.pnm[1]));
        memcpy(&_pnm.pnm[2], &m_cid.pnm2, sizeof(_pnm.pnm[2]));
        memcpy(&_pnm.pnm[3], &m_cid.pnm3, sizeof(_pnm.pnm[3]));
        memcpy(&_pnm.pnm[4], &m_cid.pnm4, sizeof(_pnm.pnm[4]));
        memcpy(&_pnm.pnm[5], &m_cid.pnm5, sizeof(_pnm.pnm[5]));
        QByteArray pnm((char*)&_pnm, sizeof(_pnm));

        struct {
            char psn[4]{0x00};
        } _psn = {};
        memcpy(&_psn.psn[0], &m_cid.psn0, sizeof(_psn.psn[0]));
        memcpy(&_psn.psn[1], &m_cid.psn1, sizeof(_psn.psn[1]));
        memcpy(&_psn.psn[2], &m_cid.psn2, sizeof(_psn.psn[2]));
        memcpy(&_psn.psn[3], &m_cid.psn3, sizeof(_psn.psn[3]));
        QByteArray psn((char*)&_psn, sizeof(_psn));

        QString prv_min(QString().sprintf("%d", (qchar)(m_cid.pdrv >> 4)));
        QString prv_maj(QString().sprintf("%d", (qchar)(m_cid.pdrv & 0xf)));
        QString mdt_month(QString().sprintf("%d", (qchar)(m_cid.mdt >> 4)));
        QString mdt_year(QString().sprintf("%d", (qshort)(m_cid.mdt & 0xf) + 2013)); //todo

        cid_info.ManufacturerId = get_hex(m_cid.mid);
        cid_info.Manufacturer = get_card_mfr_id(m_cid.mid);
        cid_info.CardBGA = get_card_type(m_cid.cbx);
        cid_info.OEMApplicationId = get_hex(m_cid.oid);
        cid_info.ProductName = QString("%0").arg(pnm.trimmed().data());
        cid_info.ProductRevision = QString(prv_min + "." + prv_maj);
        cid_info.ProductSerialNumber = QString("0x%0").arg(psn.toHex().data());
        cid_info.ManufacturingDate = QString(mdt_month + "/" + mdt_year);
    }
}

qstr EMIParser::get_pl_sig_type(qchar sig_type)
{
    switch (sig_type)
    {
        case 0x01:
            return "SIG_PHASH";
        case 0x02:
            return "SIG_SINGLE";
        case 0x03:
            return "SIG_SINGLE_AND_PHASH";
        case 0x04:
            return "SIG_MULTI";
        case 0x05:
            return "SIG_CERT_CHAIN";
        default:
            return "Unknown";
    }
}

qstr EMIParser::get_pl_flash_dev(qchar flash_dev)
{
    switch (flash_dev)
    {
        case 0x01:
            return "NOR";
        case 0x02:
            return "NAND_SEQUENTIAL";
        case 0x03:
            return "NAND_TTBL";
        case 0x04:
            return "NAND_FDM50";
        case 0x05:
            return "EMMC_BOOT";
        case 0x06:
            return "EMMC_DATA";
        case 0x07:
            return "SF";
        case 0xc:
            return "UFS_BOOT";
        default:
            return "Unknown";
    }
}

qstr EMIParser::get_dram_type(quint16 type)
{
    switch (type)
    {
        case 0x001:
            return "Discrete DDR1";
        case 0x002:
            return "Discrete LPDDR2";
        case 0x003:
            return "Discrete LPDDR3";
        case 0x004:
            return "Discrete PCDDR3";
        case 0x101:
            return "MCP(NAND+DDR1)";
        case 0x102:
            return "MCP(NAND+LPDDR2)";
        case 0x103:
            return "MCP(NAND+LPDDR3)";
        case 0x104:
            return "MCP(NAND+PCDDR3)";
        case 0x201:
            return "MCP(eMMC+DDR1)";
        case 0x202:
            return "MCP(eMMC+LPDDR2)";
        case 0x203:
            return "MCP(eMMC+LPDDR3)";
        case 0x204:
            return "MCP(eMMC+PCDDR3)";
        case 0x205:
            return "MCP(eMMC+LPDDR4)";
        case 0x206:
            return "MCP(eMMC+LPDR4X)";
        case 0x306:
            return "uMCP(eUFS+LPDDR4X)";
        case 0x308:
            return "uMCP(eUFS+LPDDR5)";
        default:
            return qstr("%0:Unknown").arg(get_hex(type));
    }
}

qstr EMIParser::get_card_mfr_id(qchar mid)
{
    switch (mid)
    {
        case 0x02:
            return "Sandisk_New"; //what?
        case 0x11:
            return"Toshiba";
        case 0x13:
            return "Micron";
        case 0x15:
            return "Samsung";
        case 0x45:
            return "Sandisk";
        case 0x70:
            return "Kingston";
        case 0x74:
            return "Transcend";
        case 0x88:
            return "Foresee";
        case 0x90:
            return "SkHynix";
        case 0x8f:
            return "UNIC";
        case 0xf4:
            return "Biwin";
        case 0xfe:
            return "Micron"; //mmm?
        default:
            return "Unknown";
    }
}

qstr EMIParser::get_card_type(qchar type)
{
    switch (type)
    {
        case 0x00:
            return "RemovableDevice";
        case 0x01:
            return "BGA (Discrete embedded)";
        case 0x02:
            return "POP";
        case 0x03:
            return "RSVD";
        default:
            return"Unknown";
    }
}

qstr EMIParser::get_unit(qlong bytes)
{
    const qlong kb = 1024;
    const qlong mb = 1024 * kb;
    const qlong gb = 1024 * mb;
    const qlong tb = 1024 * gb;
    if (bytes >= tb)
        return QLocale().toString(bytes / (double)tb, 'f', 2) + qstr::fromLatin1("TB");
    if (bytes >= gb)
        return QLocale().toString(bytes / (double)gb, 'f', 2) + qstr::fromLatin1("GB");
    if (bytes >= mb)
        return QLocale().toString(bytes / (double)mb, 'f', 2) + qstr::fromLatin1("MB");
    if (bytes >= kb)
        return QLocale().toString(bytes / (double)kb, 'f', 2) + qstr::fromLatin1("KB");

    return QLocale().toString(bytes) + qstr::fromLatin1("B");
}

qstr EMIParser::get_hex(qlong num)
{
    return qstr("0x%0").arg(qstr().setNum(num , 0x10).toLower());
}
