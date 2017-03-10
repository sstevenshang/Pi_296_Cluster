
CFLAGS = -I. -Wall -Winit-self -Wextra -Wno-nonnull  -Wno-unused-variable -Wno-unused-result -pthread -Wno-unused-parameter
# Source, Executable, Includes, Library Defines
INCL   = networkManager.h
SRC    = networkManager.c vm_server.c vm_client.c
OBJ    = $(SRC:.c=.o)

# Compiler, Linker Defines
CC      = /usr/bin/gcc
LIBPATH = -L.
LDFLAGS = -o $(EXE) $(LIBPATH)
CFDEBUG = -ansi -pedantic -Wall -g -DDEBUG $(LDFLAGS)
RM      = /bin/rm -f

# Compile and Assemble C Source Files into Object Files
%.o: %.c
	$(CC) -c $(CFLAGS) $*.c

# Link all Object Files with external Libraries into Binaries
all: vm_server vm_client pi_server pi_client clean

vm_server: networkManager.o vm_server.o
	gcc networkManager.o vm_server.o $(CFLAGS) -o vm_server_exe

vm_client: networkManager.o vm_client.o
	gcc networkManager.o vm_client.o $(CFLAGS) -o vm_client_exe

pi_server: networkManager.o pi_server.o
	gcc networkManager.o pi_server.o $(CFLAGS) -o pi_server_exe

pi_client: networkManager.o pi_client.o
	gcc networkManager.o pi_client.o $(CFLAGS) -o pi_client_exe


# Objects depend on these Libraries
$(OBJ): $(INCL)

# Create a gdb/dbx Capable Executable with DEBUG flags turned on
debug:
	$(CC) $(CFDEBUG) $(SRC)

# Clean Up Objects, Exectuables, Dumps out of source directory
clean:
	rm -rf *.o
