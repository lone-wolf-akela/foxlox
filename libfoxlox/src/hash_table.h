#pragma once
import <string>;
import <string_view>;
import <functional>;
import <vector>;
import <concepts>;
import <optional>;

import <gsl/gsl>;

#include "value.h"

namespace foxlox
{
  struct StringPoolEntry
  {
    uint32_t hash;
    bool tombstone;
    String* str;

    bool key_is_null();
  };

  class StringPool
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

  template<typename K, typename V>
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

  template<typename K, typename V>
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

  template<typename K, typename V>
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

    friend class HashTableIter<K ,V>;
  };
}