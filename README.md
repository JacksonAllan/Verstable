<picture><img src="./header.svg" alt="Verstable"></picture>

**Verstable** is a versatile generic hash table intended to bring the speed and memory efficiency of state-of-the-art C++ hash tables such as [Abseil/Swiss](https://abseil.io/about/design/swisstables), [Boost](https://bannalia.blogspot.com/2022/11/inside-boostunorderedflatmap.html), and [Bytell](https://probablydance.com/2018/05/28/a-new-fast-hash-table-in-response-to-googles-new-fast-hash-table/) to C.

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
- High speed mostly impervious to load factor.
- Only two bytes of overhead per bucket.
- Tombstones-free deletion.

</td>
</tr>
</table>

Benchmarks comparing **Verstable** to the aforementioned hash tables and several [Robin Hood](https://www.sebastiansylvan.com/post/robin-hood-hashing-should-be-your-default-hash-table-implementation/)-based hash tables are available here.-------

**Verstable** is distributed under the MIT license.

Try it online [here](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1AB9U8lJL6yAngGVG6AMKpaAVxYMQAZi%2BkHAGTwGTAA5dwAjTGIQAA5SAAdUBUJbBmc3D29fROSbAUDgsJZI6LjLTGtUoQImYgJ0908fC0wrPIZq2oIC0Iio2IsauobM5oUh7qDe4v6YgEoLVFdiZHYOAFIAJi8g5DcsAGp1r0dx/FQAOgRj7HWNAEE7%2B4B6Z4OASQZxwxsmGyMDkwDkoCAcCJgWPEDOCLk8tl4sDRggcQvcALLYA5BAjGEFw7aIqYHADS2AAmsYACpkrGCfE7Bh7VyHY6OBAEAjxBQgV7EJgAdwuwEICFc4VcShWskYBAuaBYzwAUkxkABrBQCe60AwMZ4ANSi33C9GeADdDTVjZgrjc4Q9Xh8vjVBHg/kFgICDiwmPEwRCoX9rfTCcjURjaTjscZvfFg5gkZgSeSqTTsXGEwc9fd/CmI/TdvtE6z2ZzubyBUKRWKJVE0IIZXLUArlWqNQwtTr9RamFazd2rTavLcHk9sV6mEEIAdTag8OgDnM4QB2ABCTwOBwdQkwsrt9w3UZBByWxFxO%2BOa4eG9NkYYhCnWwAbCez6DF15L48r5u3p9Jf8PVVTAAE8FFhb9%2BGIKcxzwI4vAAEQODQLyxODHAOLhkI/I5NjXXDYMXb91lXdcNwjV9jEIYgsQIajjkQm9KK%2BKICAfTZn2WV9SFQ99PzIrEqCnRi8AUYwHGg2iFwXUj%2BMwVR7ww6TsIdAB5VxQVQKgvQhEhgPA/cjiXeC9w3B1sD5ZIASA0D9I3SDoMEVD6KQlDYNZDCsJXJzcOcrxpO/a8cSiJglDYjjTxBbiCIvEyfwOAAlHdiDwTBTXdA5rLA0j7IjJyEJc7C3JODyUK2PCV2ioiSIC8iQUoySqLghicWAHcwpfSKeJimq8EEg4wDAYTRPEmjqLmfyDP4%2BJksEPqtk2LYAFZ53mqLaIudA/iYABaG5rKUvjDOM78HWUGbwXQEBFM2A5JAORaDiXA4Ylih13nBPl2lsg57Jk2qd3q6jGucxiaGIcZ2s4o9eL%2BgahrEhh0Aksbusm0amunHFglUVj0cIgz8am865s2BbNmWnDNjW4gNq23ah32mGTreM7sUwS7bvug4bpiRSl1ixi9kwQxXHiSGIp3A7XreNEfX00ioyjGNj04mNUcCpiFKfF9laZr8DLe5i6nSzLAUR6cxFcTAsogkgHNBIrEM8vL0Mw0qfPwiaN2Iw6D0EJicRjQH0fov6NaCf9xejH0oqinCvK4KWer6%2BGRsa8bCf4g45IUxPeLitSNK0tgWF077iOO/XTLecyQpNkCFDN%2BdTUt63vpymCMedorXe7%2BPfK9sjGOC0KcPC6P4jjvXSIdRLaJStKrIbpuLbcNvsrt3LHYKrye5K7CyrKyqDJ9v7Ff9oPGuB/LGNa3HtdVmOuo/M%2B%2BrhyNhsR5GpMzonsUEsOZF5pLUuiAympBAEHnWptGo9NsDWQgTVfiVFaawJuC3WggC9beyMjPFm512ZXS4CAG6mxvC3RAA9RaIBHyPViM9EAABOaWHwPpugEO3O2Z9/ZKx9MHa%2BzVjBgwhmPHWPok5o3fpRT%2BSM8bqzIgIzGxhsa43TqRX%2BBxpr/wgIA4B5NQH6PAYAlBMCdp7RAogtGUCaamLgRgv62C4qs3rBzSQlD7o0O5uQmITCMIkLoS9Ec35Bb0BFmLURj9fTT1wQ8DgCxaCcEWrwTwHAtCkFQJwdCGplirBwl4HgpACCaDiQsBAwssDRAgAsVUNDGEXC4JsGIjCuCMJiI%2BDQj5NhLi4HEBJHBJDJOKekzgvBuQaEKcUhYcBYBIDkhUdSJByCUFqMABQyhDCtCEAgVA/IUkFPlPEOg7CGDrOCLQLZOyUlpIOXQfoyBgANKpjc%2BgxAQisDWLwZ5URC4XN2UMuZyB7jEFWSMvwqgKjVHwCk3g/BBAiDEOwKQMhBCKBUOoVJOg9A6hMGYfQeBwjckgAsVA8R2jcg4LwVA5piDJSwISqppBiCuBdGwSkqAXD0oWNklYiLBhQtOZs7ZfzuC8FopgNYBT%2BR8niJwHg8TEmDIxcMjg2BwXIAWdRVQbTtqPjusAZAyAMKbAuDdCAjhuK4EICQPJXA5iismaU8p/QGU1K8BoC4MRukaBiBoLgXhGFeCXI0qmfSBmkCuZS0FYyJkYrmPKjgmxFVpIyRSmNWg42kGpckOwkggA%3D).

## Installation

Just [download](verstable.h?raw=1) `verstable.h` and place it in your project's directory or your common header directory.

## Example

<table>
<tr></tr>
<tr>
<td valign="top">
Using the generic macro API (C11 and later):

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

**Verstable** is an open-addressing hash table using quadratic probing and the following additions:

- All keys that hash (i.e. "belong") to the same bucket (their "home bucket") are linked together by an 11-bit integer specifying the quadratic displacement, relative to that bucket, of the next key in the chain.

- If a chain of keys exists for a given bucket, then it always begins at that bucket. To maintain this policy, a 1-bit flag is used to mark whether the key occupying a bucket belongs there. When inserting a new key, if the bucket it belongs to is occupied by a key that does not belong there, then the occupying key is evicted and the new key takes the bucket.

- A 4-bit fragment of each key's hash code is also stored.

- The aforementioned metadata associated with each bucket (the 4-bit hash fragment, the 1-bit flag, and the 11-bit link to the next key in the chain) are stored together in a `uint16_t` array rather than in the bucket alongside the key and (optionally) the value.

One way to conceptualize this scheme is as a chained hash table in which overflowing keys are stored not in separate memory allocations but in otherwise unused buckets. In this regard, it shares similarities with Malte Skarupke’s [Bytell](https://www.youtube.com/watch?v=M2fKMP47slQ) hash table and traditional "coalesced hashing".

Advantages of this scheme include:

- Fast lookups impervious to load factor: If the table contains any key belonging to the lookup key's home bucket, then that bucket contains the first in a traversable chain of all keys belonging to it. Hence, only the home bucket and other buckets containing keys belonging to it are ever probed. Moreover, the stored hash fragments allow skipping most non-matching keys in the chain without accessing the actual buckets array or calling the (potentially expensive) key comparison function.

- Fast insertions: Insertions are faster than they are in other schemes that move keys around (e.g. Robin Hood) because they only move, at most, one existing key.

- Fast, tombstone-free deletions: Deletions, which usually require tombstones in quadratic-probing hash tables, are tombstone-free and only move, at most, one existing key.

- Fast iteration: The separate metadata array allows keys in sparsely populated tables to be found without incurring the frequent cache misses that would result from traversing the buckets array.

The generic macro API available in C11 is based on the extendible-`_Generic` mechanism detailed [here](https://github.com/JacksonAllan/CC/blob/main/articles/Better_C_Generics_Part_1_The_Extendible_Generic.md).

### How is it tested?

**Verstable** has been tested under GCC, Clang, MinGW, and MSVC. `tests/unit_tests.c` includes unit tests for sets and maps, with an emphasis on corner cases. `tests/tests_against_stl.cpp` includes randomized tests that perform the same operations on **Verstable** sets and maps, on one hand, and C++'s `std::unordered_set` and `std::unordered_map`, on the other, and then check that they remain in sync. Both test suites use a tracking and randomly failing memory allocator in order to detect memory leaks and test out-of-memory conditions.

### Why the name?

The name is a contraction of "versatile table". **Verstable** handles various conditions that strain other hash table schemes—such as large keys or values that are expensive to move, high load factors, expensive hash or comparison functions, and frequent deletions, iteration, and unsuccessful lookups—without significant performance loss. In other words, it is designed to be a good default choice of hash table for most use cases.
