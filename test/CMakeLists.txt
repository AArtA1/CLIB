include(${PROJECT_SOURCE_DIR}/cmake/doctest.cmake)

add_executable(clib-unit-tests test_main.cpp)
target_sources(clib-unit-tests PRIVATE clib/Flexfloat.cpp)
target_link_libraries(clib-unit-tests
    PRIVATE
        clib::library
        doctest::doctest
)

find_program(GCOVR_EXECUTABLE gcovr)
if(CLIB_COVERAGE AND GCOVR_EXECUTABLE)
    message(STATUS "Измерение покрытия кода тестами включено")

    target_compile_options(clib-unit-tests PRIVATE --coverage)
    target_link_options(clib-unit-tests PRIVATE -fprofile-arcs)

    target_compile_options(clib_library PRIVATE --coverage)
    target_link_options(clib_library PRIVATE -fprofile-arcs)

    add_custom_target(coverage
        COMMAND
            ${GCOVR_EXECUTABLE}
                --root=${PROJECT_SOURCE_DIR}/
                --filter=${PROJECT_SOURCE_DIR}/include
                --filter=${PROJECT_SOURCE_DIR}/src
                --object-directory=${PROJECT_BINARY_DIR}
        DEPENDS
            check
    )
elseif(CLIB_COVERAGE AND NOT GCOVR_EXECUTABLE)
    set(CLIB_COVERAGE OFF)
    message(WARNING "Для замеров покрытия кода тестами требуется программа gcovr")
endif()

add_custom_target(check ALL COMMAND clib-unit-tests)
