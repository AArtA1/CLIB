#pragma once
#include "common.hpp"

#include <boost/log/trivial.hpp>

namespace clib {

// We define our own severity levels
enum severity_level
{
    trace,
    debug,
    info,
    warning,
    error,
    fatal
};

extern boost::log::sources::severity_logger< severity_level > slg;

// The operator puts a human-friendly representation of the severity level to the stream
std::ostream& operator<< (std::ostream& strm, severity_level level);

#define LOG(lvl) BOOST_LOG_SEV(slg, lvl)

void init_logs();

void init_filter();

}