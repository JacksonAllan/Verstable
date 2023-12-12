/*------------------------------------------------- VERSTABLE v1.0.0 ---------------------------------------------------

Verstable is a C99-compatible, open-addressing hash table using quadratic probing and the following additions:

- All keys that hash (i.e. "belong") to the same bucket (their "home bucket") are linked together by an 11-bit integer
  specifying the quadratic displacement, relative to that bucket, of the next key in the chain.

- If a chain of keys exists for a given bucket, then it always begins at that bucket. To maintain this policy, a 1-bit
  flag is used to mark whether the key occupying a bucket belongs there. When inserting a new key, if the bucket it
  belongs to is occupied by a key that does not belong there, then the occupying key is evicted and the new key takes
  the bucket.

- A 4-bit fragment of each key's hash code is also stored.

- The aforementioned metadata associated with each bucket (the 4-bit hash fragment, the 1-bit flag, and the 11-bit link
  to the next key in the chain) are stored together in a uint16_t array rather than in the bucket alongside the key and
  (optionally) the value.

One way to conceptualize this scheme is as a chained hash table in which overflowing keys are stored not in separate
memory allocations but in otherwise unused buckets. In this regard, it shares similarities with Malte Skarupke’s Bytell
hash table (https://www.youtube.com/watch?v=M2fKMP47slQ) and traditional "coalesced hashing".

Advantages of this scheme include:

- Fast lookups impervious to load factor: If the table contains any key belonging to the lookup key's home bucket, then
  that bucket contains the first in a traversable chain of all keys belonging to it. Hence, only the home bucket and
  other buckets containing keys belonging to it are ever probed. Moreover, the stored hash fragments allow skipping most
  non-matching keys in the chain without accessing the actual buckets array or calling the (potentially expensive) key
  comparison function.

- Fast insertions: Insertions are faster than they are in other schemes that move keys around (e.g. Robin Hood) because
  they only move, at most, one existing key.

- Fast, tombstone-free deletions: Deletions, which usually require tombstones in quadratic-probing hash tables, are
  tombstone-free and only move, at most, one existing key.

- Fast iteration: The separate metadata array allows keys in sparsely populated tables to be found without incurring the
  frequent cache misses that would result from traversing the buckets array.

Usage example:

  +---------------------------------------------------------+----------------------------------------------------------+
  | Using the generic macro API (C11 and later):            | Using the prefixed functions API (C99 and later):        |
  |---------------------------------------------------------+----------------------------------------------------------+
  | #include <stdio.h>                                      | #include <stdio.h>                                       |
  |                                                         |                                                          |
  | // Instantiating a set template.                        | // Instantiating a set template.                         |
  | #define NAME int_set                                    | #define NAME int_set                                     |
  | #define KEY_TY int                                      | #define KEY_TY int                                       |
  | #include "verstable.h"                                  | #define HASH_FN vt_hash_integer                          |
  |                                                         | #define CMPR_FN vt_cmpr_integer                          |
  | // Instantiating a map template.                        | #include "verstable.h"                                   |
  | #define NAME int_int_map                                |                                                          |
  | #define KEY_TY int                                      | // Instantiating a map template.                         |
  | #define VAL_TY int                                      | #define NAME int_int_map                                 |
  | #include "verstable.h"                                  | #define KEY_TY int                                       |
  |                                                         | #define VAL_TY int                                       |
  | int main( void )                                        | #define HASH_FN vt_hash_integer                          |
  | {                                                       | #define CMPR_FN vt_cmpr_integer                          |
  |   // Set.                                               | #include "verstable.h"                                   |
  |                                                         |                                                          |
  |   int_set our_set;                                      | int main( void )                                         |
  |   vt_init( &our_set );                                  | {                                                        |
  |                                                         |   // Set.                                                |
  |   // Inserting keys.                                    |                                                          |
  |   for( int i = 0; i < 10; ++i )                         |   int_set our_set;                                       |
  |   {                                                     |   int_set_init( &our_set );                              |
  |     int_set_itr itr = vt_insert( &our_set, i );         |                                                          |
  |     if( vt_is_end( itr ) )                              |   // Inserting keys.                                     |
  |       exit( 1 ); // Out of memory.                      |   for( int i = 0; i < 10; ++i )                          |
  |   }                                                     |   {                                                      |
  |                                                         |     int_set_itr itr =                                    |
  |   // Erasing keys.                                      |       int_set_insert( &our_set, i );                     |
  |   for( int i = 0; i < 10; i += 3 )                      |     if( int_set_is_end( itr ) )                          |
  |   vt_erase( &our_set, i );                              |       exit( 1 ); // Out of memory.                       |
  |                                                         |   }                                                      |
  |   // Retrieving keys.                                   |                                                          |
  |   for( int i = 0; i < 10; ++i )                         |   // Erasing keys.                                       |
  |   {                                                     |   for( int i = 0; i < 10; i += 3 )                       |
  |     int_set_itr itr = vt_get( &our_set, i );            |     int_set_erase( &our_set, i );                        |
  |     if( !vt_is_end( itr ) )                             |                                                          |
  |       printf( "%d ", itr.data->key );                   |   // Retrieving keys.                                    |
  |   }                                                     |   for( int i = 0; i < 10; ++i )                          |
  |   // Printed: 1 2 4 5 7 8                               |   {                                                      |
  |                                                         |     int_set_itr itr = int_set_get( &our_set, i );        |
  |   // Iteration.                                         |     if( !int_set_is_end( itr ) )                         |
  |   for(                                                  |       printf( "%d ", itr.data->key );                    |
  |     int_set_itr itr = vt_first( &our_set );             |   }                                                      |
  |     !vt_is_end( itr );                                  |   // Printed: 1 2 4 5 7 8                                |
  |     itr = vt_next( itr )                                |                                                          |
  |   )                                                     |   // Iteration.                                          |
  |     printf( "%d ", itr.data->key );                     |   for(                                                   |
  |   // Printed: 4 5 2 8 1 7                               |     int_set_itr itr =                                    |
  |                                                         |       int_set_first( &our_set );                         |
  |   vt_cleanup( &our_set );                               |     !int_set_is_end( itr );                              |
  |                                                         |     itr = int_set_next( itr )                            |
  |   // Map.                                               |   )                                                      |
  |                                                         |     printf( "%d ", itr.data->key );                      |
  |   int_int_map our_map;                                  |   // Printed: 4 5 2 8 1 7                                |
  |   vt_init( &our_map );                                  |                                                          |
  |                                                         |   int_set_cleanup( &our_set );                           |
  |   // Inserting keys and values.                         |                                                          |
  |   for( int i = 0; i < 10; ++i )                         |   // Map.                                                |
  |   {                                                     |                                                          |
  |     int_int_map_itr itr =                               |   int_int_map our_map;                                   |
  |       vt_insert( &our_map, i, i + 1 );                  |   int_int_map_init( &our_map );                          |
  |     if( vt_is_end( itr ) )                              |                                                          |
  |       exit( 1 ); // Out of memory.                      |   // Inserting keys and values.                          |
  |   }                                                     |   for( int i = 0; i < 10; ++i )                          |
  |                                                         |   {                                                      |
  |   // Erasing keys and values.                           |     int_int_map_itr itr =                                |
  |   for( int i = 0; i < 10; i += 3 )                      |       int_int_map_insert( &our_map, i, i + 1 );          |
  |     vt_erase( &our_map, i );                            |     if( int_int_map_is_end( itr ) )                      |
  |                                                         |       exit( 1 ); // Out of memory.                       |
  |   // Retrieving keys and values.                        |   }                                                      |
  |   for( int i = 0; i < 10; ++i )                         |                                                          |
  |   {                                                     |   // Erasing keys and values.                            |
  |     int_int_map_itr itr = vt_get( &our_map, i );        |   for( int i = 0; i < 10; i += 3 )                       |
  |     if( !vt_is_end( itr ) )                             |     int_int_map_erase( &our_map, i );                    |
  |       printf(                                           |                                                          |
  |         "%d:%d ",                                       |   // Retrieving keys and values.                         |
  |         itr.data->key,                                  |   for( int i = 0; i < 10; ++i )                          |
  |         itr.data->val                                   |   {                                                      |
  |       );                                                |     int_int_map_itr itr =                                |
  |   }                                                     |       int_int_map_get( &our_map, i );                    |
  |   // Printed: 1:2 2:3 4:5 5:6 7:8 8:9                   |     if( !int_int_map_is_end( itr ) )                     |
  |                                                         |       printf(                                            |
  |   // Iteration.                                         |         "%d:%d ",                                        |
  |   for(                                                  |         itr.data->key,                                   |
  |     int_int_map_itr itr = vt_first( &our_map );         |         itr.data->val                                    |
  |     !vt_is_end( itr );                                  |     );                                                   |
  |     itr = vt_next( itr )                                |   }                                                      |
  |   )                                                     |   // Printed: 1:2 2:3 4:5 5:6 7:8 8:9                    |
  |     printf(                                             |                                                          |
  |       "%d:%d ",                                         |   // Iteration.                                          |
  |       itr.data->key,                                    |   for(                                                   |
  |       itr.data->val                                     |     int_int_map_itr itr =                                |
  |     );                                                  |       int_int_map_first( &our_map );                     |
  |   // Printed: 4:5 5:6 2:3 8:9 1:2 7:8                   |     !int_int_map_is_end( itr );                          |
  |                                                         |     itr = int_int_map_next( itr )                        |
  |   vt_cleanup( &our_map );                               |   )                                                      |
  | }                                                       |     printf(                                              |
  |                                                         |       "%d:%d ",                                          |
  |                                                         |       itr.data->key,                                     |
  |                                                         |       itr.data->val                                      |
  |                                                         |     );                                                   |
  |                                                         |   // Printed: 4:5 5:6 2:3 8:9 1:2 7:8                    |
  |                                                         |                                                          |
  |                                                         |   int_int_map_cleanup( &our_map );                       |
  |                                                         | }                                                        |
  |                                                         |                                                          |
  +---------------------------------------------------------+----------------------------------------------------------+

API:

  Instantiating a hash table template:

    Create a new hash table type in the following manner:

      #define NAME   <your chosen type name>
      #define KEY_TY <type>
      #include "verstable.h"

    The NAME macro specifies the name of hash table type that the library will declare, the prefix for the functions
    associated with it, and the prefix for the associated iterator type.

    The KEY_TY macro specifies the key type.

    In C99, it is also always necessary to define HASH_FN and CMPR_FN (see below) before including the header.

    The following macros may also be defined before including the header:

      #define VAL_TY <type>

        The type of the value associated with each key.
        If this macro is defined, the hash table acts as a map associating keys with values.
        Otherwise, it acts as a set containing only keys.

      #define HASH_FN <function name>

        The name of the existing function used to hash each key.
        The function should have the signature uint64_t ( KEY_TY ) and return a 64-bit hash code.
        For best performance, the hash function should provide a high level of entropy across all bits.
        There are two default hash functions: vt_hash_integer for all integer types up to 64 bits in size, and
        vt_hash_string for NULL-terminated strings (i.e. char *).
        When KEY_TY is one of such types and the compiler is in C11 mode or later, HASH_FN may be left undefined, in
        which case the appropriate default function is inferred from KEY_TY.
        Otherwise, HASH_FN must be defined.

      #define CMPR_FN <function name>

        The name of the existing function used to compare two keys.
        The function should have the signature bool ( KEY_TY, KEY_TY ) and return true if the two keys are equal.
        There are two default comparison functions: vt_cmpr_integer for all integer types up to 64 bits in size, and
        vt_cmpr_string for NULL-terminated strings (i.e. char *).
        As with the default hash functions, in C11 or later the appropriate default comparison function is inferred if
        KEY_TY is one of such types and CMPR_FN is left undefined.
        Otherwise, CMPR_FN must be defined.

      #define MAX_LOAD <floating point value>

        The floating-point load factor at which the hash table automatically doubles the size of its internal buckets
        array.
        The default is 0.9, i.e. 90%.

      #define KEY_DTOR_FN <function name>

        The name of the existing destructor function, with the signature void ( KEY_TY ), called on a key when it is
        erased from the table or replaced by a newly inserted key.
        The API functions that may call the key destructor are NAME_insert, NAME_erase, NAME_erase_itr, NAME_clear,
        and NAME_cleanup.

      #define VAL_DTOR_FN <function name>

        The name of the existing destructor function, with the signature void ( VAL_TY ), called on a value when it is
        erased from the table or replaced by a newly inserted value.
        The API functions that may call the value destructor are NAME_insert, NAME_erase, NAME_erase_itr, NAME_clear,
        and NAME_cleanup.

      #define MALLOC_FN <function name>

        The name of the existing function, with the signature void *( size_t ), used to allocate memory.
        The default is stdlib.h's malloc.

      #define FREE_FN <function name>

        The name of the existing function, with the signature void ( void * ), used to free memory.
        The default is stdlib.h's free.

      #define HEADER_MODE
      #define IMPLEMENTATION_MODE

        By default, all hash table functions are defined as static inline functions, the intent being that a given hash
        table template should be instantiated once per translation unit; for best performance, this is the recommended
        way to use the library.
        However, it is also possible separate the struct definitions and function declarations from the function
        definitions such that one implementation can be shared across all translation units (as in a traditional header
        and source file pair).
        In that case, instantiate a template wherever it is needed by defining HEADER_MODE, along with only NAME,
        KEY_TY, and (optionally) VAL_TY and header guards, and including the library, e.g.:

          #ifndef INT_INT_MAP_H
          #define INT_INT_MAP_H
          #define NAME   int_int_map
          #define KEY_TY int
          #define VAL_TY int
          #define HEADER_MODE
          #include "verstable.h"
          #endif

        In one source file, define IMPLEMENTATION_MODE, along with NAME, KEY_TY, and any of the aforementioned optional
        macros, and include the library, e.g.:

          #define NAME     int_int_map
          #define KEY_TY   int
          #define VAL_TY   int
          #define HASH_FN  vt_hash_integer // C99.
          #define CMPR_FN  vt_cmpr_integer // C99.
          #define MAX_LOAD 0.8
          #define IMPLEMENTATION_MODE
          #include "verstable.h"

    Including the library automatically undefines all the aforementioned macros after they have been used to instantiate
    the template.

  Functions:

    The functions associated with a hash table type are all prefixed with the name the user supplied via the NAME macro.
    In C11 and later, the generic "vt_"-prefixed macros may be used to automatically select the correct version of the
    specified function based on the arguments.

    void NAME_init( NAME *table ) // C11 generic macro: vt_init.

      Initializes the table for use.

    bool NAME_init_clone( NAME *table, NAME *source ) // C11 generic macro: vt_init_clone.

      Initializes the table as a shallow copy of the specified source table.
      Returns false in the case of memory allocation failure.

    size_t NAME_size( NAME *table ) // C11 generic macro: vt_size.

      Returns the number of keys currently in the table.

    size_t NAME_bucket_count( NAME *table ) // C11 generic macro: vt_bucket_count.

      Returns the table's current bucket count.

    NAME_itr NAME_insert( NAME *table, KEY_TY key )
    NAME_itr NAME_insert( NAME *table, KEY_TY key, VAL_TY val )
    // C11 generic macro: vt_insert.

      Inserts the specified key (and value, if VAL_TY was defined) into the hash table.
      If the same key already exists, then the new key (and value) replaces the existing key (and value).
      Returns an iterator to the new key, or an end iterator in the case of memory allocation failure.

    NAME_itr NAME_get_or_insert( NAME *table, KEY_TY key )
    NAME_itr NAME_get_or_insert( NAME *table, KEY_TY key, VAL_TY val )
    // C11 generic macro: vt_get_or_insert.

      Inserts the specified key (and value, if VAL_TY was defined) if it does not already exist in the table.
      Returns an iterator to new key if it was inserted, or an iterator to the existing key, or an end iterator if
      the key did not exist but the new key could not be inserted because of memory allocation failure.
      Determine whether the key was inserted by comparing the table's size before and after the call.

    NAME_itr NAME_get( NAME *table, KEY_TY key ) // C11 generic macro: vt_get.

      Returns a iterator to the specified key, or an end iterator if no such key exists.

    bool NAME_erase( NAME *table, KEY_TY key ) // C11 generic macro: vt_erase.

      Erases the specified key (and associated value, if VAL_TY was defined), if it exists.
      Returns true if a key was erased.

    NAME_itr NAME_erase_itr( NAME *table, NAME_itr itr ) // C11 generic macro: vt_erase_itr.

      Erases the key (and associated value, if VAL_TY was defined) pointed to by the specified iterator.
      Returns an iterator to the next key in the table, or an end iterator if the erased key was the last one.

    bool NAME_reserve( NAME *table, size_t size ) // C11 generic macro: vt_reserve.

      Ensures that the bucket count is large enough to support the specified key count (i.e. size) without rehashing.
      Returns false if unsuccessful due to memory allocation failure.

    bool NAME_shrink( NAME *table ) // C11 generic macro: vt_shrink.

      Shrinks the bucket count to best accommodate the current size.
      Returns false if unsuccessful due to memory allocation failure.

    NAME_itr NAME_first( NAME *table ) // C11 generic macro: vt_first.

      Returns an iterator to the first key in the table, or an end iterator if the table is empty.

    bool NAME_is_end( NAME *table, NAME_itr itr ) // C11 generic macro: vt_is_end.

      Returns true if the iterator is an end iterator.

    NAME_itr NAME_next( NAME_itr itr ) // C11 generic macro: vt_next.

      Returns an iterator to the key after the one pointed to by the specified iterator, or an end iterator if the
      specified iterator points to the last key in the table.

    void NAME_clear( NAME *table ) // C11 generic macro: vt_clear.

      Erases all keys (and values, if VAL_TY was defined) in the table.

    void NAME_cleanup( NAME *table ) // C11 generic macro: vt_cleanup.

      Erases all keys (and values, if VAL_TY was defined) in the table, frees all memory associated with it, and
      initializes it for reuse.

  Iterators:

    Access the key (and value, if VAL_TY was defined) that an iterator points to using the NAME_itr struct's data field:

      itr.data->key
      itr.data->val

    Functions that may insert new keys (NAME_insert and NAME_get_or_insert), erase keys (NAME_erase and NAME_erase_itr),
    or reallocate the internal bucket array (NAME_reserve and NAME_shrink) invalidate all exiting iterators.
    To delete keys during iteration and resume iterating, use the return value of NAME_erase_itr.

Version history:

  12/12/2023 1.0.0: Initial release.

License (MIT):

  Copyright (c) 2023 Jackson L. Allan

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

/*--------------------------------------------------------------------------------------------------------------------*/
/*                                               Common header section                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef VERSTABLE_H
#define VERSTABLE_H

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Two-way concatenation macro.
#define VT_CAT_( a, b ) a##b
#define VT_CAT( a, b ) VT_CAT_( a, b )

// Masks for manipulating and extracting data from a bucket's uint16_t metadatum.
#define VT_EMPTY               0x0000
#define VT_HASH_FRAG_MASK      0xF000 // 0b1111000000000000.
#define VT_IN_HOME_BUCKET_MASK 0x0800 // 0b0000100000000000.
#define VT_DISPLACEMENT_MASK   0x07FF // 0b0000011111111111, also denotes the displacement limit. Set to VT_LOAD to 1.0
                                      // to test proper handling of encroachment on the displacement limit during
                                      // inserts.

// Extracts a hash fragment from a uint64_t hash code.
// We take the highest four bits so that keys that map (via modulo) to the same bucket have distinct hash fragments.
#define VT_HASH_FRAG( hash ) ( ( (hash) >> 48 ) & VT_HASH_FRAG_MASK )

// Standard quadratic probing formula that guarantees that all buckets are visited when the bucket count is a power of
// two (at least in theory, because the displacement limit could terminate the search early when the bucket count is
// high).
#define VT_QUADRATIC( displacement ) ( ( (displacement) * (displacement) + (displacement) ) / 2 )

#define VT_MIN_NONZERO_BUCKET_COUNT  8 // Must be a power of two.

// Function to find the left-most non-zero uint16_t in a uint64_t.
// This function is used when we scan four buckets at a time while iterating and relies on compiler intrinsics wherever
// possible.

#if defined( __GNUC__ ) && ULLONG_MAX == 0xFFFFFFFFFFFFFFFF

static inline int vt_first_nonzero_uint16( uint64_t val )
{
  const uint16_t endian_checker = 0x0001;
  if( *(char *)&endian_checker ) // Little-endian (the compiler will optimize away the check at -O1 and above).
    return __builtin_ctzll( val ) / 16;
  
  return __builtin_clzll( val ) / 16;
}

#elif defined( _MSC_VER ) && ( defined( _M_X64 ) || defined( _M_ARM64 ) )

#include <intrin.h>
#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_BitScanReverse64)

static inline int vt_first_nonzero_uint16( uint64_t val )
{
  unsigned long result;

  const uint16_t endian_checker = 0x0001;
  if( *(char *)&endian_checker )
    _BitScanForward64( &result, val );
  else
    _BitScanReverse64( &result, val );

  return result / 16;
}

#else

static inline int vt_first_nonzero_uint16( uint64_t val )
{
  int result = 0;

  uint32_t half;
  memcpy( &half, &val, sizeof( uint32_t ) );
  if( !half )
    result += 2;
  
  uint32_t quarter;
  memcpy( &quarter, (char *)&val + result * 2, sizeof( uint16_t ) );
  if( !quarter )
    result += 1;
  
  return result;
}

#endif

// Setting the metadata pointer to point to a placeholder buffer, rather than NULL, when the bucket count is zero allows
// us to avoid NULL-pointer checks while iterating.
// The placeholder array must also have excess elements to ensure that our iteration functions, which read four buckets
// at a time, never read beyond the end of it.
static const uint16_t vt_placeholder_metadata_buffer[ 4 ] = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };

// Default hash and comparison functions:

static inline uint64_t vt_hash_integer( uint64_t key )
{
  key ^= key >> 33;
  key *= 0xff51afd7ed558ccdull;
  key ^= key >> 33;
  key *= 0xc4ceb9fe1a85ec53ull;
  key ^= key >> 33;
  return key;
}

// FNV-1a.
static inline uint64_t vt_hash_string( char *key )
{
  uint64_t hash = 0xcbf29ce484222325ull;
  while( *key )
    hash = ( (unsigned char)*key++ ^ hash ) * 0x100000001b3ull;

  return hash;
}

static inline bool vt_cmpr_integer( uint64_t key_1, uint64_t key_2 )
{
  return key_1 == key_2;
}

static inline bool vt_cmpr_string( char *key_1, char *key_2 )
{
  return strcmp( key_1, key_2 ) == 0;
}

// The rest of the common header section pertains to the C11 generic macro API.
// This interface is based on the extendible-_Generic mechanism documented in detail at
// https://github.com/JacksonAllan/CC/blob/main/articles/Better_C_Generics_Part_1_The_Extendible_Generic.md.
// In summary, instantiating a template also defines wrappers for the template's types and functions with names in the
// pattern of vt_table_NNN and vt_init_NNN, where NNN is an automatically generated integer unique to the template
// instance in the current translation unit.
// These wrappers plug in to _Generic-based API macros, which use preprocessor magic to automatically generate _Generic
// slots for every existing template instance.
#if __STDC_VERSION__ >= 201112L && !defined( VT_NO_C11_GENERIC_API )

// Octal counter that supports up to 511 hash table templates.
#define VT_TEMPLATE_COUNT_D1 0 // Digit 1, i.e. least significant digit.
#define VT_TEMPLATE_COUNT_D2 0
#define VT_TEMPLATE_COUNT_D3 0

// Four-way concatenation macro.
#define VT_CAT_4_( a, b, c, d ) a##b##c##d
#define VT_CAT_4( a, b, c, d )  VT_CAT_4_( a, b, c, d )

// Provides the current value of the counter as a three-digit octal number preceded by 0.
#define VT_TEMPLATE_COUNT VT_CAT_4( 0, VT_TEMPLATE_COUNT_D3, VT_TEMPLATE_COUNT_D2, VT_TEMPLATE_COUNT_D1 )

// _Generic-slot generation macros.

#define VT_GENERIC_SLOT( ty, fn, n ) , VT_CAT( ty, n ): VT_CAT( fn, n )
#define VT_R1_0( ty, fn, d3, d2 )
#define VT_R1_1( ty, fn, d3, d2 ) VT_GENERIC_SLOT( ty, fn, VT_CAT_4( 0, d3, d2, 0 ) )
#define VT_R1_2( ty, fn, d3, d2 ) VT_GENERIC_SLOT( ty, fn, VT_CAT_4( 0, d3, d2, 1 ) ) VT_R1_1( ty, fn, d3, d2 )
#define VT_R1_3( ty, fn, d3, d2 ) VT_GENERIC_SLOT( ty, fn, VT_CAT_4( 0, d3, d2, 2 ) ) VT_R1_2( ty, fn, d3, d2 )
#define VT_R1_4( ty, fn, d3, d2 ) VT_GENERIC_SLOT( ty, fn, VT_CAT_4( 0, d3, d2, 3 ) ) VT_R1_3( ty, fn, d3, d2 )
#define VT_R1_5( ty, fn, d3, d2 ) VT_GENERIC_SLOT( ty, fn, VT_CAT_4( 0, d3, d2, 4 ) ) VT_R1_4( ty, fn, d3, d2 )
#define VT_R1_6( ty, fn, d3, d2 ) VT_GENERIC_SLOT( ty, fn, VT_CAT_4( 0, d3, d2, 5 ) ) VT_R1_5( ty, fn, d3, d2 )
#define VT_R1_7( ty, fn, d3, d2 ) VT_GENERIC_SLOT( ty, fn, VT_CAT_4( 0, d3, d2, 6 ) ) VT_R1_6( ty, fn, d3, d2 )
#define VT_R1_8( ty, fn, d3, d2 ) VT_GENERIC_SLOT( ty, fn, VT_CAT_4( 0, d3, d2, 7 ) ) VT_R1_7( ty, fn, d3, d2 )
#define VT_R2_0( ty, fn, d3 )
#define VT_R2_1( ty, fn, d3 ) VT_R1_8( ty, fn, d3, 0 )
#define VT_R2_2( ty, fn, d3 ) VT_R1_8( ty, fn, d3, 1 ) VT_R2_1( ty, fn, d3 )
#define VT_R2_3( ty, fn, d3 ) VT_R1_8( ty, fn, d3, 2 ) VT_R2_2( ty, fn, d3 )
#define VT_R2_4( ty, fn, d3 ) VT_R1_8( ty, fn, d3, 3 ) VT_R2_3( ty, fn, d3 )
#define VT_R2_5( ty, fn, d3 ) VT_R1_8( ty, fn, d3, 4 ) VT_R2_4( ty, fn, d3 )
#define VT_R2_6( ty, fn, d3 ) VT_R1_8( ty, fn, d3, 5 ) VT_R2_5( ty, fn, d3 )
#define VT_R2_7( ty, fn, d3 ) VT_R1_8( ty, fn, d3, 6 ) VT_R2_6( ty, fn, d3 )
#define VT_R2_8( ty, fn, d3 ) VT_R1_8( ty, fn, d3, 7 ) VT_R2_7( ty, fn, d3 )
#define VT_R3_0( ty, fn )
#define VT_R3_1( ty, fn ) VT_R2_8( ty, fn, 0 )
#define VT_R3_2( ty, fn ) VT_R2_8( ty, fn, 1 ) VT_R3_1( ty, fn )
#define VT_R3_3( ty, fn ) VT_R2_8( ty, fn, 2 ) VT_R3_2( ty, fn )
#define VT_R3_4( ty, fn ) VT_R2_8( ty, fn, 3 ) VT_R3_3( ty, fn )
#define VT_R3_5( ty, fn ) VT_R2_8( ty, fn, 4 ) VT_R3_4( ty, fn )
#define VT_R3_6( ty, fn ) VT_R2_8( ty, fn, 5 ) VT_R3_5( ty, fn )
#define VT_R3_7( ty, fn ) VT_R2_8( ty, fn, 6 ) VT_R3_6( ty, fn )

#define VT_GENERIC_SLOTS( ty, fn )                                                           \
VT_CAT( VT_R1_, VT_TEMPLATE_COUNT_D1 )( ty, fn, VT_TEMPLATE_COUNT_D3, VT_TEMPLATE_COUNT_D2 ) \
VT_CAT( VT_R2_, VT_TEMPLATE_COUNT_D2 )( ty, fn, VT_TEMPLATE_COUNT_D3 )                       \
VT_CAT( VT_R3_, VT_TEMPLATE_COUNT_D3 )( ty, fn )                                             \

// Actual generic API macros.

#define vt_init( table ) _Generic( *( table ) VT_GENERIC_SLOTS( vt_table_, vt_init_ ) )( table )

#define vt_init_clone( table, ... ) _Generic( *( table ) \
  VT_GENERIC_SLOTS( vt_table_, vt_init_clone_ )          \
)( table, __VA_ARGS__ )                                  \

#define vt_size( table )_Generic( *( table ) VT_GENERIC_SLOTS( vt_table_, vt_size_ ) )( table )

#define vt_bucket_count( table ) _Generic( *( table ) VT_GENERIC_SLOTS( vt_table_, vt_bucket_count_ ) )( table )

#define vt_is_end( itr ) _Generic( itr VT_GENERIC_SLOTS( vt_table_itr_, vt_is_end_ ) )( itr )

#define vt_insert( table, ... ) _Generic( *( table ) VT_GENERIC_SLOTS( vt_table_, vt_insert_ ) )( table, __VA_ARGS__ )

#define vt_get_or_insert( table, ... ) _Generic( *( table ) \
  VT_GENERIC_SLOTS( vt_table_, vt_get_or_insert_ )          \
)( table, __VA_ARGS__ )                                     \

#define vt_get( table, ... ) _Generic( *( table ) VT_GENERIC_SLOTS( vt_table_, vt_get_ ) )( table, __VA_ARGS__ )

#define vt_erase( table, ... ) _Generic( *( table ) VT_GENERIC_SLOTS( vt_table_, vt_erase_ ) )( table, __VA_ARGS__ )

#define vt_next( itr ) _Generic( itr VT_GENERIC_SLOTS( vt_table_itr_, vt_next_ ) )( itr )

#define vt_erase_itr( table, ... ) _Generic( *( table ) \
  VT_GENERIC_SLOTS( vt_table_, vt_erase_itr_ )          \
)( table, __VA_ARGS__ )                                 \

#define vt_reserve( table, ... ) _Generic( *( table ) VT_GENERIC_SLOTS( vt_table_, vt_reserve_ ) )( table, __VA_ARGS__ )

#define vt_shrink( table ) _Generic( *( table ) VT_GENERIC_SLOTS( vt_table_, vt_shrink_ ) )( table )

#define vt_first( table ) _Generic( *( table ) VT_GENERIC_SLOTS( vt_table_, vt_first_ ) )( table )

#define vt_clear( table ) _Generic( *( table ) VT_GENERIC_SLOTS( vt_table_, vt_clear_ ) )( table )

#define vt_cleanup( table ) _Generic( *( table ) VT_GENERIC_SLOTS( vt_table_, vt_cleanup_ ) )( table )

#endif

#endif

/*--------------------------------------------------------------------------------------------------------------------*/
/*                                                  Prefixed structs                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef IMPLEMENTATION_MODE

typedef struct
{
  KEY_TY key;
  #ifdef VAL_TY
  VAL_TY val;
  #endif
} VT_CAT( NAME, _bucket );

typedef struct
{
  VT_CAT( NAME, _bucket ) *data;
  uint16_t *metadatum;
  uint16_t *metadata_end; // Iterators carry an internal end pointer so that NAME_is_end does not need the table to be
                          // passed in as an argument.
  size_t home_bucket; // SIZE_MAX if home bucket is unknown.
} VT_CAT( NAME, _itr );

typedef struct
{
  size_t key_count;
  size_t bucket_count;
  uint16_t *metadata; // As described above, each metadatum consists of a 4-bit hash-code fragment (X), a 1-bit flag
                      // indicating whether the key in this bucket begins a chain associated with the bucket (Y), and
                      // an 11-bit value indicating the quadratic displacement of the next key in the chain (Z):
                      // XXXXYZZZZZZZZZZZ.
  VT_CAT( NAME, _bucket ) *buckets;
} NAME;

#endif

/*--------------------------------------------------------------------------------------------------------------------*/
/*                                                Function prototypes                                                 */
/*--------------------------------------------------------------------------------------------------------------------*/

