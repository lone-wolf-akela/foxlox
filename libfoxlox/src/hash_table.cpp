#include <cassert>
#include <cstring>
#include <algorithm>
#include <bit>
#include <iostream>

#include <gsl/gsl>
#include <fmt/format.h>

#include <foxlox/except.h>
#include <foxlox/config.h>
#include "object.h"

#include "hash_table.h"

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
    for(auto c: str)
    {
      hash ^= gsl::narrow_cast<uint8_t>(c);
      hash *= 16777619u;
    }
    return hash;
  }

  uint32_t str_hash(std::string_view str1, std::string_view str2) noexcept
  {
    uint32_t hash = 2166136261u;
    for (auto c : str1)
    {
      hash ^= gsl::narrow_cast<uint8_t>(c);
      hash *= 16777619u;
    }
    for (auto c : str2)
    {
      hash ^= gsl::narrow_cast<uint8_t>(c);
      hash *= 16777619u;
    }
    return hash;
  }

  GSL_SUPPRESS(type.1)
  uint32_t ptr_hash(void * p) noexcept
  {
    // MurmurHash3, a lot faster than FNV-1a for pointer
    // from https://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html
    uintptr_t p_int = reinterpret_cast<uintptr_t>(p);
    p_int ^= (p_int >> 33);
    p_int *= 0xff51afd7ed558ccdull;
    p_int ^= (p_int >> 33);
    p_int *= 0xc4ceb9fe1a85ec53ull;
    p_int ^= (p_int >> 33);
    return gsl::narrow_cast<uint32_t>(p_int);
  }
}
namespace foxlox
{
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
      if (e.tombstone || e.str == nullptr)
      {
        continue;
      }
      new_count++;
      uint32_t idx = e.hash & (new_capacity - 1);
      while (true)
      {
        GSL_SUPPRESS(bounds.1)
        if (new_entries[idx].str == nullptr)
        {
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

  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  HashTable<T>::~HashTable()
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

  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  void HashTable<T>::clean()
  {
    if (entries != nullptr)
    {
      GSL_SUPPRESS(type.1)
      deallocator(reinterpret_cast<char*>(entries), capacity * sizeof(*entries));
    }
  }

  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  void HashTable<T>::set_entry(String* name, T value)
  {
    if (count + 1 > capacity * STRING_POOL_MAX_LOAD)
    {
      grow_capacity(this);
    }
    const uint32_t hash = ptr_hash(name);
    const auto entry = find_entry(name, hash);
    if (entry->str == nullptr && !entry->tombstone)
    {
      // new entry
      count++;
    }
    if constexpr (std::same_as<T, Value>)
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
    entry->str = name;
    entry->value = value;
  }
  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  void HashTable<T>::try_add_entry(String* name, T value)
  {
    if (count + 1 > capacity * STRING_POOL_MAX_LOAD)
    {
      grow_capacity(this);
    }
    const uint32_t hash = ptr_hash(name);
    const auto entry = find_entry(name, hash);
    if (entry->str != nullptr && !entry->tombstone)
    {
      // key already exist, do nothing
      return;
    }
    if (entry->str == nullptr && !entry->tombstone)
    {
      // new entry
      count++;
    }
    entry->hash = hash;
    entry->tombstone = false;
    entry->str = name;
    entry->value = value;
  }
  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  std::optional<T> HashTable<T>::get_value(String* name)
  {
    const uint32_t hash = ptr_hash(name);
    const auto entry = find_entry(name, hash);
    if (entry->str != nullptr && !entry->tombstone)
    {
      // key found
      return entry->value;
    }
    else
    {
      return std::nullopt;
    }
  }
  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  HashTableIter<T> HashTable<T>::begin() noexcept
  {
    for (auto& e : std::span(entries, capacity))
    {
      if (e.str != nullptr && !e.tombstone)
      {
        return HashTableIter<T>(this, &e);
      }
     }
    return end();
  }
  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  HashTableIter<T> HashTable<T>::end() noexcept
  {
    return HashTableIter<T>(this, nullptr);
  }

  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
    GSL_SUPPRESS(bounds.1) GSL_SUPPRESS(lifetime.4) GSL_SUPPRESS(lifetime.1)
  HashTableEntry<T>* HashTable<T>::next_entry(HashTableEntry<T>* p) noexcept
  {
    Expects(entries <= p && p < entries + capacity);
    while (++p < entries + capacity)
    {
      if (p->str != nullptr && !p->tombstone)
      {
        return p;
      }
    }
    return nullptr;
  }
  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
    GSL_SUPPRESS(f.6)
  HashTable<T>::HashTable(HashTable&& o) noexcept :
    allocator(o.allocator),
    deallocator(o.deallocator),
    entries(o.entries),
    count(o.count),
    capacity(o.capacity)
  {
    o.entries = nullptr;
  }
  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  HashTable<T>& HashTable<T>::operator=(HashTable&& o) noexcept
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
  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  void HashTable<T>::init_entries()
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
  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  gsl::not_null<HashTableEntry<T>*> HashTable<T>::find_entry(gsl::not_null<String*> name, uint32_t hash) noexcept
  {
    uint32_t idx = hash & (capacity - 1);
    HashTableEntry<T>* first_tombstone = nullptr;
    GSL_SUPPRESS(bounds.1)
    while (true)
    {
      if (entries[idx].tombstone)
      {
        if (first_tombstone == nullptr) { first_tombstone = &entries[idx]; }
      }
      else if (entries[idx].str == nullptr)
      {
        return first_tombstone ? first_tombstone : &entries[idx];
      }
      else if (entries[idx].str == name)
      {
        return &entries[idx];
      }
      idx = (idx + 1) & (capacity - 1);
    }
  }

  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  void HashTable<T>::delete_entry(HashTableEntry<T>& e) noexcept
  {
    if (e.str != nullptr)
    {
      e.tombstone = true;
    }
  }

  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  HashTableIter<T>::HashTableIter(HashTable<T>* table, HashTableEntry<T>* entry) noexcept :
    p_table(table), p_entry(entry) 
  {
  }

  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  bool HashTableIter<T>::operator==(const HashTableIter<T>& rhs) const noexcept
  {
    return p_entry == rhs.p_entry;
  }

  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  HashTableEntry<T>& HashTableIter<T>::operator*() const noexcept
  {
    return *p_entry;
  }

  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  HashTableIter<T>& HashTableIter<T>::operator++() noexcept
  {
    p_entry = p_table->next_entry(p_entry);
    return *this;
  }

  // Explicit template instantiation
  template class HashTable<Subroutine*>;
  template class HashTable<Value>;

  template class HashTableIter<Subroutine*>;
  template class HashTableIter<Value>;
}
