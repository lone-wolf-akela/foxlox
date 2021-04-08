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
  uint32_t nonstr_hash(void * p) noexcept
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
    
    // TODO: use bit_cast when gcc supports.
    const uint32_t* data = reinterpret_cast<uint32_t*>(&v);
    constexpr int nblocks = 4;
    constexpr uint32_t seed = 0;
    uint32_t h1 = seed;

    //----------
    // body
    for (int i = 0; i < nblocks; i++)
    {
      uint32_t k1 = data[i];

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
    const uint32_t hash = nonstr_hash(key);
    const auto entry = find_entry(key, hash);
    if (!entry->key_is_null() && !entry->tombstone)
    {
      // key found
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
    return *p_entry;
  }

  template<typename K, typename V>
  HashTableIter<K, V>& HashTableIter<K, V>::operator++() noexcept
  {
    p_entry = p_table->next_entry(p_entry);
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