#if defined( HEADER_MODE ) || defined( IMPLEMENTATION_MODE )
#define VT_API_FN_QUALIFIERS
#else
#define VT_API_FN_QUALIFIERS static inline
#endif

#ifndef IMPLEMENTATION_MODE

VT_API_FN_QUALIFIERS void VT_CAT( NAME, _init )( NAME * );

VT_API_FN_QUALIFIERS bool VT_CAT( NAME, _init_clone )( NAME *, NAME * );

VT_API_FN_QUALIFIERS size_t VT_CAT( NAME, _size )( NAME * );

VT_API_FN_QUALIFIERS size_t VT_CAT( NAME, _bucket_count )( NAME * );

VT_API_FN_QUALIFIERS bool VT_CAT( NAME, _is_end )( VT_CAT( NAME, _itr ) );

VT_API_FN_QUALIFIERS VT_CAT( NAME, _itr ) VT_CAT( NAME, _insert )(
  NAME *,
  KEY_TY
  #ifdef VAL_TY
  , VAL_TY
  #endif
);

VT_API_FN_QUALIFIERS VT_CAT( NAME, _itr ) VT_CAT( NAME, _get_or_insert )(
  NAME *,
  KEY_TY
  #ifdef VAL_TY
  , VAL_TY
  #endif
);

