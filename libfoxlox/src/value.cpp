module;
module foxlox:value;

import :chunk;
// value.cpp

namespace
{
  using namespace foxlox;

  template<typename ... Args>
  std::string wrongtype_msg_fmt(Args ... expected)
    requires ((std::same_as<Args, ValueType> || std::same_as<Args, ObjType>) && ...)
  {
    std::string msg = "Value type error. Expected: ";
    (fmt::format_to(std::back_inserter(msg), "{}, ", magic_enum::enum_name(expected)), ...);
    return msg;
  }

  template<typename T1, typename T2, typename ... Args>
  ValueError exception_wrongtype_binop(T1 got1, T2 got2, Args ... expected)
    requires (std::same_as<T1, ValueType> || std::same_as<T1, ObjType>) &&
    (std::same_as<T2, ValueType> || std::same_as<T2, ObjType>) &&
    ((std::same_as<Args, ValueType> || std::same_as<Args, ObjType>) && ...)
  {
    std::string msg = wrongtype_msg_fmt(expected...);
    fmt::format_to(std::back_inserter(msg), "got: {} and {}.", magic_enum::enum_name(got1), magic_enum::enum_name(got2));
    return ValueError(msg);
  }

  template<typename T, typename ... Args>
  ValueError exception_wrongtype(T got, Args ... expected)
    requires (std::same_as<T, ValueType> || std::same_as<T, ObjType>) &&
    ((std::same_as<Args, ValueType> || std::same_as<Args, ObjType>) && ...)
  {
    std::string msg = wrongtype_msg_fmt(expected...);
    fmt::format_to(std::back_inserter(msg), "got: {}.", magic_enum::enum_name(got));
    return ValueError(msg);
  }

  constexpr void type_check(const Value& got, ValueType expected)
  {
    if (got.type != expected)
    {
      throw exception_wrongtype(got.type, expected);
    }
  }

  constexpr void type_check(const Value& got, ObjType expected)
  {
    if (got.type != ValueType::OBJ)
    {
      throw exception_wrongtype(got.type, expected);
    }
    Expects(got.v.obj != nullptr);
    if (got.v.obj->type != expected)
    {
      throw exception_wrongtype(got.v.obj->type, expected);
    }
  }
}

namespace foxlox
{
  // we only support 64bit arch
  static_assert(CHAR_BIT == 8);
  static_assert(sizeof(void*)* CHAR_BIT == 64);
  // keep Value small and fast!
  static_assert(sizeof(Value) == 16);
  static_assert(std::is_trivially_copyable_v<Value>);
  // I want to make sure a Value which is memset to all 0 is a nil
  // but I cannot check that at compile time
  // so that test is now in unittest

  ObjBase::ObjBase(ObjType t) noexcept :
    type(t)
  {
  }

  double Value::get_double() const
  {
    if (type == ValueType::F64) { return v.f64; }
    if (type != ValueType::I64)
    {
      throw exception_wrongtype(type, ValueType::I64, ValueType::F64);
    }
    return static_cast<double>(v.i64);
  }
  int64_t Value::get_int64() const
  {
    if (type == ValueType::I64) { return v.i64; }
    if (type != ValueType::F64)
    {
      throw exception_wrongtype(type, ValueType::I64, ValueType::F64);
    }
    return static_cast<int64_t>(v.f64);
  }

  bool Value::is_truthy() const noexcept
  {
    if (type == ValueType::NIL || (type == ValueType::BOOL && v.b == false))
    {
      return false;
    }
    return true;
  }

  Instance* Value::get_instance() const
  {
    type_check(*this, ObjType::INSTANCE);
    return v.instance;
  }

  std::string_view Value::get_strview() const
  {
    type_check(*this, ObjType::STR);
    return v.str->get_view();
  }

  std::span<Value> Value::get_tuplespan() const
  {
    type_check(*this, ObjType::TUPLE);
    return v.tuple->get_span();
  }

  Subroutine* Value::method_func() const noexcept
  {
    GSL_SUPPRESS(type.1)
      return reinterpret_cast<Subroutine*>(method_func_ptr);
  }

