#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QSpecialInteger>

#include <preloader_parser.h>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    a.isSetuidAllowed();
    a.setApplicationName("MTK Preloader Parser V4.0000.0");
    a.setApplicationVersion("4.0000.0");
    a.setOrganizationName("Mediatek");
    a.setQuitLockEnabled(0);

    qInfo("................ MTK Preloader Parser ...............");
    qInfo(".....................................................");

    if(argc < 2)
        qInfo("Drag and drop the preloader/boot_region file here0!");

    while (1) {

        QByteArray path(0xff, Qt::Uninitialized);
        std::cin.get((char*)path.data(), 0xff);

        qInfo(".....................................................");
        qInfo().noquote() << QString("Reading emi file %0").arg(path.data());
        QFile emi_dev(QDir::toNativeSeparators(path));
        if (!emi_dev.size())
        {
            qInfo().noquote() << QString("please input a valid file!.");
            std::cin.ignore();
        }

        if (emi_dev.open(QIODevice::ReadOnly))
        {
            QVector<mtkPreloader::MTKEMIInfo> emis = {};
            EMIParser::PrasePreloader(emi_dev, emis);
            emi_dev.close();

            qsizetype idx = 0;
            for (QVector<mtkPreloader::MTKEMIInfo>::iterator it =
                 emis.begin(); it != emis.end(); it++)
            {
                mtkPreloader::MTKEMIInfo emi = *it;
                qInfo().noquote() << qstr("EMIInfo{%0}:%1:%2:%3:%4:%5:%6:DRAM:%7:%8").arg(emi.index,
                                                                                          emi.flash_id,
                                                                                          emi.manufacturer_id,
                                                                                          emi.manufacturer,
                                                                                          emi.ProductName,
                                                                                          emi.OEMApplicationId,
                                                                                          emi.CardBGA,
                                                                                          emi.dram_type,
                                                                                          emi.dram_size);

//                qInfo().noquote() << qstr("EMIInfo{%0}:version:%1:emi_content:%2").arg(emi.index, qstr::number(emi.m_emi_ver), emi.m_emi_info.toHex().data());
                //! see EMIInfoV20
                idx++;
            }
        }

        path.clear();
        std::cin.ignore();
        qInfo("Drag and drop the preloader/boot_region file here!");
    }

    return a.exec();
}
