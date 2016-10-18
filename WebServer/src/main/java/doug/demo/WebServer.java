package doug.demo;

import java.nio.charset.StandardCharsets;
import java.util.HashSet;
import java.util.Set;

import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttAsyncClient;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttAsyncClient;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import io.vertx.core.Vertx;
import io.vertx.core.http.HttpServer;
import io.vertx.core.http.HttpServerResponse;
import io.vertx.core.http.ServerWebSocket;
import io.vertx.ext.web.Router;
import io.vertx.ext.web.handler.StaticHandler;

public class WebServer implements MqttCallback {

	private Vertx vertx = Vertx.vertx();
	private IMqttAsyncClient mqttClient;
	private String color = "green";
	private HttpServerResponse response;
	private Set<ServerWebSocket> sockets = new HashSet<>();

	public static void main(String[] args) throws Exception {
		new WebServer().run();
	}
	
	private void run() throws Exception {
		HttpServer server = vertx.createHttpServer();
		Router router = Router.router(vertx);

		router.get("/dist").handler(context -> {
			context.response().end(color);
		});
		router.put("/command").handler(context -> {
			response = context.response();
			String command = context.getBodyAsString();
			try {
				mqttClient.publish("/command", command.getBytes(StandardCharsets.UTF_8), 0, false);
			} catch (Exception e) {
				e.printStackTrace();
			}
		});
		router.route().handler(StaticHandler.create().setCachingEnabled(false));
		
		server.requestHandler(router::accept);
		
		server.websocketHandler(socket -> {
			System.out.println("New WebSocket");
			sockets.add(socket);
			socket.writeFinalTextFrame(color);
			socket.endHandler(v -> {
				sockets.remove(socket);
			});
			socket.frameHandler(frame -> {
				System.out.println("command: " + frame.binaryData().toString());
				try {
					mqttClient.publish("/command", frame.binaryData().getBytes(), 0, false);
				} catch (Exception e) {
					e.printStackTrace();
				}
			});
		});
		
		server.listen(8090);
		System.out.println("Listening on port 8090");
		
		mqttClient = new MqttAsyncClient("tcp://localhost:1883", "server");
		mqttClient.setCallback(this);
		mqttClient.connect(null, new IMqttActionListener() {
			@Override
			public void onSuccess(IMqttToken asyncActionToken) {
				try {
					System.out.println("Mqtt Connected");
					mqttClient.subscribe("/dist", 0);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
			
			@Override
			public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
				exception.printStackTrace();
			}
		});
	}

	@Override
	public void messageArrived(String topic, MqttMessage message) throws Exception {
		color = new String(message.getPayload(), StandardCharsets.UTF_8);
		System.out.println("Color: " + color);
		vertx.runOnContext(v -> {
			for (ServerWebSocket socket : sockets) {
				socket.writeFinalTextFrame(color);
			}
		});
	}

	@Override
	public void connectionLost(Throwable cause) {
		try {
			System.out.println("Reconnecting MQTT...");
			mqttClient.connect();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	@Override
	public void deliveryComplete(IMqttDeliveryToken token) {
		if (response != null) {
			response.end();
			response = null;
		}
	}

}
