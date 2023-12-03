# CC API Reference

## Instantiating a hash table template

Create a new hash table type in the following manner:

```c
#define NAME   [...]
#define KEY_TY [...]
#include "fastmap.h"
```

The `NAME` macro specifies the name of hash table type that the library will declare, the prefix for the functions associated with it, and the prefix for the associated iterator type.

The `KEY_TY` macro specifies the key type.

In C99, it is also always necessary to define `HASH_FN` and `CMPR_FN` (see below) before including the header.

The following macros may also be defined before including the header:

<dl><dd>

```c
VAL_TY [...]
```

The type of the value associated with each key.  
If this macro is defined, the hash table acts as a map associating keys with values.  
Otherwise, it acts as a set containing only keys.

```c
HASH_FN [...]
```

The name of the existing function used to hash each key.  
The function should have the signature `uint64_t ( KEY_TY )` and return a 64-bit hash code.  
For best performance, the hash function should provide a high level of entropy across all bits.  
There are two default hash functions: `fm_hash_integer` for all integer types up to 64 bits in size, and `fm_hash_string` for `NULL`-terminated strings (i.e. `char *`).  
When `KEY_TY` is one of such types and the compiler is in C11 mode or later, `HASH_FN` may be left undefined, in which case the appropriate default function is inferred from `KEY_TY`.  
Otherwise, `HASH_FN` must be defined.

```c
CMPR_FN [...]
```

The name of the existing function used to compare two keys.  
The function should have the signature `bool ( KEY_TY, KEY_TY )` and return `true` if the two keys are equal.
There are two default comparison functions: `fm_cmpr_integer` for all integer types up to 64 bits in size, and `fm_cmpr_string` for `NULL`-terminated strings (`i.e. char *`).  
As with the default hash functions, in C11 or later the appropriate default comparison function is inferred if `KEY_TY` is one of such types and `CMPR_FN` is left undefined.  
Otherwise, `CMPR_FN` must be defined.

```c
MAX_LOAD [...]
```

The floating-point load factor at which the hash table automatically doubles the size of its internal buckets array.  
The default is `0.9`, i.e. 90%.

```c
KEY_DTOR_FN [...]
```

The name of the existing destructor function, with the signature `void ( KEY_TY )`, called on a key when it is erased from the table or replaced by a newly inserted key.  
The API functions that may call the key destructor are `NAME_insert`, `NAME_erase`, `NAME_erase_itr`, `NAME_clear`, and `NAME_cleanup`.

```c
VAL_DTOR_FN [...]
```

The name of the existing destructor function, with the signature `void ( VAL_TY )`, called on a value when it is erased from the table or replaced by a newly inserted value.  
The API functions that may call the value destructor are `NAME_insert`, `NAME_erase`, `NAME_erase_itr`, `NAME_clear`, and `NAME_cleanup`.

```c
MALLOC_FN [...]
```

The name of the existing function, with the signature `void *( size_t )`, used to allocate memory.  
The default is stdlib.h's malloc.

```c
FREE_FN [...]
```

The name of the existing function, with the signature `void ( void * )`, used to free memory.  
The default is stdlib.h's free.

```c
HEADER_MODE
IMPLEMENTATION_MODE
```

By default, all hash table functions are defined as static inline functions, the intent being that a given hash table template should be instantiated once per translation unit; For best performance, this is the recommended way to use the library.  
However, it is also possible separate the struct definitions and function declarations from the function definitions such that one implementation can be shared across all translation units (as in a traditional header and source file pair).  
In that case, instantiate a template wherever it is needed by defining `HEADER_MODE`, along with only `NAME`, `KEY_TY`, and (optionally) `VAL_TY` and header guards, and including the library, e.g.:

```c
#ifndef INT_INT_MAP_H
#define INT_INT_MAP_H
#define NAME   int_int_map
#define KEY_TY int
#define VAL_TY int
#define HEADER_MODE
#include "fastmap.h"
#endif
```

In one source file, define `IMPLEMENTATION_MODE`, along with `NAME`, `KEY_TY`, and any of the aforementioned optional macros, and include the library, e.g.:

