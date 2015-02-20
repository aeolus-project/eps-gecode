################################################################################
# Copyright (c) 2015, Mohamed REZGUI - University of Nice Sophia Antipolis     # #                                                                              #
# All rights reserved.                                                         #
# Redistribution and use in source and binary forms, with or without           #
# modification, are permitted provided that the following conditions are met:  #
#                                                                              #
#     * Redistributions of source code must retain the above copyright         #
#       notice, this list of conditions and the following disclaimer.          #
#     * Redistributions in binary form must reproduce the above copyright      #
#       notice, this list of conditions and the following disclaimer in the    #
#       documentation and/or other materials provided with the distribution.   #
#     * Neither the name of Mohamed REZGUI nor the			       #
#       names of its contributors may be used to endorse or promote products   #
#       derived from this software without specific prior written permission.  #
#                                                                              #
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY  #
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED    #
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE       #
# DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY #
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES   #
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; #
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND  #
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT   #
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS#
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                 #
################################################################################

################ Project Properties ####################
CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (eps-gecode)
SET (APPLICATION_NAME "eps-gecode")
SET (APPLICATION_CODENAME "${PROJECT_NAME}")
SET (APPLICATION_COPYRIGHT_YEARS "2015")
SET (APPLICATION_VERSION_MAJOR 1)
SET (APPLICATION_VERSION_MINOR 0)
SET (APPLICATION_VERSION_PATCH 0)
SET (APPLICATION_VERSION_TYPE SNAPSHOT)
SET (APPLICATION_VERSION_STRING "${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR}.${APPLICATION_VERSION_PATCH}-${APPLICATION_VERSION_TYPE}")
SET (APPLICATION_VENDOR_ID "mrezgui")
SET (APPLICATION_VENDOR_NAME "mrezgui")
#SET (APPLICATION_VENDOR_URL "www.i3s.unice.fr")
SET (APPLICATION_ID "${APPLICATION_VENDOR_ID}.${PROJECT_NAME}")

################ FORCE SET TYPE OF BUILD ####################
SET (DEBUG_BUILD FALSE)
# DEBUG or RELEASE
#SET (CMAKE_BUILD_TYPE "DEBUG")
#build Debug only when explicitally set
if(CMAKE_BUILD_TYPE MATCHES DEBUG OR CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "debug")
	message(STATUS "Building Debug")
	SET (CMAKE_BUILD_TYPE "Debug")
    SET (DEBUG_BUILD TRUE)
else()
	message(STATUS "Building Release")
	SET (CMAKE_BUILD_TYPE "Release")
endif()

################ FORCE COMPILATION 32 Bits ####################
#SET (FORCE_32 TRUE)
SET (FORCE_32 FALSE)

################ DETECT SYSTEM AND ARCHITECTURE ####################
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	SET (OS_SYSTEM "linux")
	IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
	    SET (ARCH_SYSTEM "64")
	ELSE (CMAKE_SIZEOF_VOID_P EQUAL 8)
	    SET (ARCH_SYSTEM "32")
	ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)

elseif (WIN32)
	SET (OS_SYSTEM "windows")
	if (CMAKE_CL_64)
	    SET (ARCH_SYSTEM "64")
	else()
	    SET (ARCH_SYSTEM "32")
	endif(CMAKE_CL_64)
elseif (APPLE)
	SET (OS_SYSTEM "osx")
	SET (ARCH_SYSTEM "64")

endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

message(STATUS "OS_TYPE: ${OS_SYSTEM}" )
message(STATUS "ARCH_TYPE: ${ARCH_SYSTEM} bits" )
message(STATUS "FORCE_32: ${FORCE_32}" )


################ # DEBUG or RELEASE ####################
SET (CMAKE_VERBOSE_MAKEFILE 1) # Use 1 for debugging, 0 for release

################ Project Output Paths ####################
SET (MAINFOLDER ${PROJECT_SOURCE_DIR})

