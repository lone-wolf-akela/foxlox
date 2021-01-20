#pragma once
#include <string>
#include <string_view>
#include <functional>
#include <vector>
#include <concepts>
#include <optional>

#include "value.h"

namespace foxlox
{
  struct StringPoolEntry
  {
    uint32_t hash;
    bool tombstone;
    String* str;
  };

  class StringPool
  {
  public:
    template<Allocator A, Deallocator D>
    StringPool(A alloc, D dealloc) :
      allocator(alloc), 
      deallocator(dealloc), 
      count(0),
      entries{},
      capacity_mask{}
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
    size_t count;
    StringPoolEntry* entries;
    size_t capacity_mask;

    template<typename U>
    friend void grow_capacity(U* table);
  };

  template<typename T>
  struct HashTableEntry
  {
    uint32_t hash;
    bool tombstone;
    String* str;
    T value;
  };

  template<typename T> requires std::same_as<T, Subroutine*> || std::same_as<T, Value>
  class HashTable
  {
  public:
    template<Allocator A, Deallocator D>
    HashTable(A alloc, D dealloc) :
      allocator(alloc),
      deallocator(dealloc),
      count(0),
      entries{},
      capacity_mask{}
    {
      init_entries();
    }
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;
    HashTable(HashTable&& o) noexcept;
    HashTable& operator=(HashTable&& o) noexcept;
    ~HashTable();

    void set_entry(String* name, T value);
    void try_add_entry(String* name, T value);
    std::optional<T> get_value(String* name);
    HashTableEntry<T>* first_entry() noexcept;
    HashTableEntry<T>* next_entry(HashTableEntry<T>* p) noexcept;
  private:
    void init_entries();
    void clean();
    gsl::not_null<HashTableEntry<T>*> find_entry(gsl::not_null<String*> name, uint32_t hash) noexcept;
    void delete_entry(HashTableEntry<T>& e) noexcept;

    std::function<char* (size_t)> allocator;
    std::function<void(char* const, size_t)> deallocator;
    size_t count;
    HashTableEntry<T>* entries;
    size_t capacity_mask;

    template<typename U>
    friend void grow_capacity(U* table);
  };
}