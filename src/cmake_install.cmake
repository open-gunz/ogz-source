# Install script for directory: C:/Users/unknown/source/repos/interlunar/source/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/unknown/source/repos/interlunar/source/src/../install")
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

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/unknown/source/repos/interlunar/source/src/sdk/zlib/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/sdk/curl/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/sdk/libsodium/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/sdk/sqlite/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/sdk/rapidxml/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/sdk/rapidjson/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/sdk/ini/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/sdk/bullet/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/sdk/portaudio/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/sdk/opus/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/sdk/imgui/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/cml/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/launcher/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/PatchCreator/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/CSCommon/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/RealSpace2/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/SafeUDP/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/Locator/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/MatchServer/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/Mint2/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/RealSound/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/Gunz/cmake_install.cmake")
  include("C:/Users/unknown/source/repos/interlunar/source/src/PartsIndexer/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Users/unknown/source/repos/interlunar/source/src/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
