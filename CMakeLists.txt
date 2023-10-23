###################################################################################################
##
##      Информация о проекте
##
###################################################################################################

cmake_minimum_required(VERSION 3.14)
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

if (IS_SUBPROJECT AND NOT DEFINED CLIB_WANDBOX)
    message(STATUS "Онлайн-песочница для clib выключена в режиме подмодуля")
    set(CLIB_WANDBOX OFF)
endif()

option(CLIB_TESTING "Включить модульное тестирование" ON)
option(CLIB_COVERAGE "Включить измерение покрытия кода тестами" OFF)
option(CLIB_DOC "Включить документирование" ON)
option(CLIB_WANDBOX "Включить онлайн-песочницу" ON)

###################################################################################################
##
##      Опции компиляции
##
###################################################################################################

if((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    add_compile_options(
        -Werror

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
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4 /WX)
endif()

if(NOT CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

###################################################################################################
##
##      Заголовочная библиотека
##
###################################################################################################

add_library(clib_headers INTERFACE)
target_include_directories(clib_headers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(clib_headers INTERFACE cxx_std_17)

add_library(clib::headers ALIAS clib_headers)

###################################################################################################
##
##      Компилируемая библиотека
##
###################################################################################################

add_library(clib_library src/clib/source.cpp)
target_link_libraries(clib_library PUBLIC clib_headers)

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
##      Онлайн-песочница
##
###################################################################################################

if(NOT CLIB_WANDBOX)
    message(STATUS "Онлайн-песочница для проекта clib выключена")
else()
    add_subdirectory(online)
endif()
