module;
#include <fmt/format.h>
export module foxlox:except;

import <stdexcept>;
import <string>;
import <string_view>;

namespace foxlox
{
    export class FatalError : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };
    export class UnimplementedError : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };
    export class InternalRuntimeError : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };
    export class RuntimeLibError : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };
    export class VMError : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };
    export class RuntimeError : public std::runtime_error
    {
    public:
        RuntimeError(std::string_view message, int line_num, std::string_view src_code) :
            std::runtime_error(fmt::format("[line {}] at \"{}\"\n{}", line_num, src_code, message)),
            msg(message),
            line(line_num),
            source(src_code)
        {
        }

        std::string msg;
        int line;
        std::string source;
    };
}
