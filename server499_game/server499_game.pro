TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -L /usr/lib/i386-linux-gnu/libpthread.a -lpthread

SOURCES += \
    server499.c \
    errnohandle.c \
    deck.c

HEADERS += \
    server499.h \
    errnohandle.h \
    deck.h