################ Project Search Paths ####################
LIST (APPEND CMAKE_PREFIX_PATH "${MAINFOLDER}")
LIST (APPEND CMAKE_PREFIX_PATH "${MAINFOLDER}/tools")
LIST (APPEND CMAKE_PREFIX_PATH "${MAINFOLDER}/thirdparty")
SET (CMAKE_MODULE_PATH "${MAINFOLDER}/tools/share/cmake")

################ BOOST Lib Check ####################
#FIND_PACKAGE (Boost REQUIRED)
#INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
#LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

################ XML2 Lib Check ####################
# FIND_PACKAGE(XML2 REQUIRED)
# INCLUDE_DIRECTORIES(${XML2_INCLUDE_DIRS})
# LINK_DIRECTORIES(${XML2_LIBRARY_DIRS})

################ GLPK Lib Check ####################
#FIND_PACKAGE(GLPK)
#INCLUDE_DIRECTORIES(${GLPK_INCLUDE_DIRS})
#LINK_DIRECTORIES(${GLPK_LIBRARY_DIRS})

################ LPSOLVE Lib Check ####################
#FIND_PACKAGE(LPSOLVE)
#INCLUDE_DIRECTORIES(${LPSOLVE_INCLUDE_DIRS})
#LINK_DIRECTORIES(${LPSOLVE_LIBRARY_DIRS})

################ Gecode ####################
INCLUDE_DIRECTORIES("~/gecode/")

LINK_DIRECTORIES("~/gecode/")

################ INCLUDE others Libs ####################
SET (CMAKE_INCLUDE_PATH "${MAINFOLDER}/include")
if(FORCE_32)
	SET (CMAKE_LINKER_PATH "${MAINFOLDER}/lib/${OS_SYSTEM}/lib32")
else()
	SET (CMAKE_LINKER_PATH "${MAINFOLDER}/lib/${OS_SYSTEM}/lib${ARCH_SYSTEM}")
endif(FORCE_32)

message(STATUS "INCLUDE_PATH: ${CMAKE_INCLUDE_PATH}" )
message(STATUS "LINKER_PATH: ${CMAKE_LINKER_PATH}" )

INCLUDE_DIRECTORIES(${CMAKE_INCLUDE_PATH})
LINK_DIRECTORIES(${CMAKE_LINKER_PATH})

#################### COMPILER FLAGS ####################
#Set environment variables to use solvers
#These macros are used on source files for specific solvers
#set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)       # remove -Wl,-Bdynamic
#set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
#set(CMAKE_SHARED_LIBRARY_C_FLAGS)         # remove -fPIC
#set(CMAKE_SHARED_LIBRARY_CXX_FLAGS)
#set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)    # remove -rdynamic
#set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

