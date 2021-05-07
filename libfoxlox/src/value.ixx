module;
#include <fmt/format.h>
#include <magic_enum.hpp>
export module foxlox:value;

import <cassert>;
import <cstring>;
import <array>;
import <stdexcept>;
import <cstdint>;
import <string>;
import <compare>;
import <string_view>;
import <span>;
import <concepts>;
import <type_traits>;
import <utility>;
import <climits>;
import <version>;
import <functional>;
import <vector>;
import <concepts>;
import <optional>;
import <bit>;
import <algorithm>;
import <iostream>;
import <gsl/gsl>;

import "config.h";
import :except;
import :util;

// value.h
namespace foxlox
{
  export class ValueError : public std::runtime_error
  { 
  public:
    using std::runtime_error::runtime_error;
  };

  template<typename T1, typename T2>
  concept remove_cv_same_as = std::same_as<std::remove_cv_t<T1>, std::remove_cv_t<T2>>;

  template<typename T>
  concept IntegralExcludeBool = std::integral<T> && !std::same_as<std::remove_cv_t<T>, bool>;
  
  template<typename F>
  concept Allocator = std::is_invocable_r_v<char*, F, size_t>;

  template<typename F>
  concept Deallocator = std::is_invocable_r_v<void, F, char* const, size_t>;

  export class String;
  export class Tuple;
  export class Subroutine;
  export class Class;
  export class Instance;
  export class Dict;
  export struct Value;
  export class VM;
  export class Chunk;

  export enum class ObjType : uint8_t
  {
    STR, TUPLE, CLASS, INSTANCE, DICT, 
    // Not Impl yet:
    ARRAY, BYTES
  };
  export enum class ValueType : uintptr_t
  {
    NIL, OBJ, BOOL, F64, I64, FUNC, CPP_FUNC, METHOD,
    // Not Impl yet:
    CPP_INSTANCE, CPP_CLASS
  };
  export class ObjBase
  {
  public:
    ObjBase(ObjType t) noexcept;
    ObjType type;
  };

  export using CppFunc = Value(VM&, std::span<Value>);

  export struct Value
  {
    /* pointer packing */
    // according to https://unix.stackexchange.com/questions/509607/how-a-64-bit-process-virtual-address-space-is-divided-in-linux
    // and https://docs.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/virtual-address-spaces
    // on both windows & linux, and on both amd64 and arm64
    // the userspace address range is not longer than 56bit
    constexpr static auto userspace_addr_bits = 56;
    ValueType type : sizeof(uintptr_t)* CHAR_BIT - userspace_addr_bits;
    uintptr_t method_func_ptr : userspace_addr_bits;

    union
    {
      bool b;
      double f64;
      int64_t i64;
      String* str;
      Tuple* tuple;
      Subroutine* func;
      CppFunc* cppfunc;
      Class* klass;
      Instance* instance;
      Dict* dict;
      ObjBase* obj;
    } v;


    constexpr Value() noexcept : 
      type(ValueType::NIL), 
      method_func_ptr(0),
      v{ .i64 = 0 } 
    {}

    template<std::convertible_to<String*> T>
    constexpr Value(T str) noexcept : 
      type(ValueType::OBJ), 
      method_func_ptr(0),
      v{ .str = str } 
    {}

    template<std::convertible_to<Tuple*> T>
    constexpr Value(T tuple) noexcept : 
      type(ValueType::OBJ),
      method_func_ptr(0),
      v{ .tuple = tuple } 
    {}

    template<std::convertible_to<Subroutine*> T>
    constexpr Value(T func) noexcept : 
      type(ValueType::FUNC), 
      method_func_ptr(0),
      v{ .func = func } 
    {}

    template<std::convertible_to<CppFunc*> T>
    constexpr Value(T cppfunc) noexcept : 
      type(ValueType::CPP_FUNC), 
      method_func_ptr(0),
      v{ .cppfunc = cppfunc } 
    {}