VT_API_FN_QUALIFIERS VT_CAT( NAME, _itr ) VT_CAT( NAME, _get )(
  NAME *table,
  KEY_TY key
);

VT_API_FN_QUALIFIERS bool VT_CAT( NAME, _erase )( NAME *, KEY_TY );

VT_API_FN_QUALIFIERS VT_CAT( NAME, _itr ) VT_CAT( NAME, _next )( VT_CAT( NAME, _itr ) );

VT_API_FN_QUALIFIERS bool VT_CAT( NAME, _reserve )( NAME *, size_t );

VT_API_FN_QUALIFIERS bool VT_CAT( NAME, _shrink )( NAME * );

VT_API_FN_QUALIFIERS VT_CAT( NAME, _itr ) VT_CAT( NAME, _first )( NAME * );

VT_API_FN_QUALIFIERS void VT_CAT( NAME, _clear )( NAME * );

VT_API_FN_QUALIFIERS void VT_CAT( NAME, _cleanup )( NAME * );

// Not an API function, but must be prototyped anyway because it is called by the inline NAME_erase_itr below.
VT_API_FN_QUALIFIERS bool VT_CAT( NAME, _erase_itr_raw ) ( NAME *, VT_CAT( NAME, _itr ) );

// Erases the key pointed to by itr and returns an iterator to the next key in the table.
// This function must be inlined to ensure that the compiler optimizes away the NAME_fast_forward call if the returned
// iterator is discarded.
#ifdef __GNUC__
__attribute__((always_inline)) static inline
#elif defined( _MSC_VER )
__forceinline
#else
static inline
#endif
VT_CAT( NAME, _itr ) VT_CAT( NAME, _erase_itr )( NAME *table, VT_CAT( NAME, _itr ) itr )
{
  if( VT_CAT( NAME, _erase_itr_raw )( table, itr ) )
    return VT_CAT( NAME, _next )( itr );

  return itr;
}

