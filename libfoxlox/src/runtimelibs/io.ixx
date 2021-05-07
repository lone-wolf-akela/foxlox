module;
#include <fmt/format.h>
export module foxlox:runtimelibs.io;

import <iostream>;
import <ranges>;
import <span>;

import :runtimelib;
import :vm;
import :except;
import :value;

namespace foxlox::lib
{
	export foxlox::Value print(foxlox::VM& vm, std::span<foxlox::Value> values)
	{
		if (values.empty())
		{
			throw RuntimeLibError("[print]: Requires at least one parameter to print.");
		}
		const bool multi_args = (values.size()) > 1;
		if (multi_args && !values.front().is_str())
		{
			throw RuntimeLibError("[print]: When print multiple values, the first parameter must be a format string.");
		}

		fmt::dynamic_format_arg_store<fmt::format_context> store;
		for (auto& v : values | std::ranges::views::drop(multi_args ? 1 : 0))
		{
			switch (v.type)
			{
			case ValueType::I64:
				store.push_back(v.v.i64);
				break;
			case ValueType::F64:
				store.push_back(v.v.f64);
				break;
			case ValueType::BOOL:
				store.push_back(v.v.b);
				break;
			default:
				if (v.is_str())
				{
					store.push_back(v.v.str->get_view());
				}
				else
				{
					store.push_back(v.to_string());
				}
				break;
			}
		}
		std::cout << fmt::vformat(multi_args ? values.front().v.str->get_view() : "{}", store);
		return Value();
	}
	export foxlox::Value println(foxlox::VM& vm, std::span<foxlox::Value> values)
	{
		print(vm, values);
		std::cout << std::endl;
		return Value();
	}

	export const RuntimeLib io = {
	  { "print", print },
	  { "println", println},
	};
}