#pragma once

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include <coreutils/log.h>

#include <sys/stat.h>

#ifdef _WIN32
#    include <direct.h>
#endif

namespace utils {
/**
 * A `path` is defined by a set of _segments_ and a flag
 * that says if the path is _absolute_ or _relative.
 *
 * If the last segment is empty, it is a path without a filename
 * component.
 *
 */

class path
{
    enum Format
    {
        Unknown,
        Win,
        Unix
    };
    Format format = Format::Unknown;
    bool isRelative = true;
    bool hasRootDir = false;
    std::vector<std::string> segments;
    static std::string& empty_string()
    {
        static std::string e = "";
        return e;
    }

    void init(std::string const& name)
    {
        size_t start = 0;
        isRelative = true;
        if (name.size() > 0 && name[0] == '/') {
            format = Format::Unix;
            start++;
            isRelative = false;
        } else if (name.size() > 1 && name[1] == ':') {
            format = Format::Win;
            segments.push_back(name.substr(0, 2));
            hasRootDir = true;
            start += 2;
            if (name[2] == '\\' || name[2] == '/') {
                isRelative = false;
                start++;
            }
        }
        for (size_t i = start; i < name.length(); i++) {
            if (name[i] == '/' || name[i] == '\\') {
                if (format == Format::Unknown)
                    format = name[i] == '/' ? Format::Unix : Format::Win;
                segments.push_back(name.substr(start, i - start));
                start = ++i;
            }
        }
        // if (start < name.length())
        segments.push_back(name.substr(start));
    }

    std::string& segment(int i)
    {
        return i < 0 ? segments[segments.size() + i] : segments[i];
    }

    const std::string& segment(int i) const
    {
        return i < 0 ? segments[segments.size() + i] : segments[i];
    }

public:
    path() = default;
    path(std::string const& name) { init(name); }
    path(const char* name) { init(name); }

    void set_relative(bool rel) { isRelative = rel; }

    bool is_absolute() const { return !isRelative; }
    bool is_relative() const { return isRelative; }

    path& operator=(const char* name)
    {
        init(name);
        return *this;
    }

    path& operator=(std::string const& name)
    {
        init(name);
        return *this;
    }

    std::vector<std::string> parts() const { return segments; }

    path& operator/=(path const& p)
    {
        if (p.is_absolute()) {
            *this = p;
        } else {
            if (!empty() && segment(-1) == "")
                segments.resize(segments.size() - 1);
            segments.insert(std::end(segments), std::begin(p.segments),
                            std::end(p.segments));
        }

        return *this;
    }

    path filename() const
    {
        path p = *this;
        p.isRelative = true;
        if (!empty()) {
            p.segments[0] = segment(-1);
            p.segments.resize(1);
        }
        return p;
    }

    std::string extension() const
    {
        if (empty())
            return "";
        const auto& filename = segment(-1);
        auto dot = filename.find_last_of(".");
        if (dot != std::string::npos)
            return filename.substr(dot);
        return "";
    }

    std::string stem() const
    {
        if (empty())
            return "";
        const auto& filename = segment(-1);
        auto dot = filename.find_last_of(".");
        if (dot != std::string::npos)
            return filename.substr(0, dot);
        return "";
    }

    void replace_extension(std::string const& ext)
    {
        if (empty())
            return;
        auto& filename = segment(-1);
        auto dot = filename.find_last_of(".");
        if (dot != std::string::npos) {
            filename = filename.substr(0, dot) + ext;
        }
    }

    path parent_path() const
    {
        path p = *this;
        if (!empty())
            p.segments.resize(segments.size() - 1);
        return p;
    }

    bool empty() const { return segments.empty(); }

    auto begin() const { return segments.begin(); }

    auto end() const { return segments.end(); }

    std::string string() const
    {
        std::string target;
        auto l = (int)segments.size();
        int i = 0;
        std::string separator = (format == Format::Win ? "\\" : "/");
        if (!isRelative) {
            if (hasRootDir)
                target = segment(i++);
            target += separator;
        }
        for (; i < l; i++) {
            target = target + segment(i);
            if (i != l - 1)
                target += separator;
        }
        return target;
    }

    operator std::string() const { return string(); }

    bool operator==(const char* other) const
    {
        return strcmp(other, string().c_str()) == 0;
    }

    friend std::ostream& operator<<(std::ostream& os, const path& p)
    {
        os << (std::string)p;
        return os;
    }
};

inline path operator/(path const& a, path const& b)
{
    return path(a) /= b;
}

inline bool exists(path const& p)
{
    struct stat sb;
    return stat(p.string().c_str(), &sb) >= 0;
}

inline void create_directory(path const& p)
{
#ifdef _WIN32
    _mkdir(p.string().c_str());
#else
    mkdir(p.string().c_str(), 07777);
#endif
}

inline void create_directories(path const& p)
{
    path dir;
    dir.set_relative(p.is_relative());
    for (const auto& part : p) {
        dir = dir / part;
        create_directory(dir);
    }
}

inline bool remove(path const& p)
{
    return (std::remove(p.string().c_str()) != 0);
}

inline bool copy(path const& source, path const& target)
{
    std::ifstream src(source.string(), std::ios::binary);
    std::ofstream dst(target.string(), std::ios::binary);
    dst << src.rdbuf();
    return true;
}

inline path absolute(path const& name)
{
    std::string resolvedPath;
    char* resolvedPathRaw = new char[16384];
#ifdef _WIN32
    char* result = _fullpath(resolvedPathRaw, name.string().c_str(), PATH_MAX);
#else
    char* result = realpath(name.string().c_str(), resolvedPathRaw);
#endif
    if (result)
        resolvedPath = resolvedPathRaw;
    else
        resolvedPath = name;
    delete[] resolvedPathRaw;

    return path(resolvedPath);
}

} // namespace utils