#endif

/*--------------------------------------------------------------------------------------------------------------------*/
/*                                              Function implementations                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef HEADER_MODE

// Default settings:

#ifndef MAX_LOAD
#define MAX_LOAD 0.9
#endif

#if !defined( MALLOC ) || !defined( FREE )
#include <stdlib.h>
#endif

#ifndef MALLOC_FN
#define MALLOC_FN malloc
#endif

#ifndef FREE_FN
#define FREE_FN free
#endif

#ifndef HASH_FN
#if __STDC_VERSION__ >= 201112L
#define HASH_FN _Generic( ( KEY_TY ){ 0 }, char *: vt_hash_string, default: vt_hash_integer )
#else
#error Hash function inference is only available in C11 and later. In C99, you need to define HASH_FN manually to \
vt_hash_integer, vt_hash_string, or your own custom function with the signature uint64_t ( KEY_TY ).
#endif
#endif

#ifndef CMPR_FN
#if __STDC_VERSION__ >= 201112L
#define CMPR_FN _Generic( ( KEY_TY ){ 0 }, char *: vt_cmpr_string, default: vt_cmpr_integer )
#else
#error Comparison function inference is only available in C11 and later. In C99, you need to define CMPR_FN manually \
to vt_cmpr_integer, vt_cmpr_string, or your own custom function with the signature bool ( KEY_TY, KEY_TY ).
#endif
#endif

VT_API_FN_QUALIFIERS void VT_CAT( NAME, _init )( NAME *table )
{
  table->key_count = 0;
  table->bucket_count = 0;
  table->metadata = (uint16_t *)vt_placeholder_metadata_buffer;
  table->buckets = NULL;
}

VT_API_FN_QUALIFIERS bool VT_CAT( NAME, _init_clone )( NAME *table, NAME *source )
{
  table->key_count = source->key_count;
  table->bucket_count = source->bucket_count;

  if( source->bucket_count == 0 )
  {
    table->metadata = (uint16_t *)vt_placeholder_metadata_buffer;
    table->buckets = NULL;
    return true;
  }

  table->metadata = (uint16_t *)MALLOC_FN( ( table->bucket_count + 4 ) * sizeof( uint16_t ) );
  table->buckets = (VT_CAT( NAME, _bucket ) *)MALLOC_FN( table->bucket_count * sizeof( VT_CAT( NAME, _bucket ) ) );

  if( !table->metadata || !table->buckets )
  {
    FREE_FN( table->metadata );
    FREE_FN( table->buckets );
    return false;
  }

  memcpy( table->metadata, source->metadata, ( table->bucket_count + 4 ) * sizeof( uint16_t ) );
  memcpy( table->buckets, source->buckets, table->bucket_count * sizeof( VT_CAT( NAME, _bucket ) ) );

  return true;
}

VT_API_FN_QUALIFIERS size_t VT_CAT( NAME, _size )( NAME *table )
{
  return table->key_count;
}

VT_API_FN_QUALIFIERS size_t VT_CAT( NAME, _bucket_count )( NAME *table )
{
  return table->bucket_count;
}

VT_API_FN_QUALIFIERS bool VT_CAT( NAME, _is_end )( VT_CAT( NAME, _itr ) itr )
{
  return itr.metadatum == itr.metadata_end;
}

// Finds the earliest empty bucket in which a key belonging to home_bucket can be placed, assuming that home_bucket
// is already occupied.
// The reason to begin the search at home_bucket, rather than the end of the existing chain, is that keys deleted from
// other chains might have freed buckets that could potentially fall in this chain before the final key.
// Returns true if an empty bucket within the range of the displacement limit was found, in which case the final two
// pointer arguments contain the index of the empty bucket and its quadratic displacement from home_bucket.
static inline bool VT_CAT( NAME, _find_first_empty )(
  NAME *table,
  size_t home_bucket,
  size_t *empty,
  uint16_t *displacement
)
{
  *displacement = 1;
  size_t linear_dispacement = 1;

  while( true )
  {
    *empty = ( home_bucket + linear_dispacement ) & ( table->bucket_count - 1 );
    if( table->metadata[ *empty ] == VT_EMPTY )
      return true;

    if( ++*displacement == VT_DISPLACEMENT_MASK )
      return false;

    linear_dispacement += *displacement;
  }
}

// Finds the key in the chain beginning in home_bucket after which to link a new key with displacement_to_empty
// quadratic displacement and returns the index of the bucket containing that key.
// Although the new key could simply be linked to the end of the chain, keeping the chain ordered by displacement
// theoretically improves cache locality during lookups.
static inline size_t VT_CAT( NAME, _find_insert_location_in_chain )(
  NAME *table,
  size_t home_bucket,
  size_t displacement_to_empty
)
{
  size_t candidate = home_bucket;
  while( true )
  {
    uint16_t displacement = table->metadata[ candidate ] & VT_DISPLACEMENT_MASK;

    if( displacement > displacement_to_empty )
      return candidate;

    candidate = ( home_bucket + VT_QUADRATIC( displacement ) ) & ( table->bucket_count - 1 );
  }
}

// Frees up a bucket occupied by a key not belonging there so that a new key belonging there can be placed there as the
// beginning of a new chain.
// This requires:
// * Finding the previous key in the chain to which the occupying belongs by rehashing it and then traversing the chain.
// * Disconnecting the key from the chain.
// * Finding the appropriate empty bucket to which to move the key.
// * Moving the key (and value) data to the empty bucket.
// * Re-linking the key to the chain.
// Returns true if the eviction succeeded, or false if no empty bucket to which to evict the occupying key can be found
// within the displacement limit.
static inline bool VT_CAT( NAME, _evict )( NAME *table, size_t bucket )
{
  // Find the previous key in chain.
  size_t home_bucket = HASH_FN( table->buckets[ bucket ].key ) & ( table->bucket_count - 1 );
  size_t prev = home_bucket;
  while( true )
  {
    size_t next = ( home_bucket + VT_QUADRATIC( table->metadata[ prev ] & VT_DISPLACEMENT_MASK ) ) & (
      table->bucket_count - 1 );
    if( next == bucket )
      break;

    prev = next;
  }

  // Disconnect the key from chain.
  table->metadata[ prev ] = ( table->metadata[ prev ] & ~VT_DISPLACEMENT_MASK ) | ( table->metadata[ bucket ] &
    VT_DISPLACEMENT_MASK );

  // Find the empty bucket to which to move the key.
  size_t empty;
  uint16_t displacement;
  if( !VT_CAT( NAME, _find_first_empty )( table, home_bucket, &empty, &displacement ) )
    return false;

  // Find the key in the chain after which to link the moved key.
  prev = VT_CAT( NAME, _find_insert_location_in_chain )( table, home_bucket, displacement );

  // Move the key (and value) data.
  table->buckets[ empty ] = table->buckets[ bucket ];

  // Re-link the key to the chain from its new bucket.
  table->metadata[ empty ] = ( table->metadata[ bucket ] & VT_HASH_FRAG_MASK ) | ( table->metadata[ prev ] &
    VT_DISPLACEMENT_MASK );
  table->metadata[ prev ] = ( table->metadata[ prev ] & ~VT_DISPLACEMENT_MASK ) | displacement;

  return true;
}

// Returns an end iterator, i.e. any iterator for which .metadatum == .metadata_end.
// This function just cleans up the library code in functions that return an end iterator as a failure indicator.
static inline VT_CAT( NAME, _itr ) VT_CAT( NAME, _end_itr )( NAME *table )
{
  VT_CAT( NAME, _itr ) itr = { NULL, NULL, NULL, 0 };
  return itr;
}

// Inserts a key, optionally replacing the existing key if it already exists.
// There are two cases that must be handled:
// * If the key's home bucket is empty or occupied by a key that does not belong there, then the key is inserted there,
//   evicting the occupying key if there is one.
// * Otherwise, the chain of keys beginning at the home bucket is (if unique is false) traversed in search of a matching
//   key.
//   If none is found, then the new key is inserted at the earliest available bucket, per quadratic probing from the
//   home bucket, and then linked to the chain in a manner that maintains its quadratic order.
// The unique argument tells the function whether to skip searching for the key before inserting it (on rehashing, this
// step is unnecessary).
// The replace argument tells the function whether to replace an exiting key.
// If replace is true, the function returns an iterator to the inserted key, or an end iterator if the key was not
// inserted because of the maximum load factor or displacement limit constraints.
// If replace is false, then the return value is as described above, except that if the key already exists, the function
// returns an iterator to the existing key.
static inline VT_CAT( NAME, _itr ) VT_CAT( NAME, _insert_raw )(
  NAME *table,
  KEY_TY key,
  #ifdef VAL_TY
  VAL_TY val,
  #endif
  bool unique,
  bool replace
)
{
  if( !table->bucket_count )
    return VT_CAT( NAME, _end_itr )( table );

  uint64_t hash = HASH_FN( key );
  size_t home_bucket = hash & ( table->bucket_count - 1 );
  uint16_t hashfrag = VT_HASH_FRAG( hash );

  // Case 1: The home bucket is empty or contains a key that doesn't belong there.
  if( !( table->metadata[ home_bucket ] & VT_IN_HOME_BUCKET_MASK ) )
  {
    if( table->key_count + 1 > table->bucket_count * MAX_LOAD )
      return VT_CAT( NAME, _end_itr )( table );

    if( table->metadata[ home_bucket ] != VT_EMPTY )
      if( !VT_CAT( NAME, _evict )( table, home_bucket ) )
        return VT_CAT( NAME, _end_itr )( table );

    table->buckets[ home_bucket ].key = key;
    #ifdef VAL_TY
    table->buckets[ home_bucket ].val = val;
    #endif
    table->metadata[ home_bucket ] = hashfrag | VT_IN_HOME_BUCKET_MASK | VT_DISPLACEMENT_MASK;

    ++table->key_count;

    VT_CAT( NAME, _itr ) itr = {
      table->buckets + home_bucket,
      table->metadata + home_bucket,
      table->metadata + table->bucket_count,
      home_bucket
    };
    return itr;
  }

  // Case 2: The home bucket contains the beginning of a chain.

  // Optionally, check the existing chain.
  if( !unique )
  {
    size_t bucket = home_bucket;
    while( true )
    {
      if(
        ( table->metadata[ bucket ] & VT_HASH_FRAG_MASK ) == hashfrag &&
        CMPR_FN( table->buckets[ bucket ].key, key )
      )
      {
        if( replace )
        {
          #ifdef KEY_DTOR_FN
          KEY_DTOR_FN( table->buckets[ bucket ].key );
          #endif
          table->buckets[ bucket ].key = key;

          #ifdef VAL_TY
          #ifdef VAL_DTOR_FN
          VAL_DTOR_FN( table->buckets[ bucket ].val );
          #endif
          table->buckets[ bucket ].val = val;
          #endif
        }

        VT_CAT( NAME, _itr ) itr = {
          table->buckets + bucket,
          table->metadata + bucket,
          table->metadata + table->bucket_count,
          home_bucket
        };
        return itr;
      }

      uint16_t displacement = table->metadata[ bucket ] & VT_DISPLACEMENT_MASK;
      if( displacement == VT_DISPLACEMENT_MASK )
        break;

      bucket = ( home_bucket + VT_QUADRATIC( displacement ) ) & ( table->bucket_count - 1 );
    }
  }

  if( table->key_count + 1 > table->bucket_count * MAX_LOAD )
    return VT_CAT( NAME, _end_itr )( table );

  // Find the earliest empty bucket, per quadratic probing.
  size_t empty;
  uint16_t displacement;
  if( !VT_CAT( NAME, _find_first_empty )( table, home_bucket, &empty, &displacement ) )
    return VT_CAT( NAME, _end_itr )( table );

  size_t prev = VT_CAT( NAME, _find_insert_location_in_chain )( table, home_bucket, displacement );

  // Insert the new key (and value) in the empty bucket and link it to the chain.
  table->buckets[ empty ].key = key;
  #ifdef VAL_TY
  table->buckets[ empty ].val = val;
  #endif
  table->metadata[ empty ] = hashfrag | ( table->metadata[ prev ] & VT_DISPLACEMENT_MASK );
  table->metadata[ prev ] = ( table->metadata[ prev ] & ~VT_DISPLACEMENT_MASK ) | displacement;

  ++table->key_count;


  VT_CAT( NAME, _itr ) itr = {
    table->buckets + empty,
    table->metadata + empty,
    table->metadata + table->bucket_count,
    home_bucket
  };
  return itr;
}

// Resizes the bucket array.
// This function assumes that bucket_count is a power of two and large enough to accommodate all keys without violating
// the maximum load factor.
// Returns false in the case of allocation failure.
static inline bool VT_CAT( NAME, _rehash )( NAME *table, size_t bucket_count )
{
  // The attempt to resize the bucket array and rehash the keys must occur inside a loop that incrementally doubles the
  // target bucket count because a failure could theoretically occur at any load factor due to the displacement limit.
  while( true )
  {
    NAME new_table = {
      0,
      bucket_count,
      (uint16_t *)MALLOC_FN( ( bucket_count + 4 ) * sizeof( uint16_t ) ),
      (VT_CAT( NAME, _bucket ) *)MALLOC_FN( bucket_count * sizeof( VT_CAT( NAME, _bucket ) ) )
    };

    if( !new_table.metadata || !new_table.buckets )
    {
      FREE_FN( new_table.metadata );
      FREE_FN( new_table.buckets );
      return false;
    }

    memset( new_table.metadata, 0x00, bucket_count * sizeof( uint16_t ) );

    // Like the placeholder metadata array, any allocated metadata array requires excess elements to ensure that
    // iteration functions never read beyond the end of it.
    // The first excess metadatum, at least, must be non-zero in order to ensure that our iteration functions stop at
    // the end of the actual metadata (producing an end iterator).
    memset( new_table.metadata + bucket_count, 0xFF, 4 * sizeof( uint16_t ) );

    for( size_t bucket = 0; bucket < table->bucket_count; ++bucket )
      if( table->metadata[ bucket ] != VT_EMPTY )
      {
        VT_CAT( NAME, _itr ) itr = VT_CAT( NAME, _insert_raw )(
          &new_table,
          table->buckets[ bucket ].key,
          #ifdef VAL_TY
          table->buckets[ bucket ].val,
          #endif
          true,
          false
        );

        if( VT_CAT( NAME, _is_end )( itr ) )
        {
          FREE_FN( new_table.metadata );
          FREE_FN( new_table.buckets );
          bucket_count *= 2;
          continue;
        }
      }

    if( table->bucket_count )
    {
      FREE_FN( table->metadata );
      FREE_FN( table->buckets );
    }

    *table = new_table;
    return true;
  }
}

// Inserts a key, replacing the existing key if it already exists.
// This function wraps insert_raw in a loop that handles growing and rehashing the table if a new key cannot be inserted
// because of the maximum load factor or displacement limit constraints.
// Returns an iterator to the inserted key, or an end iterator in the case of allocation failure.
VT_API_FN_QUALIFIERS VT_CAT( NAME, _itr ) VT_CAT( NAME, _insert )(
  NAME *table,
  KEY_TY key
  #ifdef VAL_TY
  , VAL_TY val
  #endif
)
{
  while( true )
  {
    VT_CAT( NAME, _itr ) itr = VT_CAT( NAME, _insert_raw )(
      table,
      key,
      #ifdef VAL_TY
      val,
      #endif
      false,
      true
    );

    if(
      !VT_CAT( NAME, _is_end )( itr ) ||
      !VT_CAT( NAME, _rehash )( table, table->bucket_count ? table->bucket_count * 2 : VT_MIN_NONZERO_BUCKET_COUNT )
    )
      return itr;
  }
}

// Same as NAME_insert, except that if the key already exists, no insertion occurs and the function returns an iterator
// to the existing key.
VT_API_FN_QUALIFIERS VT_CAT( NAME, _itr ) VT_CAT( NAME, _get_or_insert )(
  NAME *table,
  KEY_TY key
  #ifdef VAL_TY
  , VAL_TY val
  #endif
)
{
  while( true )
  {
    VT_CAT( NAME, _itr ) itr = VT_CAT( NAME, _insert_raw )(
      table,
      key,
      #ifdef VAL_TY
      val,
      #endif
      false,
      false
    );

    if(
      !VT_CAT( NAME, _is_end )( itr ) ||
      !VT_CAT( NAME, _rehash )( table, table->bucket_count ? table->bucket_count * 2 : VT_MIN_NONZERO_BUCKET_COUNT )
    )
      return itr;
  }
}

// Returns an iterator for the specified key, or an end iterator if the key does not exist.
VT_API_FN_QUALIFIERS VT_CAT( NAME, _itr ) VT_CAT( NAME, _get )(
  NAME *table,
  KEY_TY key
)
{
  if( !table->key_count )
    return VT_CAT( NAME, _end_itr )( table );

  uint64_t hash = HASH_FN( key );
  size_t home_bucket = hash & ( table->bucket_count - 1 );
  uint16_t hashfrag = VT_HASH_FRAG( hash );

  // If the home bucket is empty or contains a key that does not belong there, then our key does not exist.
  if( !( table->metadata[ home_bucket ] & VT_IN_HOME_BUCKET_MASK ) )
    return VT_CAT( NAME, _end_itr )( table );

  // Traverse the chain of keys belonging to the home bucket.
  size_t bucket = home_bucket;
  while( true )
  {
    if( ( table->metadata[ bucket ] & VT_HASH_FRAG_MASK ) == hashfrag && CMPR_FN( table->buckets[ bucket ].key, key ) )
    {
      VT_CAT( NAME, _itr ) itr = {
        table->buckets + bucket,
        table->metadata + bucket,
        table->metadata + table->bucket_count,
        home_bucket
      };
      return itr;
    }

    uint16_t displacement = table->metadata[ bucket ] & VT_DISPLACEMENT_MASK;
    if( displacement == VT_DISPLACEMENT_MASK )
      return VT_CAT( NAME, _end_itr )( table );

    bucket = ( home_bucket + VT_QUADRATIC( displacement ) ) & ( table->bucket_count - 1 );
  }
}

// Erases the key pointed to by the specified iterator.
// The erasure always occurs at the end of the chain to which the key belongs.
// If the key to be erased is not the last in the chain, it is swapped with the last so that erasure occurs at the end.
// This helps keeps a chain's keys close to their home bucket for the sake of cache locality.
// Returns true if, in the case of iteration from first to end, NAME_next should now be called on the iterator to find
// the next key.
// This return value is necessary because at the iterator location, the erasure could result in an empty bucket, a
// bucket containing a moved key already visited during the iteration, or a bucket containing a moved key not yet
// visited.
VT_API_FN_QUALIFIERS bool VT_CAT( NAME, _erase_itr_raw ) ( NAME *table, VT_CAT( NAME, _itr ) itr )
{
  --table->key_count;
  size_t itr_bucket = itr.metadatum - table->metadata;

  #ifdef KEY_DTOR_FN
  KEY_DTOR_FN( table->buckets[ itr_bucket ].key );
  #endif
  #ifdef VAL_DTOR_FN
  VAL_DTOR_FN( table->buckets[ itr_bucket ].val );
  #endif

  // Case 1: The key is the only one in its chain, so just remove it.
  if(
    table->metadata[ itr_bucket ] & VT_IN_HOME_BUCKET_MASK &&
    ( table->metadata[ itr_bucket ] & VT_DISPLACEMENT_MASK ) == VT_DISPLACEMENT_MASK
  )
  {
    table->metadata[ itr_bucket ] = VT_EMPTY;
    return true;
  }

  // Case 2 and 3 require that we know the key's home bucket, which the iterator may not have recorded.
  if( itr.home_bucket == SIZE_MAX )
  {
    if( table->metadata[ itr_bucket ] & VT_IN_HOME_BUCKET_MASK )
      itr.home_bucket = itr_bucket;
    else
      itr.home_bucket = HASH_FN( table->buckets[ itr_bucket ].key ) & ( table->bucket_count - 1 );
  }

  // Case 2: The key is the last in a multi-key chain.
  // Traverse the chain from the beginning and find the penultimate key.
  // Then disconnect the key and erase.
  if( ( table->metadata[ itr_bucket ] & VT_DISPLACEMENT_MASK ) == VT_DISPLACEMENT_MASK )
  {
    size_t bucket = itr.home_bucket;
    while( true )
    {
      uint16_t displacement = table->metadata[ bucket ] & VT_DISPLACEMENT_MASK;
      size_t next = ( itr.home_bucket + VT_QUADRATIC( displacement ) ) & ( table->bucket_count - 1 );
      if( next == itr_bucket )
      {
        table->metadata[ bucket ] |= VT_DISPLACEMENT_MASK;
        table->metadata[ itr_bucket ] = VT_EMPTY;
        return true;
      }

      bucket = next;
    }
  }

  // Case 3: The chain has multiple keys, and the key is not the last one.
  // Traverse the chain from the key to be erased and find the last and penultimate keys.
  // Disconnect the last key from the chain, and swap it with the key to erase.
  size_t bucket = itr_bucket;
  while( true )
  {
    size_t prev = bucket;
    bucket = ( itr.home_bucket + VT_QUADRATIC( table->metadata[ bucket ] & VT_DISPLACEMENT_MASK ) ) & (
      table->bucket_count - 1 );

    if( ( table->metadata[ bucket ] & VT_DISPLACEMENT_MASK ) == VT_DISPLACEMENT_MASK )
    {
      table->buckets[ itr_bucket ] = table->buckets[ bucket ];

      table->metadata[ itr_bucket ] = ( table->metadata[ itr_bucket ] & ~VT_HASH_FRAG_MASK ) | (
        table->metadata[ bucket ] & VT_HASH_FRAG_MASK );

      table->metadata[ prev ] |= VT_DISPLACEMENT_MASK;
      table->metadata[ bucket ] = VT_EMPTY;

      // Whether the iterator should be incremented depends on whether the key moved to the iterator bucket came from
      // before or after that bucket.
      // In the former case, the iteration would already have hit the moved key, so the iterator should still be
      // incremented.
      if( bucket > itr_bucket )
        return false;

      return true;
    }
  }
}

// Erases the specified key, if it exists.
// Returns true if a key was erased.
VT_API_FN_QUALIFIERS bool VT_CAT( NAME, _erase )( NAME *table, KEY_TY key )
{
  VT_CAT( NAME, _itr ) itr = VT_CAT( NAME, _get)( table, key );
  if( VT_CAT( NAME, _is_end )( itr ) )
    return false;

  VT_CAT( NAME, _erase_itr_raw )( table, itr );
  return true;
}

// Finds the first occupied bucket at or after the bucket pointed to by itr.
// This function scans four buckets at a time, ideally using intrinsics.
static inline void VT_CAT( NAME, _fast_forward )( VT_CAT( NAME, _itr ) *itr )
{
  while( true )
  {
    uint64_t metadatum;
    memcpy( &metadatum, itr->metadatum, 8 );
    if( metadatum )
    {
      int offset = vt_first_nonzero_uint16( metadatum );
      itr->data += offset;
      itr->metadatum += offset;
      itr->home_bucket = SIZE_MAX;
      return;
    }

    itr->data += 4;
    itr->metadatum += 4;
  }
}

VT_API_FN_QUALIFIERS VT_CAT( NAME, _itr ) VT_CAT( NAME, _next )( VT_CAT( NAME, _itr ) itr )
{
  ++itr.data;
  ++itr.metadatum;
  VT_CAT( NAME, _fast_forward )( &itr );
  return itr;
}

// Returns the minimum bucket count required to accommodate a certain number of keys, which is governed by the maximum
// load factor.
static inline size_t VT_CAT( NAME, _min_bucket_count_for_size )( size_t size )
{
  if( size == 0 )
    return 0;

  // Round up to a power of two.
  size_t bucket_count = VT_MIN_NONZERO_BUCKET_COUNT;
  while( size > bucket_count * MAX_LOAD )
    bucket_count *= 2;

  return bucket_count;
}

VT_API_FN_QUALIFIERS bool VT_CAT( NAME, _reserve )( NAME *table, size_t size )
{
  size_t bucket_count = VT_CAT( NAME, _min_bucket_count_for_size )( size );

  if( bucket_count <= table->bucket_count )
    return true;

  return VT_CAT( NAME, _rehash )( table, bucket_count );
}

VT_API_FN_QUALIFIERS bool VT_CAT( NAME, _shrink )( NAME *table )
{
  size_t bucket_count = VT_CAT( NAME, _min_bucket_count_for_size )( table->key_count );

  if( bucket_count == table->bucket_count ) // Shrink unnecessary.
    return true;

  if( bucket_count == 0 )
  {
    FREE_FN( table->metadata );
    FREE_FN( table->buckets );
    table->bucket_count = 0;
    table->metadata = (uint16_t *)vt_placeholder_metadata_buffer;
    return true;
  }

  return VT_CAT( NAME, _rehash )( table, bucket_count );
}

VT_API_FN_QUALIFIERS VT_CAT( NAME, _itr ) VT_CAT( NAME, _first )( NAME *table )
{
  VT_CAT( NAME, _itr ) itr = { table->buckets, table->metadata, table->metadata + table->bucket_count, SIZE_MAX };
  VT_CAT( NAME, _fast_forward )( &itr );
  return itr;
}

VT_API_FN_QUALIFIERS void VT_CAT( NAME, _clear )( NAME *table )
{
  for( size_t i = 0; i < table->bucket_count; ++i )
  {
    if( table->metadata[ i ] != VT_EMPTY )
    {
      #ifdef KEY_DTOR_FN
      KEY_DTOR_FN( table->buckets[ i ].key );
      #endif
      #ifdef VAL_DTOR_FN
      VAL_DTOR_FN( table->buckets[ i ].val );
      #endif
    }

    table->metadata[ i ] = VT_EMPTY;
  }

  table->key_count = 0;
}

VT_API_FN_QUALIFIERS void VT_CAT( NAME, _cleanup )( NAME *table )
{
  if( !table->bucket_count )
    return;

  #if defined( KEY_DTOR_FN ) || defined( VAL_DTOR_FN )
  VT_CAT( NAME, _clear )( table );
  #endif

  FREE_FN( table->metadata );
  FREE_FN( table->buckets );
  VT_CAT( NAME, _init )( table );
}

#endif

/*--------------------------------------------------------------------------------------------------------------------*/
/*                                Wrapper types and functions for the C11 generic API                                 */
/*--------------------------------------------------------------------------------------------------------------------*/

