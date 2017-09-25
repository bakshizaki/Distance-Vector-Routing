IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj

_DEPS = command_handling.h common_methods.h error_codes.h server_node.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = mbakshi_proj2.o command_handling.o common_methods.o error_codes.o server_node.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mbakshi_proj2: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ mbakshi_proj2
