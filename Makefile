all:
	gcc protool.c cJSON.c -lm -lsqlite3 -o protool

protoold:
	/opt/xtools/arm920t/bin/arm-linux-gcc protoold.c cJSON.c eeprom_protoold.c -lm  -L ./ -o protoold
