#pragma once
#include "common.hpp"

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

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

//
// global logger
//
extern boost::log::sources::severity_logger< severity_level > slg;

// 
// The operator puts a human-friendly representation of the severity level to the stream
//
std::ostream& operator<< (std::ostream& strm, severity_level level);

#define CLOG(lvl) BOOST_LOG_SEV(slg, lvl)
#define LOG(lvl)  BOOST_LOG_SEV(clib::slg, clib::lvl)

void init_logs();

void default_filter();

void sev_filter(severity_level sev);

void tag_filter(std::string tag);

void tag_sev_filter(std::string tag, severity_level sev);

void turn_logs_off();

void turn_logs_on();

}