OBJ:=main.o fops.o get_tem_hum.o
FLAG:= -pthread
CC:= arm-linux-gnueabihf-gcc
#CC:=gcc
ser:$(OBJ)
	$(CC) $^ -o $@ $(FLAG)

clean:
	rm ser $(OBJ)

install:
	cp ser ~/nfs/rootfs/armc/