    template<std::convertible_to<Instance*> T>
    constexpr Value(T instance) noexcept :  
      type(ValueType::OBJ), 
      method_func_ptr(0),
      v{ .instance = instance } 
    {}

    template<std::convertible_to<Dict*> T>
    constexpr Value(T dict) noexcept : 
      type(ValueType::OBJ), 
      method_func_ptr(0),
      v{ .dict = dict } 
    {}

    template<std::convertible_to<Instance*> I, std::convertible_to<Subroutine*> S >
    GSL_SUPPRESS(type.1)
    constexpr Value(I instance, S func) noexcept :
      type(ValueType::METHOD), 
      method_func_ptr(reinterpret_cast<uintptr_t>(static_cast<Subroutine*>(func))),
      v{ .instance = instance } 
    {}

    template<std::convertible_to<Class*> T>
    constexpr Value(T klass) noexcept : 
      type(ValueType::OBJ), 
      method_func_ptr(0),
      v{ .klass = klass } 
    {}

    template<remove_cv_same_as<bool> T>
    constexpr Value(T b) noexcept : 
      type(ValueType::BOOL), 
      method_func_ptr(0),
      v{ .b = b } 
    {}

    template<std::floating_point T>
    constexpr Value(T f64) noexcept : 
      type(ValueType::F64), 
      method_func_ptr(0),
      v{ .f64 = f64 } 
    {}

    template<IntegralExcludeBool T>
    constexpr Value(T i64) noexcept : 
      type(ValueType::I64), 
      method_func_ptr(0),
      v{ .i64 = i64 } 
    {}

    constexpr bool is_nil() const noexcept
    {
      return (type == ValueType::NIL);
    }

    constexpr bool is_str() const noexcept
    {
      return (type == ValueType::OBJ) && (v.obj != nullptr) && (v.obj->type == ObjType::STR);
    }

    constexpr bool is_tuple() const noexcept
    {
      return (type == ValueType::OBJ) && (v.obj != nullptr) && (v.obj->type == ObjType::TUPLE);
    }

    constexpr bool is_class() const noexcept
    {
      return (type == ValueType::OBJ) && (v.obj != nullptr) && (v.obj->type == ObjType::CLASS);
    }

    constexpr bool is_instance() const noexcept
    {
      return (type == ValueType::OBJ) && (v.obj != nullptr) && (v.obj->type == ObjType::INSTANCE);
    }

    constexpr bool is_dict() const noexcept
    {
      return (type == ValueType::OBJ) && (v.obj != nullptr) && (v.obj->type == ObjType::DICT);
    }

    constexpr bool is_array() const noexcept
    {
      return (type == ValueType::OBJ) && (v.obj != nullptr) && (v.obj->type == ObjType::ARRAY);
    }

    double get_double() const;
    int64_t get_int64() const;
    
    Instance* get_instance() const;
    Dict* get_dict() const;
    std::string_view get_strview() const;
    std::span<Value> get_tuplespan() const;
    Value get_property(gsl::not_null<String*> name);

    bool is_truthy() const noexcept;
    Subroutine* method_func() const noexcept;

    friend double operator/(const Value& l, const Value& r);
    friend Value operator*(const Value& l, const Value& r);
    friend Value operator+(const Value& l, const Value& r);

    template<Allocator F>
    static Tuple* tuplecat(F allocator, const Value& l, const Value& r);

    friend Value operator-(const Value& l, const Value& r);
    friend Value operator-(const Value& val);
    friend bool operator!(const Value& val);
    friend int64_t intdiv(const Value& l, const Value& r);
    friend std::partial_ordering operator<=>(const Value& l, const Value& r);
    friend bool operator==(const Value& l, const Value& r) noexcept;

    std::string to_string() const;
    std::array<uint64_t, 2> serialize() const noexcept;

    bool debug_type_is_valid() noexcept;
  };
}

// hash_table.h
namespace foxlox
{
    export struct StringPoolEntry
    {
        uint32_t hash;
        bool tombstone;
        String* str;