  std::partial_ordering operator<=>(const Value& l, const Value& r)
  {
    if (l.is_nil() && r.is_nil())
    {
      return std::partial_ordering::equivalent;
    }
    if (l.type == ValueType::I64 && r.type == ValueType::I64)
    {
      return l.v.i64 <=> r.v.i64;
    }
    if ((l.type == ValueType::I64 || l.type == ValueType::F64) &&
      (r.type == ValueType::I64 || r.type == ValueType::F64))
    {
      return l.get_double() <=> r.get_double();
    }
    if (l.type == ValueType::BOOL && r.type == ValueType::BOOL)
    {
      return l.v.b <=> r.v.b;
    }
    if (l.is_str() && r.is_str())
    {
      return *l.v.str <=> *r.v.str;
    }
    if (l.is_tuple() && r.is_tuple())
    {
      return l.v.tuple == r.v.tuple ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    if (l.type == ValueType::FUNC && r.type == ValueType::FUNC)
    {
      return l.v.func == r.v.func ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    if (l.type == ValueType::CPP_FUNC && r.type == ValueType::CPP_FUNC)
    {
      return l.v.cppfunc == r.v.cppfunc ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    if (l.type == ValueType::METHOD && r.type == ValueType::METHOD)
    {
      return l.method_func() == r.method_func() && l.v.instance == r.v.instance ?
        std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    if (l.is_class() && r.is_class())
    {
      return l.v.klass == r.v.klass ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    if (l.is_instance() && r.is_instance())
    {
      return l.v.instance == r.v.instance ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    if (l.is_dict() && r.is_dict())
    {
      return l.v.dict == r.v.dict ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    if (l.is_array() && r.is_array())
    {
      throw UnimplementedError("");
    }
    return std::partial_ordering::unordered;
  }
  double operator/(const Value& l, const Value& r)
  {
    if ((l.type == ValueType::I64 || l.type == ValueType::F64) &&
      (r.type == ValueType::I64 || r.type == ValueType::F64))
    {
      return l.get_double() / r.get_double();
    }
    throw exception_wrongtype_binop(l.type, r.type, ValueType::I64, ValueType::F64);
  }
  Value operator*(const Value& l, const Value& r)
  {
    if (l.type == ValueType::I64 && r.type == ValueType::I64)
    {
      return Value(l.v.i64 * r.v.i64);
    }
    if ((l.type == ValueType::I64 || l.type == ValueType::F64) &&
      (r.type == ValueType::I64 || r.type == ValueType::F64))
    {
      return Value(l.get_double() * r.get_double());
    }
    throw exception_wrongtype_binop(l.type, r.type, ValueType::I64, ValueType::F64);
  }
  Value operator+(const Value& l, const Value& r)
  {
    if (l.type == ValueType::I64 && r.type == ValueType::I64)
    {
      return Value(l.v.i64 + r.v.i64);
    }
    if ((l.type == ValueType::I64 || l.type == ValueType::F64) &&
      (r.type == ValueType::I64 || r.type == ValueType::F64))
    {
      return Value(l.get_double() + r.get_double());
    }
    throw exception_wrongtype_binop(l.type, r.type, ValueType::I64, ValueType::F64);
  }
  Value operator-(const Value& l, const Value& r)
  {
    if (l.type == ValueType::I64 && r.type == ValueType::I64)
    {
      return Value(l.v.i64 - r.v.i64);
    }
    if ((l.type == ValueType::I64 || l.type == ValueType::F64) &&
      (r.type == ValueType::I64 || r.type == ValueType::F64))
    {
      return Value(l.get_double() - r.get_double());
    }
    throw exception_wrongtype_binop(l.type, r.type, ValueType::I64, ValueType::F64);
  }
  Value operator-(const Value& val)
  {
    if (val.type == ValueType::F64) { return Value(-val.v.f64); }
    if (val.type != ValueType::I64)
    {
      throw exception_wrongtype(val.type, ValueType::I64, ValueType::F64);
    }
    return Value(-val.v.i64);
  }
  bool operator!(const Value& val)
  {
    type_check(val, ValueType::BOOL);
    return !val.v.b;
  }
  int64_t intdiv(const Value& l, const Value& r)
  {
    if (l.type == ValueType::I64 && r.type == ValueType::I64)
    {
      return l.v.i64 / r.v.i64;
    }
    return static_cast<int64_t>(l.get_double() / r.get_double());
  }

  bool operator==(const Value& l, const Value& r) noexcept
  {
    // we can not directly compare raw data 
    // as the same boolean may have different representation
    if (l.type != r.type)
    {
      return false;
    }
    switch (l.type)
    {
      //NIL, OBJ, BOOL, F64, I64, FUNC, CPP_FUNC, METHOD,
    case ValueType::NIL:
      return true;
    case ValueType::OBJ:
      return l.v.obj == r.v.obj;
    case ValueType::BOOL:
      return l.v.b == r.v.b;
    case ValueType::F64:
      return l.v.f64 == r.v.f64;
    case ValueType::I64:
      return l.v.i64 == r.v.i64;
    case ValueType::FUNC:
      return l.v.func == r.v.func;
    case ValueType::CPP_FUNC:
      return l.v.cppfunc == r.v.cppfunc;
    case ValueType::METHOD:
      return (l.method_func_ptr == r.method_func_ptr) && (l.v.instance == r.v.instance);
    default: // ???
      return false;
    }
  }

  Dict* Value::get_dict() const
  {
    type_check(*this, ObjType::DICT);
    return v.dict;
  }

  std::string Value::to_string() const
  {
    switch (type)
    {
    case ValueType::BOOL:
      return v.b ? "true" : "false";
    case ValueType::F64:
      return fmt::format("{}", v.f64);
    case ValueType::I64:
      return fmt::format("{}", v.i64);
    case ValueType::FUNC:
      return fmt::format("<fn {}>", v.func->get_funcname());
    case ValueType::CPP_FUNC:
      GSL_SUPPRESS(type.1)
        //return fmt::format("<native fn {}>", reinterpret_cast<void*>(v.cppfunc));
        return "<native fn>";
    case ValueType::METHOD:
      return fmt::format("<class {} method {}>", v.instance->get_class()->get_name(), method_func()->get_funcname());
    case ValueType::OBJ:
    {
      if (v.obj == nullptr) { return "nil"; }
      switch (v.obj->type)
      {
      case ObjType::STR:
        return fmt::format("\"{}\"", v.str->get_view());
      case ObjType::TUPLE:
      {
        std::string str = "(";
        auto s = v.tuple->get_span();
        for (auto& elem : s)
        {
          str += elem.to_string() + ", ";
        }
        str += ")";
        return str;
      }
      case ObjType::CLASS:
        return fmt::format("<class {}>", v.klass->get_name());
      case ObjType::INSTANCE:
        return fmt::format("<{} instance>", v.instance->get_class()->get_name());
      case ObjType::DICT:
        return "<dict>";
      case ObjType::ARRAY:
        return "<array>";
      default:
        throw FatalError(fmt::format("Unknown ObjType: {}", magic_enum::enum_name(v.obj->type)));
      }
    }
    default:
      throw FatalError(fmt::format("Unknown ValueType: {}", magic_enum::enum_name(type)));
    }
  }
  std::array<uint64_t, 2> Value::serialize() const noexcept
  {
    std::array<uint64_t, 2> data{};
    switch (type)
    {
    case ValueType::NIL:
      data.at(0) = std::bit_cast<uint64_t>(ValueType::NIL);
      data.at(1) = 0;
      return data;
    case ValueType::BOOL:
      data.at(0) = std::bit_cast<uint64_t>(ValueType::BOOL);
      data.at(1) = v.b ? 1 : 0;
      return data;
    case ValueType::OBJ:
      data.at(0) = std::bit_cast<uint64_t>(ValueType::OBJ);
      data.at(1) = std::bit_cast<uint64_t>(v.obj);
      return data;
    case ValueType::F64:
      data.at(0) = std::bit_cast<uint64_t>(ValueType::F64);
      data.at(1) = std::bit_cast<uint64_t>(v.f64);
      return data;
    case ValueType::I64:
      data.at(0) = std::bit_cast<uint64_t>(ValueType::I64);
      data.at(1) = std::bit_cast<uint64_t>(v.i64);
      return data;
    case ValueType::FUNC:
      data.at(0) = std::bit_cast<uint64_t>(ValueType::FUNC);
      data.at(1) = std::bit_cast<uint64_t>(v.func);
      return data;
    case ValueType::CPP_FUNC:
      data.at(0) = std::bit_cast<uint64_t>(ValueType::CPP_FUNC);
      data.at(1) = std::bit_cast<uint64_t>(v.cppfunc);
      return data;
    case ValueType::METHOD:
      data.at(0) = (static_cast<uint64_t>(type) << userspace_addr_bits)
        | uint64_t{ method_func_ptr };
      data.at(1) = std::bit_cast<uint64_t>(v.func);
      return data;
      /* Not Impl yet: */
    case ValueType::CPP_INSTANCE:
      data.at(0) = std::bit_cast<uint64_t>(ValueType::CPP_INSTANCE);
      data.at(1) = 0;
      return data;
    case ValueType::CPP_CLASS:
      data.at(0) = std::bit_cast<uint64_t>(ValueType::CPP_CLASS);
      data.at(1) = 0;
      return data;
    default: // ???
      data.at(0) = 0;
      data.at(1) = 0;
      return data;
    }
  }
  bool Value::debug_type_is_valid() noexcept
  {
    if (type == ValueType::OBJ)
    {
      if (v.obj == nullptr)
      {
        return false;
      }
      return (v.obj->type == ObjType::STR)
        || (v.obj->type == ObjType::TUPLE)
        || (v.obj->type == ObjType::CLASS)
        || (v.obj->type == ObjType::INSTANCE)
        || (v.obj->type == ObjType::DICT);
    }
    if (type == ValueType::NIL)
    {
      return v.i64 == 0;
    }
    if (type == ValueType::BOOL)
    {
      return (v.i64 == 0) || (v.i64 == 1);
    }
    if (type == ValueType::F64 || type == ValueType::I64)
    {
      return true;
    }
    if (type == ValueType::FUNC)
    {
      return (v.func != nullptr) && (reinterpret_cast<uintptr_t>(v.func) % alignof(decltype(*v.func)) == 0);
    }
    if (type == ValueType::CPP_FUNC)
    {
      return v.cppfunc != nullptr;
    }
    if (type == ValueType::METHOD)
    {
      return (v.instance != nullptr)
        && (reinterpret_cast<uintptr_t>(v.instance) % alignof(decltype(*v.instance)) == 0)
        && (method_func() != nullptr)
        && (reinterpret_cast<uintptr_t>(method_func()) % alignof(decltype(*method_func())) == 0);
    }
    return false;
  }
  Value Value::get_property(gsl::not_null<String*> name)
  {
    if (is_instance())
    {
      return v.instance->get_property(name);
    }
    if (is_dict())
    {
      return v.dict->get(name);
    }
    if (is_nil())
    {
      throw exception_wrongtype(ValueType::NIL, ObjType::INSTANCE, ObjType::DICT);
    }
    if (type != ValueType::OBJ)
    {
      throw exception_wrongtype(type, ObjType::INSTANCE, ObjType::DICT);
    }
    throw exception_wrongtype(v.obj->type, ObjType::INSTANCE, ObjType::DICT);
  }
}

// hash_table.cpp
namespace
{
  using namespace foxlox;

  bool str_equal(gsl::not_null<String*> l, std::string_view r) noexcept
  {
    return l->get_view() == r;
  }
  bool str_equal(gsl::not_null<String*> l, std::string_view r1, std::string_view r2) noexcept
  {
    GSL_SUPPRESS(stl.1) GSL_SUPPRESS(bounds.1)
      return l->size() == (r1.size() + r2.size()) &&
      std::equal(r1.begin(), r1.end(), l->data()) &&
      std::equal(r2.begin(), r2.end(), l->data() + r1.size());
  }

  // FNV-1a hash
  // http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-1a
  constexpr uint32_t str_hash(std::string_view str) noexcept
  {
    uint32_t hash = 2166136261u;
    for (auto c : str)
    {
      hash ^= std::bit_cast<uint8_t>(c);
      hash *= 16777619u;
    }
    return hash;
  }

  uint32_t str_hash(std::string_view str1, std::string_view str2) noexcept
  {
    uint32_t hash = 2166136261u;
    for (auto c : str1)
    {
      hash ^= std::bit_cast<uint8_t>(c);
      hash *= 16777619u;
    }
    for (auto c : str2)
    {
      hash ^= std::bit_cast<uint8_t>(c);
      hash *= 16777619u;
    }
    return hash;
  }

  GSL_SUPPRESS(type.1)
    uint32_t nonstr_hash(void* p) noexcept
  {
    // MurmurHash3Mixer, a lot faster than FNV-1a for pointer
    // from https://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html
    uintptr_t p_int = reinterpret_cast<uintptr_t>(p);
    p_int ^= (p_int >> 33);
    p_int *= 0xff51afd7ed558ccdull;
    p_int ^= (p_int >> 33);
    p_int *= 0xc4ceb9fe1a85ec53ull;
    p_int ^= (p_int >> 33);
    return gsl::narrow_cast<uint32_t>(p_int);
  }

  uint32_t nonstr_hash(Value v) noexcept
  {
    // MurmurHash3
    // from https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp

    const auto data = std::bit_cast<std::array<uint32_t, 4>>(v.serialize());
    constexpr uint32_t seed = 0;
    uint32_t h1 = seed;

    //----------
    // body
    for (uint32_t k1 : data)
    {
      k1 *= 0xcc9e2d51;
      k1 = std::rotl(k1, 15);
      k1 *= 0x1b873593;

      h1 ^= k1;
      h1 = std::rotl(h1, 13);
      h1 = h1 * 5 + 0xe6546b64;
    }
    //----------
    // finalization
    //h1 ^= len;
    //h1 = fmix32(h1);
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;
    return h1;
  }
}
namespace foxlox
{
  bool StringPoolEntry::key_is_null()
  {
    return str == nullptr;
  }

  template<typename K, typename V>
  bool HashTableEntry<K, V>::key_is_null()
  {
    if constexpr (std::same_as<K, String*>)
    {
      return key == nullptr;
    }
    else
    {
      static_assert(std::same_as<K, Value>);
      return key.is_nil();
    }
  }

  template<typename T>
  GSL_SUPPRESS(type.1) GSL_SUPPRESS(f.23)
    void grow_capacity(T* table)
  {
    if (table->count > std::bit_floor(std::numeric_limits<decltype(table->count)>::max()) / 2)
    {
      throw InternalRuntimeError("Too many strings. String pool is full.");
    }
    uint32_t new_count = 0;
    const uint32_t new_capacity = table->capacity * 2;
    auto new_entries = reinterpret_cast<decltype(table->entries)>(
      table->allocator(new_capacity * sizeof(*table->entries)));
#ifndef _MSC_VER 
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif
    std::memset(new_entries, 0, new_capacity * sizeof(*table->entries));
#ifndef _MSC_VER 
#pragma GCC diagnostic pop
#endif
    for (auto& e : std::span(table->entries, table->capacity))
    {
      if (e.tombstone || e.key_is_null())
      {
        continue;
      }
      new_count++;
      uint32_t idx = e.hash & (new_capacity - 1);
      while (true)
      {
        GSL_SUPPRESS(bounds.1)
          if (new_entries[idx].key_is_null())
          {
            /* DEBUG: check input is valid */
            if constexpr (is_specialization_v<std::remove_cvref_t<decltype(e)>, HashTableEntry>)
            {
              if constexpr (std::same_as<decltype(e.key), Value>)
              {
                assert(e.key.debug_type_is_valid());
              }
            }
            if constexpr (is_specialization_v<std::remove_cvref_t<decltype(e)>, HashTableEntry>)
            {
              if constexpr (std::same_as<decltype(e.value), Value>)
              {
                assert(e.value.debug_type_is_valid());
              }
            }
            new_entries[idx] = e;
            break;
          }
        idx = (idx + 1) & (new_capacity - 1);
      }
    }
    GSL_SUPPRESS(type.1)
      table->deallocator(reinterpret_cast<char*>(table->entries),
        table->capacity * sizeof(*table->entries));
    table->entries = new_entries;
    table->count = new_count;
    table->capacity = new_capacity;
  }

  StringPool::~StringPool()
  {
    try
    {
      clean();
    }
    catch (...)
    {
      std::terminate();
    }
  }
  gsl::not_null<String*> StringPool::add_string(std::string_view str)
  {
    if (count + 1 > capacity * STRING_POOL_MAX_LOAD)
    {
      grow_capacity(this);
    }
    const auto hash = str_hash(str);
    uint32_t idx = hash & (capacity - 1);
    StringPoolEntry* first_tombstone = nullptr;
    GSL_SUPPRESS(bounds.1)
      while (true)
      {
        if (entries[idx].tombstone)
        {
          if (first_tombstone == nullptr) { first_tombstone = &entries[idx]; }
        }
        else if (entries[idx].str == nullptr)
        {
          const gsl::not_null<String*> p = String::alloc(allocator, str.size());
          GSL_SUPPRESS(stl.1)
            std::copy(str.begin(), str.end(), p->data());

          if (first_tombstone == nullptr) { count++; }
          StringPoolEntry* entry_to_insert = first_tombstone ? first_tombstone : &entries[idx];
          entry_to_insert->hash = hash;
          entry_to_insert->str = p;
          entry_to_insert->tombstone = false;
          return p;
        }
        else if (str_equal(entries[idx].str, str))
        {
          return entries[idx].str;
        }
        idx = (idx + 1) & (capacity - 1);
      }
  }
  GSL_SUPPRESS(f.6)
    StringPool::StringPool(StringPool&& o) noexcept :
    allocator(o.allocator),
    deallocator(o.deallocator),
    entries(o.entries),
    count(o.count),
    capacity(o.capacity)
  {
    o.entries = nullptr;
  }
  StringPool& StringPool::operator=(StringPool&& o) noexcept
  {
    if (this == &o) { return *this; }
    try
    {
      clean();
      allocator = o.allocator;
      deallocator = o.deallocator;
      count = o.count;
      entries = o.entries;
      capacity = o.capacity;
      o.entries = nullptr;
      return *this;
    }
    catch (...)
    {
      std::terminate();
    }
  }
  gsl::not_null<String*> StringPool::add_str_cat(std::string_view lhs, std::string_view rhs)
  {
    if (count + 1 > capacity * STRING_POOL_MAX_LOAD)
    {
      grow_capacity(this);
    }
    const auto hash = str_hash(lhs, rhs);
    uint32_t idx = hash & (capacity - 1);
    StringPoolEntry* first_tombstone = nullptr;
    GSL_SUPPRESS(bounds.1) GSL_SUPPRESS(stl.1)
      while (true)
      {
        if (entries[idx].tombstone)
        {
          if (first_tombstone == nullptr) { first_tombstone = &entries[idx]; }
        }
        else if (entries[idx].str == nullptr)
        {
          gsl::not_null<String*> p = String::alloc(allocator, lhs.size() + rhs.size());
          const auto it = std::copy(lhs.begin(), lhs.end(), p->data());
          std::copy(rhs.begin(), rhs.end(), it);

          if (first_tombstone == nullptr) { count++; }
          StringPoolEntry* entry_to_insert = first_tombstone ? first_tombstone : &entries[idx];
          entry_to_insert->hash = hash;
          entry_to_insert->str = p;
          entry_to_insert->tombstone = false;
          return p;
        }
        else if (str_equal(entries[idx].str, lhs, rhs))
        {
          return entries[idx].str;
        }
        idx = (idx + 1) & (capacity - 1);
      }
  }
  void StringPool::sweep()
  {
    for (auto& e : std::span(entries, capacity))
    {
#ifdef FOXLOX_DEBUG_LOG_GC
      if (e.str != nullptr && !e.tombstone)
      {
        std::cout << fmt::format("sweeping {} [{}]: {}\n", static_cast<const void*>(e.str), e.str->is_marked() ? "is_marked" : "not_marked", e.str->get_view());
      }
#endif
      if (e.str != nullptr && !e.tombstone && !e.str->is_marked())
      {
        delete_entry(e);
      }
    }
  }

  void StringPool::init_entries()
  {
    capacity = HASH_TABLE_START_BUCKET;
    GSL_SUPPRESS(type.1)
      entries = reinterpret_cast<decltype(entries)>(
        allocator(HASH_TABLE_START_BUCKET * sizeof(*entries)));
    std::memset(entries, 0, HASH_TABLE_START_BUCKET * sizeof(*entries));
    // make sure HASH_TABLE_START_BUCKET is power of 2
    // otherwise capacity mask won't work
    static_assert((HASH_TABLE_START_BUCKET & (HASH_TABLE_START_BUCKET - 1)) == 0);
  }

  void StringPool::delete_entry(StringPoolEntry& e)
  {
    Expects(e.str != nullptr && !e.tombstone);
    String::free(deallocator, e.str);
    e.tombstone = true;
    // tombstone still counts in count, so we do not count-- here
  }

  void StringPool::clean()
  {
    if (entries != nullptr)
    {
      for (auto& e : std::span(entries, capacity))
      {
        if (e.str != nullptr && !e.tombstone)
        {
          delete_entry(e);
        }
      }
      GSL_SUPPRESS(type.1)
        deallocator(reinterpret_cast<char*>(entries), capacity * sizeof(*entries));
    }
  }

  template<typename K, typename V>
  HashTable<K, V>::~HashTable()
  {
    try
    {
      clean();
    }
    catch (...)
    {
      std::terminate();
    }
  }

  template<typename K, typename V>
  void HashTable<K, V>::clean()
  {
    if (entries != nullptr)
    {
      GSL_SUPPRESS(type.1)
        deallocator(reinterpret_cast<char*>(entries), capacity * sizeof(*entries));
    }
  }

  template<typename K, typename V>
  void HashTable<K, V>::set_entry(K key, V value)
  {
    /* DEBUG: check input is valid */
    if constexpr (std::same_as<K, Value>)
    {
      assert(key.debug_type_is_valid());
    }
    if constexpr (std::same_as<V, Value>)
    {
      assert(value.debug_type_is_valid());
    }
    /*******************************/
    if (count + 1 > capacity * STRING_POOL_MAX_LOAD)
    {
      grow_capacity(this);
    }
    const uint32_t hash = nonstr_hash(key);
    const auto entry = find_entry(key, hash);
    if (entry->key_is_null() && !entry->tombstone)
    {
      // new entry
      count++;
    }

    if constexpr (std::same_as<V, Value>)
    {
      // set a entry value to nil means to delete it
      if (value.is_nil())
      {
        delete_entry(*entry);
        return;
      }
    }
    entry->hash = hash;
    entry->tombstone = false;
    entry->key = key;
    entry->value = value;
  }
  template<typename K, typename V>
  void HashTable<K, V>::try_add_entry(K key, V value)
  {
    /* DEBUG: check input is valid */
    if constexpr (std::same_as<K, Value>)
    {
      assert(key.debug_type_is_valid());
    }
    if constexpr (std::same_as<V, Value>)
    {
      assert(value.debug_type_is_valid());
    }
    /*******************************/
    if (count + 1 > capacity * STRING_POOL_MAX_LOAD)
    {
      grow_capacity(this);
    }
    const uint32_t hash = nonstr_hash(key);
    const auto entry = find_entry(key, hash);

    if (!entry->key_is_null() && !entry->tombstone)
    {
      // key already exist, do nothing
      return;
    }
    if (entry->key_is_null() && !entry->tombstone)
    {
      // new entry
      count++;
    }

    entry->hash = hash;
    entry->tombstone = false;
    entry->key = key;
    entry->value = value;
  }
  template<typename K, typename V>
  std::optional<V> HashTable<K, V>::get_value(K key)
  {
    /* DEBUG: check input is valid */
    if constexpr (std::same_as<K, Value>)
    {
      assert(key.debug_type_is_valid());
    }
    /*******************************/
    const uint32_t hash = nonstr_hash(key);
    const auto entry = find_entry(key, hash);
    if (!entry->key_is_null() && !entry->tombstone)
    {
      // key found
      /* DEBUG: check output is valid */
      if constexpr (std::same_as<V, Value>)
      {
        assert(entry->value.debug_type_is_valid());
      }
      /********************************/
      return entry->value;
    }
    return std::nullopt;
  }
  template<typename K, typename V>
  HashTableIter<K, V> HashTable<K, V>::begin() noexcept
  {
    for (auto& e : std::span(entries, capacity))
    {
      if (!e.key_is_null() && !e.tombstone)
      {
        /* DEBUG: check input is valid */
        if constexpr (std::same_as<K, Value>)
        {
          assert(e.key.debug_type_is_valid());
        }
        if constexpr (std::same_as<V, Value>)
        {
          assert(e.value.debug_type_is_valid());
        }
        /*******************************/
        return HashTableIter<K, V>(this, &e);
      }
    }
    return end();
  }
  template<typename K, typename V>
  HashTableIter<K, V> HashTable<K, V>::end() noexcept
  {
    return HashTableIter<K, V>(this, nullptr);
  }

  template<typename K, typename V>
  GSL_SUPPRESS(bounds.1) GSL_SUPPRESS(lifetime.4) GSL_SUPPRESS(lifetime.1)
    HashTableEntry<K, V>* HashTable<K, V>::next_entry(HashTableEntry<K, V>* p) noexcept
  {
    Expects(entries <= p && p < entries + capacity);
    while (++p < entries + capacity)
    {
      if (!p->key_is_null() && !p->tombstone)
      {
        return p;
      }
    }
    return nullptr;
  }
  template<typename K, typename V>
  GSL_SUPPRESS(f.6)
    HashTable<K, V>::HashTable(HashTable&& o) noexcept :
    allocator(o.allocator),
    deallocator(o.deallocator),
    entries(o.entries),
    count(o.count),
    capacity(o.capacity)
  {
    o.entries = nullptr;
  }
  template<typename K, typename V>
  HashTable<K, V>& HashTable<K, V>::operator=(HashTable&& o) noexcept
  {
    if (this == &o) { return *this; }
    try
    {
      clean();
      allocator = o.allocator;
      deallocator = o.deallocator;
      count = o.count;
      entries = o.entries;
      capacity = o.capacity;
      o.entries = nullptr;
      return *this;
    }
    catch (...)
    {
      std::terminate();
    }
  }
  template<typename K, typename V>
  void HashTable<K, V>::init_entries()
  {
    capacity = HASH_TABLE_START_BUCKET;
    GSL_SUPPRESS(type.1)
      entries = reinterpret_cast<decltype(entries)>(
        allocator(HASH_TABLE_START_BUCKET * sizeof(*entries)));
#ifndef _MSC_VER 
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif
    std::memset(entries, 0, HASH_TABLE_START_BUCKET * sizeof(*entries));
#ifndef _MSC_VER 
#pragma GCC diagnostic pop
#endif
    // make sure HASH_TABLE_START_BUCKET is power of 2
    // otherwise capacity mask won't work
    static_assert((HASH_TABLE_START_BUCKET & (HASH_TABLE_START_BUCKET - 1)) == 0);
  }
  template<typename K, typename V>
  gsl::not_null<HashTableEntry<K, V>*> HashTable<K, V>::find_entry(K key, uint32_t hash) noexcept
  {
    uint32_t idx = hash & (capacity - 1);
    HashTableEntry<K, V>* first_tombstone = nullptr;
    GSL_SUPPRESS(bounds.1)
      while (true)
      {
        if (entries[idx].tombstone)
        {
          if (first_tombstone == nullptr) { first_tombstone = &entries[idx]; }
        }
        else if (entries[idx].key_is_null())
        {
          return first_tombstone ? first_tombstone : &entries[idx];
        }
        else if (entries[idx].key == key)
        {
          return &entries[idx];
        }
        idx = (idx + 1) & (capacity - 1);
      }
  }

  template<typename K, typename V>
  void HashTable<K, V>::delete_entry(HashTableEntry<K, V>& e) noexcept
  {
    if (!e.key_is_null())
    {
      e.tombstone = true;
    }
  }

  template<typename K, typename V>
  HashTableIter<K, V>::HashTableIter(HashTable<K, V>* table, HashTableEntry<K, V>* entry) noexcept :
    p_table(table), p_entry(entry)
  {
  }

  template<typename K, typename V>
  bool HashTableIter<K, V>::operator==(const HashTableIter<K, V>& rhs) const noexcept
  {
    return p_entry == rhs.p_entry;
  }

  template<typename K, typename V>
  HashTableEntry<K, V>& HashTableIter<K, V>::operator*() const noexcept
  {
    /* DEBUG: check input is valid */
    if constexpr (std::same_as<K, Value>)
    {
      assert(p_entry->key.debug_type_is_valid());
    }
    if constexpr (std::same_as<V, Value>)
    {
      assert(p_entry->value.debug_type_is_valid());
    }
    /*******************************/
    return *p_entry;
  }

  template<typename K, typename V>
  HashTableIter<K, V>& HashTableIter<K, V>::operator++() noexcept
  {
    p_entry = p_table->next_entry(p_entry);
    /* DEBUG: check input is valid */
    if constexpr (std::same_as<K, Value>)
    {
      assert(p_entry == nullptr || p_entry->key.debug_type_is_valid());
    }
    if constexpr (std::same_as<V, Value>)
    {
      assert(p_entry == nullptr || p_entry->value.debug_type_is_valid());
    }
    /*******************************/
    return *this;
  }

  // Explicit template instantiation
  template class HashTable<String*, Subroutine*>;
  template class HashTable<String*, Value>;
  template class HashTable<Value, Value>;

  template class HashTableIter<String*, Subroutine*>;
  template class HashTableIter<String*, Value>;
  template class HashTableIter<Value, Value>;
}

// object.cpp
namespace foxlox
{
  GSL_SUPPRESS(type.6)
    String::String(size_t l) noexcept :
    SimpleObj(ObjType::STR, l)
  {
  }
  bool operator==(const String& l, const String& r) noexcept
  {
    return l.get_view() == r.get_view();
  }
  std::string_view String::get_view() const noexcept
  {
    return std::string_view(data(), size());
  }
  GSL_SUPPRESS(type.6)
    Tuple::Tuple(size_t l) noexcept :
    SimpleObj(ObjType::TUPLE, l)
  {
  }
  std::span<const Value> Tuple::get_span() const noexcept
  {
    GSL_SUPPRESS(bounds.3)
      return std::span{ data(), size() };
  }
  std::span<Value> Tuple::get_span() noexcept
  {
    GSL_SUPPRESS(bounds.3)
      return std::span{ data(), size() };
  }
  Class* Instance::get_class() const noexcept { return klass; }
  Value Instance::get_property(gsl::not_null<String*> name)
  {
    if (auto func = klass->get_method(name); func.has_value())
    {
      GSL_SUPPRESS(lifetime.3)
        return Value(this, *func);
    }
    // return nil when the field is not found
    return fields.get_value(name).value_or(Value());
  }
  Value Instance::get_super_method(gsl::not_null<String*> name)
  {
    if (auto func = klass->get_super()->get_method(name); func.has_value())
    {
      GSL_SUPPRESS(lifetime.3)
        return Value(this, *func);
    }
    else
    {
      throw ValueError(fmt::format("Super class has no method with name `{}'", name->get_view()));
    }
  }
  HashTable<String*, Value>& Instance::get_hash_table() noexcept
  {
    return fields;
  }
  void Instance::set_property(gsl::not_null<String*> name, Value value)
  {
    if (klass->has_method(name))
    {
      throw ValueError("Attempt to rewrite class method. This is not allowed");
    }
    fields.set_entry(name, value);
  }
  bool Instance::is_marked() const noexcept
  {
    return gc_mark;
  }
  void Instance::mark() noexcept
  {
    gc_mark = true;
  }
  void Instance::unmark() noexcept
  {
    gc_mark = false;
  }
  GSL_SUPPRESS(r.11) GSL_SUPPRESS(i.11)
    Class::Class(std::string_view name) :
    ObjBase(ObjType::CLASS),
    gc_mark(false),
    superclass(nullptr),
    class_name(name),
    methods([](size_t l) {return new char[l]; }, [](char* p, size_t) {delete[] p; })
  {
  }
  void Class::add_method(String* name, Subroutine* func)
  {
    methods.set_entry(name, func);
  }
  void Class::set_super(gsl::not_null<Class*> super)
  {
    superclass = super;
    for (auto& entry : super->get_hash_table())
    {
      // if we already have a method with the same name,
      // do nothing (to shadow the base class method)
      methods.try_add_entry(entry.key, entry.value);
    }
  }
  Class* Class::get_super() noexcept
  {
    return superclass;
  }
  bool Class::has_method(String* name)
  {
    return methods.get_value(name).has_value();
  }
  std::optional<Subroutine*> Class::get_method(String* name)
  {
    return methods.get_value(name);
  }
  HashTable<String*, Subroutine*>& Class::get_hash_table() noexcept
  {
    return methods;
  }
  bool Class::is_marked() const noexcept
  {
    return gc_mark;
  }
  void Class::mark() noexcept
  {
    gc_mark = true;
  }
  void Class::unmark() noexcept
  {
    gc_mark = false;
  }
  Value Dict::get(gsl::not_null<String*> name)
  {
    // return nil when the field is not found
    return fields.get_value(name).value_or(Value());
  }
  HashTable<Value, Value>& Dict::get_hash_table() noexcept
  {
    return fields;
  }
  void Dict::set(gsl::not_null<String*> name, Value value)
  {
    fields.set_entry(name, value);
  }
  bool Dict::is_marked() const noexcept
  {
    return gc_mark;
  }
  void Dict::mark() noexcept
  {
    gc_mark = true;
  }
  void Dict::unmark() noexcept
  {
    gc_mark = false;
  }
}
