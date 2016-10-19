#ifndef QTMQTTCLIENT_H_
#define QTMQTTCLIENT_H_

#include <QObject>
#include <MQTTAsync.h>

class QtMQTTClient : public QObject {
	Q_OBJECT
public:
	QtMQTTClient(QString address, QString id, QObject *parent = NULL);

	void connect();

	Q_INVOKABLE void subscribe(QString topic);
	Q_INVOKABLE void publish(QString topic, QString msg);

signals:
	void connected();
	void message(QString topic, QString msg);

private:
	MQTTAsync client;
};

#endif /* QTMQTTCLIENT_H_ */
