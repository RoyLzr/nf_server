.PHONY: clean

TAR_DIR := ./
TAR_OBJ := lib_nf_svr.a
COMMON_DIR := ./commonn
 
OBJS := net.o nf_server.o rapool.o timer.o nf_server_core.o \
        nf_server_app.o lfpool.o configParser.o sapool.o \
        queue.o memCache.o asynLog.o nf_server_app.o 
SVR_OBJS := net.o nf_server.o rapool.o nf_server_core.o \
            nf_server_app.o lfpool.o sapool.o
COM_OBJS := timer.o queue.o memCache.o asynLog.o

CXX := g++
AR := ar
AFLAGS := -cvr
CFLAGS := -g -c -Wall
THREAD := -lpthread

EXECUTE := $(TAR_OBJ)

$(EXECUTE) : $(OBJS)
	$(AR) $(AFLAGS) $(TAR_OBJ) $(OBJS)
	
nf_server_app.o : nf_server_app.cpp
	$(CXX) $(CFLAGS) nf_server_app.cpp 

nf_server.o : nf_server.cpp
	$(CXX) $(CFLAGS) nf_server.cpp 

rapool.o : rapool.cpp
	$(CXX) $(CFLAGS) rapool.cpp 

sapool.o : sapool.cpp
	$(CXX) $(CFLAGS) sapool.cpp 

lfpool.o : lfpool.cpp
	$(CXX) $(CFLAGS) lfpool.cpp 

nf_server_core.o : nf_server_core.cpp
	$(CXX) $(CFLAGS) nf_server_core.cpp 

configParser.o : $(COMMON_DIR)/configParser.cpp
	$(CXX) $(CFLAGS) $(COMMON_DIR)/configParser.cpp 


asynLog.o : $(COMMON_DIR)/asynLog.cpp
	$(CXX) $(CFLAGS) $(COMMON_DIR)/asynLog.cpp 

timer.o : $(COMMON_DIR)/timer.cpp
	$(CXX) $(CFLAGS) $(COMMON_DIR)/timer.cpp 

queue.o : $(COMMON_DIR)/queue.cpp
	$(CXX) $(CFLAGS) $(COMMON_DIR)/queue.cpp 

memCache.o : $(COMMON_DIR)/memCache.cpp
	$(CXX) $(CFLAGS) $(COMMON_DIR)/memCache.cpp 


clean:
	rm -rf *.o
