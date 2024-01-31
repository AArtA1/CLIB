#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

#include <clib/logs.hpp>

#include <string>

#ifdef BOOST_LOGS
boost::log::sources::severity_logger<clib::severity_level> slg;
#endif

int main(int argc, char **argv)
{
#ifdef BOOST_LOGS
    clib::init_logs();
    clib::sev_filter(clib::debug);
#endif

    doctest::Context context;
    context.addFilter("test-case", "Test Demosaic");
    context.applyCommandLine(argc, argv);

    // overrides
    context.setOption("no-breaks", true); // don't break in the debugger when assertions fail
    int res = context.run(); // run queries, or run tests unless --no-run is specified
    if (context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
        return res;           // propagate the result of the tests
    context.clearFilters(); // removes all filters added up to this point

    return res;
}
