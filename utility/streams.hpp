/**
 * @file streams.hpp
 * @author Vaclav Blazek <vaclav.blazek@citationtech.net>
 *
 * This module adds utilities for C++ iostream.
 */

#ifndef utility_streams_hpp_included_
#define utility_streams_hpp_included_

#include <string>

#include <boost/filesystem.hpp>

#include "detail/streams.hpp"

/** This module adds support for C++ iostream.
 */
namespace utility {

using detail::resetFailOnEof;

/** Consumes next character from stream and checks it with given pattern.
 *  Skips whitespaces if skipws is set.
 */
template<typename CharT>
inline detail::Expect<CharT> expect(CharT c) {
    return {c};
}

/** Similar to expect(c) but returns char back on mismatch.
 */
template<typename CharT>
inline detail::Match<CharT> match(CharT c) {
    return {c};
}

template<typename T, std::size_t size>
detail::ArrayPrinter<T, size>
arrayPrinter(const T(&data)[size], const std::string &separator = ", ")
{
    return {data, separator};
}

template<typename T, std::size_t size>
detail::ArrayPrinter<T, size>
arrayPrinter(const std::array<T, size> &array
             , const std::string &separator = ", ")
{
    return {&array[0], separator};
}

template <typename Dumpable>
detail::Dumper<Dumpable> dump(const Dumpable &dumpable
                              , const std::string &prefix = "")
{
    return {dumpable, prefix};
}

struct LManip {
    typedef std::function<void (std::ostream&)> type;
    const type &l;
    LManip(const type &l) : l(l) {}
};

inline std::ostream& operator<<(std::ostream &os, const LManip &l)
{
    l.l(os);
    return os;
}

template <typename Container>
detail::Join<Container> join(const Container &c, const std::string &sep)
{
    return {c, sep};
}

template<typename T, int size>
inline void write(std::ostream &os, const T(&v)[size]) {
    os.write(reinterpret_cast<const char*>(v), size * sizeof(T));
}

template<typename T, int size>
inline void write(const boost::filesystem::path &file, const T(&v)[size])
{
    std::ofstream f;
    f.exceptions(std::ios::badbit | std::ios::failbit);
    f.open(file.string(), std::ios_base::out | std::ios_base::trunc);
    write(f, v);
    f.close();
}

inline std::string read(const boost::filesystem::path &file)
{
    std::ifstream f;
    f.exceptions(std::ios::badbit | std::ios::failbit);
    f.open(file.string(), std::ios_base::in);
    f.seekg(0, std::ifstream::end);
    auto size(f.tellg());
    f.seekg(0);
    std::vector<char> tmp(size);
    f.read(&tmp[0], tmp.size());
    f.close();
    return { tmp.begin(), tmp.end() };
}

// helper classes for stream I/O with increased buffer size

namespace detail {
    /* This class holds the buffer used iin ofstreambuf/istreambuf. Buffer
     * cannot be destroyed before std::ofstream/std::istream dtor (since
     * stream's dtor flushes unwritten data) therefore it cannot be direct
     * member of ofstreambuf/ifstreambuf class. Plugging this small helper class
     * before actual stream keeps buffer alive during its dtor.
     */
    struct BufHolder {
        BufHolder(std::streamsize size) : size(size), buffer(new char[size]) {}
        ~BufHolder() { delete [] buffer; }
        std::streamsize size;
        char* buffer;
    };
} // namespace detail

class ofstreambuf
    : private detail::BufHolder
    , public std::ofstream
{
public:
    static constexpr std::streamsize DefaultBufSize = 1024*1024;

    ofstreambuf(std::streamsize bufSize = DefaultBufSize)
        : detail::BufHolder(bufSize), std::ofstream()
    {
        initBuffer();
    }

    explicit
    ofstreambuf(const char* filename,
                std::ios_base::openmode mode =
                    std::ios_base::out | std::ios_base::trunc,
                std::streamsize bufSize = DefaultBufSize)
        : detail::BufHolder(bufSize), std::ofstream()
    {
        initBuffer();
        open(filename, mode);
    }

    explicit
    ofstreambuf(const std::string& filename,
                std::ios_base::openmode mode =
                    std::ios_base::out | std::ios_base::trunc,
                std::streamsize bufSize = DefaultBufSize)
        : detail::BufHolder(bufSize), std::ofstream()
    {
        initBuffer();
        open(filename, mode);
    }

private:
    void initBuffer() {
        rdbuf()->pubsetbuf(detail::BufHolder::buffer
                           , detail::BufHolder::size);
    }
};

class ifstreambuf
    : private detail::BufHolder
    , public std::ifstream
{
public:
    static constexpr std::streamsize DefaultBufSize = 1024*1024;

    ifstreambuf(std::streamsize bufSize = DefaultBufSize)
        : detail::BufHolder(bufSize), std::ifstream()
    {
        initBuffer();
    }

    explicit
    ifstreambuf(const char* filename,
                std::ios_base::openmode mode = std::ios_base::in,
                std::streamsize bufSize = DefaultBufSize)
        : detail::BufHolder(bufSize), std::ifstream()
    {
        initBuffer();
        open(filename, mode);
    }

    explicit
    ifstreambuf(const std::string& filename,
                std::ios_base::openmode mode = std::ios_base::in,
                std::streamsize bufSize = DefaultBufSize)
        : detail::BufHolder(bufSize), std::ifstream()
    {
        initBuffer();
        open(filename, mode);
    }

private:
    void initBuffer() {
        rdbuf()->pubsetbuf(detail::BufHolder::buffer
                           , detail::BufHolder::size);
    }
};

} // namespace utility

#endif // utility_streams_hpp_included_
