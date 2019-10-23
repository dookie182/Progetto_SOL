#!/bin/bash

echo "Avvio Supervisor con 8 Server"
	
	./supervisor 8 1>supervisor.log &
	PID=$!
	
echo "Avvio 20 Client"


for((i=0;i<10;i++)); do
	./client 5 8 20 1>>client.log &
	./client 5 8 20 1>>client.log &
	sleep 1
done

echo "Invio SIGINT a distanza di 10 sec"

for((i=0;i<6;i++)); do
	kill -2 $PID
	sleep 10
done

echo "Termino il Supervisor con doppio SIGINT"

	kill -2 $PID
	sleep 0.1
	kill -2 $PID
	
	sleep 1
	
	./misura.sh
