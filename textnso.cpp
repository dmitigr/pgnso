// -*- C++ -*-
// Copyright (C) 2020 Dmitry Igrishin
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Dmitry Igrishin
// dmitigr@gmail.com

#include <cstring>
#include <cwchar>
#include <cwctype>

extern "C" {

#include "postgres.h"
#include "fmgr.h" // magic block, PG_RETURN_* macros, etc
#include "catalog/pg_collation.h"
#include "mb/pg_wchar.h" // GetDatabaseEncoding()
#include "utils/fmgrprotos.h" // textsend(), textrecv()

PG_MODULE_MAGIC;

void _PG_init(){}
void _PG_fini(){}

/**
 * @brief An alias to varlena.
 *
 * @remarks The data length is always VARSIZE_ANY_EXHDR(ptr).
 */
using Textnso = varlena;

/*
 * Alignment doesn't matter for this type.
 */
#define DatumGetTextNsoPP(X) ((Textnso *) PG_DETOAST_DATUM_PACKED(X))
#define PG_GETARG_TEXTNSO_PP(n) DatumGetTextNsoPP(PG_GETARG_DATUM(n))

// -----------------------------------------------------------------------------
// I/O
// -----------------------------------------------------------------------------

PGDLLEXPORT Datum textnso_in(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(textnso_in);

Datum textnso_in(PG_FUNCTION_ARGS)
{
  return textin(fcinfo);
}

// -----------------------------------------------------------------------------

PGDLLEXPORT Datum textnso_out(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(textnso_out);

Datum textnso_out(PG_FUNCTION_ARGS)
{
  return textout(fcinfo);
}

// -----------------------------------------------------------------------------

PGDLLEXPORT Datum textnso_recv(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(textnso_recv);

Datum textnso_recv(PG_FUNCTION_ARGS)
{
  return textrecv(fcinfo);
}

// -----------------------------------------------------------------------------

PGDLLEXPORT Datum textnso_send(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(textnso_send);

Datum textnso_send(PG_FUNCTION_ARGS)
{
  return textsend(fcinfo);
}

// -----------------------------------------------------------------------------
// Comparison operations
// -----------------------------------------------------------------------------

/**
 * @brief Comparison object.
 */
struct Cmp_obj final {
  /// @brief The stack-allocated data size, including terminating zero.
  constexpr static std::size_t stack_data_size = 128;

  /// @brief The type of comparison object.
  enum class Type {
    str,
    num
  };

  /**
   * @brief Clears `obj` and fills it with a data of known Type by reading from `p`.
   *
   * @return The pointer that immediately followed the object read.
   */
  static const wchar_t* get(Cmp_obj& obj, const wchar_t* p)
  {
    obj.clear();

    if (!p || *p == L'\0')
      return p;

    if (std::iswdigit(static_cast<wint_t>(*p))) {
      obj.type_ = Type::num;

      // First, skip zeroes.
      while (*p != L'\0' && *p == L'0')
        ++p;

      while (*p != L'\0' && std::iswdigit(static_cast<wint_t>(*p))) {
        ++obj.size_;
        ++p;
      }
      p -= obj.size_;
      obj.init_data__(p);
    } else {
      obj.type_ = Type::str;

      while (*p != L'\0' && !std::iswdigit(static_cast<wint_t>(*p))) {
        ++obj.size_;
        ++p;
      }
      p -= obj.size_;
      obj.init_data__(p);
    }
    return p + obj.size_;
  }

  /**
   * @returns -1 if `lhs` less than `rhs`, 0 if `lhs` equals to `rhs` and
   * 1 if `lhs` greater than `rhs`.
   */
  static int cmp(const Cmp_obj& lhs, const Cmp_obj& rhs)
  {
    Assert(lhs.data_);
    Assert(rhs.data_);

    if (*lhs.data_ == L'\0' && *rhs.data_ == L'\0')
      return 0;
    else if (*lhs.data_ == L'\0')
      return -1;
    else if (*rhs.data_ == L'\0')
      return 1;

    if (lhs.type_ == Type::num) {
      if (rhs.type_ == Type::num) {
        if (lhs.size_ == rhs.size_)
          return std::wcscmp(lhs.data_, rhs.data_);
        else if (lhs.size_ < rhs.size_)
          return -1;
        else
          return 1;
      } else
        return -1;
    } else {
      if (rhs.type_ == Type::str)
        return std::wcscoll(lhs.data_, rhs.data_);
      else
        return 1;
    }

    // Should not happen.
    elog(ERROR, "unknown comparison object type");
    return 0;
  }

  /// @returns Pointer to the null-terminated wide string.
  wchar_t* data() { return data_; }

  /// @returns The size of wide string in characters.
  std::size_t size() const { return size_; }

  /// @returns The type of comparison object.
  Type type() const { return type_; }

  /// @brief Resets the internal state, but do not deallocates the memory.
  void clear()
  {
    static_assert(sizeof(stack_data_) > 0);
    Assert(data_);

    *data_ = L'\0';
    size_ = {};
    type_ = {};
  }

  /// @brief Resets the internal state and deallocates the memory.
  void reset()
  {
    Assert(data_);
    if (data_ != stack_data_) {
      pfree(data_);
      data_ = stack_data_;
      capacity_ = stack_data_size - 1;
    }
    clear();
  }

private:
  /// @brief Allocates the memory and fills the data.
  void init_data__(const wchar_t* const p)
  {
    Assert(p);
    Assert(data_);
    Assert(capacity_ >= stack_data_size - 1);

    if (size_ > 0) {
      const std::size_t data_size_in_bytes = sizeof(wchar_t) * size_;
      if (size_ > capacity_) {
        if (data_ != stack_data_)
          pfree(data_);
        data_ = static_cast<wchar_t*>(palloc(data_size_in_bytes + sizeof(wchar_t)));
        capacity_ = size_;
      }
      std::memcpy(data_, p, data_size_in_bytes);
      data_[size_] = L'\0';
    }
  }

  wchar_t stack_data_[stack_data_size]; // optimization
  wchar_t* data_{stack_data_};
  std::size_t size_{};
  std::size_t capacity_{stack_data_size - 1}; // optimization
  Type type_{};
};

/**
 * @brief Wide character string.
 *
 * @remarks The user is responsible for memory deallocation by calling reset().
 */
struct Wcs final {
  /// @brief The stack-allocated data size, including terminating zero.
  constexpr static std::size_t stack_data_size = Cmp_obj::stack_data_size * 2;

  /**
   * @brief Resets `wcs` and fills it with a data converted from a multibyte
   * string `str` by using `std::mbstowsc()`.
   */
  static void get(Wcs& wcs, const char* const str, const std::size_t str_size)
  {
    Assert(wcs.data_);
    Assert(wcs.capacity_ >= stack_data_size - 1);
    Assert(str);

    char str_copy_data[stack_data_size];
    char* str_copy = str_copy_data;
    if (str_size > wcs.capacity_) {
      str_copy = static_cast<char*>(palloc(str_size + sizeof(char)));

      if (wcs.data_ != wcs.stack_data_)
        pfree(wcs.data_);
      const std::size_t wcs_size_in_bytes = str_size * sizeof(wchar_t);
      wcs.data_ = static_cast<wchar_t*>(palloc(wcs_size_in_bytes + sizeof(wchar_t)));

      wcs.capacity_ = str_size;
    }

    std::memcpy(str_copy, str, str_size);
    str_copy[str_size] = '\0';

#ifndef _WIN32
    wcs.size_ = std::mbstowcs(wcs.data_, str_copy, str_size);
#else
    wcs.size_ = ::mbstowcs(wcs.data_, str_copy, str_size);
#endif
    if (wcs.size_ == static_cast<std::size_t>(-1))
      ereport(ERROR,
        (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
          errmsg("invalid multibyte sequence was encountered")));
    else
      wcs.data_[wcs.size_] = L'\0';

    if (str_copy != str_copy_data)
      pfree(str_copy);
  }

  /// @returns Pointer to the null-terminated wide string.
  wchar_t* data() { return data_; }

  /// @returns The size of wide string in characters.
  std::size_t size() { return size_; }

  /// @brief Resets the internal state, but preserves the memory.
  void clear()
  {
    static_assert(sizeof(stack_data_) > 0);
    Assert(data_);
    *data_ = L'\0';
    size_ = {};
  }

  /// @brief Resets the internal state and deallocates the memory.
  void reset()
  {
    Assert(data_);
    if (data_ != stack_data_) {
      pfree(data_);
      data_ = stack_data_;
      capacity_ = stack_data_size - 1;
    }
    clear();
  }

private:
  wchar_t stack_data_[stack_data_size]; // optimization
  wchar_t* data_{stack_data_};
  std::size_t capacity_{stack_data_size - 1}; // optimization
  std::size_t size_{}; // size in wchar_t's
};

/**
 * @brief Compares the `lhs` and `rhs` considering the numbers they contain.
 *
 * Examples:
 *   - "str"    < "str1";
 *   - "str9"   < "str10";
 *   - "v1.2.3" < "v1.10.1".
 *
 * @returns `-1` if `lhs` < `rhs`, `0` if `lhs` == `rhs`, `1` if `lhs` > `rhs`.
 */
static int textnso_cmp_internal(const char* const lhs, const int lhs_size,
  const char* const rhs, const int rhs_size, const Oid coll_id)
{
  // Checking collation.
  if (!OidIsValid(coll_id)) {
    ereport(ERROR,
      (errcode(ERRCODE_INDETERMINATE_COLLATION),
        errmsg("could not determine which collation to use for string comparison"),
        errhint("Use the COLLATE clause to set the collation explicitly.")));
  }

  // Currently works only with default collation.
  if (coll_id != DEFAULT_COLLATION_OID)
    ereport(ERROR,
      (errmsg("only default collation is supported by strwnum type")));

  // Currently works only with UTF8 encoding.
  if (GetDatabaseEncoding() != PG_UTF8)
    ereport(ERROR,
      (errmsg("only UTF-8 encoding is supported by strwnum type")));

  int result{};
  Wcs lhs_wcs;
  Wcs::get(lhs_wcs, lhs, lhs_size);
  Wcs rhs_wcs;
  Wcs::get(rhs_wcs, rhs, rhs_size);
  const wchar_t* lhs_wcs_p = lhs_wcs.data();
  const wchar_t* rhs_wcs_p = rhs_wcs.data();
  Cmp_obj lhs_cmp_obj;
  Cmp_obj rhs_cmp_obj;
  while ((*lhs_wcs_p != L'\0' || *rhs_wcs_p != L'\0') && result == 0) {
    lhs_wcs_p = Cmp_obj::get(lhs_cmp_obj, lhs_wcs_p);
    rhs_wcs_p = Cmp_obj::get(rhs_cmp_obj, rhs_wcs_p);
    result = Cmp_obj::cmp(lhs_cmp_obj, rhs_cmp_obj);
    lhs_cmp_obj.clear();
    rhs_cmp_obj.clear();
  }
  rhs_cmp_obj.reset();
  lhs_cmp_obj.reset();
  rhs_wcs.reset();
  lhs_wcs.reset();
  return result;
}

// -----------------------------------------------------------------------------
// API
// -----------------------------------------------------------------------------

static int textnso_op_impl(PG_FUNCTION_ARGS)
{
  Textnso* const lhs_arg = PG_GETARG_TEXTNSO_PP(0);
  Textnso* const rhs_arg = PG_GETARG_TEXTNSO_PP(1);
  const char* const lhs = VARDATA_ANY(lhs_arg);
  const char* const rhs = VARDATA_ANY(rhs_arg);
  const int lhs_size = VARSIZE_ANY_EXHDR(lhs_arg);
  const int rhs_size = VARSIZE_ANY_EXHDR(rhs_arg);
  const Oid coll_id = PG_GET_COLLATION();
  const int result = textnso_cmp_internal(lhs, lhs_size, rhs, rhs_size, coll_id);
  PG_FREE_IF_COPY(lhs_arg, 0);
  PG_FREE_IF_COPY(rhs_arg, 1);
  return result;
}

// -----------------------------------------------------------------------------

PGDLLEXPORT Datum textnso_lt(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(textnso_lt);

Datum textnso_lt(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(textnso_op_impl(fcinfo) < 0);
}

// -----------------------------------------------------------------------------

PGDLLEXPORT Datum textnso_le(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(textnso_le);

Datum textnso_le(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(textnso_op_impl(fcinfo) <= 0);
}

// -----------------------------------------------------------------------------

PGDLLEXPORT Datum textnso_eq(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(textnso_eq);

Datum textnso_eq(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(textnso_op_impl(fcinfo) == 0);
}

// -----------------------------------------------------------------------------

PGDLLEXPORT Datum textnso_ne(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(textnso_ne);

Datum textnso_ne(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(textnso_op_impl(fcinfo) != 0);
}

// -----------------------------------------------------------------------------

PGDLLEXPORT Datum textnso_ge(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(textnso_ge);

Datum textnso_ge(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(textnso_op_impl(fcinfo) >= 0);
}

// -----------------------------------------------------------------------------

PGDLLEXPORT Datum textnso_gt(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(textnso_gt);

Datum textnso_gt(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(textnso_op_impl(fcinfo) > 0);
}

// -----------------------------------------------------------------------------

PGDLLEXPORT Datum textnso_cmp(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(textnso_cmp);

Datum textnso_cmp(PG_FUNCTION_ARGS)
{
  PG_RETURN_INT32(textnso_op_impl(fcinfo));
}

} // extern "C"