        bool key_is_null();
    };

    export class StringPool
    {
    public:
        template<Allocator A, Deallocator D>
        StringPool(A alloc, D dealloc) :
            allocator(alloc),
            deallocator(dealloc),
            entries{},
            count(0),
            capacity{}
        {
            init_entries();
        }
        StringPool(const StringPool&) = delete;
        StringPool& operator=(const StringPool&) = delete;
        StringPool(StringPool&& o) noexcept;
        StringPool& operator=(StringPool&& o) noexcept;
        ~StringPool();

        gsl::not_null<String*> add_string(std::string_view str);
        gsl::not_null<String*> add_str_cat(std::string_view lhs, std::string_view rhs);
        void sweep();
    private:
        void init_entries();
        void clean();
        void delete_entry(StringPoolEntry& e);

        std::function<char* (size_t)> allocator;
        std::function<void(char* const, size_t)> deallocator;
        StringPoolEntry* entries;
        uint32_t count;
        uint32_t capacity;

        template<typename U>
        friend void grow_capacity(U* table);
    };

    export template<typename K, typename V>
    struct HashTableEntry
    {
        uint32_t hash;
        bool tombstone;
        K key;
        V value;

        bool key_is_null();
    };

    template<typename K, typename V>
    class HashTable;

    export template<typename K, typename V>
    class HashTableIter
    {
    public:
        HashTableIter(HashTable<K, V>* table, HashTableEntry<K, V>* entry) noexcept;
        bool operator==(const HashTableIter& rhs) const noexcept;
        HashTableEntry<K, V>& operator*() const noexcept;
        HashTableIter& operator++() noexcept;
    private:
        HashTable<K, V>* p_table;
        HashTableEntry<K, V>* p_entry;
    };

    export template<typename K, typename V>
    class HashTable
    {
    public:
        template<Allocator A, Deallocator D>
        HashTable(A alloc, D dealloc) :
            allocator(alloc),
            deallocator(dealloc),
            entries{},
            count(0),
            capacity{}
        {
            static_assert(
                (std::same_as<K, String*>&& std::same_as<V, Subroutine*>) ||
                (std::same_as<K, String*> && std::same_as<V, Value>) ||
                (std::same_as<K, Value> && std::same_as<V, Value>)
                );
            init_entries();
        }
        HashTable(const HashTable&) = delete;
        HashTable& operator=(const HashTable&) = delete;
        HashTable(HashTable&& o) noexcept;
        HashTable& operator=(HashTable&& o) noexcept;
        ~HashTable();

        void set_entry(K key, V value);
        void try_add_entry(K key, V value);
        std::optional<V> get_value(K key);

        HashTableIter<K, V> begin() noexcept;
        HashTableIter<K, V> end() noexcept;
    private:
        void init_entries();
        void clean();
        gsl::not_null<HashTableEntry<K, V>*> find_entry(K key, uint32_t hash) noexcept;
        void delete_entry(HashTableEntry<K, V>& e) noexcept;
        HashTableEntry<K, V>* next_entry(HashTableEntry<K, V>* p) noexcept;

        std::function<char* (size_t)> allocator;
        std::function<void(char* const, size_t)> deallocator;
        HashTableEntry<K, V>* entries;
        uint32_t count;
        uint32_t capacity;

        template<typename U>
        friend void grow_capacity(U* table);

        friend class HashTableIter<K, V>;
    };
}

// object.h
namespace foxlox
{
    export template<typename T>
    class SimpleObj : public ObjBase
    {
    private:
        bool gc_mark;
        uint32_t m_size;
    public:
        SimpleObj(ObjType t, size_t l) noexcept :
            ObjBase(t), gc_mark(false), m_size(gsl::narrow_cast<uint32_t>(l))
        {
            Expects(l <= std::numeric_limits<decltype(m_size)>::max());
        }
        SimpleObj(const SimpleObj&) = delete;
        SimpleObj(SimpleObj&&) = delete;
        SimpleObj& operator=(const SimpleObj&) = delete;
        SimpleObj& operator=(SimpleObj&&) = delete;

