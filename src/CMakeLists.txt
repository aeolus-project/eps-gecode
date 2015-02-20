
SET (EXECUTABLE_OUTPUT_PATH "${MAINFOLDER}/bin/${CMAKE_BUILD_TYPE}")
SET (LIBRARY_OUTPUT_PATH "${MAINFOLDER}/bin/${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES("${MAINFOLDER}/src")

#FORCE CXX COMPILATION FOR ALL SOURCES FILES
FILE (GLOB_RECURSE project_SRCS *.cpp *.cxx *.cc *.C *.c)
FOREACH (SRC_FILE ${project_SRCS})
	set_source_files_properties(${SRC_FILE} PROPERTIES LANGUAGE CXX)
ENDFOREACH (SRC_FILE ${project_SRCS})

#IMPORTANT : MUST RESPECT this order of linking

# version 3.7.3
# libgecodeflatzinc.so
# libgecodedriver.so
# libgecodeminimodel.so
# libgecodeset.so
# libgecodeint.so
# libgecodesearch.so
# libgecodekernel.so
# libgecodesupport.so

# version 4.0.0
# libgecodedriver.a
# libgecodeflatzinc.a
# libgecodefloat.a
# libgecodeint.a
# libgecodekernel.a
# libgecodeminimodel.a
# libgecodesupport.a
# libgecodeset.a


# version 3.7.3
# SET (LIBS gecodeflatzinc gecodedriver gecodefloat gecodeint gecodeminimodel gecodeset gecodeint gecodesearch gecodekernel gecodesupport)

#mpi open-pal open-rte mpi_cxx

# version 4.2.0
SET (LIBS pthread gecodedriver gecodeflatzinc gecodeminimodel gecodekernel gecodefloat gecodeint gecodeset gecodesupport gecodesearch)

SET (project_BIN ${PROJECT_NAME})
SET (project_LIB ${project_BIN}_lib)

#QT4_WRAP_CPP(project_MOC_SRCS_GENERATED ${project_MOC_HEADERS})
#ADD_EXECUTABLE(${project_BIN}_jemalloc ${project_SRCS})
#ADD_EXECUTABLE(${project_BIN}_tcmalloc ${project_SRCS})
#ADD_EXECUTABLE(${project_BIN}_malloc ${project_SRCS})

#ADD_LIBRARY(${project_LIB} SHARED ${project_SRCS})
#TARGET_LINK_LIBRARIES(${project_LIB} ${LIBS})

ADD_EXECUTABLE(${project_BIN} ${project_SRCS})

TARGET_LINK_LIBRARIES(${project_BIN} ${LIBS})
SET_TARGET_PROPERTIES(${project_BIN} PROPERTIES VERSION "${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR}" OUTPUT_NAME ${project_BIN} CLEAN_DIRECT_OUTPUT 1)

#INSTALL(TARGETS ${project_BIN} DESTINATION bin)
