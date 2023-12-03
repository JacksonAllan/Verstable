<picture><img src="./header.svg" alt="Fastmap"></picture>

**Fastmap** is a generic hash table intended to bring the speed and memory efficiency of state-of-the-art C++ hash tables such as Abseil, Boost, and Bytell to C.

Its API features include:

* Type safety.
* Customizable hash, comparison, and destructor functions.
* Single header.
* C99 compatibility.
* Generic API in C11 and later.

Its performance-related features include:

* High speed almost immune to high load factors.
* Only two bytes of overhead per bucket.
* No tombstones.

Benchmarks comparing **Fastmap** to the aforementioned hash tables and several Robin Hood hash tables can be found here.

**Fastmap** is distributed under the MIT license.

Try it online here.--------

## Installation

Just download `fastmap.h` and place it in your project's directory or your shared header directory.

## Example

<table>
<tr></tr>
<tr>
<td>
Using the generic API (C11 and later):

```c
#include <stdio.h>

// Instantiating a set template.
#define NAME int_set
#define KEY_TY int
#include "../fastmap.h"

// Instantiating a map template.
#define NAME int_int_map
#define KEY_TY int
#define VAL_TY int
#include "../fastmap.h"

int main( void )
{
  // Set.

  int_set our_set;
  fm_init( &our_set );

  // Inserting keys.
  for( int i = 0; i < 10; ++i )
  {
    int_set_itr itr = fm_insert( &our_set, i );
    if( fm_is_end( itr ) )
      exit( 1 ); // Out of memory.
  }

  // Erasing keys.
  for( int i = 0; i < 10; i += 3 )
    fm_erase( &our_set, i );

  // Retrieving keys.
  for( int i = 0; i < 10; ++i )
  {
    int_set_itr itr = fm_get( &our_set, i );
    if( !fm_is_end( itr ) )
      printf( "%d ", itr.data->key );
  }
  // Printed: 1 2 4 5 7 8

  // Iteration.
  for(
    int_set_itr itr = fm_first( &our_set );
    !fm_is_end( itr );
    itr = fm_next( itr )
  )
    printf( "%d ", itr.data->key );
  // Printed: 4 5 2 8 1 7

  fm_cleanup( &our_set );

  // Map.

  int_int_map our_map;
  fm_init( &our_map );

  // Inserting keys and values.
  for( int i = 0; i < 10; ++i )
  {
    int_int_map_itr itr =
      fm_insert( &our_map, i, i + 1 );
    if( fm_is_end( itr ) )
      exit( 1 ); // Out of memory.
  }

  // Erasing keys and values.
  for( int i = 0; i < 10; i += 3 )
    fm_erase( &our_map, i );

  // Retrieving keys and values.
  for( int i = 0; i < 10; ++i )
  {
    int_int_map_itr itr = fm_get( &our_map, i );
    if( !fm_is_end( itr ) )
      printf(
        "%d:%d ",
        itr.data->key,
        itr.data->val
      );
  }
  // Printed: 1:2 2:3 4:5 5:6 7:8 8:9

  // Iteration.
  for(
    int_int_map_itr itr = fm_first( &our_map );
    !fm_is_end( itr );
    itr = fm_next( itr )
  )
    printf(
      "%d:%d ",
      itr.data->key,
      itr.data->val
    );
  // Printed: 4:5 5:6 2:3 8:9 1:2 7:8

  fm_cleanup( &our_map );
}










```

</td>
<td>
Using the prefixed functions API (C99 and later):

```c
#include <stdio.h>

// Instantiating a set template.
#define NAME int_set
#define KEY_TY int
#define HASH_FN fm_hash_integer
#define CMPR_FN fm_cmpr_integer
#include "../fastmap.h"

// Instantiating a map template.
#define NAME int_int_map
#define KEY_TY int
#define VAL_TY int
#define HASH_FN fm_hash_integer
#define CMPR_FN fm_cmpr_integer
#include "../fastmap.h"

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

## FAQ

### How does it work?

Fastmap is an open-addressing hash table using quadratic probing and the following innovations:

- All keys that hash (i.e. "belong") to the same bucket (their "home bucket") are linked together by an 11-bit integer specifying the quadratic displacement, relative to that bucket, of the next key in the chain.

- If a chain of keys exists for a given bucket, then it always begins at that bucket. To maintain this policy, a 1-bit flag is used to mark whether the key occupying a bucket belongs there. When inserting a new key, if the bucket it belongs to is occupied by a key that does not belong there, then the occupying key is evicted and the new key takes the bucket.

- A 4-bit fragment of each key's hash code is also stored.

- The aforementioned metadata associated with each bucket (the 4-bit hash fragment, the 1-bit flag, and the 11-bit link to the next key in the chain) are stored together in a uint16_t array rather than in the bucket alongside the key and (optionally) the value.

One way to conceptualize this design is as a chained hash table in which overflowing keys are stored not in separate memory allocations but in otherwise unused buckets. In this regard, it is similar to the design discussed by Malte Skarupke's [Bytell hash table](https://www.youtube.com/watch?v=M2fKMP47slQ).

Advantages of this design include:

- Fast lookups impervious to load factor: If the table contains any key belonging to the lookup key's home bucket, then that bucket contains the first in a traversable chain of all keys belonging to it. Hence, only the home bucket and other buckets containing keys belonging to it are ever probed. Moreover, the stored hash fragments allow skipping most non-matching keys in the chain without accessing the actual buckets array or calling the (potentially expensive) key comparison function.

- Fast insertions: Insertions are faster than they are in other designs that move keys around (e.g., Robin Hood) because they only move, at most, one existing key.

- Fast, tombstone-free deletions: Deletions, which usually require tombstones in quadratic-probing hash tables, are tombstone-free and only move, at most, one existing key.

- Consistently fast iteration: The separate metadata array allows keys in sparsely populated tables to be found without incurring the frequent cache misses that would result from traversing the buckets array.

### How is it tested?

**Fastmap** has been tested under GCC, Clang, MinGW, and MSVC. `tests/unit_tests.c` includes unit tests for all container types, with an emphasis on corner cases. `tests/tests_against_stl.cpp` includes randomized tests that perform the same operations on Fastmap sets and maps, on one hand, and C++'s `std::unordered_set` and `std::unordered_map`, on the other, and then check that they remain in sync. Both test suites use a tracking and randomly failing memory allocator in order to detect memory leaks and test out-of-memory conditions.

## API

Full API documentation is available [here](api_reference.md).
