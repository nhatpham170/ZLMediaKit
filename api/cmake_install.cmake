# Install script for directory: E:/Repositories/ZLMediaKit/api

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/ZLMediaKit")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files (x86)/ZLMediaKit/include/mk_common.h;C:/Program Files (x86)/ZLMediaKit/include/mk_events.h;C:/Program Files (x86)/ZLMediaKit/include/mk_events_objects.h;C:/Program Files (x86)/ZLMediaKit/include/mk_frame.h;C:/Program Files (x86)/ZLMediaKit/include/mk_h264_splitter.h;C:/Program Files (x86)/ZLMediaKit/include/mk_httpclient.h;C:/Program Files (x86)/ZLMediaKit/include/mk_media.h;C:/Program Files (x86)/ZLMediaKit/include/mk_mediakit.h;C:/Program Files (x86)/ZLMediaKit/include/mk_player.h;C:/Program Files (x86)/ZLMediaKit/include/mk_proxyplayer.h;C:/Program Files (x86)/ZLMediaKit/include/mk_pusher.h;C:/Program Files (x86)/ZLMediaKit/include/mk_recorder.h;C:/Program Files (x86)/ZLMediaKit/include/mk_rtp_server.h;C:/Program Files (x86)/ZLMediaKit/include/mk_tcp.h;C:/Program Files (x86)/ZLMediaKit/include/mk_thread.h;C:/Program Files (x86)/ZLMediaKit/include/mk_track.h;C:/Program Files (x86)/ZLMediaKit/include/mk_transcode.h;C:/Program Files (x86)/ZLMediaKit/include/mk_util.h;C:/Program Files (x86)/ZLMediaKit/include/mk_export.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/ZLMediaKit/include" TYPE FILE FILES
    "E:/Repositories/ZLMediaKit/api/include/mk_common.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_events.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_events_objects.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_frame.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_h264_splitter.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_httpclient.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_media.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_mediakit.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_player.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_proxyplayer.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_pusher.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_recorder.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_rtp_server.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_tcp.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_thread.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_track.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_transcode.h"
    "E:/Repositories/ZLMediaKit/api/include/mk_util.h"
    "E:/Repositories/ZLMediaKit/api/mk_export.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/ZLMediaKit/lib/mk_api.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/ZLMediaKit/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "E:/Repositories/ZLMediaKit/release/windows/Debug/Debug/mk_api.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/ZLMediaKit/lib/mk_api.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/ZLMediaKit/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "E:/Repositories/ZLMediaKit/release/windows/Debug/Release/mk_api.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/ZLMediaKit/lib/mk_api.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/ZLMediaKit/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "E:/Repositories/ZLMediaKit/release/windows/Debug/MinSizeRel/mk_api.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/ZLMediaKit/lib/mk_api.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/ZLMediaKit/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "E:/Repositories/ZLMediaKit/release/windows/Debug/RelWithDebInfo/mk_api.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/ZLMediaKit/bin/mk_api.dll")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/ZLMediaKit/bin" TYPE SHARED_LIBRARY FILES "E:/Repositories/ZLMediaKit/release/windows/Debug/Debug/mk_api.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/ZLMediaKit/bin/mk_api.dll")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/ZLMediaKit/bin" TYPE SHARED_LIBRARY FILES "E:/Repositories/ZLMediaKit/release/windows/Debug/Release/mk_api.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/ZLMediaKit/bin/mk_api.dll")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/ZLMediaKit/bin" TYPE SHARED_LIBRARY FILES "E:/Repositories/ZLMediaKit/release/windows/Debug/MinSizeRel/mk_api.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/ZLMediaKit/bin/mk_api.dll")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/ZLMediaKit/bin" TYPE SHARED_LIBRARY FILES "E:/Repositories/ZLMediaKit/release/windows/Debug/RelWithDebInfo/mk_api.dll")
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("E:/Repositories/ZLMediaKit/api/tests/cmake_install.cmake")

endif()

