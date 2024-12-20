CC=g++

INSTALL_DIR = ../bin
#INCLUDE_DIR = -I. -I.. -I../cpr/include/ -I../json/include/ -I../restbed/source
INCLUDE_DIR = -I. -I.. -I../cpr/include/  -I../restbed/source


EXE      = dexplexd
SOURCE   = src/main.cpp src/dex-rest.cpp src/dex-mgr.cpp src/bg_cache.cpp src/common.cpp src/restServer.cpp
LINKLIBS = -L/usr/lib/libdaemon/ -L../cpr/build/lib/ -lspdlog -lcpr -lcurl -ldaemon -lmut -lstdc++fs -lboost_date_time -lfmt -lpthread 
STATICLIBS =  -L ../restbed/build/ -lrestbed

OBJS     = $(SOURCE:.cpp=.o) 
DEPS     = $(SOURCE:.cpp=.d) 

-include $(DEPS)

CFLAGSALL     = -std=c++17 -Wall -Wextra -Werror -Wshadow -march=native -fno-default-inline -fno-stack-protector -pthread -Wall -Werror -pedantic -Wextra -Waddress -Warray-bounds -Wno-builtin-macro-redefined -Wundef
CFLAGSALL     = -std=c++17 
CFLAGSALL    += -DSPDLOG_COMPILED_LIB
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
	$(CC) -o $(EXE) $(OBJS) $(STATICLIBS) $(LINKLIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE_DIR) -MMD -MP -c $< -o $@

install : all
	install -d $(INSTALL_DIR)
	install -m 750 $(EXE) $(INSTALL_DIR)

uninstall :
	/bin/rm -rf $(INSTALL_DIR)

clean :
	rm -f src/*.o src/*.d $(EXE) 
