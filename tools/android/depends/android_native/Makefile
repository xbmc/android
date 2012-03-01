#this build is a bit awkward because I've tried to mimic ndk-build exactly.

include ../Makefile.include
include ../android-sdk.mk

GLUEPATH=$(NDKROOT)/sources/android/native_app_glue
GLUESRC=android_native_app_glue.c
CFLAGS+=-I$(NDKROOT)/sources/android/native_app_glue/
LDFLAGS+=-llog -landroid -lEGL -lGLESv2 -lstdc++ -ldl -lstdc++
SOURCE=main.c
OBJ=$(SOURCE:.c=.o)
SOLIB=libxbmcapp.so

all: .installed

$(GLUESRC:.c=.a):
	$(CC) $(CFLAGS) -o $(GLUESRC:.c=.o) -c $(GLUEPATH)/$(GLUESRC)
	$(AR) crs $(GLUESRC:.c=.a) $(GLUESRC:.c=.o)

$(OBJ): $(GLUESRC:.c=.a) $(SOURCE)
	$(CC) $(CFLAGS) -o $(OBJ) -c $(SOURCE)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(SOLIB) -shared main.o $(GLUESRC:.c=.a)

.installed : $(OBJ)
	install $(SOLIB) $(PREFIX)/lib/$(SOLIB)
	@touch $@
clean:
	-@rm -f $(OBJ) $(SOLIB) android_native_app_glue.* .compiled .installed 2>/dev/null || true
