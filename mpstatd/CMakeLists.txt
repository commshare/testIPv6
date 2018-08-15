
cmake_minimum_required(VERSION 3.0)
project(mpstatd VERSION 1.0 LANGUAGES C)

set(CMAKE_INSTALL_PREFIX "/usr")

include(GNUInstallDirs)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(GLIB2 REQUIRED glib-2.0>=2.16)
    include_directories(${GLIB2_INCLUDE_DIRS})
    link_directories(${GLIB2_LIBRARY_DIRS})

    pkg_check_modules(UUID REQUIRED uuid)
    include_directories(${UUID_INCLUDE_DIRS})
    link_directories(${UUID_LIBRARY_DIRS})
endif()

find_program(PCAP_CONFIG pcap-config)
if(PCAP_CONFIG STREQUAL "PCAP_CONFIG-NOTFOUND")
    message(SEND_ERROR "libpcap library not found")
else()
    execute_process(COMMAND ${PCAP_CONFIG} --libs
        OUTPUT_VARIABLE PCAP_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

add_library(list OBJECT list.c)
add_library(common OBJECT common.c)
add_library(msignal OBJECT msignal.c)
add_library(mthread OBJECT mthread.c)
add_library(mepoll OBJECT mepoll.c)
add_library(mpipe OBJECT mpipe.c)
add_library(mpacket OBJECT mpacket.c)
add_library(mpcap OBJECT mpcap.c)
add_library(mconn_relay OBJECT mconn_relay.c)
add_library(mconn OBJECT mconn.c)
add_library(mseqlist OBJECT mseqlist.c)
add_library(mtrack OBJECT mtrack.c)

add_executable(mpstatd
    mpstatd.c
    $<TARGET_OBJECTS:list>
    $<TARGET_OBJECTS:common>
    $<TARGET_OBJECTS:msignal>
    $<TARGET_OBJECTS:mthread>
    $<TARGET_OBJECTS:mepoll>
    $<TARGET_OBJECTS:mpipe>
    $<TARGET_OBJECTS:mpacket>
    $<TARGET_OBJECTS:mpcap>
    $<TARGET_OBJECTS:mconn_relay>
    $<TARGET_OBJECTS:mconn>
    $<TARGET_OBJECTS:mseqlist>
    $<TARGET_OBJECTS:mtrack>
)

target_link_libraries(mpstatd
PUBLIC
    Threads::Threads
    ${PCAP_LDFLAGS}
    ${GLIB2_LIBRARIES}
    ${UUID_LIBRARIES}
)

install(TARGETS mpstatd RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR})