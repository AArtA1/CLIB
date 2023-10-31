#include "clib/logs.hpp"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/core/null_deleter.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

namespace clib {

boost::log::sources::severity_logger< severity_level > slg;

std::ostream& operator<< (std::ostream& strm, severity_level level)
{
    static const char* strings[] =
    {
        "trace",
        "debug",
        "info",
        "warning",
        "error",
        "fatal"
    };

    if (static_cast< std::size_t >(level) < sizeof(strings) / sizeof(*strings))
        strm << strings[level];
    else
        strm << static_cast< int >(level);

    return strm;
}



namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(tag_attr, "Tag", std::string)



#define FMT_OUT_black   "0"	
#define FMT_OUT_red     "1"	
#define FMT_OUT_green   "2"	
#define FMT_OUT_yellow  "3"	
#define FMT_OUT_blue    "4"	
#define FMT_OUT_magenta "5"	
#define FMT_OUT_cyan    "6"	
#define FMT_OUT_white   "7;0"

#define FMT_OUT_normal ";0"
#define FMT_OUT_bold   ";1"
#define FMT_OUT_faint  ";2"

void coloring_formatter(
    logging::record_view const& rec, logging::formatting_ostream& strm)
{
    auto sseverity = rec[severity];
    if (sseverity)
    {
        // Set the color
        switch (sseverity.get())
        {
        case trace:
            strm << "\033[3" FMT_OUT_white FMT_OUT_faint "m";
            break;
        case debug:
            strm << "\033[3" FMT_OUT_cyan FMT_OUT_bold "m";
            break;
        case info:
            strm << "\033[3" FMT_OUT_green FMT_OUT_normal "m";
            break;
        case warning:
            strm << "\033[3" FMT_OUT_yellow FMT_OUT_normal "m";
            break;
        case error:
            strm << "\033[3" FMT_OUT_red FMT_OUT_normal "m";
            break;
        case fatal:
            strm << "\033[3" FMT_OUT_red FMT_OUT_normal "m";
            break;
        default:
            break;
        }
    }

    strm << std::setw(6) << std::setfill('0') << rec[line_id] << std::setfill(' ')
        << ": <" << rec[severity] << ">\t"
        << expr::stream << "[" << rec[tag_attr] << "] "
        << rec[expr::smessage];


    if (sseverity)
    {
        // Restore the default color
        strm << "\033[0m";
    }
}

void init_logs()
{
    // Setup the common formatter for all sinks
    
    logging::formatter fmt = expr::stream
        << std::setw(6) << std::setfill('0') << line_id << std::setfill(' ')
        << ": <" << severity << ">\t"
        << expr::if_(expr::has_attr(tag_attr))
           [
               expr::stream << "[" << tag_attr << "] "
           ]
        << expr::smessage;

    // Initialize sink
    typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;

     // create sink to stdout
    boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();
    sink->locked_backend()->add_stream(
        boost::shared_ptr<std::ostream>(&std::cout, boost::null_deleter{}));

    // flush
    sink->locked_backend()->auto_flush(true);

    sink->set_formatter(&coloring_formatter);

    logging::core::get()->add_sink(sink);

    // Add attributes
    logging::add_common_attributes();
    //->
}

void default_filter()
{   
    logging::core::get()->set_filter(severity >= error);
}

void severity_level_filter(severity_level sev)
{   
    logging::core::get()->set_filter(severity >= sev);
}

void tag_filter(std::string tag)
{   
    logging::core::get()->set_filter(expr::has_attr(tag_attr) && tag_attr == tag);
}

void turn_logs_off()
{   
    logging::core::get()->set_filter(expr::has_attr(tag_attr) && tag_attr == "THIS_TAG_WILL_BE_NEVER_EXIST");
}

void turn_logs_on()
{   
    logging::core::get()->set_filter(severity >= trace);
}


}
