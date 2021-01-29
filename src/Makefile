TARGET = libbravo.a
OBJECTS := $(patsubst %.c, %.o, $(wildcard *.c))
CFLAGS := -W -Wall -O3

$(TARGET) : $(OBJECTS)
	$(AR) rcv $@ $^
	ranlib $@

clean :
	$(RM) $(OBJECTS) $(TARGET)
