export module foxlox:compiletime_value;

import <cstdint>;
import <string>;
import <string_view>;
import <variant>;
import <vector>;
import <span>;

import :serialization;
import :util;
import :except;
import :value;

namespace foxlox
{
  export struct CompiletimeValue
  {
    std::variant<std::nullptr_t, double, int64_t, std::string, bool> v;
    CompiletimeValue() noexcept
    {
        v = nullptr;
    }
    CompiletimeValue(double f64) noexcept
    {
        v = f64;
    }
    CompiletimeValue(int64_t i64) noexcept
    {
        v = i64;
    }
    CompiletimeValue(std::string_view str)
    {
        v = std::string(str);
    }
    CompiletimeValue(bool b) noexcept
    {
        v = b;
    }

    std::string to_string() const
    {
        if (std::holds_alternative<std::nullptr_t>(v))
        {
            return "nil";
        }
        else if (std::holds_alternative<double>(v))
        {
            return std::to_string(std::get<double>(v));
        }
        else if (std::holds_alternative<int64_t>(v))
        {
            return std::to_string(std::get<int64_t>(v));
        }
        else if (std::holds_alternative<std::string>(v))
        {
            return std::get<std::string>(v);
        }
        else if (std::holds_alternative<bool>(v))
        {
            return std::get<bool>(v) ? "true" : "false";
        }
        throw FatalError("Unknown value type.");
    }
  };

  export class CompiletimeClass
  {
  public:
    void dump(std::ostream& strm) const
    {
        dump_str(strm, classname);
        dump_int64(strm, ssize(methods));
        for (const auto& [name_idx, func_idx] : methods)
        {
            dump_uint16(strm, name_idx);
            dump_uint16(strm, func_idx);
        }
    }
    static CompiletimeClass load(std::istream& strm)
    {
        const std::string name = load_str(strm);
        CompiletimeClass klass(name);

        const int64_t len = load_int64(strm);
        klass.methods.reserve(len);
        for (int64_t i = 0; i < len; i++)
        {
            const uint16_t name_idx = load_uint16(strm);
            const uint16_t func_idx = load_uint16(strm);
            klass.methods.emplace_back(name_idx, func_idx);
        }
        return klass;
    }

    CompiletimeClass(std::string_view name) :
        classname(name)
    {
    }
    void add_method(uint16_t name_idx, uint16_t subroutine_idx)
    {
        methods.emplace_back(name_idx, subroutine_idx);
    }
    std::string_view get_name() const noexcept
    {
        return classname;
    }
    std::span<const std::pair<uint16_t, uint16_t>> get_methods() const noexcept
    {
        return methods;
    }
  private:
    std::string classname;
    std::vector<std::pair<uint16_t, uint16_t>> methods; // store the name str idx & subroutine idx
  };

  export struct CompiletimeExport
  {
    CompiletimeExport(uint16_t name, uint16_t value) :
        name_idx(name),
        value_idx(value)
    {
    }
    uint16_t name_idx;
    uint16_t value_idx;
  };
}