```c
#define NAME     int_int_map
#define KEY_TY   int
#define VAL_TY   int
#define HASH_FN  fm_hash_integer // C99.
#define CMPR_FN  fm_cmpr_integer // C99.
#define MAX_LOAD 0.8
#define IMPLEMENTATION_MODE
#include "fastmap.h"
```

</dd></dl>

Including the library automatically undefines all the aforementioned macros after they have been used to instantiate the template.

## Functions

## Iterators

```c
#define CC_NO_SHORT_NAMES
```

By default, **CC** exposes API macros without the `cc_` prefix.  
Define this flag before including the library to withhold the unprefixed names.

```c
#define CC_REALLOC our_realloc
```

Causes API macros to use a custom `realloc` function rather than the one in the standard library.  
This definition affects all calls to API macros where it is visible.

```c
#define CC_FREE our_free
```

Causes API macros to use a custom `free` function rather than the one in the standard library.  
This definition affects all calls to API macros where it is visible.

## All containers

The following macros behave the same way for all containers:

```c
void init( <any container type> *cntr )
```

Initializes `cntr` for use.  
This call cannot fail (it does not allocate memory).

```c
bool init_clone( <any container type> *cntr, <same container type> *src )
```

Initializes `cntr` as a shallow copy of `src`.  
Returns `true`, or `false` if unsuccessful due to memory allocation failure.

```c
size_t size( <any container type> *cntr )
```

Returns the number of elements.

```c
void clear( <any container type> *cntr )
```

Erases all elements, calling the element and key types' destructors if they exist.

```c
void cleanup( <any container type> *cntr )
```

