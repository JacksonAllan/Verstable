<picture><img src="./header.svg" alt="Fastmap"></picture>

**Fastmap** is a generic hash table intended to bring the speed and memory efficiency of state-of-the-art C++ hash tables such as Abseil, Boost, and Bytell to C.

Its API features include:

* Type safety.
* Customizable hash, comparison, and destructor functions.
* Single header.
* C99 compatibility.
* Generic API for C11 and later.

Its performance-related features include:

* High speed almost immune to high load factors.
* Only two bytes of overhead per bucket.
* No tombstones.

Benchmarks comparing **Fastmap** to the aforementioned hash tables and several Robin Hood hash tables can be found here.

**Fastmap** is distributed under the MIT license.

# Installation

Just download fastmap.h and place it in your project's directory or your shared header directory.

# Example

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
