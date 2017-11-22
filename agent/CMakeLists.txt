# Cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
INCLUDE (CheckIncludeFiles)

# project
PROJECT(molten_agent)

OPTION(ENABLE_DTRACE "build with dtrace" OFF)

# use dtrace -DENABLE_DTRACE
if (ENABLE_SYSTEMTAP)
    CHECK_INCLUDE_FILES("sys/sdt.h" HAVE_SYSTEMTAP)
endif()

# config header
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_SOURCE_DIR}/config.h)

SET(CMAKE_C_FLAGS "-std=gnu99")
SET(SRC_LIST timer.c common.c socket.c protocol/protocol.c sstring.c net_client.c smalloc.c event_loop.c server.c ../common/molten_slog.c)
SET(CLIENT_SRC_LIST socket.c protocol/message.c sstring.c net_client.c client.c smalloc.c ../common/molten_slog.c)

# with debug info
#set(CMAKE_BUILD_TYPE "Debug")
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")  
#SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")  

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/protocol/)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../common/)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/systemtap/)

if (HAVE_SYSTEMTAP)
    SET(SRC_LIST ${SRC_LIST} systemtap/probes.o)
    SET(CLIENT_SRC_LIST ${CLIENT_SRC_LIST} systemtap/probes.o)
endif()

ADD_EXECUTABLE(molten_agent ${SRC_LIST})
ADD_EXECUTABLE(molten_client ${CLIENT_SRC_LIST})
TARGET_LINK_LIBRARIES(molten_agent pthread)
MESSAGE(STATUS "Build over")
