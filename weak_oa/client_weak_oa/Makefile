TAR:=client_oa
OBJS:=main_client_weak_oa.o client_ops.o client_init.o client_00.o client_66.o client_88.o
FLAG:=-Wall

$(TAR):$(OBJS)
	gcc $^ -o $@ $(FLAG)

clean:
	rm $(TAR) $(OBJS)