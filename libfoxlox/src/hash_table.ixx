export module foxlox:hash_table;
import <string>;
import <string_view>;
import <functional>;
import <vector>;
import <concepts>;
import <optional>;

import :value;
import :except;
import :config;
import :util;

namespace foxlox
{
  bool str_equal(gsl::not_null<String*> l, std::string_view r) noexcept;
  bool str_equal(gsl::not_null<String*> l, std::string_view r1, std::string_view r2) noexcept;

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

  export template<typename K, typename V>
    struct HashTableEntry
  {
    uint32_t hash;
    bool tombstone;
    K key;
    V value;

    bool key_is_null() noexcept
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
  };

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

  export struct StringPoolEntry
  {
    uint32_t hash;
    bool tombstone;
    String* str;

    bool key_is_null() noexcept
    {
      return str == nullptr;
    }
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
    StringPool(StringPool&& o) noexcept :
      allocator(o.allocator),
      deallocator(o.deallocator),
      entries(o.entries),
      count(o.count),
      capacity(o.capacity)
    {
      o.entries = nullptr;
    }
    StringPool& operator=(StringPool&& o) noexcept
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
    ~StringPool()
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

    gsl::not_null<String*> add_string(std::string_view str);
    gsl::not_null<String*> add_str_cat(std::string_view lhs, std::string_view rhs);

    void sweep();
  private:
    void init_entries()
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
    void clean()
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
    void delete_entry(StringPoolEntry& e);

    std::function<char* (size_t)> allocator;
    std::function<void(char* const, size_t)> deallocator;
    StringPoolEntry* entries;
    uint32_t count;
    uint32_t capacity;

    template<typename U>
    friend void grow_capacity(U* table);
  };

  template<typename K, typename V>
  class HashTable;

  export template<typename K, typename V>
    class HashTableIter
  {
  public:
    HashTableIter(HashTable<K, V>* table, HashTableEntry<K, V>* entry) noexcept :
      p_table(table), p_entry(entry)
    {
    }
    bool operator==(const HashTableIter& rhs) const noexcept
    {
      return p_entry == rhs.p_entry;
    }
    HashTableEntry<K, V>& operator*() const noexcept
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
    HashTableIter& operator++() noexcept
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
    HashTable(HashTable&& o) noexcept :
      allocator(o.allocator),
      deallocator(o.deallocator),
      entries(o.entries),
      count(o.count),
      capacity(o.capacity)
    {
      o.entries = nullptr;
    }
    HashTable& operator=(HashTable&& o) noexcept
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
    ~HashTable()
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

    void set_entry(K key, V value)
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
    void try_add_entry(K key, V value)
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
    std::optional<V> get_value(K key)
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

    HashTableIter<K, V> begin() noexcept
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
    HashTableIter<K, V> end() noexcept
    {
      return HashTableIter<K, V>(this, nullptr);
    }
  private:
    void init_entries()
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
    void clean()
    {
      if (entries != nullptr)
      {
        GSL_SUPPRESS(type.1)
          deallocator(reinterpret_cast<char*>(entries), capacity * sizeof(*entries));
      }
    }
    gsl::not_null<HashTableEntry<K, V>*> find_entry(K key, uint32_t hash) noexcept
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
    void delete_entry(HashTableEntry<K, V>& e) noexcept
    {
      if (!e.key_is_null())
      {
        e.tombstone = true;
      }
    }
    HashTableEntry<K, V>* next_entry(HashTableEntry<K, V>* p) noexcept
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

