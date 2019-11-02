#!/bin/bash
echo "Analisi dei file di log"


awk '/BASED ON/ {print $3 " " $5 " " $8}' supervisor.log | sort -nk1 | uniq > supervisor_last.log

awk '/SECRET/ {print $4 " " $2}' client.log | sort -nk1 > client_last.log



IFS=$'\n'

CORRECT=0

TOT=0

ERR_TOT=0

DELTA=0



for line in $(cat supervisor_last.log); 

    do

        ESTIM=$(cut -d' ' -f1 <<< $line)

        ID=$(cut -d' ' -f2 <<< $line)

        CLIENT_LINE=$(grep $ID client_last.log)

        CLIENT_SECRET=$(($(cut -d' ' -f1 <<< $CLIENT_LINE) + 0))

        CLIENT_ID=$(cut -d' ' -f2 <<< $CLIENT_LINE)

        printf "CLIENT_ID: %8s SECRET: %4d SERVER ESTIMATE: %4d\n" "$ID" "$CLIENT_SECRET" "$ESTIM"

        ((TOT++))

        DELTA=$(($ESTIM - $CLIENT_SECRET))
        
        if [ $DELTA -lt 0 ]; then
		DELTA=$((0 - $DELTA))
        fi

        if [ $DELTA -lt 25 ]; then
        	ERR_TOT=$(($ERR_TOT + $DELTA))
            	((CORRECT++))
	fi

done

echo "SECRET STIMATI CORRETTAMENTE: $CORRECT SU $TOT"

awk -v errt="$ERR_TOT" -v tot="$TOT" 'BEGIN {print "ERRORE MEDIO: " (errt/tot) ; exit}'

