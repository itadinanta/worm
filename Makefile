LIBOPTS=-lm -lstdc++ -lpng
LIBGL=-lGL -lGLU -lglut
COPTS=-ggdb -fpermissive -fmessage-length=0 -Wwrite-strings
CC=gcc
OBJECTS=vecmath.o spline.o animation.o model.o texture_storage.o gameboard.o \
	mainchar.o scanner.o log.o

all: worm

test: test.C $(OBJECTS)
	$(CC) $(COPTS) $(LIBOPTS) $(LIBGL) test.C -o test $(OBJECTS) && ./test

worm: worm.C $(OBJECTS)
	$(CC) $(COPTS) worm.C -o worm $(OBJECTS) $(LIBOPTS) $(LIBGL)

compile: $(OBJECTS)

vecmath.o: vecmath.C vecmath.h
	$(CC) $(COPTS) -c vecmath.C -o vecmath.o

spline.o: spline.C spline.h vecmath.h linearmap.h
	$(CC) $(COPTS) -c spline.C -o spline.o

model.o: model.C model.h vecmath.h scanner.h log.h
	$(CC) $(COPTS) -c model.C -o model.o

gameboard.o: gameboard.C gameboard.h model.h vecmath.h log.h
	$(CC) $(COPTS) -c gameboard.C -o gameboard.o

animation.o: animation.C animation.h \
	vecmath.h spline.h rgba.h model.h linearmap.h texture_storage.h \
	scanner.h
	$(CC) $(COPTS) -c animation.C -o animation.o

texture_storage.o: texture_storage.C texture_storage.h
	$(CC) $(COPTS) -c texture_storage.C -o texture_storage.o

mainchar.o: mainchar.C mainchar.h vecmath.h
	$(CC) $(COPTS) -c mainchar.C -o mainchar.o

scanner.o: scanner.C scanner.h config.h log.h
	$(CC) $(COPTS) -c scanner.C -o scanner.o

log.o: log.C log.h
	$(CC) $(COPTS) -c log.C -o log.o
