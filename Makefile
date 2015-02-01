# Set compiler args
CC=g++
CFLAGS=-Wall -c -fno-tree-vectorize
LDFLAGS=
LDLIBS=-L /usr/lib $$(pkg-config --cflags --libs opencv) -pthread
ifeq ($(shell arch), armv7l)
	LDLIBS += -lpfm 
endif
SOURCES=sobel.cpp pc.cpp sobel_st.cpp sobel_mt.cpp sobel_calc.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=sobel
TAR=lab1.tar.gz
SUBMIT_FILES=lab1/*.cpp lab1/*.h lab1/README lab1/Makefile

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE):$(OBJECTS)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS) $(LDLIBS)  

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

run:
	./sobel
clean:
	\rm -f *.o $(EXECUTABLE) $(TAR)

submit: clean
	ln -s . lab1
	tar -czf $(TAR) $(SUBMIT_FILES)
	rm -f lab1
