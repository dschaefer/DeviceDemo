cmake_minimum_required (VERSION 2.6)

project (DeviceDemo)

add_subdirectory(GPIO)
add_subdirectory(NeoPixel/src)
add_subdirectory(paho.mqtt.embedded-c/MQTTPacket/src)

add_executable(DeviceDemo
	DeviceDemo.cpp
	DeviceESP8266.cpp
	DeviceQNX.cpp
)
target_link_libraries(DeviceDemo NeoPixel GPIO MQTTPacket)

if(ESP8266)
target_link_libraries(DeviceDemo ${ESP8266_LIBS})
endif(ESP8266)

if(QNXNTO)
target_link_libraries(DeviceDemo -lsocket)
endif(QNXNTO)