        auto data() noexcept
        {
            GSL_SUPPRESS(type.2) GSL_SUPPRESS(bounds.3)
                return static_cast<T*>(this)->m_data;
        }
        auto data() const noexcept
        {
            GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(type.2)
                return static_cast<const T*>(this)->m_data;
        }
        bool is_marked() const noexcept
        {
            return gc_mark;
        }
        void mark() noexcept
        {
            gc_mark = true;
        }
        void unmark() noexcept
        {
            gc_mark = false;
        }
        size_t size() const noexcept
        {
            return m_size;
        }
    };

    export class String : public SimpleObj<String>
    {
        friend class SimpleObj<String>;
    private:
#pragma warning(disable:4200) // nonstandard extension used : zero-sized array in struct/union
        char m_data[0];
#pragma warning(default:4200)
    public:
        String(size_t l) noexcept;
        ~String() = default;

        template<Allocator F>
        static gsl::not_null<String*> alloc(F allocator, size_t l)
        {
            const gsl::not_null<char*> data = allocator(sizeof(String) + l * sizeof(char));
            return new(data) String(l);
        }

        template<Deallocator F>
        static void free(F deallocator, gsl::not_null<String*> p)
        {
            const auto size = p->size();
            p->~String();
            GSL_SUPPRESS(type.1)
                deallocator(reinterpret_cast<char*>(p.get()), sizeof(String) + size * sizeof(char));
        }

        std::string_view get_view() const noexcept;
        friend auto operator<=>(const String& l, const String& r) noexcept
        {
            return l.get_view() <=> r.get_view();
        }
        friend bool operator==(const String& l, const String& r) noexcept;
    };


    export class Tuple : public SimpleObj<Tuple>
    {
        friend class SimpleObj<Tuple>;
    private:
#pragma warning(disable:4200) // nonstandard extension used : zero-sized array in struct/union
        Value m_data[0];
#pragma warning(default:4200)
    public:
        Tuple(size_t l) noexcept;
        ~Tuple() = default;

        template<Allocator F>
        static gsl::not_null<Tuple*> alloc(F allocator, size_t l)
        {
            const gsl::not_null<char*> data = allocator(sizeof(Tuple) + l * sizeof(Value));
            return new(data) Tuple(l);
        }

        template<Deallocator F>
        static void free(F deallocator, gsl::not_null<Tuple*> p)
        {
            const auto size = p->size();
            p->~Tuple();
            GSL_SUPPRESS(type.1)
                deallocator(reinterpret_cast<char*>(p.get()), sizeof(Tuple) + size * sizeof(Value));
        }

        std::span<const Value> get_span() const noexcept;
        std::span<Value> get_span() noexcept;
    };

    export class Class : public ObjBase
    {
    public:
        Class(std::string_view name);
        std::string_view get_name() const noexcept { return class_name; }
        void add_method(String* name, Subroutine* func);
        void set_super(gsl::not_null<Class*> super);
        Class* get_super() noexcept;
        bool has_method(String* name);
        std::optional<Subroutine*> get_method(String* name);
        HashTable<String*, Subroutine*>& get_hash_table() noexcept;

        bool is_marked() const noexcept;
        void mark() noexcept;
        void unmark() noexcept;
    private:
        bool gc_mark;
        Class* superclass;
        std::string class_name;
        HashTable<String*, Subroutine*> methods;
    };

