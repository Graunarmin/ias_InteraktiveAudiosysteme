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

DESTDIR = ./bin

CONFIG(debug, debug|release){
  TARGET = iasD
  OBJECTS_DIR = ./build/ias/debug
  MOC_DIR = ./build/ias/debug
}

CONFIG(release, debug|release){
  TARGET = ias
  OBJECTS_DIR = ./build/ias/release
  MOC_DIR = ./build/ias/release
}

DEFINES += PORTAUDIO

QMAKE_LIBDIR += .

INCLUDEPATH += .
INCLUDEPATH += ./include
INCLUDEPATH += ./include/portaudio-snapshot

CONFIG += thread qt warn_on exceptions

CONFIG += console c++17 cmdline

QT += core network widgets

HEADERS += \
    #source/signals-slots/observer.h \
    #source/signals-slots/reporter.h
    source/transceiver/server.h

SOURCES +=  source/main.cpp \
            #source/logging/BasicLogging.cpp \
            #source/signals-slots/observer.cpp \
            #source/signals-slots/reporter.cpp
            source/transceiver/server.cpp

DISTFILES +=