#if __STDC_VERSION__ >= 201112L && !defined( IMPLEMENTATION_MODE ) && !defined( VT_NO_C11_GENERIC_API )

typedef NAME VT_CAT( vt_table_, VT_TEMPLATE_COUNT );
typedef VT_CAT( NAME, _itr ) VT_CAT( vt_table_itr_, VT_TEMPLATE_COUNT );

static inline void VT_CAT( vt_init_, VT_TEMPLATE_COUNT )( NAME *table )
{
  VT_CAT( NAME, _init )( table );
}

static inline bool VT_CAT( vt_init_clone_, VT_TEMPLATE_COUNT )( NAME *table, NAME* source )
{
  return VT_CAT( NAME, _init_clone )( table, source );
}

static inline size_t VT_CAT( vt_size_, VT_TEMPLATE_COUNT )( NAME *table )
{
  return VT_CAT( NAME, _size )( table );
}

static inline size_t VT_CAT( vt_bucket_count_, VT_TEMPLATE_COUNT )( NAME *table )
{
  return VT_CAT( NAME, _bucket_count )( table );
}

static inline bool VT_CAT( vt_is_end_, VT_TEMPLATE_COUNT )( VT_CAT( NAME, _itr ) itr )
{
  return VT_CAT( NAME, _is_end )( itr );
}

