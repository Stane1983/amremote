DEPS = common.h keyDefine.h rc_common.h
OBJ = main.o parseFile.o setPara.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

remotecfg: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o *~ remotecfg

.PHONY: install

install:
	cp -f remotecfg $(TARGET_DIR)/usr/bin/
	cp -f remote-mouse.tab $(TARGET_DIR)/etc
	cp -f remote.cfg $(TARGET_DIR)/etc
	cp -f remote.tab $(TARGET_DIR)/etc

.PHONY: uninstall

uninstall:
	rm -f $(TARGET_DIR)/usr/bin/remotecfg
