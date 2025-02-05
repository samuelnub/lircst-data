cmake_minimum_required(VERSION 3.28)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

SET(CMAKE_CXX_FLAGS_DEBUG "-g3")
set(CMAKE_CXX_FLAGS_RELEASE "-g0 -O3")

project(LircstData)

# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF to build a batch mode only executable
find_package(Geant4 REQUIRED ui_all vis_all)

# Setup Geant4 include directories and compile definitions
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)

# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

# Add the executable, and link it to the Geant4 libraries
add_executable(lircstData main.cc ${sources} ${headers})
target_link_libraries(lircstData ${Geant4_LIBRARIES})

set(LIRCSTDATA_SCRIPTS
    vis.mac
    )

foreach(_script ${LIRCSTDATA_SCRIPTS})
    configure_file(
        ${PROJECT_SOURCE_DIR}/macro/${_script}
        ${PROJECT_BINARY_DIR}/${_script}
        COPYONLY
    )
endforeach()

# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
install(TARGETS lircstData DESTINATION bin)