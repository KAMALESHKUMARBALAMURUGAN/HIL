# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/kamalesh.kb/ESP/v5.2/esp-idf/components/bootloader/subproject"
  "C:/Users/kamalesh.kb/HIL_PACKCURR/HIL/i2c_oled/build/bootloader"
  "C:/Users/kamalesh.kb/HIL_PACKCURR/HIL/i2c_oled/build/bootloader-prefix"
  "C:/Users/kamalesh.kb/HIL_PACKCURR/HIL/i2c_oled/build/bootloader-prefix/tmp"
  "C:/Users/kamalesh.kb/HIL_PACKCURR/HIL/i2c_oled/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/kamalesh.kb/HIL_PACKCURR/HIL/i2c_oled/build/bootloader-prefix/src"
  "C:/Users/kamalesh.kb/HIL_PACKCURR/HIL/i2c_oled/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/kamalesh.kb/HIL_PACKCURR/HIL/i2c_oled/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/kamalesh.kb/HIL_PACKCURR/HIL/i2c_oled/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