Erases all elements (calling the element and key types' destructors if they exist), frees any other memory associated with the container, and initializes the container for reuse.

```c
for_each( <any container type> *cntr, i_name )
```

Creates a loop iterating over all elements from first to last.  
This macro declares a pointer-iterator (`el_ty *`) named `i_name`.  
It is equivalent to `for( el_ty *i_name = first( cntr ); i_name != end( cntr ); i_name = next( cntr, i_name ) )` and should be followed by the loop body.

## Vector

A `vec` is a dynamic array that stores elements in contiguous memory.

```c
vec( el_ty ) cntr
```

Declares an uninitialized vector named `cntr`.

```c
size_t cap( vec( el_ty ) *cntr )
```

Returns the current capacity.

```c
bool reserve( vec( el_ty ) *cntr, size_t n )
```

Ensures that the the capacity is large enough to support `n` elements.  
Returns `true`, or `false` if unsuccessful due to memory allocation failure.

```c
bool resize( vec( el_ty ) *cntr, size_t n )
```

Sets the number of elements to `n`.  
If `n` is above the current size, the new elements are uninitialized.  
If `n` is below the current size, the element type's destructor (if it exists) is called for each erased element.  
Returns `true`, or `false` if unsuccessful due to memory allocation failure.

```c
bool shrink( vec( el_ty ) *cntr )
```

Shrinks the capacity to the current size.  
Returns `true`, or `false` if unsuccessful due to memory allocation failure.

```c
el_ty *get( vec( el_ty ) *cntr, size_t i )
```

Returns an a pointer-iterator to the element at index `i`.

```c
el_ty *push( vec( el_ty ) *cntr, el_ty el )
```

Inserts `el` at the end of the vector.  
Returns a pointer-iterator to the new element, or `NULL` in the case of memory allocation failure.

```c
el_ty *push_n( vec( el_ty ) *cntr, el_ty *els, size_t n )
```

Inserts `n` elements from array `els` at the end of the vector.  
Returns a pointer-iterator to the first new element, or `NULL` in the case of memory allocation failure.

```c
el_ty *insert( vec( el_ty ) *cntr, size_t i, el_ty el )
```

Inserts `el` at index `i`.  
Returns a pointer-iterator to the new element, or `NULL` in the case of memory allocation failure.

```c
el_ty *insert_n( vec( el_ty ) *cntr, size_t i, el_ty *els, size_t n )
```

Inserts `n` elements from array `els` at index `i`.  
Returns a pointer-iterator to the first new element, or `NULL` in the case of memory allocation failure.

```c
el_ty *erase( vec( el_ty ) *cntr, size_t i )
```

Erases the element at index `i`, calling the element type's destructor if it exists.  
Returns a pointer-iterator to the element after the erased element, or an `end` pointer-iterator if there is no subsequent element.

```c
el_ty *erase_n( vec( el_ty ) *cntr, size_t i, size_t n )
```

Erases `n` elements beginning at index `i`, calling the element type's destructor, if it exists, for each erased element.  
Returns a pointer-iterator to the element after the erased elements, or an `end` pointer-iterator if there is no subsequent element.

```c
el_ty *end( vec( el_ty ) *cntr )
```

Returns an `end` pointer-iterator.  
This call is synonymous with `get( cntr, size( cntr ) )`.

```c
el_ty *first( vec( el_ty ) *cntr )
```

Returns an pointer-iterator to the first element, or an `end` pointer-iterator if the vector is empty.  
This call is synonymous with `get( cntr, 0 )`.

```c
el_ty *last( vec( el_ty ) *cntr )
```

Returns a pointer-iterator to the last element.  
This call is synonymous with `get( cntr, size( cntr ) - 1 )`.  
It assumes that at the vector is not empty.

```c
el_ty *next( vec( el_ty ) *cntr, el_ty *i )
```

Returns a pointer-iterator to the element after the element pointed to by `i`, or an `end` pointer-iterator if `i` points to the last element.

> **Note**
> Vector pointer-iterators (including `end`) are invalidated by any API calls that cause memory reallocation.

## List

A `list` is a doubly linked list with sentinels.

```c
list( el_ty ) cntr
```

Declares an uninitialized list named `cntr`.

```c
el_ty *insert( list( el_ty ) *cntr, el_ty *i, el_ty el )
```

Inserts `el` before pointer-iterator `i`.  
Returns a pointer-iterator to the new element, or `NULL` in the case of memory allocation failure.

```c
el_ty *push( list( el_ty ) *cntr, el_ty el )
```

Inserts `el` at the end of the list.  
Returns a pointer-iterator to the new element, or `NULL` in the case of memory allocation failure.  
This call is synonymous with `insert( cntr, end( cntr ), el )`.

```c
el_ty *erase( list( el_ty ) *cntr, el_ty *i )
```

Erases element pointed to by pointer-iterator `i`, calling the element type's destructor if it exists.  
Returns a pointer-iterator to the element after `i`, or an `end` pointer-iterator if `i` was the last element.

```c
bool splice( list( el_ty ) *cntr, el_ty *i, list( el_ty ) src, el_ty *src_i )
```

Removes element pointed to by pointer-iterator `src_i` from `src` and inserts it before the element pointed to by pointer-iterator `i` in `cntr`.  
Returns `true`, or `false` if unsuccessful.  
This call only allocates memory, and therefore can only fail, if the list has not had any element inserted, pushed, or spliced into it since it was initialized.

```c
el_ty *first( list( el_ty ) *cntr )
```

Returns a pointer-iterator to the first element, or an `end` pointer-iterator if the list is empty.

```c
el_ty *last( list( el_ty ) *cntr )
```

Returns a pointer-iterator to the last element, or an `r_end` pointer-iterator if the list is empty.

```c
el_ty *r_end( list( el_ty ) *cntr )
```

Returns an `r_end` (reverse end) pointer-iterator for the list.  
`r_end` acts as a sentinel node.

```c
el_ty *end( list( el_ty ) *cntr )
```

Returns an `end` pointer-iterator for the list.  
`end` acts as a sentinel node.

```c
el_ty *next( list( el_ty ) *cntr, el_ty *i )
```

Returns a pointer-iterator to the element after the one pointed to by `i`.  
If `i` points to the last element, the return value is an `end` pointer-iterator.  
If `i` points to `r_end`, the return value is a pointer-iterator to the first element, or an `end` pointer-iterator if the list is empty.

```c
el_ty *prev( list( el_ty ) *cntr, el_ty *i )
```

Returns a pointer-iterator to the element before the one pointed to by `i`.  
If `i` points to the first element, the return value is an `r_end` pointer-iterator.  
If `i` points to `end`, then the return value is a pointer-iterator to the last element, or an `r_end` pointer-iterator if the list is empty.

```c
r_for_each( list( el_ty ) *cntr, i_name )
```

Creates a loop iterating over all elements from last to first.  
This macro declares an `el_ty *` pointer-iterator named `i_name`.  
It is equivalent to `for( el_ty *i_name = last( cntr ); i_name != r_end( cntr ); i_name = prev( cntr, i_name ) )` and should be followed by the body of the loop.

> **Note**
> List pointer-iterators (including `r_end` and `end`) are not invalidated by any API calls besides `init` and `cleanup`, unless they point to erased elements.

### Map

A `map` is an unordered container associating elements with keys, implemented as a Robin Hood hash table.

```c
map( key_ty, el_ty ) cntr
```

Declares an uninitialized map named `cntr`.  
`key_ty` must be a type, or alias for a type, for which comparison and hash functions have been defined.  
This requirement is enforced internally such that neglecting it causes a compiler error.  
For types with in-built comparison and hash functions, and for details on how to declare new comparison and hash functions, see _Destructor, comparison, and hash functions and custom max load factors_ below.

```c
size_t cap( map( key_ty, el_ty ) *cntr )
```

Returns the current capacity, i.e. bucket count.  
Note that the number of elements a map can support without rehashing is not its capacity but its capacity multiplied by the max load factor associated with its key type.

```c
bool reserve( map( key_ty, el_ty ) *cntr, size_t n )
```

Ensures that the capacity is large enough to support `n` elements without rehashing.  
Returns `true`, or `false` if unsuccessful due to memory allocation failure.

```c
bool shrink( map( key_ty, el_ty ) *cntr )
```

Shrinks the capacity to best accommodate the current size.  
Returns `true`, or `false` if unsuccessful due to memory allocation failure.

```c
el_ty *insert( map( key_ty, el_ty ) *cntr, key_ty key, el_ty el )
```

Inserts element `el` with the specified key.  
If an element with the same key already exists, the existing element is replaced.  
Returns a pointer-iterator to the new element, or `NULL` in the case of memory allocation failure.  
If adding one element would violate the map's max load factor, failure can occur even if it already contains the key.

```c
el_ty *get( map( key_ty, el_ty ) *cntr, key_ty key )
```

Returns a pointer-iterator to the element with the specified key, or `NULL` if no such element exists.

```c
el_ty *get_or_insert( map( key_ty, el_ty ) *cntr, key_ty key, el_ty el )
```

Inserts element `el` if no element with the specified key already exist.  
Returns a pointer-iterator to the new element if it was inserted, or a pointer-iterator to the existing element with the same key, or `NULL` in the case of memory allocation failure.  
If adding one element would violate the map's max load factor, failure can occur even if it already contains the key.  
Determine whether an element was inserted by comparing the map's size before and after the call.

```c
const key_ty *key_for( map( key_ty, el_ty ) *cntr, el_ty *i )
```

Returns a `const` pointer to the key for the element pointed to by pointer-iterator `i`.

```c
bool erase( map( key_ty, el_ty ) *cntr, key_ty key )
```

Erases the element with the specified key, if it exists.  
Returns `true` if an element was erased, or `false` if no such element exists.

```c
void erase_itr( map( key_ty, el_ty ) *cntr, el_ty *i )
```

Erases the element pointed to by pointer-iterator `i`.

```c
el_ty *first( map( key_ty, el_ty ) *cntr )
```

Returns a pointer-iterator to the first element, or an `end` pointer-iterator if the map is empty.

```c
el_ty *last( map( key_ty, el_ty ) *cntr )
```

Returns a pointer-iterator to the last element, or an `r_end` pointer-iterator if the map is empty.

```c
el_ty *r_end( map( key_ty, el_ty ) *cntr )
```

Returns an `r_end` (reverse end) pointer-iterator for the map.

```c
el_ty *end( map( key_ty, el_ty ) *cntr )
```

Returns an `end` pointer-iterator for the map.

```c
el_ty *next( map( key_ty, el_ty ) *cntr, el_ty *i )
```

Returns a pointer-iterator to the element after the one pointed to by `i`.  
If `i` points to the last element, the value returned is an `end` pointer-iterator.  
If `i` points to `r_end`, the value returned points to the first element, or is an `end` pointer-iterator if the map is empty.

```c
el_ty *prev( map( key_ty, el_ty ) *cntr, el_ty *i )
```

Returns a pointer-iterator to the element before the one pointed to by `i`.  
If `i` points to the first element, the value returned is an `r_end` pointer-iterator.  
If `i` points to `end`, then the value returned points to the last element, or is an `r_end` pointer-iterator if the map is empty.

```c
for_each( map( key_ty, el_ty ) *cntr, key_ptr_name, i_name )
```

Creates a loop iterating over all elements from first to last, with easy access to the corresponding keys.  
This macro declares a pointer to the key (`const key_ty *`) named `key_ptr_name` and a pointer-iterator (`el_ty *`) named `i_name`.  
It should be followed by the body of the loop.

```c
r_for_each( map( key_ty, el_ty ) *cntr, i_name )
```

Creates a loop iterating over all elements from last to first.  
This macro declares an `el_ty *` pointer-iterator named `i_name`.  
It is equivalent to `for( el_ty *i_name = last( cntr ); i_name != r_end( cntr ); i_name = prev( cntr, i_name ) )` and should be followed by the body of the loop.

```c
r_for_each( map( key_ty, el_ty ) *cntr, key_ptr_name, i_name )
```

Creates a loop iterating over all elements from last to first, with easy access to the corresponding keys.  
This macro declares a pointer to the key (`const key_ty *`) named `key_ptr_name` and a pointer-iterator (`el_ty *`) named `i_name`.  
It should be followed by the body of the loop.

> **Note**
> Map pointer-iterators (including `r_end` and `end`) may be invalidated by any API calls that cause memory reallocation.

## Set

A `set` is a Robin Hood hash table for elements without a separate key.

```c
set( el_ty ) cntr
```

Declares an uninitialized set named `cntr`.  
`el_ty` must be a type, or alias for a type, for which comparison and hash functions have been defined.  
This requirement is enforced internally such that neglecting it causes a compiler error.  
For types with in-built comparison and hash functions, and for details on how to declare new comparison and hash functions, see _Destructor, comparison, and hash functions and custom max load factors_ below.

```c
size_t cap( set( el_ty ) *cntr )
```

Returns the current capacity, i.e. bucket count.  
Note that the number of elements a set can support without rehashing is not its capacity but its capacity multiplied by the max load factor associated with its key type.

```c
bool reserve( set( el_ty ) *cntr, size_t n )
```

Ensures that the capacity is large enough to support `n` elements without rehashing.  
Returns `true`, or `false` if unsuccessful due to memory allocation failure.

```c
bool shrink( set( el_ty ) *cntr )
```

Shrinks the capacity to best accommodate the current size.  
Returns `true`, or `false` if unsuccessful due to memory allocation failure.

```c
el_ty *insert( set( el_ty ) *cntr, el_ty el )
```

Inserts element `el`.  
If the element already exists, the existing element is replaced.  
Returns a pointer-iterator to the new element, or `NULL` in the case of memory allocation failure.  
Note that if adding one element would violate the set's max load factor, failure can occur even if it already contains `el`.

```c
el_ty *get( set( el_ty ) *cntr, el_ty el )
```

Returns a pointer-iterator to element `el`, or `NULL` if no such element exists.

```c
el_ty *get_or_insert( set( el_ty ) *cntr, el_ty el )
```

Inserts element `el` if it does not already exist.  
Returns a pointer-iterator to the new element if it was inserted, or a pointer-iterator to the existing element, or `NULL` in the case of memory allocation failure.  
If adding one element would violate the set's max load factor, failure can occur even if it already contains the element.  
Determine whether an element was inserted by comparing the set's size before and after the call.

```c
bool erase( set( el_ty ) *cntr, el_ty el )
```

Erases the element `el`, if it exists.  
Returns `true` if an element was erased, or `false` if no such element exists.

```c
el_ty *first( set( el_ty ) *cntr )
```

Returns a pointer-iterator to the first element, or an `end` pointer-iterator if the set is empty.

```c
el_ty *last( set( el_ty ) *cntr )
```

Returns a pointer-iterator to the last element, or an `r_end` pointer-iterator if the set is empty.

```c
el_ty *r_end( set( el_ty ) *cntr )
```

Returns an `r_end` (reverse end) pointer-iterator for the set.

```c
el_ty *end( set( el_ty ) *cntr )
```

Returns an `end` pointer-iterator for the set.

```c
el_ty *next( set( el_ty ) *cntr, el_ty *i )
```

Returns a pointer-iterator to the element after the one pointed to by `i`.  
If `i` points to the last element, the pointer-iterator returned is an `end` pointer-iterator.  
If `i` points to `r_end`, then the pointer-iterator returned points to the first element, or is an `end` pointer-iterator if the set is empty.

```c
el_ty *prev( set( el_ty ) *cntr, el_ty *i )
```

Returns a pointer-iterator to the element before the one pointed to by `i`.  
If `i` points to the first element, the return value is an `r_end` pointer-iterator.  
If `i` points to `end`, then the pointer-iterator returned points to the last element, or is an `r_end` pointer-iterator if the set is empty.

```c
r_for_each( set( el_ty ) *cntr, i_name )
```

Creates a loop iterating over all elements from last to first.  
This macro declares an `el_ty *` pointer-iterator named `i_name`.  
It is equivalent to `for( el_ty *i_name = last( cntr ); i_name != r_end( cntr ); i_name = prev( cntr, i_name ) )` and should be followed by the body of the loop.

> **Note**
> Set pointer-iterators (including `r_end` and `end`) may be invalidated by any API calls that cause memory reallocation.

## Destructor, comparison, and hash functions and custom max load factors

This part of the API allows the user to define custom destructor, comparison, and hash functions and max load factors for a type.  
Once these functions are defined, any container using that type for its elements or keys will call them automatically.  
Once the max load factor is defined, any map using the type for its key and any set using the type for its elements will use the defined load factor to determine when rehashing is necessary.

```c
#define CC_DTOR ty, { function body }
#include "cc.h"
```

Defines a destructor for type `ty`.  
The signature of the function is `void ( ty val )`.

```c
#define CC_CMPR ty, { function body }
#include "cc.h"
```

Defines a comparison function for type `ty`.  
The signature of the function is `int ( ty val_1, ty val_2 )`.  
The function should return `0` if `val_1` and `val_2` are equal, a negative integer if `val_1` is less than `val_2`, and a positive integer if `val_1` is greater than `val_2`.

```c
#define CC_HASH ty, { function body }
#include "cc.h"
```

Defines a hash function for type `ty`.  
The signature of the function is `size_t ( ty val )`.  
The function should return the hash of `val`.

```c
#define CC_LOAD ty, max_load_factor
```

Defines the max load factor for type `ty`.  
`max_load_factor` should be a `float` or `double` between `0.0` and `1.0`.  
The default max load factor is `0.8`.

Trivial example:

```c
typedef struct { int x; } our_type;
#define CC_DTOR our_type, { printf( "!%d\n", val.x ); }
#define CC_CMPR our_type, { return ( val_1.x > val_2.x ) - ( val_1.x < val_2.x ); }
#define CC_HASH our_type, { return val.x * 2654435761ull; }
#define CC_LOAD our_type, 0.5
#include "cc.h"
```

> **Note**
> * These functions are inline and have static scope, so you need to either redefine them in each translation unit from which they should be called or (preferably) define them in a shared header. For structs or unions, a sensible place to define them would be immediately after the definition of the struct or union.
> * Only one destructor, comparison, or hash function or max load factor should be defined by the user for each type.
> * Including `cc.h` in these cases does not include the full header, so you still need to include it separately at the top of your files.
> * In-built comparison and hash functions are already defined for the following types: `char`, `unsigned char`, `signed char`, `unsigned short`, `short`, `unsigned int`, `int`, `unsigned long`, `long`, `unsigned long long`, `long long`, `size_t`, and `char *` (a `NULL`-terminated string). Defining a comparsion or hash function for one of these types will overwrite the in-built function.
