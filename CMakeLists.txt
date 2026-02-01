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
project(CSchem VERSION 0.3.2)
set(CMAKE_PROJECT_HOMEPAGE_URL "http://danielwagenaar.net/cschem")

######################################################################
# Specify the C++ standard [must be done _before_ adding executable!]
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if (UNIX AND NOT APPLE) 
  add_compile_options(-fPIC) # ... but in fact, this is required on Linux
  add_compile_options(-W -Wall)
endif()

# Qt stuff
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 COMPONENTS Widgets Svg SvgWidgets PrintSupport REQUIRED)
set(QT_MAC_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

# Clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set_source_files_properties(tags PROPERTIES GENERATED true)
add_custom_command(OUTPUT tags
    COMMAND ctags -R --c++-kinds=+p --fields=+iaS --extra=+q . 
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

######################################################################
# Add the executables
add_executable(cschem WIN32 MACOSX_BUNDLE cschem/cschem.cpp tags)
add_executable(cpcb WIN32 MACOSX_BUNDLE cpcb/main.cpp tags)
configure_file(config.h.in config.h)
include(cschem/CMakeLists.txt)
include(cpcb/CMakeLists.txt)

# Libraries to link to - Qt must be mentioned
target_link_libraries(cschem Qt6::Widgets)
target_link_libraries(cschem Qt6::Svg)
target_link_libraries(cschem Qt6::SvgWidgets)
target_link_libraries(cschem Qt6::PrintSupport)
target_link_libraries(cpcb Qt6::Widgets)
target_link_libraries(cpcb Qt6::Svg)

# Produce configuration files
configure_file(config.h.in config.h)

# Include paths - Qt must be mentioned
target_include_directories(cschem PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(cschem PUBLIC "${Qt6Widgets_INCLUDE_DIRS}")
target_include_directories(cpcb PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(cpcb PUBLIC "${Qt6Widgets_INCLUDE_DIRS}")


######################################################################
# General packaging / installation
set(CPACK_PACKAGE_VENDOR "Daniel Wagenaar")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CSchem and CPCB")
set(CPACK_PACKAGE_DESCRIPTION
  "CSchem and CPCB are a pair of programs for drawing electronic circuit diagrams and laying those circuits out on a printed circuit board.")
set(CPACK_PACKAGE_EXECUTABLES "cschem;CSchem")

if (WIN32)
  include(tools/WinPack.CMakeLists.txt)
  include(CPack)
elseif (APPLE)
  include(tools/MacDMG.CMakeLists.txt)
elseif (UNIX)
  include(tools/UnixInstall.CMakeLists.txt)
  include(tools/DebPack.CMakeLists.txt)
  include(CPack)
endif()

# Packaging

