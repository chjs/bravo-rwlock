TARGET1 = pthread
TARGET2 = bravo
CFLAGS = -W -Wall -O3

all: $(TARGET1) $(TARGET2)

$(TARGET1) : test.c
	$(CC) $(CFLAGS) -o $@ test.c -lpthread

$(TARGET2) : test.c libbravo.a
	$(CC) $(CFLAGS) -o $@ test.c -lpthread -lbravo -Lsrc -Isrc -DBRAVO

libbravo.a :
	cd src && make

clean :
	cd src && make clean
	$(RM) $(TARGET1) $(TARGET2)