IF(CMAKE_COMPILER_IS_GNUCXX)
	#RESET DEFAULT FLAGS
	SET(CMAKE_CXX_FLAGS " -Wsign-compare -static-libgcc ")

    #-Wall -Wstrict-aliasing -Wextra
	SET(GECODE_FLAGS "-fcx-limited-range -fno-signaling-nans -fno-rounding-math -ffinite-math-only -fno-math-errno -fno-strict-aliasing -O3 -fvisibility=hidden -pipe -fPIC")

	SET(COMMON_FLAGS "${GECODE_FLAGS} -Wno-enum-compare -Wall -DGLIBCPP_FORCE_NEW=1 -DGLIBCXX_FORCE_NEW=1 -fno-omit-frame-pointer -std=c++0x -Wno-deprecated -fpermissive -fPIC ${USE_SOLVERS}")

	IF(DEBUG_BUILD)
	    SET(COMMON_FLAGS "-ggdb ${COMMON_FLAGS}")
	ELSE()
	    SET(COMMON_FLAGS "${COMMON_FLAGS}")
	ENDIF()

	if(FORCE_32)
		SET(CMAKE_CXX_COMPILE_OBJECT
		    "<CMAKE_CXX_COMPILER>  <DEFINES> <FLAGS> -c -m32  ${COMMON_FLAGS} <SOURCE> -o <OBJECT>")

		SET(CMAKE_CXX_LINK_EXECUTABLE
		    "<CMAKE_CXX_COMPILER>  <FLAGS> -m32  ${COMMON_FLAGS} <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
	else()

		SET(CMAKE_CXX_COMPILE_OBJECT
		    "<CMAKE_CXX_COMPILER>  <DEFINES> <FLAGS> -c ${COMMON_FLAGS} <SOURCE> -o <OBJECT>")

		SET(CMAKE_CXX_LINK_EXECUTABLE
		    "<CMAKE_CXX_COMPILER>  <FLAGS> -fmessage-length=0 ${COMMON_FLAGS} <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>")
	endif(FORCE_32)

endif(CMAKE_COMPILER_IS_GNUCXX)


#################### Configure Files ####################
FILE (GLOB_RECURSE CONFIGINPUTS1 include/*.in.h.cmake)
FILE (GLOB_RECURSE CONFIGINPUTS2 include/*.h.in.cmake)
FILE (GLOB_RECURSE CONFIGINPUTS3 src/*.in.h.cmake)
FILE (GLOB_RECURSE CONFIGINPUTS4 src/*.h.in.cmake)
LIST (APPEND CONFIGINPUTS "${CONFIGINPUTS1}")
LIST (APPEND CONFIGINPUTS "${CONFIGINPUTS2}")
LIST (APPEND CONFIGINPUTS "${CONFIGINPUTS3}")
LIST (APPEND CONFIGINPUTS "${CONFIGINPUTS4}")
LIST (APPEND CONFIGINPUTS "${CONFIGINPUTS5}")
LIST (APPEND CONFIGINPUTS "${CONFIGINPUTS6}")
FOREACH (CONFIGINPUT ${CONFIGINPUTS})
    STRING (REPLACE ".in.h.cmake" ".h" CONFIGOUTPUT1 "${CONFIGINPUT}")
    STRING (REPLACE ".h.in.cmake" ".h" CONFIGOUTPUT2 "${CONFIGOUTPUT1}")
    CONFIGURE_FILE ("${CONFIGINPUT}" "${CONFIGOUTPUT2}")
ENDFOREACH (CONFIGINPUT ${CONFIGINPUTS})

#ADD_SUBDIRECTORY(srclib)
ADD_SUBDIRECTORY(src)

#################### Add Build Targets ####################
#Build the test drivers if necessary
SET(BUILD_TESTS FALSE)
if(BUILD_TESTS)
    FILE (GLOB_RECURSE CONFIGINPUTS5 test/*.in.h.cmake)
    FILE (GLOB_RECURSE CONFIGINPUTS6 test/*.h.in.cmake)
    ADD_SUBDIRECTORY(test)

    message(STATUS "Building Test")
endif (BUILD_TESTS)

#################### Add Install Targets ####################
IF (EXISTS "${MAINFOLDER}/include/${PROJECT_NAME}" AND IS_DIRECTORY "${MAINFOLDER}/include/${PROJECT_NAME}")
    INSTALL(DIRECTORY "${MAINFOLDER}/include/${PROJECT_NAME}" DESTINATION "include")
ENDIF (EXISTS "${MAINFOLDER}/include/${PROJECT_NAME}" AND IS_DIRECTORY "${MAINFOLDER}/include/${PROJECT_NAME}")

#################### Add Documentation Targets ####################
#INCLUDE (DocumentationTargets)

#ADD MODULE (Cotire to speedup the build)
#set (CMAKE_MODULE_PATH "${MAINFOLDER}/tools/share/cmake")
#include(cotire)

#Add Executable with cotire to speed up building
#!IMPORTANT : Add All Executables at the end otherwise it does not work with cotire
#cotire(${PROJECT_NAME})
#cotire(${PROJECT_NAME}-tests)

