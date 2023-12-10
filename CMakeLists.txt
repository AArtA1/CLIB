## clib

cmake_minimum_required(VERSION 3.14)


##################################################################################################
##
##      Проверка системы
##
##################################################################################################


message("-- CMAKE_SYSTEM_INFO_FILE: ${CMAKE_SYSTEM_INFO_FILE}")
message("-- CMAKE_SYSTEM_NAME:      ${CMAKE_SYSTEM_NAME}")
message("-- CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message("-- CMAKE_SYSTEM:           ${CMAKE_SYSTEM}")


# Check if ubuntu
find_program(LSB_RELEASE_EXEC lsb_release)
execute_process(COMMAND ${LSB_RELEASE_EXEC} -is
    OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# By default - debug
if (NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

message(STATUS "Текущая операционная система: ${LSB_RELEASE_ID_SHORT}")


###################################################################################################
##
##      Информация о проекте
##
###################################################################################################


# Работает только с g++-12
find_program(CMAKE_COMPILER NAMES "g++-12")
if (NOT CMAKE_COMPILER)
    message(FATAL_ERROR "Установите GCC версии 12.3.0:\
                         \nsudo apt install g++-12 gcc-12")
endif()
set(CXX "g++-12")

# By default - debug
if (NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

project(clib VERSION 1.0 LANGUAGES CXX)
get_directory_property(IS_SUBPROJECT PARENT_DIRECTORY)


###################################################################################################
##
##      Опции проекта
##
###################################################################################################

if (IS_SUBPROJECT AND NOT DEFINED CLIB_TESTING)
    message(STATUS "clib не тестируется в режиме подмодуля")
    set(CLIB_TESTING OFF)
endif()

if (IS_SUBPROJECT AND NOT DEFINED CLIB_DOC)
    message(STATUS "Автодокументация для clib выключена в режиме подмодуля")
    set(CLIB_DOC OFF)
endif()

option(CLIB_TESTING "Включить модульное тестирование" ON)
option(CLIB_COVERAGE "Включить измерение покрытия кода тестами" OFF)
option(CLIB_DOC "Включить документирование" ON)

set(MY_SOURCES
    src/clib/Flexfloat.cpp
    src/clib/Flexfixed.cpp
    src/clib/logs.cpp
    src/clib/Uint32.cpp
    src/clib/converter.cpp
    src/clib/image.cpp
    src/clib/video.cpp
)

###################################################################################################
##
##      Флаги
##
###################################################################################################

# add_compile_options(
#     -fPIC
# )
set(MY_FLAGS

    -fPIC

    -Wall
    -Wextra
    -Wpedantic
    -Wcast-align
    -Wcast-qual
    -Wconversion
    -Wctor-dtor-privacy
    -Wenum-compare
    -Wfloat-equal
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Woverloaded-virtual
    -Wredundant-decls
    -Wsign-conversion
    -Wsign-promo
    -fdiagnostics-show-option
    -Wlogical-op
    -Wundef
    -Wshadow
    -Wstrict-overflow=2
    -Wwrite-strings
    -Wpointer-arith
    -Wformat=2
    -Wswitch-default
    -Wmissing-include-dirs
    -Wswitch-enum
    -Wdisabled-optimization
    -Weffc++
    -Winvalid-pch
    -Wunused-but-set-variable
    -Wunused-result
)

if(NOT CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

###################################################################################################
##
##      Библиотеки
##
###################################################################################################

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
set(Boost_VERSION 1.74.0)
set(Boost_INCLUDE_LIBRARIES log log_setup)
set(BOOST_ENABLE_CMAKE ON)

find_package(Boost COMPONENTS ${Boost_INCLUDE_LIBRARIES}) 
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(X11 REQUIRED)

###################################################################################################
##
##      Заголовочная библиотека
##
###################################################################################################

add_library(clib_headers INTERFACE)
target_include_directories(clib_headers INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_include_directories(clib_headers SYSTEM INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cimg>
    ${Boost_INCLUDE_DIRS}
    ${X11_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${JPEG_INCLUDE_DIR}
)
target_compile_features(clib_headers INTERFACE cxx_std_17)

add_library(clib::headers ALIAS clib_headers)

###################################################################################################
##
##      Компилируемая библиотека
##
###################################################################################################

add_library(clib_library ${MY_SOURCES})
target_compile_options(clib_library PRIVATE ${MY_FLAGS})

target_link_libraries(clib_library 
  PRIVATE 
    ${X11_LIBRARIES}
    ${PNG_LIBRARY}
    ${JPEG_LIBRARY}
  PUBLIC
    ${Boost_LIBRARIES}
    clib_headers
)

add_library(clib::library ALIAS clib_library)

###################################################################################################
##
##      Установка
##
###################################################################################################

install(DIRECTORY include/clib DESTINATION include)

install(TARGETS clib_headers clib_library EXPORT clibConfig)
install(EXPORT clibConfig NAMESPACE clib:: DESTINATION share/clib/cmake)

include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_BINARY_DIR}/clibConfigVersion.cmake"
    VERSION
        ${PROJECT_VERSION}
    COMPATIBILITY
        AnyNewerVersion
)
install(FILES "${PROJECT_BINARY_DIR}/clibConfigVersion.cmake" DESTINATION share/clib/cmake)

###################################################################################################
##
##      Тесты
##
###################################################################################################

if(NOT CLIB_TESTING)
    message(STATUS "Тестирование проекта clib выключено")
else()
    add_subdirectory(test)
endif()

###################################################################################################
##
##      Документация
##
###################################################################################################

if(NOT CLIB_DOC)
    message(STATUS "Автодокументация проекта clib выключена")
else()
    add_subdirectory(doc)
endif()

###################################################################################################
##
##      cppcheck
##
###################################################################################################

# find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
# if (CMAKE_CXX_CPPCHECK)
#     message(STATUS "cppcheck включен")
#     list(
#         APPEND CMAKE_CXX_CPPCHECK 
#             "--enable=all"
#             "--inconclusive"
#             "--force" 
#             "--inline-suppr"
#             "--suppressions-list=CppCheckSuppressions.txt"
#     )
# endif()
