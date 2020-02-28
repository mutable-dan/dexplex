CC=g++-8

INSTALL_DIR = ../bin
INCLUDE_DIR = -I. -I../cpr/include/


EXE      = dexplex
SOURCE   = rest.cpp
LINKLIBS = -lpthread -L../cpr/lib/ -lcpr -lcurl

OBJS     = $(SOURCE:.cpp=.o) 
DEPS     = $(SOURCE:.cpp=.d) 

-include $(DEPS)

CFLAGSALL     = -std=c++17 -Wall -Wextra -Werror -Wshadow -march=native -fno-default-inline -fno-stack-protector -pthread -Wall -Werror -pedantic -Wextra -Weffc++ -Waddress -Warray-bounds -Wno-builtin-macro-redefined -Wundef
CFLAGSRELEASE = -O2 -DNDEBUG $(CFLAGSALL)
CFLAGSDEBUG   = -ggdb3 -DDEBUG $(CFLAGSALL)

.PHONY: release
release: CFLAGS = $(CFLAGSRELEASE)
release: all

.PHONY: debug
debug: CFLAGS = $(CFLAGSDEBUG)
debug: all


# compile and link

all : $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(LINKLIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE_DIR) -MMD -MP -c $< -o $@

install : all
	install -d $(INSTALL_DIR)
	install -m 750 $(EXE) $(INSTALL_DIR)

uninstall :
	/bin/rm -rf $(INSTALL_DIR)

clean :
	rm -f *.o $(EXE) *.d
