target = parser
objects = main.o operation.o process.o

CFLAGS += -Wall -g

$(target) : $(objects)
	$(CC) -o $(target) $(objects) $(LINKFLAGS)

%.o: %.c %.h io.h
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm $(target) $(objects)
