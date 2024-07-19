linux{
  CONFIG   += x11

  DEFINES  += __LINUX_ALSA__
  DEFINES  += __LITTLE_ENDIAN__

  QMAKE_LIBDIR += ./lib/linux/pa
  QMAKE_LIBDIR += ./lib/linux/celt

  LIBS     += -lportaudio
  LIBS     += -lpthread
  LIBS     += -lasound
  LIBS     += -lopus
  LIBS     += -lrt
}

macx{
  QMAKE_LIBDIR += ./lib/OSX/celt/
  QMAKE_LIBDIR += ./lib/OSX/pa/

  LIBS      += -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreServices
  LIBS      += -lportaudio
  LIBS      += -lpthread
  LIBS      += -lopus

  DEFINES  += __MACOSX_CORE__
}

win32{
  CONFIG      += windows

  DEFINES     += __LITTLE_ENDIAN__
  DEFINES     += __WINDOWS_ASIO__
  DEFINES     -= UNICODE

  QMAKE_LIBDIR     += ./lib/win/celt
  QMAKE_LIBDIR     += ./lib/win/pa

  HEADERS += ./include/opus.h \
             ./include/opus_custom.h \

  LIBS        += -lportaudio
  LIBS        += -lole32
  LIBS        += -lwinmm
  LIBS        += -luuid
  LIBS        += -lopus
  LIBS        += -lws2_32
}

DEFINES += PORTAUDIO

QMAKE_LIBDIR += .

INCLUDEPATH += .
INCLUDEPATH += ./include
INCLUDEPATH += ./include/portaudio-snapshot

CONFIG += thread qt warn_on exceptions

CONFIG += console c++17 cmdline

QT += core network widgets

#HEADERS += ias.h \

SOURCES +=  source/main.cpp \
            source/BasicLogging.cpp
            #ias.cpp \
            #server.cpp


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
