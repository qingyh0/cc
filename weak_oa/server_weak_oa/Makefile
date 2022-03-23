TAR:=server_oa
OBJS:=main_server_weak_oa.o server_ops.o sql_ops.o
FLAG:=-Wall -pthread -lsqlite3

$(TAR):$(OBJS)
	gcc $^ -o $@ $(FLAG)

clean:
	rm $(TAR) $(OBJS)