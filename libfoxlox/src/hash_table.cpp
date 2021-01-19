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

  bool str_equal(String* l, std::string_view r)
  {
    return l->get_view() == r;
  }
  bool str_equal(String* l, std::string_view r1, std::string_view r2)
  {
    return l->size() == (r1.size() + r2.size()) &&
      std::equal(r1.begin(), r1.end(), l->data()) &&
      std::equal(r2.begin(), r2.end(), l->data() + r1.size());
  }

  // FNV-1a hash
  // http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-1a
  uint32_t str_hash(std::string_view str)
  {
    uint32_t hash = 2166136261u;
    for(auto c: str)
    {
      hash ^= static_cast<uint8_t>(c);
      hash *= 16777619u;
    }
    return hash;
  }

  uint32_t str_hash(std::string_view str1, std::string_view str2)
  {
    uint32_t hash = 2166136261u;
    for (auto c : str1)
    {
      hash ^= static_cast<uint8_t>(c);
      hash *= 16777619u;
    }
    for (auto c : str2)
    {
      hash ^= static_cast<uint8_t>(c);
      hash *= 16777619u;
    }
    return hash;
  }

  uint32_t ptr_hash(void * p)
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
  void grow_capacity(T* table)
  {
    if (table->count > std::bit_floor(std::numeric_limits<decltype(table->count)>::max()) / 2)
    {
      throw InternalRuntimeError("Too many strings. String pool is full.");
    }
    size_t new_count = 0;
    // make sure use this std::bit_ceil
    // other wise capacity_mask will be broken
    const size_t new_capacity = (table->capacity_mask + 1) * 2;
    const size_t new_capacity_mask = new_capacity - 1;
    auto new_entries = reinterpret_cast<decltype(table->entries)>(
      table->allocator(new_capacity * sizeof(*table->entries)));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
    std::memset(new_entries, 0, new_capacity * sizeof(*table->entries));
#pragma GCC diagnostic pop
    for (auto& e : std::span(table->entries, table->capacity_mask + 1))
    {
      if (e.tombstone || e.str == nullptr)
      {
        continue;
      }
      new_count++;
      gsl::index idx = e.hash & new_capacity_mask;
      while (true)
      {
        if (new_entries[idx].str == nullptr)
        {
          new_entries[idx] = e;
          break;
        }
        idx = (idx + 1) & new_capacity_mask;
      }
    }
    table->deallocator(reinterpret_cast<char*>(table->entries), 
      (table->capacity_mask + 1) * sizeof(*table->entries));
    table->entries = new_entries;
    table->count = new_count;
    table->capacity_mask = new_capacity_mask;
  }

  StringPool::~StringPool()
  {
    clean();
  }
  String* StringPool::add_string(std::string_view str)
  {
    if (count + 1 > (capacity_mask + 1) * STRING_POOL_MAX_LOAD)
    {
      grow_capacity(this);
    }
    const auto hash = str_hash(str);
    gsl::index idx = hash & capacity_mask;
    StringPoolEntry* first_tombstone = nullptr;
    while (true)
    {
      if (entries[idx].tombstone)
      {
        if (first_tombstone == nullptr) { first_tombstone = &entries[idx]; }
      }
      else if (entries[idx].str == nullptr)
      {
        String* p = String::alloc(allocator, str.size());
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
      idx = (idx + 1) & capacity_mask;
    }
  }
  StringPool::StringPool(StringPool&& o) noexcept : 
    allocator(o.allocator),
    deallocator(o.deallocator),
    count(o.count),
    entries(o.entries),
    capacity_mask(o.capacity_mask)
  {
    o.entries = nullptr;
  }
  StringPool& StringPool::operator=(StringPool&& o) noexcept
  {
    if (this == &o) { return *this; }
    clean();
    allocator = o.allocator;
    deallocator = o.deallocator;
    count = o.count;
    entries = o.entries;
    capacity_mask = o.capacity_mask;
    o.entries = nullptr;
    return *this;
  }
  String* StringPool::add_str_cat(std::string_view lhs, std::string_view rhs)
  {
    if (count + 1 > (capacity_mask + 1) * STRING_POOL_MAX_LOAD)
    {
      grow_capacity(this);
    }
    const auto hash = str_hash(lhs, rhs);
    gsl::index idx = hash & capacity_mask;
    StringPoolEntry* first_tombstone = nullptr;
    while (true)
    {
      if (entries[idx].tombstone)
      {
        if (first_tombstone == nullptr) { first_tombstone = &entries[idx]; }
      }
      else if (entries[idx].str == nullptr)
      {
        String* p = String::alloc(allocator, lhs.size() + rhs.size());
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
      idx = (idx + 1) & capacity_mask;
    }
  }
  void StringPool::sweep()
  {
    for (auto& e : std::span(entries, capacity_mask + 1))
    {
#ifdef DEBUG_LOG_GC
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
    capacity_mask = HASH_TABLE_START_BUCKET - 1;
    entries = reinterpret_cast<decltype(entries)>(
      allocator(HASH_TABLE_START_BUCKET * sizeof(*entries)));
    std::memset(entries, 0, HASH_TABLE_START_BUCKET * sizeof(*entries));
    // make sure HASH_TABLE_START_BUCKET is power of 2
    // otherwise capacity_mask won't work
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
      for (auto& e : std::span(entries, capacity_mask + 1))
      {
        if (e.str != nullptr && !e.tombstone)
        {
          delete_entry(e);
        }
      }
      deallocator(reinterpret_cast<char*>(entries), (capacity_mask + 1) * sizeof(*entries));
    }
  }

  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  HashTable<T>::~HashTable()
  {
    clean();
  }

  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  void HashTable<T>::clean()
  {
    if (entries != nullptr)
    {
      deallocator(reinterpret_cast<char*>(entries), (capacity_mask + 1) * sizeof(*entries));
    }
  }

  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  void HashTable<T>::set_entry(String* name, T value)
  {
    if (count + 1 > (capacity_mask + 1) * STRING_POOL_MAX_LOAD)
    {
      grow_capacity(this);
    }
    const uint32_t hash = ptr_hash(name);
    auto entry = find_entry(name, hash);
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
    if (count + 1 > (capacity_mask + 1) * STRING_POOL_MAX_LOAD)
    {
      grow_capacity(this);
    }
    const uint32_t hash = ptr_hash(name);
    auto entry = find_entry(name, hash);
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
    auto entry = find_entry(name, hash);
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
  HashTableEntry<T>* HashTable<T>::first_entry()
  {
    for (auto& e : std::span(entries, capacity_mask + 1))
    {
      if (e.str != nullptr && !e.tombstone)
      {
        return&e;
      }
     }
    return nullptr;
  }
  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  HashTableEntry<T>* HashTable<T>::next_entry(HashTableEntry<T>* p)
  {
    while (++p <= entries + capacity_mask)
    {
      if (p->str != nullptr && !p->tombstone)
      {
        return p;
      }
    }
    return nullptr;
  }
  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  HashTable<T>::HashTable(HashTable&& o) noexcept :
    allocator(o.allocator),
    deallocator(o.deallocator),
    count(o.count),
    entries(o.entries),
    capacity_mask(o.capacity_mask)
  {
    o.entries = nullptr;
  }
  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  HashTable<T>& HashTable<T>::operator=(HashTable&& o) noexcept
  {
    if (this == &o) { return *this; }
    clean();
    allocator = o.allocator;
    deallocator = o.deallocator;
    count = o.count;
    entries = o.entries;
    capacity_mask = o.capacity_mask;
    o.entries = nullptr;
    return *this;
  }
  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  void HashTable<T>::init_entries()
  {
    capacity_mask = HASH_TABLE_START_BUCKET - 1;
    entries = reinterpret_cast<decltype(entries)>(
      allocator(HASH_TABLE_START_BUCKET * sizeof(*entries)));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
    std::memset(entries, 0, HASH_TABLE_START_BUCKET * sizeof(*entries));
#pragma GCC diagnostic pop
    // make sure HASH_TABLE_START_BUCKET is power of 2
    // otherwise capacity_mask won't work
    static_assert((HASH_TABLE_START_BUCKET & (HASH_TABLE_START_BUCKET - 1)) == 0);
  }
  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  HashTableEntry<T>* HashTable<T>::find_entry(String* name, uint32_t hash)
  {
    gsl::index idx = hash & capacity_mask;
    HashTableEntry<T>* first_tombstone = nullptr;
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
      idx = (idx + 1) & capacity_mask;
    }
  }

  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  void HashTable<T>::delete_entry(HashTableEntry<T>& e)
  {
    if (e.str != nullptr)
    {
      e.tombstone = true;
    }
  }

  // Explicit template instantiation
  template class HashTable<Subroutine*>;
  template class HashTable<Value>;
}