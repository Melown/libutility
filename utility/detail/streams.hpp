/**
 * @file detail/streams.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * This module adds utilities for C++ iostream.
 */

#ifndef utility_detail_streams_hpp_included_
#define utility_detail_streams_hpp_included_

#include <cctype>
#include <cstddef>
#include <array>
#include <istream>
#include <ostream>
#include <fstream>
#include <algorithm>

namespace utility { namespace detail {

template<typename CharT, typename Traits>
inline std::basic_istream<CharT, Traits>&
resetFailOnEof(std::basic_istream<CharT, Traits> &is)
{
    if (is.eof()) {
        is.clear(is.rdstate() & ~std::ios::failbit);
    }
    return is;
}

/** Consumes next character from stream and checks it with given pattern.
 *  Skips whitespaces if skipws is set.
 */
template<typename CharT> struct Expect { CharT c; };

template<typename CharT, typename Traits>
inline std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &is, const Expect<CharT> &ce)
{
    bool skipws(is.flags() & std::ios::skipws);
    for (;;) {
        auto c(is.get());
        if (c == ce.c) {
            // matched
            return is;
        }

        if (skipws && std::isspace(c)) {
            // skip ws
            continue;
        }
        // failed
        is.setstate(std::ios::failbit);

        break;
    }
    return is;
}

template<typename CharT> struct Match {
    CharT c;
    bool matched;

    Match(CharT c) : c(c), matched(false) {}
};

template<typename CharT, typename Traits>
inline std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits> &is, Match<CharT> &ce)
{
    ce.matched = false;

    // check for stream validity and eat whitespaces
    std::istream::sentry s(is);
    if (!s) {
        return resetFailOnEof(is);
    };

    auto c(is.get());
    if (is.eof()) {
        return resetFailOnEof(is);
    };

    if (c == ce.c) {
        // matched
        ce.matched = true;
        return is;
    }

    // not found; return back what was read
    is.unget();
    return is;
}

template<typename T, std::size_t size>
struct ArrayPrinter {
    const T *data;
    ArrayPrinter(const T *data, const std::string &separator)
        : data(data), separator(separator)
    {}

    friend std::ostream&
    operator<<(std::ostream &os, const ArrayPrinter<T, size> &a)
    {
        bool first(true);
        std::for_each(a.data, a.data + size
                      , [&](const T &d) -> void {
                          if (first) { first = false; }
                          else { os << a.separator; }
                          os << d;
                      });

        return os;
    }

    const std::string separator;
};

template<typename Container>
struct ContainerPrinter {
    const Container &container;
    ContainerPrinter(const Container &container
                     , const std::string separator = "")
        : container(container), separator(separator)
    {}

    friend std::ostream&
    operator<<(std::ostream &os, const ContainerPrinter<Container> &c)
    {
        bool first(true);
        std::for_each(c.container.begin(), c.container.end()
                      , [&](const typename Container::value_type &d) -> void {
                          if (first) { first = false; }
                          else { os << c.separator; }
                          os << d;
                      });

        return os;
    }

    const std::string separator;
};

/** Dumpable is a class having this member function:
 *  template <typename E, typename T>
 *  std::basic_ostream<E, T>&
 *  dump( std::basic_ostream<E, T> &os, const std::string & prefix) const;
 */
template <typename Dumpable>
struct Dumper {
    Dumper(const Dumpable &dumpable, const std::string &prefix)
        : dumpable(dumpable), prefix(prefix)
    {}

    const Dumpable &dumpable;
    const std::string &prefix;

private:
    Dumpable& operator=(Dumpable&); // assignment operator deleted
};

template <typename E, typename T, typename Dumpable>
std::basic_ostream<E, T>&
dump(std::basic_ostream<E,T> &os, const Dumpable &dumpable
     , const std::string &prefix = std::string())
{
    return dumpable.dump(os, prefix);
}

template <typename E, typename T, typename Dumpable>
std::basic_ostream<E, T>&
operator<<(std::basic_ostream<E,T> &os, const Dumper<Dumpable> &dumper)
{
    // return dumper.dumpable.dump(os, dumper.prefix);
    return dump(os, dumper.dumpable, dumper.prefix);
}

template <typename Container>
struct Join {
    const Container &c;
    const std::string &sep;
};


template <typename E, typename T, typename Container>
inline std::basic_ostream<E, T>&
operator<<(std::basic_ostream<E, T> &os, const Join<Container> &j)
{
    bool first = true;
    for (const auto &e : j.c) {
        if (!first) {
            os << j.sep;
        }
        os << e;
        first = false;
    }
    return os;
}

} } // namespace utility::detail

#endif // utility_detail_streams_hpp_included_
