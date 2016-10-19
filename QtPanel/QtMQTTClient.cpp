/*
 * QtMQTTClient.cpp
 *
 *  Created on: Oct 19, 2016
 *      Author: dschaefer
 */

#include <QtMQTTClient.h>

static void connectionLost(void *context, char *cause) {
	qWarning("Connection lost %s, reconnecting", cause);
	((QtMQTTClient *) context)->connect();
}

static int messageArrived(void *context, char *topicName, int, MQTTAsync_message *message) {
	QByteArray data = QByteArray((char *) message->payload, message->payloadlen);
	QString msg = QString::fromUtf8(data);
    emit ((QtMQTTClient *) context)->message(topicName, msg);

    printf("Recieved '%s'\n", data.data());

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

QtMQTTClient::QtMQTTClient(QString address, QString id, QObject *parent) : QObject(parent) {
	int rc = MQTTAsync_create(&client, address.toUtf8(), id.toUtf8(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
	if (rc != MQTTASYNC_SUCCESS) {
		qFatal("Failed to create client %d\n", rc);
	}

	rc = MQTTAsync_setCallbacks(client, this, connectionLost, messageArrived, NULL);
	if (rc != MQTTASYNC_SUCCESS) {
		qFatal("Failed to set callbacks %d\n", rc);
	}
}

static void onConnect(void *context, MQTTAsync_successData *) {
	qDebug("Connected");
	printf("Connected\n");
	emit ((QtMQTTClient *) context)->connected();
}

static void onConnectFailure(void *, MQTTAsync_failureData *response) {
	qFatal("Connect failed %s", response != NULL ? response->message : "<no message>");
}

void QtMQTTClient::connect() {
	MQTTAsync_connectOptions connectOptions = MQTTAsync_connectOptions_initializer;
	connectOptions.keepAliveInterval = 60;
	connectOptions.cleansession = 1;
	connectOptions.onSuccess = onConnect;
	connectOptions.onFailure = onConnectFailure;
	connectOptions.context = this;

	qDebug("Connecting...");
	printf("Connecting...\n");
	int rc = MQTTAsync_connect(client, &connectOptions);
	if (rc != MQTTASYNC_SUCCESS) {
		qFatal("Failed to start connect, return code %d\n", rc);
	}
}

void onSubscribed(void */*context*/, MQTTAsync_successData */*response*/) {
	// TODO emit a message
}

void onSubscribeFailure(void *, MQTTAsync_failureData *response) {
	qFatal("Subscribe failed %s", response != NULL ? response->message : "<no message>");
}

void QtMQTTClient::subscribe(QString topic) {
	MQTTAsync_responseOptions options = MQTTAsync_responseOptions_initializer;
	options.onSuccess = onSubscribed;
	options.onFailure = onSubscribeFailure;
	options.context = this;

	int rc = MQTTAsync_subscribe(client, topic.toUtf8(), 0, &options);
	if (rc != MQTTASYNC_SUCCESS) {
		qFatal("Failed so subscribe to topic %d", rc);
	}
}

void onSent(void */*context*/, MQTTAsync_successData */*response*/) {
	// TODO emit message
}

void onSendFailure(void *, MQTTAsync_failureData *response) {
	qFatal("Send failed %s", response->message);
}

void QtMQTTClient::publish(QString topic, QString msg) {
	MQTTAsync_responseOptions options = MQTTAsync_responseOptions_initializer;
	options.onSuccess = onSent;
	options.onFailure = onSendFailure;
	options.context = this;

	MQTTAsync_message message = MQTTAsync_message_initializer;
	QByteArray payload = msg.toUtf8();
	message.payload = payload.data();
	message.payloadlen = payload.size();
	message.qos = 0;
	message.retained = 0;

	printf("Sending '%s'\n", payload.data());
	int rc = MQTTAsync_sendMessage(client, topic.toUtf8(), &message, &options);
	if (rc != MQTTASYNC_SUCCESS) {
		qFatal("Failed to start sendMessage, return code %d\n", rc);
	}
}