static inline VT_CAT( NAME, _itr ) VT_CAT( vt_insert_, VT_TEMPLATE_COUNT )(
  NAME *table,
  KEY_TY key
  #ifdef VAL_TY
  , VAL_TY val
  #endif
)
{
  return VT_CAT( NAME, _insert )(
    table,
    key
    #ifdef VAL_TY
    , val
    #endif
  );
}

static inline VT_CAT( NAME, _itr ) VT_CAT( vt_get_or_insert_, VT_TEMPLATE_COUNT )(
  NAME *table,
  KEY_TY key
  #ifdef VAL_TY
  , VAL_TY val
  #endif
)
{
  return VT_CAT( NAME, _get_or_insert )(
    table,
    key
    #ifdef VAL_TY
    , val
    #endif
  );
}

static inline VT_CAT( NAME, _itr ) VT_CAT( vt_get_, VT_TEMPLATE_COUNT )( NAME *table, KEY_TY key )
{
  return VT_CAT( NAME, _get )( table, key );
}

static inline bool VT_CAT( vt_erase_, VT_TEMPLATE_COUNT )( NAME *table, KEY_TY key )
{
  return VT_CAT( NAME, _erase )( table, key );
}

static inline VT_CAT( NAME, _itr ) VT_CAT( vt_next_, VT_TEMPLATE_COUNT )( VT_CAT( NAME, _itr ) itr )
{
  return VT_CAT( NAME, _next )( itr );
}

