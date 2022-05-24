# Install script for directory: C:/Users/GooseWrld/source/repos/ogzbunz/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/GooseWrld/source/repos/ogzbunz/src/../install")
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
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/sdk/zlib/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/sdk/curl/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/sdk/libsodium/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/sdk/sqlite/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/sdk/rapidxml/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/sdk/rapidjson/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/sdk/ini/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/sdk/bullet/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/sdk/portaudio/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/sdk/opus/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/sdk/imgui/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/cml/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/launcher/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/PatchCreator/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/CSCommon/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/RealSpace2/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/SafeUDP/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/Locator/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/MatchServer/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/Mint2/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/RealSound/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/Gunz/cmake_install.cmake")
  include("C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/PartsIndexer/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Users/GooseWrld/source/repos/ogzbunz/build/win32/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
