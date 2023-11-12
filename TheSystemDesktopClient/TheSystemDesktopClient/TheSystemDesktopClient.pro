QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    blankpage.cpp \
    homepage.cpp \
    main.cpp \
    mainwindow.cpp \
    packetmanager.cpp \
    page.cpp \
    pagefactory.cpp \
    pagenavigator.cpp \
    signinpage.cpp \
    signuppage.cpp \
    sockets.cpp

HEADERS += \
    blankpage.h \
    homepage.h \
    mainwindow.h \
    packetmanager.h \
    page.h \
    pagefactory.h \
    pagenavigator.h \
    signinpage.h \
    signuppage.h \
    sockets.h

FORMS += \
    blankpage.ui \
    homepage.ui \
    mainwindow.ui \
    signinpage.ui \
    signuppage.ui

win32: LIBS += -lws2_32
LIBS += -lz

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
