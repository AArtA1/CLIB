#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

#include <clib/logs.hpp>

#include <string>

#ifndef NDEBUG
boost::log::sources::severity_logger< clib::severity_level > slg;
#endif

int main(int argc, char** argv) 
{
#ifndef NDEBUG
    clib::init_logs();
    //clib::tag_sev_filter("Flexfloat mean", clib::debug);
#endif

    doctest::Context context;

    // !!! THIS IS JUST AN EXAMPLE SHOWING HOW DEFAULTS/OVERRIDES ARE SET !!!

    // defaults
    context.addFilter("test-case-exclude", "*math*"); // exclude test cases with "math" in the name
    context.setOption("rand-seed", 324);              // if order-by is set to "rand" use this seed
    context.setOption("order-by", "file");            // sort the test cases by file and line

    context.applyCommandLine(argc, argv);

    // overrides
    context.setOption("no-breaks", true); // don't break in the debugger when assertions fail

    int res = context.run(); // run queries, or run tests unless --no-run is specified

    if(context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
        return res;          // propagate the result of the tests

    context.clearFilters(); // removes all filters added up to this point

    return res;
}
