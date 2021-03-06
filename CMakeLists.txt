cmake_minimum_required(VERSION 2.6)
#set(CMAKE_C_COMPILER ccache)
#set(CMAKE_C_COMPILER_ARG1 gcc)
#set(CMAKE_CXX_COMPILER ccache)
#set(CMAKE_CXX_COMPILER_ARG1 g++)
set(TARGET SyntheticLightcurves)
project(${TARGET})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} $ENV{HOME}/cmake_packages)

find_package(CCfits REQUIRED)
#find_package(UnitTest++ REQUIRED)
#find_package(fitsio REQUIRED)
#find_package(Qt4 COMPONENTS QtCore REQUIRED)
find_package(Boost 1.3 COMPONENTS filesystem system REQUIRED)
find_package(pugixml REQUIRED)
#find_package(glog REQUIRED)
find_package(tclap REQUIRED)
find_package(nr3 REQUIRED)

#include(${QT_USE_FILE})

include_directories(
    ${${TARGET}_SOURCE_DIR}/include
    ${CCFITS_INCLUDE_DIR}
    ${PUGIXML_INCLUDE_DIR}
   ${TCLAP_INCLUDE_DIR}
   ${Boost_INCLUDE_DIR}
   #${UNITTESTPP_INCLUDE_DIR}
   ${NR3_INCLUDE_DIR}
   #${GLOG_INCLUDE_DIR}
#    ${FITSIO_INCLUDE_DIR}
    )

link_directories(${CMAKE_BINARY_DIR})

aux_source_directory(
    ${${TARGET}_SOURCE_DIR}/src
    SRCS
    )

set(HDRS
    ${${TARGET}_SOURCE_DIR}/include/AlterTransit.h
    ${${TARGET}_SOURCE_DIR}/include/Application.h
    ${${TARGET}_SOURCE_DIR}/include/CopyFileEfficiently.h
    ${${TARGET}_SOURCE_DIR}/include/CopyParameters.h
    ${${TARGET}_SOURCE_DIR}/include/Exceptions.h
    ${${TARGET}_SOURCE_DIR}/include/FuncIntensity.h
    ${${TARGET}_SOURCE_DIR}/include/FuncOmega.h
    ${${TARGET}_SOURCE_DIR}/include/FuncSquare.h
    ${${TARGET}_SOURCE_DIR}/include/GetSystemMemory.h
    ${${TARGET}_SOURCE_DIR}/include/Lightcurve.h
    ${${TARGET}_SOURCE_DIR}/include/ObjectSkipDefs.h
    ${${TARGET}_SOURCE_DIR}/include/SortedIndex.h
    ${${TARGET}_SOURCE_DIR}/include/ToStringList.h
    ${${TARGET}_SOURCE_DIR}/include/ValidXML.h
    ${${TARGET}_SOURCE_DIR}/include/WaspDateConverter.h
    ${${TARGET}_SOURCE_DIR}/include/XMLParserPugi.h
    ${${TARGET}_SOURCE_DIR}/include/config.h.in
    ${${TARGET}_SOURCE_DIR}/include/constants.h
    ${${TARGET}_SOURCE_DIR}/include/timer.h
)


add_executable(${TARGET} ${SRCS} ${HDRS})

target_link_libraries(
   ${TARGET}
   #${UNITTESTPP_LIBRARIES}
   ${CCFITS_LIBRARIES}
   ${PUGIXML_LIBRARIES}
   ${Boost_LIBRARIES}
   #${GLOG_LIBRARIES}
#    ${FITSIO_LIBRARIES}
#    ${QT_LIBRARIES}
   )

# add the unit testing code
#add_executable(
    #TestLightcurve
    #${CMAKE_SOURCE_DIR}/tests/TestLightcurve.cpp
    ##${CMAKE_SOURCE_DIR}/src/Lightcurve.cpp
    #)


#add_executable(
    #TestConfig
    #${CMAKE_SOURCE_DIR}/tests/TestConfig.cpp
    #)

#add_library(
    #AlterLightcurves
    #SHARED
    #${SRCS}
    #)


#target_link_libraries(
    #AlterLightcurves
   #${UNITTESTPP_LIBRARIES}
   #${CCFITS_LIBRARIES}
   #${PUGIXML_LIBRARIES}
   ##${GLOG_LIBRARIES}
##    ${FITSIO_LIBRARIES}
##    ${QT_LIBRARIES}
   #)


#message(STATUS ${CMAKE_BINARY_DIR})



#target_link_libraries(
    #TestLightcurve
    #${UNITTESTPP_LIBRARIES}
    #-lAlterLightcurves
    #)


#target_link_libraries(
    #TestConfig
    #${UNITTESTPP_LIBRARIES}
    #-lAlterLightcurves
    #)

install(TARGETS ${TARGET} DESTINATION bin)
install(PROGRAMS ${${TARGET}_SOURCE_DIR}/GenerateModels.py DESTINATION bin)
install(FILES ${${TARGET}_SOURCE_DIR}/WASP11.xml ${${TARGET}_SOURCE_DIR}/NG11.xml ${${TARGET}_SOURCE_DIR}/WASP12.xml DESTINATION share)
set_property(TARGET ${TARGET} PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
