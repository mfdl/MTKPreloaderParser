QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

#get rid of auto generated debug/release folders
CONFIG -= debug_and_release debug_and_release_target

CONFIG(debug, debug|release) {DESTDIR = tmp/debug }
CONFIG(release, debug|release) {DESTDIR = tmp/release }

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.u

DESTDIR = output

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        preloader_parser.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

PRODUCT_IDENTIFIER = https://web.facebook.com/mofadal.96
PRODUCT_VERSION_NAME = 1.0.0
PRODUCT_VERSION_CODE = 1

win32:{
    DEFINES += "_CRT_SECURE_NO_WARNINGS"
}

HEADERS += \
    emi_structures.h \
    preloader_parser.h

