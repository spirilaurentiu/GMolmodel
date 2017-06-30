cmake_minimum_required(VERSION 3.0)

find_package(Simbody 3.6 CONFIG REQUIRED) # you may have to set a path for config file location e.g.  modify the environment variable list specific for find_package..
INCLUDE_DIRECTORIES( ${Simbody_INCLUDE_DIR} )
LINK_DIRECTORIES( ${Simbody_LIB_DIR} )


#INCLUDE_DIRECTORIES("/home/nash_mit/git/refactoring/gmolmodel/simbody/SimTKmath/Integrators/src")
#this is a patch for the current structure of the project, it will be refactor after redesign of the gmodmodel integrator is done
set( Integrators_PATH 
     "${CMAKE_CURRENT_LIST_DIR}/../simbody/SimTKmath/Integrators/src"
     CACHE FILEPATH "Path to Integrator.h from Simbody..." )
INCLUDE_DIRECTORIES( ${Integrators_PATH} )


set( MOL_MODEL_INCLUDE 
     "${Simbody_INCLUDE_DIR}/.."
     CACHE FILEPATH "header include folder path for Molmodel" ) 
INCLUDE_DIRECTORIES( ${MOL_MODEL_INCLUDE} )
#I should add link_directories for molmodel and add them to TARTGET_LINK!!!
#or maybe not.. at the moment I'm not convince that gmolmodel dependes by molmodel lib mostly because it's an extension that it's already compiled into gmolmode. but I need to look more into the code for that
#path for libs is the same as simbody's so, no need to add it!


find_package(Eigen3 REQUIRED)
#find_package( PkgConfig )
#pkg_check_modules( EIGEN3 REQUIRED eigen3 )
include_directories( ${EIGEN3_INCLUDE_DIRS} )


#this is done because /home/nash_mit/Anaconda/envs/mmtk_environment/include/python2.7/MMTK/arrayobject.h need to make a decison
#and we only have numpy/oldnumeric.h
add_definitions(-DNUMPY=1)

set(Python_ADDITIONAL_VERSIONS 2)
FIND_PACKAGE(PythonInterp)
FIND_PACKAGE(PythonLibs REQUIRED)
include_directories( ${PYTHON_INCLUDE_DIRS} )
link_directories( ${PYTHON_LIBRARIES} )

set(mmtk_include 
    "${PYTHON_INCLUDE_DIRS}/MMTK" 
    CACHE FILEPATH "Adding header path to MMTK installed location")
include_directories( ${mmtk_include} )

execute_process(
  COMMAND "${PYTHON_EXECUTABLE}" -c "from distutils.sysconfig import get_python_lib; print get_python_lib()" 
  OUTPUT_VARIABLE PYTHON_SITE_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE ) 

set(PYTHON_SITE-PACKAGES_PATH
    ${PYTHON_SITE_PATH} 
    CACHE FILEPATH "side-packages python path")

#set(mmtk_libs "/home/nash_mit/git/refactoring/gmolmodel/mmtk_legacy/build/lib.linux-x86_64-2.6/MMTK/linux2")
set(mmtk_libs 
    "${PYTHON_SITE-PACKAGES_PATH}/MMTK/linux2"
    CACHE FILEPATH "Addinng path to MMTK libs folder")
link_directories( ${mmtk_libs} )

#not finished yet... in the end it should export a list of libs without "lib" and without ".so"
#the resoult variable should be puted in TARGET_LINK...
execute_process(
  COMMAND ls "${mmtk_libs}"
  OUTPUT_VARIABLE mmtk_LIBRARIES
  OUTPUT_STRIP_TRAILING_WHITESPACE )
#message( ${mmtk_LIBRARIES} )

#set(numpy_include "/home/nash_mit/Anaconda/envs/mmtk_environment/lib/python2.7/site-packages/numpy/core/include" )
set(numpy_include 
    "${PYTHON_SITE-PACKAGES_PATH}/numpy/core/include" 
    CACHE FILEPATH "Header path to numpy installed location")
include_directories( ${numpy_include} )


#set (Boost_NO_SYSTEM_PATHS ON)
#set (Boost_USE_MULTITHREADED ON)
#set (Boost_USE_STATIC_LIBS ON)
#set (Boost_USE_STATIC_RUNTIME OFF)
#set (BOOST_ALL_DYN_LINK OFF)

find_package(Boost 1.55 REQUIRED) #I'm using 1.58
INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIR})
LINK_DIRECTORIES(${BOOST_LIBRARY_DIRS})


set(LIBGMOLMODEL_SRC 
	bAddParams.cpp 
	bMainResidue.cpp 
	bMoleculeReader.cpp 
	bSystem.cpp
	)

include_directories(../include/gmolmodel)

add_library(GMOLMODEL_dynamic SHARED ${LIBGMOLMODEL_SRC})
add_library(GMOLMODEL_static  STATIC ${LIBGMOLMODEL_SRC})


# link Simbody libraries
TARGET_LINK_LIBRARIES(GMOLMODEL_dynamic 
		${Simbody_LIBRARIES}
		SimTKmolmodel #adding molmodel lib as a dependence.. this needs to be refactor since the name should be changeable 
		${BOOST_LIBRARIES} 
                ${PYTHON_LIBRARIES} # do I realy need to link python lib into gmolmodel? it make no sense... but I still don't know verry well code...
		m )
		#${mmtk_libs} )
TARGET_LINK_LIBRARIES(GMOLMODEL_static 
		${Simbody_LIBRARIES}
		SimTKmolmodel #adding molmodel lib as a dependence.. this needs to be refactor since the name should be changeable
		${BOOST_LIBRARIES}
                ${PYTHON_LIBRARIES}  # same as for dynamic lib... I don't think python shoud be part of this lib...
		m )
                #${mmtk_libs} )

#set_target_properties(GMOLMODEL_dynamic PROPERTIES OUTPUT_NAME "gmolmodel")
#set_target_properties(GMOLMODEL_dynamic PROPERTIES VERSION 1.2 SOVERSION 1)

#set_target_properties(GMOLMODEL_static PROPERTIES OUTPUT_NAME "gmoldel")

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)