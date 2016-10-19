import QtQuick 2.3
import QtQuick.Window 2.2

Window {
	visible: true

	Grid {
		columns: 1
		spacing: 5
		anchors.centerIn: parent

		Item {
			id: state
			width: 400
			height: 200
			property alias color: rectangle.color
			
			Rectangle {
				id: rectangle
				anchors.fill: parent
				border.color: "black"
				color: "blue"
			}
		}
		
		Item {
			id: button
			width: 400
			height: 100
			property bool on: false
			property alias text: text.text

			Rectangle {
				border.color: "black"
				anchors.fill: parent

				Text {
					id: text
					text: "Flash"
					anchors.centerIn: parent
				}
	
				MouseArea {
					anchors.fill: parent
					onClicked: {
						var command;
						if (button.on) {
							command = "Off";
							button.text = "Flash";
							button.on = false;
						} else {
							command = "Flash";
							button.text = "Off";
							button.on = true;
						}
						mqttClient.publish("/command", command);
					}
				}
			}
		}

	}

	Connections {
		target: mqttClient
		onConnected: {
			mqttClient.subscribe("/dist");
			mqttClient.publish("/command", "Ping");
		}
		onMessage: {
			state.color = msg;
		}
	}

}