static inline VT_CAT( NAME, _itr ) VT_CAT( vt_erase_itr_, VT_TEMPLATE_COUNT )( NAME *table, VT_CAT( NAME, _itr ) itr )
{
  return VT_CAT( NAME, _erase_itr )( table, itr );
}

static inline bool VT_CAT( vt_reserve_, VT_TEMPLATE_COUNT )( NAME *table, size_t bucket_count )
{
  return VT_CAT( NAME, _reserve )( table, bucket_count );
}

static inline bool VT_CAT( vt_shrink_, VT_TEMPLATE_COUNT )( NAME *table )
{
  return VT_CAT( NAME, _shrink )( table );
}

static inline VT_CAT( NAME, _itr ) VT_CAT( vt_first_, VT_TEMPLATE_COUNT )( NAME *table )
{
  return VT_CAT( NAME, _first )( table );
}

static inline void VT_CAT( vt_clear_, VT_TEMPLATE_COUNT )( NAME *table )
{
  VT_CAT( NAME, _clear )( table );
}

static inline void VT_CAT( vt_cleanup_, VT_TEMPLATE_COUNT )( NAME *table )
{
  VT_CAT( NAME, _cleanup )( table );
}

// Increment the template counter.
#if     VT_TEMPLATE_COUNT_D1 == 0
#undef  VT_TEMPLATE_COUNT_D1
#define VT_TEMPLATE_COUNT_D1 1
#elif   VT_TEMPLATE_COUNT_D1 == 1
#undef  VT_TEMPLATE_COUNT_D1
#define VT_TEMPLATE_COUNT_D1 2
#elif   VT_TEMPLATE_COUNT_D1 == 2
#undef  VT_TEMPLATE_COUNT_D1
#define VT_TEMPLATE_COUNT_D1 3
#elif   VT_TEMPLATE_COUNT_D1 == 3
#undef  VT_TEMPLATE_COUNT_D1
#define VT_TEMPLATE_COUNT_D1 4
#elif   VT_TEMPLATE_COUNT_D1 == 4
#undef  VT_TEMPLATE_COUNT_D1
#define VT_TEMPLATE_COUNT_D1 5
#elif   VT_TEMPLATE_COUNT_D1 == 5
#undef  VT_TEMPLATE_COUNT_D1
#define VT_TEMPLATE_COUNT_D1 6
#elif   VT_TEMPLATE_COUNT_D1 == 6
#undef  VT_TEMPLATE_COUNT_D1
#define VT_TEMPLATE_COUNT_D1 7
#elif   VT_TEMPLATE_COUNT_D1 == 7
#undef  VT_TEMPLATE_COUNT_D1
#define VT_TEMPLATE_COUNT_D1 0
#if     VT_TEMPLATE_COUNT_D2 == 0
#undef  VT_TEMPLATE_COUNT_D2
#define VT_TEMPLATE_COUNT_D2 1
#elif   VT_TEMPLATE_COUNT_D2 == 1
#undef  VT_TEMPLATE_COUNT_D2
#define VT_TEMPLATE_COUNT_D2 2
#elif   VT_TEMPLATE_COUNT_D2 == 2
#undef  VT_TEMPLATE_COUNT_D2
#define VT_TEMPLATE_COUNT_D2 3
#elif   VT_TEMPLATE_COUNT_D2 == 3
#undef  VT_TEMPLATE_COUNT_D2
#define VT_TEMPLATE_COUNT_D2 4
#elif   VT_TEMPLATE_COUNT_D2 == 4
#undef  VT_TEMPLATE_COUNT_D2
#define VT_TEMPLATE_COUNT_D2 5
#elif   VT_TEMPLATE_COUNT_D2 == 5
#undef  VT_TEMPLATE_COUNT_D2
#define VT_TEMPLATE_COUNT_D2 6
#elif   VT_TEMPLATE_COUNT_D2 == 6
#undef  VT_TEMPLATE_COUNT_D2
#define VT_TEMPLATE_COUNT_D2 7
#elif   VT_TEMPLATE_COUNT_D2 == 7
#undef  VT_TEMPLATE_COUNT_D2
#define VT_TEMPLATE_COUNT_D2 0
#if     VT_TEMPLATE_COUNT_D3 == 0
#undef  VT_TEMPLATE_COUNT_D3
#define VT_TEMPLATE_COUNT_D3 1
#elif   VT_TEMPLATE_COUNT_D3 == 1
#undef  VT_TEMPLATE_COUNT_D3
#define VT_TEMPLATE_COUNT_D3 2
#elif   VT_TEMPLATE_COUNT_D3 == 2
#undef  VT_TEMPLATE_COUNT_D3
#define VT_TEMPLATE_COUNT_D3 3
#elif   VT_TEMPLATE_COUNT_D3 == 3
#undef  VT_TEMPLATE_COUNT_D3
#define VT_TEMPLATE_COUNT_D3 4
#elif   VT_TEMPLATE_COUNT_D3 == 4
#undef  VT_TEMPLATE_COUNT_D3
#define VT_TEMPLATE_COUNT_D3 5
#elif   VT_TEMPLATE_COUNT_D3 == 5
#undef  VT_TEMPLATE_COUNT_D3
#define VT_TEMPLATE_COUNT_D3 6
#elif   VT_TEMPLATE_COUNT_D3 == 6
#undef  VT_TEMPLATE_COUNT_D3
#define VT_TEMPLATE_COUNT_D3 7
#elif   VT_TEMPLATE_COUNT_D3 == 7
#error  Sorry, the number of template instances is limited to 511. Define VT_NO_C11_GENERIC_API globally and use the \
C99 prefixed function API to circumvent this restriction.
#endif
#endif
#endif

#endif

#undef NAME
#undef KEY_TY
#undef VAL_TY
#undef HASH_FN
#undef CMPR_FN
#undef MAX_LOAD
#undef KEY_DTOR_FN
#undef VAL_DTOR_FN
#undef MALLOC_FN
#undef FREE_FN
#undef HEADER_MODE
#undef IMPLEMENTATION_MODE
#undef VT_API_FN_QUALIFIERS
