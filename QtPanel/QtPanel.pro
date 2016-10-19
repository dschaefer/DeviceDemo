TEMPLATE = app

QT += qml quick
CONFIG += c++11

RESOURCES += QtPanel.qrc

qml.files = QtPanel.qml

launch_modeall {
	CONFIG(debug, debug|release) {
	    DESTDIR = debug
	} else {
	    DESTDIR = release
	}
}

SOURCES += QtPanel.cpp \
           QtMQTTClient.cpp

HEADERS += QtMQTTClient.h

PAHO = $$PWD/../../paho.mqtt.c

INCLUDEPATH += $$PAHO/src

macx:LIBS += -L$$PAHO/build/default/src
qnx-armle-v7-qcc:LIBS += -L$$PAHO/build/cmake.run.qcc.armle/src

LIBS += -lpaho-mqtt3a

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.11