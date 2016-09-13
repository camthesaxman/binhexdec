CC:=gcc
LD:=$(CC)
RC:=windres
CFLAGS:=-Wall -Wextra -pedantic -std=c99 -O3 -DUNICODE -D_UNICODE
LDFLAGS:=-mwindows
SOURCES:=binhexdec.c
RESOURCE:=res.rc
LIBS:=-lcomctl32 -lgdi32
TARGET:=binhexdec.exe
OBJECTS:=$(patsubst %.c, %.o, $(SOURCES)) $(patsubst %.rc, %.o, $(RESOURCE))

$(TARGET): $(OBJECTS)
	$(LD) $^ $(LDFLAGS) $(LIBS) -o $@
	
clean:
	rm -f $(TARGET) $(OBJECTS)
	
res.rc: manifest.xml
	
%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@
	
%.o: %.rc
	$(RC) $^ -o $@