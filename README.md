<picture><img src="./header.svg" alt="Verstable"></picture>

**Verstable** is a generic hash table intended to bring the speed and memory efficiency of state-of-the-art C++ hash tables such as [Abseil/Swiss](https://abseil.io/about/design/swisstables), [Boost](https://bannalia.blogspot.com/2022/11/inside-boostunorderedflatmap.html), and [Bytell](https://probablydance.com/2018/05/28/a-new-fast-hash-table-in-response-to-googles-new-fast-hash-table/) to C.

Its features include:

<table>
<tr>
<td valign="top" width="50%">

API:
- Type safety.
- Customizable hash, comparison, and destructor functions.
- Single header.
- C99 compatibility.
- Generic API in C11 and later.

</td>
<td valign="top" width="50%">

Performance:
- High speed impervious to load factor.
- Two bytes of overhead per bucket.
- Tombstones-free deletion.

</td>
</tr>
</table>


<table>
<tr>
<td valign="top">
API:
<ul>
<li>Type safety.</li>
<li>Customizable hash, comparison, and destructor functions.</li>
<li>Single header.</li>
<li>C99 compatibility.</li>
<li>Generic API in C11 and later.</li>
</ul>
</td>
<td valign="top">
Performance:
<ul>
<li>High speed impervious to load factor.</li>
<li>Two bytes of overhead per bucket.</li>
<li>Tombstones-free deletion.</li>
</ul>
</td>
</tr>
</table>

Benchmarks comparing **Verstable** to the aforementioned hash tables and several Robin Hood-based hash tables are available here.----

**Verstable** is distributed under the MIT license.

Try it online here.--------

## Installation

Just download `verstable.h` and place it in your project's directory or your shared header directory.

## Example

<table>
<tr></tr>
<tr>
<td valign="top">
Using the generic API (C11 and later):

```c
#include <stdio.h>

// Instantiating a set template.
#define NAME int_set
#define KEY_TY int
#include "verstable.h"

// Instantiating a map template.
#define NAME int_int_map
#define KEY_TY int
#define VAL_TY int
#include "verstable.h"

int main( void )
{
  // Set.

  int_set our_set;
  vt_init( &our_set );

  // Inserting keys.
  for( int i = 0; i < 10; ++i )
  {
    int_set_itr itr = vt_insert( &our_set, i );
    if( vt_is_end( itr ) )
      exit( 1 ); // Out of memory.
  }

  // Erasing keys.
  for( int i = 0; i < 10; i += 3 )
    vt_erase( &our_set, i );

  // Retrieving keys.
  for( int i = 0; i < 10; ++i )
  {
    int_set_itr itr = vt_get( &our_set, i );
    if( !vt_is_end( itr ) )
      printf( "%d ", itr.data->key );
  }
  // Printed: 1 2 4 5 7 8

  // Iteration.
  for(
    int_set_itr itr = vt_first( &our_set );
    !vt_is_end( itr );
    itr = vt_next( itr )
  )
    printf( "%d ", itr.data->key );
  // Printed: 4 5 2 8 1 7

  vt_cleanup( &our_set );

  // Map.

  int_int_map our_map;
  vt_init( &our_map );

  // Inserting keys and values.
  for( int i = 0; i < 10; ++i )
  {
    int_int_map_itr itr =
      vt_insert( &our_map, i, i + 1 );
    if( vt_is_end( itr ) )
      exit( 1 ); // Out of memory.
  }

  // Erasing keys and values.
  for( int i = 0; i < 10; i += 3 )
    vt_erase( &our_map, i );

  // Retrieving keys and values.
  for( int i = 0; i < 10; ++i )
  {
    int_int_map_itr itr = vt_get( &our_map, i );
    if( !vt_is_end( itr ) )
      printf(
        "%d:%d ",
        itr.data->key,
        itr.data->val
      );
  }
  // Printed: 1:2 2:3 4:5 5:6 7:8 8:9

  // Iteration.
  for(
    int_int_map_itr itr = vt_first( &our_map );
    !vt_is_end( itr );
    itr = vt_next( itr )
  )
    printf(
      "%d:%d ",
      itr.data->key,
      itr.data->val
    );
  // Printed: 4:5 5:6 2:3 8:9 1:2 7:8

  vt_cleanup( &our_map );
}
```

</td>
<td valign="top">
Using the prefixed functions API (C99 and later):

```c
#include <stdio.h>

// Instantiating a set template.
#define NAME int_set
#define KEY_TY int
#define HASH_FN vt_hash_integer
#define CMPR_FN vt_cmpr_integer
#include "verstable.h"

// Instantiating a map template.
#define NAME int_int_map
#define KEY_TY int
#define VAL_TY int
#define HASH_FN vt_hash_integer
#define CMPR_FN vt_cmpr_integer
#include "verstable.h"

int main( void )
{
  // Set.

  int_set our_set;
  int_set_init( &our_set );

  // Inserting keys.
  for( int i = 0; i < 10; ++i )
  {
    int_set_itr itr =
      int_set_insert( &our_set, i );
    if( int_set_is_end( itr ) )
      exit( 1 ); // Out of memory.
  }

  // Erasing keys.
  for( int i = 0; i < 10; i += 3 )
    int_set_erase( &our_set, i );

  // Retrieving keys.
  for( int i = 0; i < 10; ++i )
  {
    int_set_itr itr = int_set_get( &our_set, i );
    if( !int_set_is_end( itr ) )
      printf( "%d ", itr.data->key );
  }
  // Printed: 1 2 4 5 7 8

  // Iteration.
  for(
  	int_set_itr itr =
      int_set_first( &our_set );
  	!int_set_is_end( itr );
  	itr = int_set_next( itr )
  )
    printf( "%d ", itr.data->key );
  // Printed: 4 5 2 8 1 7

  int_set_cleanup( &our_set );

  // Map.

  int_int_map our_map;
  int_int_map_init( &our_map );

  // Inserting keys and values.
  for( int i = 0; i < 10; ++i )
  {
    int_int_map_itr itr =
      int_int_map_insert( &our_map, i, i + 1 );
    if( int_int_map_is_end( itr ) )
      exit( 1 ); // Out of memory.
  }

  // Erasing keys and values.
  for( int i = 0; i < 10; i += 3 )
    int_int_map_erase( &our_map, i );

  // Retrieving keys and values.
  for( int i = 0; i < 10; ++i )
  {
    int_int_map_itr itr =
      int_int_map_get( &our_map, i );
    if( !int_int_map_is_end( itr ) )
      printf(
      	"%d:%d ",
      	itr.data->key,
      	itr.data->val
    );
  }
  // Printed: 1:2 2:3 4:5 5:6 7:8 8:9

  // Iteration.
  for(
    int_int_map_itr itr =
      int_int_map_first( &our_map );
  	!int_int_map_is_end( itr );
  	itr = int_int_map_next( itr )
  )
    printf(
      "%d:%d ",
      itr.data->key,
      itr.data->val
    );
  // Printed: 4:5 5:6 2:3 8:9 1:2 7:8

  int_int_map_cleanup( &our_map );
}
```

</td>
</tr>
</table>

## API

Full API documentation is available [here](api_reference.md).

## FAQ

### How does it work?

**Verstable** is an open-addressing hash table using quadratic probing and the following innovations:

- All keys that hash (i.e. "belong") to the same bucket (their "home bucket") are linked together by an 11-bit integer specifying the quadratic displacement, relative to that bucket, of the next key in the chain.

- If a chain of keys exists for a given bucket, then it always begins at that bucket. To maintain this policy, a 1-bit flag is used to mark whether the key occupying a bucket belongs there. When inserting a new key, if the bucket it belongs to is occupied by a key that does not belong there, then the occupying key is evicted and the new key takes the bucket.

- A 4-bit fragment of each key's hash code is also stored.

- The aforementioned metadata associated with each bucket (the 4-bit hash fragment, the 1-bit flag, and the 11-bit link to the next key in the chain) are stored together in a `uint16_t` array rather than in the bucket alongside the key and (optionally) the value.

One way to conceptualize this design is as a chained hash table in which overflowing keys are stored not in separate memory allocations but in otherwise unused buckets. In this regard, it is similar to the design discussed by Malte Skarupke's Bytell hash table, which is detailed [here](https://www.youtube.com/watch?v=M2fKMP47slQ).

Advantages of this design include:

- Fast lookups impervious to load factor: If the table contains any key belonging to the lookup key's home bucket, then that bucket contains the first in a traversable chain of all keys belonging to it. Hence, only the home bucket and other buckets containing keys belonging to it are ever probed. Moreover, the stored hash fragments allow skipping most non-matching keys in the chain without accessing the actual buckets array or calling the (potentially expensive) key comparison function.

- Fast insertions: Insertions are faster than they are in other designs that move keys around (e.g. Robin Hood) because they only move, at most, one existing key.

- Fast, tombstone-free deletions: Deletions, which usually require tombstones in quadratic-probing hash tables, are tombstone-free and only move, at most, one existing key.

- Consistently fast iteration: The separate metadata array allows keys in sparsely populated tables to be found without incurring the frequent cache misses that would result from traversing the buckets array.

The generic API available in C11 is based on the extendible-`_Generic` mechanism detailed [here](https://github.com/JacksonAllan/CC/blob/main/articles/Better_C_Generics_Part_1_The_Extendible_Generic.md).

### How is it tested?

**Verstable** has been tested under GCC, Clang, MinGW, and MSVC. `tests/unit_tests.c` includes unit tests for sets and maps, with an emphasis on corner cases. `tests/tests_against_stl.cpp` includes randomized tests that perform the same operations on **Verstable** sets and maps, on one hand, and C++'s `std::unordered_set` and `std::unordered_map`, on the other, and then check that they remain in sync. Both test suites use a tracking and randomly failing memory allocator in order to detect memory leaks and test out-of-memory conditions.

## Why "Verstable"?

