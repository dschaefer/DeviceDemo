cmake_minimum_required (VERSION 2.6)

project (DeviceDemo)

add_subdirectory(GPIO)
add_subdirectory(NeoPixel/src)

add_executable(DeviceDemo DeviceDemo.cpp)

target_link_libraries(DeviceDemo NeoPixel GPIO)

if(ESP8266)
target_link_libraries(DeviceDemo ${ESP8266_LIBS})
endif(ESP8266)
