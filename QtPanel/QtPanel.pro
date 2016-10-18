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

SOURCES += QtPanel.cpp
