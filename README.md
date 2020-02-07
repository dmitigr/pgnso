# Natural sort order for PostgreSQL

`dmitigr_nso` - is a tiny extension for [PostgreSQL] to sort strings in
[natural sort order][natural_sort_order]. Currently it implements the
type `textnso` which is binary-coercible to/from the `text` type. It's
possible to declare table columns of type `textnso` and create indexes
on them.

**ATTENTION, this software is "beta" quality, use it at your own risk!**

## Usage

Suppose there is a table already with a column of one of the
[character type][datatype-character] defined as follows:

```sql
create table t1(id serial not null primary key,
                dat  text not null);
```

In such a case to naturally order by `dat` column it's enough to just cast
it to the type `textnso` in the SQL query expression:

```sql
select * from t1 order by dat::textnso;
```

A table with column(s) of type `textnso` can also be defined. In addition
Btree-indexes can be created on such column(s) for performance reasons.

## Limitations

Currenly, works only with a default collation and UTF-8 encoding. (Which
is good enough for most cases.)

## Caveats

Because of type conversions rules defined by PostgreSQL care should be taken
when using `textnso` type in the expressions. For example:

```sql
-- could be a problem, the result type is text
select 'foo'::textnso||'bar';

-- ok, the result type is textnso
select ('foo'::textnso||'bar')::textnso;
```

## Installation

In order to build `dmitigr_nso` extension from source the following software
are required:

- [CMake] 3.10+;
- C++11 compiler (tested on [GCC] 7.4 and [Microsoft Visual C++][Visual_Studio] 15.9).

In summary, the loadable module will be placed to the directory specified via
`PG_PKGLIB_DIR` CMake variable, and SQL and control files will be placed to the
directory specified via the `PG_SHARE_DIR` CMake variable. After the build and
install procedures described below, the extension `dmitigr_nso` must be created:

```sql
create extension dmitigr_nso;
```

See [CREATE EXTENSION command documentation][sql-createextension] for details.

### CMake variables

The table below (may need to use horizontal scrolling for full view) contains
variables which can be passed to CMake for customization.

|CMake variable|Description|Default|
|:-------------|:----------|:------|
|**The type of the build**|||
|CMAKE_BUILD_TYPE|`Debug` \| `Release` \| `RelWithDebInfo` \| `MinSizeRel`|`Release`|
|**Locations**|||
|PG_INCLUDE_DIR_SERVER|Path to header files for the PostgreSQL server|*$(pg_config --includedir-server)*|
|PG_PKGLIB_DIR|Path to dynamically loadable modules of the PostgreSQL server|*$(pg_config --pkglibdir)*|
|PG_SHARE_DIR|Path to architecture-independent support files of the PostgreSQL server|*$(pg_config --sharedir)*|

The following example shows how to specify a custom location of header files
for the PostgreSQL server via CMake variable:

    $ cmake -DPG_INCLUDE_DIR_SERVER=/path/to/headers ..

### Installation on Linux

    $ git clone https://github.com/dmitigr/pgnso.git
    $ mkdir -p pgnso/build
    $ cd pgnso/build
    $ cmake ..
    $ make
    $ sudo make install

### Installation on Microsoft Windows

Run Developer Command Prompt for Visual Studio and type:

    > git clone https://github.com/dmitigr/pgnso.git
    > mkdir pgnso\build
    > cd pgnso\build
    > cmake -G "Visual Studio 15 2017 Win64" ..
    > cmake --build .

Next, run the elevated command prompt and type:

    > cd pgnso\build
    > cmake -P cmake_install.cmake

## License

`dmitigr_nso` is distributed under zlib license. For conditions of distribution
and use, see file `LICENSE.txt`.

## Copyright

Copyright (C) [Dmitry Igrishin][dmitigr_mail]

[dmitigr_mail]: mailto:dmitigr@gmail.com

[datatype-character]: https://www.postgresql.org/docs/current/datatype-character.html
[sql-createextension]: https://www.postgresql.org/docs/current/sql-createextension.html

[CMake]: https://cmake.org/
[GCC]: https://gcc.gnu.org/
[natural_sort_order]: https://en.wikipedia.org/wiki/Natural_sort_order
[PostgreSQL]: https://www.postgresql.org/
[Visual_Studio]: https://www.visualstudio.com/
