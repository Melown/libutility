#ifndef utility_uri_hpp_included_
#define utility_uri_hpp_included_

/*
 * Uri parsing
 *
 * Based on code from https://github.com/CovenantEyes/uri-parser (BSD-licenced)
 *
 * Modified by Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * TODO: decode/encode url-encoded characters in path etc
 */

#include <string>
#include <sstream>
#include <cstdlib>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>

namespace utility {

struct Uri {
    std::string schema;
    std::string user;
    std::string password;
    std::string host;
    std::string path;
    std::string search;
    int port;

    std::string join() const;
};

namespace detail {

//--- Helper Functions --------------------------------------------------------
inline std::string tailSlice(std::string &subject, const std::string &delimiter
                             , bool keep_delim = false)
{
    // Chops off the delimiter and everything that follows
    // (destructively) returns everything after the delimiter
    auto delimiterLocation = subject.find(delimiter);

    if (delimiterLocation == std::string::npos) {
        return {};
    }

    auto start(keep_delim ? delimiterLocation
               : delimiterLocation + delimiter.length());
    auto end(subject.length() - start);
    auto output(subject.substr(start, end));
    subject = subject.substr(0, delimiterLocation);

    return output;
}

inline std::string headSlice(std::string &subject
                             , const std::string &delimiter)
{
    // Chops off the delimiter and everything that precedes (destructively)
    // returns everthing before the delimeter
    auto delimiterLocation(subject.find(delimiter));

    if (delimiterLocation == std::string::npos) {
        return {};
    }

    auto output(subject.substr(0, delimiterLocation));
    subject = subject.substr
        (delimiterLocation + delimiter.length()
         , subject.length() - (delimiterLocation + delimiter.length()));
    return output;
}


//--- Extractors -----------------------
inline int extractPort(std::string &hostport) {
    try {
        return boost::lexical_cast<int>(tailSlice(hostport, ":"));
    } catch (const boost::bad_lexical_cast&) {
        return -1;
    }
}

inline std::string extractPath(std::string &in) {
    return tailSlice(in, "/", true);
}

inline std::string extractSchema(std::string &in) {
    return headSlice(in, "://");
}

inline std::string extractSearch(std::string &in) {
    return tailSlice(in, "?");
}

inline std::string extractPassword(std::string &userpass) {
    return tailSlice(userpass, ":");
}

inline std::string extractUserpass(std::string &in) {
    return headSlice(in, "@");
}

} // namespace detail

std::string urlEncode(const std::string &in, bool plus = true);

//--- Public Interface --------------------------------------------------------
inline Uri parseUri(std::string in) {
    Uri ret;
    ret.port = -1;

    ret.schema
        = boost::algorithm::to_lower_copy(detail::extractSchema(in));
    ret.search = detail::extractSearch(in);
    ret.path = detail::extractPath(in);
    std::string userpass = detail::extractUserpass(in);
    ret.password = detail::extractPassword(userpass);
    ret.user = userpass;
    ret.port = detail::extractPort(in);
    ret.host = boost::algorithm::to_lower_copy(in);

    return ret;
}

inline std::string Uri::join() const
{
    std::ostringstream os;

    os << schema << "://";
    if (!user.empty()) {
        os << user;
        if (!password.empty()) {
            os << ':' << password;
        }
        os << '@';
    }
    if (!host.empty()) {
        os << host;
        if (port > 0) {
            os << ':' << port;
        }
    }
    if (!path.empty()) {
        if (path[0] != '/') { os << '/'; };
        os << path;
    }

    if (!search.empty()) {
        os << '?' << search;
    }

    return os.str();
}

} // namespace utility

#endif // utility_uri_hpp_included_
