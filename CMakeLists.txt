# CMakeLists.txt - Part of CSchem, (C) Daniel Wagenaar 2021

# NEW BUILD INSTRUCTIONS (as of version 0.2.0)
# Type:
#   cd build
#   cmake ..
# to build CSCHEM.


######################################################################
cmake_minimum_required(VERSION 3.11.0)

######################################################################
# Set the project name and version
project(CSchem VERSION 0.2.3)
set(CMAKE_PROJECT_HOMEPAGE_URL "http://danielwagenaar.net/cschem")

######################################################################
# Specify the C++ standard [must be done _before_ adding executable!]
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # I thought this would do the trick...
if (UNIX) 
  add_compile_options(-fPIC) # ... but in fact, this is required on Linux
endif()

# Qt stuff
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5 COMPONENTS Widgets Svg PrintSupport REQUIRED)

######################################################################
# Add the executables
add_executable(cschem WIN32 MACOSX_BUNDLE cschem/cschem.cpp)
add_executable(cpcb WIN32 MACOSX_BUNDLE cpcb/main.cpp)
configure_file(config.h.in config.h)
include(cschem/CMakeLists.txt)
include(cpcb/CMakeLists.txt)

# Libraries to link to - Qt must be mentioned
target_link_libraries(cschem Qt5::Widgets)
target_link_libraries(cschem Qt5::Svg)
target_link_libraries(cschem Qt5::PrintSupport)
target_link_libraries(cpcb Qt5::Widgets)
target_link_libraries(cpcb Qt5::Svg)


if (WIN32)
  target_sources(cschem PRIVATE cschem/cschem.iconset/cschem.rc)
  target_sources(cpcb PRIVATE cpcb/cpcb.iconset/cpcb.rc)
  target_link_libraries(cschem secur32.lib)
  target_link_libraries(cpcb secur32.lib)
endif()
if (APPLE)
  set(MACOSX_BUNDLE_ICON_FILE cschem.icns)
  add_custom_command(TARGET cschem POST_BUILD
    COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/cschem.app/Contents/Resources
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/cschem/cschem.icns 
      ${CMAKE_CURRENT_BINARY_DIR}/cschem.app/Contents/Resources
    COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/symbols
      ${CMAKE_CURRENT_BINARY_DIR}/cschem.app/Contents/Resources
  )
  add_custom_command(TARGET cpcb POST_BUILD
    COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/cpcb.app/Contents/Resources
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/cpcb/cpcb.icns 
      ${CMAKE_CURRENT_BINARY_DIR}/cpcb.app/Contents/Resources/cschem.icns
    COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/pcb-outlines
      ${CMAKE_CURRENT_BINARY_DIR}/cpcb.app/Contents/Resources
  )
endif()

# Produce configuration files
configure_file(config.h.in config.h)

# Include paths - Qt must be mentioned
target_include_directories(cschem PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(cschem PUBLIC "${Qt5Widgets_INCLUDE_DIRS}")
target_include_directories(cpcb PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(cpcb PUBLIC "${Qt5Widgets_INCLUDE_DIRS}")

######################################################################
if (UNIX AND NOT APPLE)
  add_subdirectory("docs")
endif()

######################################################################
# General packaging / installation
set(CPACK_PACKAGE_VENDOR "Daniel Wagenaar")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CSchem and CPCB")
set(CPACK_PACKAGE_DESCRIPTION
  "CSchem and CPCB are a pair of programs for drawing electronic circuit diagrams and laying those circuits out on a printed circuit board.")
set(CPACK_PACKAGE_EXECUTABLES "cschem;CSchem")

if (WIN32)
  # NSIS packaging - to be updated
  include(WinPack.CMakeLists.txt)
elseif (UNIX)
  include(UnixInstall.CMakeLists.txt)
  include(DebPack.CMakeLists.txt)
endif()

# Packaging
include(CPack)

