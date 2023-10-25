set(CLIB_DOCTEST_VERSION 2.4.7)
set(CLIB_DOCTEST_REPOSITORY https://github.com/doctest/doctest.git)

find_package(doctest ${CLIB_DOCTEST_VERSION})

if (doctest_FOUND)
    message(STATUS "Найден doctest ${doctest_VERSION}: ${doctest_DIR}")
else()
    message(STATUS
        "doctest ${CLIB_DOCTEST_VERSION} будет взят с гитхаба: ${CLIB_DOCTEST_REPOSITORY}")

    Set(FETCHCONTENT_QUIET FALSE)
    include(FetchContent)
    FetchContent_Declare(doctest
        USES_TERMINAL_DOWNLOAD TRUE 
        GIT_PROGRESS TRUE
        GIT_REPOSITORY ${CLIB_DOCTEST_REPOSITORY}
        GIT_TAG ${CLIB_DOCTEST_VERSION}
    )
    FetchContent_MakeAvailable(doctest)
endif()
