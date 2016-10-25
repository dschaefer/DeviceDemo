#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "QtMQTTClient.h"

#if defined(__APPLE__)
#define MQTT_URL "tcp://localhost:1883"
#elif defined(__QNXNTO__)
#define MQTT_URL "tcp://192.168.0.100:1883"
#endif

int main(int argc, char *argv[]) {
	QGuiApplication app(argc, argv);

	QQmlApplicationEngine engine;

	QtMQTTClient mqttClient(MQTT_URL, "Panel");
	engine.rootContext()->setContextProperty("mqttClient", &mqttClient);
	engine.load(QUrl(QStringLiteral("qrc:/QtPanel.qml")));

	mqttClient.connect();
	return app.exec();
}
