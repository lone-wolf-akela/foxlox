module foxlox:hash_table;

import <iostream>;

import :object;

namespace foxlox
{
  bool str_equal(gsl::not_null<String*> l, std::string_view r) noexcept
  {
    return l->get_view() == r;
  }
  bool str_equal(gsl::not_null<String*> l, std::string_view r1, std::string_view r2) noexcept
  {
    //TODO: deduce this
    GSL_SUPPRESS(stl.1) GSL_SUPPRESS(bounds.1)
      return l->size() == (r1.size() + r2.size()) &&
      std::equal(r1.begin(), r1.end(), l->data<String>()) &&
      std::equal(r2.begin(), r2.end(), l->data<String>() + r1.size());
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
          //TODO: deduce this
          GSL_SUPPRESS(stl.1)
            std::copy(str.begin(), str.end(), p->data<String>());

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
          //TODO: deduce this
          const auto it = std::copy(lhs.begin(), lhs.end(), p->data<String>());
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
        std::cout << std::format("sweeping {} [{}]: {}\n", static_cast<const void*>(e.str), e.str->is_marked() ? "is_marked" : "not_marked", e.str->get_view());
      }
#endif
      if (e.str != nullptr && !e.tombstone && !e.str->is_marked())
      {
        delete_entry(e);
      }
    }
  }
  void StringPool::delete_entry(StringPoolEntry& e)
  {
    Expects(e.str != nullptr && !e.tombstone);
    String::free(deallocator, e.str);
    e.tombstone = true;
    // tombstone still counts in count, so we do not count-- here
  }
}