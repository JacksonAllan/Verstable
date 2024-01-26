# Verstable API Reference

## Instantiating a hash table template

Create a new hash table type in the following manner:

```c
#define NAME   <your chosen type name>
#define KEY_TY <type>
#include "verstable.h"
```

The `NAME` macro specifies the name of hash table type that the library will declare, the prefix for the functions associated with it, and the prefix for the associated iterator type.

The `KEY_TY` macro specifies the key type.

In C99, it is also always necessary to define `HASH_FN` and `CMPR_FN` (see below) before including the header.

The following macros may also be defined before including the header:

<dl><dd>

```c
#define VAL_TY <type>
```

The type of the value associated with each key.  
If this macro is defined, the hash table acts as a map associating keys with values.  
Otherwise, it acts as a set containing only keys.

```c
#define HASH_FN <function name>
```

The name of the existing function used to hash each key.  
The function should have the signature `uint64_t ( KEY_TY key )` and return a 64-bit hash code.  
For best performance, the hash function should provide a high level of entropy across all bits.  
There are two default hash functions: `vt_hash_integer` for all integer types up to 64 bits in size, and `vt_hash_string` for `NULL`-terminated strings (i.e. `char *`).  
When `KEY_TY` is one of such types and the compiler is in C11 mode or later, `HASH_FN` may be left undefined, in which case the appropriate default function is inferred from `KEY_TY`.  
Otherwise, `HASH_FN` must be defined.

```c
#define CMPR_FN <function name>
```

The name of the existing function used to compare two keys.  
The function should have the signature `bool ( KEY_TY key_1, KEY_TY key_2 )` and return `true` if the two keys are equal.  
There are two default comparison functions: `vt_cmpr_integer` for all integer types up to 64 bits in size, and `vt_cmpr_string` for `NULL`-terminated strings (i.e. `char *`).  
As with the default hash functions, in C11 or later the appropriate default comparison function is inferred if `KEY_TY` is one of such types and `CMPR_FN` is left undefined.  
Otherwise, `CMPR_FN` must be defined.

```c
#define MAX_LOAD <floating point value>
```

The floating-point load factor at which the hash table automatically doubles the size of its internal buckets array.  
The default is `0.9`, i.e. 90%.

```c
#define KEY_DTOR_FN <function name>
```

The name of the existing destructor function, with the signature `void ( KEY_TY key )`, called on a key when it is erased from the table or replaced by a newly inserted key.  
The API functions that may call the key destructor are `NAME_insert`, `NAME_erase`, `NAME_erase_itr`, `NAME_clear`, and `NAME_cleanup`.

```c
#define VAL_DTOR_FN <function name>
```

The name of the existing destructor function, with the signature `void ( VAL_TY val )`, called on a value when it is erased from the table or replaced by a newly inserted value.  
The API functions that may call the value destructor are `NAME_insert`, `NAME_erase`, `NAME_erase_itr`, `NAME_clear`, and `NAME_cleanup`.

```c
#define CTX_TY <type>
```

The type of the hash table type's `ctx` (context) member.  
This member only exists if `CTX_TY` was defined.  
It is intended to be used in conjunction with `MALLOC_FN` and `FREE_FN` (see below).

```c
#define MALLOC_FN <function name>
```

The name of the existing function used to allocate memory.  
If `CTX_TY` was defined, the signature should be `void *( size_t size, CTX_TY *ctx )`, where size is the number of bytes to allocate and ctx points to the table's ctx member.  
Otherwise, the signature should be `void *( size_t size )`.  
The default wraps `stdlib.h`'s malloc.

```c
#define FREE_FN <function name>
```

The name of the existing function used to free memory.  
If `CTX_TY` was defined, the signature should be `void ( void *ptr, size_t size, CTX_TY *ctx )`, where ptr points to the memory to free, size is the number of bytes that were allocated, and ctx points to the table's ctx member.  
Otherwise, the signature should be `void ( void *ptr, size_t size )`.  
The default wraps `stdlib.h`'s free.

```c
#define HEADER_MODE
#define IMPLEMENTATION_MODE
```

By default, all hash table functions are defined as `static inline` functions, the intent being that a given hash table template should be instantiated once per translation unit; for best performance, this is the recommended way to use the library.  
However, it is also possible separate the struct definitions and function declarations from the function definitions such that one implementation can be shared across all translation units (as in a traditional header and source file pair).  
In that case, instantiate a template wherever it is needed by defining `HEADER_MODE`, along with only `NAME`, `KEY_TY`, and (optionally) `VAL_TY`, `CTX_TY`, and header guards, and including the library, e.g.:

```c
#ifndef INT_INT_MAP_H
#define INT_INT_MAP_H
#define NAME   int_int_map
#define KEY_TY int
#define VAL_TY int
#define HEADER_MODE
#include "verstable.h"
#endif
```

In one source file, define `IMPLEMENTATION_MODE`, along with `NAME`, `KEY_TY`, and any of the aforementioned optional macros, and include the library, e.g.:

