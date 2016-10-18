rm -fr ca.* server.* client.*

# CA
openssl req -new -x509 -days 356 -extensions v3_ca -keyout ca.key -out ca.crt

# server
openssl genrsa -out server.key 1024
openssl req -out server.csr -key server.key -new
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 356

# client
openssl genrsa -out client.key 1024
openssl req -out client.csr -key client.key -new
openssl x509 -req -in client.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client.crt -days 356

# client JKS
keytool -import -trustcacerts -alias root -file ca.crt -keystore client.jks
cat client.crt client.key > client.all
keytool -import -trustcacerts -alias doug -file client.all -keystore client.jks

# client bin
echo -n $1 > client.bin 
printf "\0" >> client.bin

cat ca.crt >> client.bin
printf "\0" >> client.bin

cat client.crt >> client.bin
printf "\0" >> client.bin

cat client.key >> client.bin
printf "\0" >> client.bin

/Applications/Momentics.app/Contents/Eclipse/ESP8266_RTOS_SDK/tools/esptool -cp /dev/cu.SLAB_USBtoUART -cd nodemcu -ca 0x70000 -cf client.bin
