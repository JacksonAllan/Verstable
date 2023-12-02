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

<table>
<tr>
</tr>
<tr>
<td>

```c
// CC
#include <stdio.h>
#include "cc.h"

int main( void )
{
  vec( int ) our_vec;
  init( &our_vec );
  push( &our_vec, 5 );
  printf( "%d\n", *get( &our_vec, 0 ) );
  cleanup( &our_vec );

  map( int, float ) our_map;
  init( &our_map );
  insert( &our_map, 5, 0.5f );
  printf( "%f\n", *get( &our_map, 5 ) );
  cleanup( &our_map );
}




```

</td>
<td>

```c
// Template-instantiation paradigm
#include <stdio.h>
#include "other_container_lib.h"

DEFINE_VEC( int, int_vec )
DEFINE_MAP( int, float, int_float_map )

int main( void )
{
  int_vec our_vec;
  int_vec_init( &our_vec );
  int_vec_push( &our_vec, 5 );
  printf( "%d\n", *int_vec_get( &our_vec, 0 ) );
  int_vec_cleanup( &our_vec );

  int_float_map our_map;
  int_float_map_init( &our_map );
  int_float_map_insert( &our_map, 5, 0.5f );
  printf( "%f\n", *int_float_map_get( &our_map, 5 ) );
  int_float_map_cleanup( &our_map );
}
```

</td>
</tr>
</table>