    export class Instance : public ObjBase
    {
    public:
        template<Allocator A, Deallocator D>
        Instance(A allocator, D deallocator, Class* from_class) :
            ObjBase(ObjType::INSTANCE),
            gc_mark(false),
            klass(from_class),
            fields(allocator, deallocator)
        {
        }
        Instance(const Instance&) = delete;
        Instance(Instance&&) = delete;
        Instance& operator=(const Instance&) = delete;
        Instance& operator=(Instance&&) = delete;
        ~Instance() = default;
        Class* get_class() const noexcept;
        Value get_property(gsl::not_null<String*> name);
        Value get_super_method(gsl::not_null<String*> name);
        HashTable<String*, Value>& get_hash_table() noexcept;
        void set_property(gsl::not_null<String*> name, Value value);
        bool is_marked() const noexcept;
        void mark() noexcept;
        void unmark() noexcept;

        template<Allocator A, Deallocator D>
        static gsl::not_null<Instance*> alloc(A allocator, D deallocator, Class* klass)
        {
            const gsl::not_null<char*> data = allocator(sizeof(Instance));
            return new(data) Instance(allocator, deallocator, klass);
        }

        template<Deallocator F>
        static void free(F deallocator, gsl::not_null<Instance*> p)
        {
            // call dtor to delete the map inside
            p->~Instance();
            GSL_SUPPRESS(type.1)
                deallocator(reinterpret_cast<char*>(p.get()), sizeof(Instance));
        }
    private:
        bool gc_mark;
        Class* klass;
        HashTable<String*, Value> fields;
    };

    template<Allocator F>
    Tuple* Value::tuplecat(F allocator, const Value& l, const Value& r)
    {
        Expects(l.is_tuple() || r.is_tuple());
        if (l.is_tuple() && r.is_tuple())
        {
            const auto s1 = l.v.tuple->get_span();
            const auto s2 = r.v.tuple->get_span();
            const gsl::not_null p = Tuple::alloc(allocator, s1.size() + s2.size());
            GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(stl.1)
                const auto it = std::copy(s1.begin(), s1.end(), p->data());
            GSL_SUPPRESS(stl.1)
                std::copy(s2.begin(), s2.end(), it);
            return p;
        }
        else if (l.is_tuple())
        {
            const auto s1 = l.v.tuple->get_span();
            const gsl::not_null p = Tuple::alloc(allocator, s1.size() + 1);
            GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(stl.1)
                const auto it = std::copy(s1.begin(), s1.end(), p->data());
            *it = r;
            return p;
        }
        else
        {
            const auto s2 = r.v.tuple->get_span();
            const gsl::not_null p = Tuple::alloc(allocator, 1 + s2.size());
            GSL_SUPPRESS(bounds.1)
                p->data()[0] = l;
            GSL_SUPPRESS(bounds.3) GSL_SUPPRESS(stl.1) GSL_SUPPRESS(bounds.1)
                std::copy(s2.begin(), s2.end(), p->data() + 1);
            return p;
        }
    }

    export class Dict : public ObjBase
    {
    public:
        template<Allocator A, Deallocator D>
        Dict(A allocator, D deallocator) :
            ObjBase(ObjType::DICT),
            gc_mark(false),
            fields(allocator, deallocator)
        {
        }
        Dict(const Instance&) = delete;
        Dict(Instance&&) = delete;
        Dict& operator=(const Instance&) = delete;
        Dict& operator=(Instance&&) = delete;
        ~Dict() = default;
        Value get(gsl::not_null<String*> name);
        HashTable<Value, Value>& get_hash_table() noexcept;
        void set(gsl::not_null<String*> name, Value value);
        bool is_marked() const noexcept;
        void mark() noexcept;
        void unmark() noexcept;

        template<Allocator A, Deallocator D>
        static gsl::not_null<Dict*> alloc(A allocator, D deallocator)
        {
            const gsl::not_null<char*> data = allocator(sizeof(Dict));
            return new(data) Dict(allocator, deallocator);
        }

        template<Deallocator F>
        static void free(F deallocator, gsl::not_null<Dict*> p)
        {
            // call dtor to delete the map inside
            p->~Dict();
            GSL_SUPPRESS(type.1)
                deallocator(reinterpret_cast<char*>(p.get()), sizeof(Dict));
        }
    private:
        bool gc_mark;
        HashTable<Value, Value> fields;
    };
}