```c
#define NAME     int_int_map
#define KEY_TY   int
#define VAL_TY   int
#define HASH_FN  vt_hash_integer // C99.
#define CMPR_FN  vt_cmpr_integer // C99.
#define MAX_LOAD 0.8
#define IMPLEMENTATION_MODE
#include "verstable.h"
```

</dd></dl>

Including the library automatically undefines all the aforementioned macros after they have been used to instantiate the template.

## Functions

The functions associated with a hash table type are all prefixed with the name the user supplied via the `NAME` macro.  
In C11 and later, the generic `vt_`-prefixed macros may be used to automatically select the correct version of the specified function based on the arguments.

```c
void NAME_init( NAME *table )
void NAME_init( NAME *table, CTX_TY ctx )
// C11 generic macro: vt_init.
```

Initializes the table for use.  
If `CTX_TY` was defined, `ctx` sets the table's `ctx` member.

```c
bool NAME_init_clone( NAME *table, NAME *source )
bool NAME_init_clone( NAME *table, NAME *source, CTX_TY ctx )
// C11 generic macro: vt_init_clone.
```

Initializes the table as a shallow copy of the specified source table.  
If `CTX_TY` was defined, `ctx` sets the table's `ctx` member.  
Returns `false` in the case of memory allocation failure.

```c
size_t NAME_size( NAME *table ) // C11 generic macro: vt_size.
```

Returns the number of keys currently in the table.

```c
size_t NAME_bucket_count( NAME *table ) // C11 generic macro: vt_bucket_count.
```

Returns the table's current bucket count.

```c
NAME_itr NAME_insert( NAME *table, KEY_TY key )
NAME_itr NAME_insert( NAME *table, KEY_TY key, VAL_TY val )
// C11 generic macro: vt_insert.
```

Inserts the specified key (and value, if `VAL_TY` was defined) into the hash table.  
If the same key already exists, then the new key (and value) replaces the existing key (and value).  
Returns an iterator to the new key, or an end iterator in the case of memory allocation failure.

```c
NAME_itr NAME_get_or_insert( NAME *table, KEY_TY key )
NAME_itr NAME_get_or_insert( NAME *table, KEY_TY key, VAL_TY val )
// C11 generic macro: vt_get_or_insert.
```

Inserts the specified key (and value, if `VAL_TY` was defined) if it does not already exist in the table.  
Returns an iterator to the new key if it was inserted, or an iterator to the existing key, or an end iterator if the key did not exist but the new key could not be inserted because of memory allocation failure.  
Determine whether the key was inserted by comparing the table's size before and after the call.

```c
NAME_itr NAME_get( NAME *table, KEY_TY key ) // C11 generic macro: vt_get.
```

Returns a iterator to the specified key, or an end iterator if no such key exists.

```c
bool NAME_erase( NAME *table, KEY_TY key ) // C11 generic macro: vt_erase.
```

Erases the specified key (and associated value, if `VAL_TY` was defined), if it exists.  
Returns `true` if a key was erased.

```c
NAME_itr NAME_erase_itr( NAME *table, NAME_itr itr ) // C11 generic macro: vt_erase_itr.
```

Erases the key (and associated value, if `VAL_TY` was defined) pointed to by the specified iterator.  
Returns an iterator to the next key in the table, or an end iterator if the erased key was the last one.

```c
bool NAME_reserve( NAME *table, size_t size ) // C11 generic macro: vt_reserve.
```

Ensures that the bucket count is large enough to support the specified key count (i.e. size) without rehashing.  
Returns `false` if unsuccessful due to memory allocation failure.

```c
bool NAME_shrink( NAME *table ) // C11 generic macro: vt_shrink.
```

Shrinks the bucket count to best accommodate the current size.  
Returns `false` if unsuccessful due to memory allocation failure.

```c
NAME_itr NAME_first( NAME *table ) // C11 generic macro: vt_first.
```

Returns an iterator to the first key in the table, or an end iterator if the table is empty.

```c
bool NAME_is_end( NAME *table, NAME_itr itr ) // C11 generic macro: vt_is_end.
```

Returns `true` if the iterator is an end iterator.

```c
NAME_itr NAME_next( NAME_itr itr ) // C11 generic macro: vt_next.
```

Returns an iterator to the key after the one pointed to by the specified iterator, or an end iterator if the specified iterator points to the last key in the table.

```c
void NAME_clear( NAME *table ) // C11 generic macro: vt_clear.
```

Erases all keys (and values, if `VAL_TY` was defined) in the table.

```c
void NAME_cleanup( NAME *table ) // C11 generic macro: vt_cleanup.
```

Erases all keys (and values, if `VAL_TY` was defined) in the table, frees all memory associated with it, and initializes it for reuse.

## Iterators

Access the key (and value, if `VAL_TY` was defined) that an iterator points to using the `NAME_itr` struct's `data` member:

```c
itr.data->key
itr.data->val
```

Functions that may insert new keys (`NAME_insert` and `NAME_get_or_insert`), erase keys (`NAME_erase` and `NAME_erase_itr`), or reallocate the internal bucket array (`NAME_reserve` and `NAME_shrink`) invalidate all exiting iterators.  
To delete keys during iteration and resume iterating, use the return value of `NAME_erase_itr`.
