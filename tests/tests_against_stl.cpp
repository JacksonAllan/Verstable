/*

Verstable - tests/tests_against_stl.cpp - v2.1.0

This file tests Verstable sets and maps against equivalent C++'s unordered_set and unordered_map.
Primarily, it checks that a Verstable hash table and its equivalent STL container finish in the same state after a
random series of the same operations are performed on both.
It also checks that results of API calls that return iterators are as expected.

License (MIT):

  Copyright (c) 2023-2024 Jackson L. Allan

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
  documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
  persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
  Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <ctime>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

// Assert macro that is not disabled by NDEBUG.
#define ALWAYS_ASSERT( xp )                                                                               \
( (xp) ? (void)0 : ( fprintf( stderr, "Assertion failed at line %d: %s\n", __LINE__, #xp ), exit( 0 ) ) ) \

// Macros to control the number of random operations to perform in each container test and the number of tests to
// perform on each container.
#define N_OPS 50000
#define N_TESTS 5

// Disable this macro to turn off failing malloc.
#define SIMULATE_ALLOC_FAILURES

// If malloc can fail, then we need a macro to repeat each call until it succeeds.
#ifdef SIMULATE_ALLOC_FAILURES
#define UNTIL_SUCCESS( xp ) while( !(xp) )
#else
#define UNTIL_SUCCESS( xp ) xp
#endif

// Custom malloc and free functions that track the number of outstanding allocations.
// If SIMULATE_ALLOC_FAILURES is defined above, the malloc function will also sporadically fail.

size_t simulated_alloc_failures;
std::unordered_set<void *> oustanding_allocs;

void *unreliable_tracking_malloc( size_t size )
{
#ifdef SIMULATE_ALLOC_FAILURES
  if( rand() % 5 == 0 )
  {
    ++simulated_alloc_failures;
    return NULL;
  }
#endif

  void *ptr = malloc( size );
  ALWAYS_ASSERT( ptr );

  oustanding_allocs.insert( ptr );

  return ptr;
}

void tracking_free( void *ptr, size_t size )
{
  (void)size;

  if( ptr )
    oustanding_allocs.erase( ptr );

  free( ptr );
}

// Instantiate hash table templates.

#define NAME      integer_map
#define KEY_TY    int
#define VAL_TY    int
#define HASH_FN   vt_hash_integer // C99 and C++.
#define CMPR_FN   vt_cmpr_integer // C99 and C++.
#define MAX_LOAD  0.95
#define MALLOC_FN unreliable_tracking_malloc
#define FREE_FN   tracking_free
#include "../verstable.h"

#define NAME      integer_set
#define KEY_TY    int
#define HASH_FN   vt_hash_integer // C99 and C++.
#define CMPR_FN   vt_cmpr_integer // C99 and C++.
#define MAX_LOAD  0.95
#define MALLOC_FN unreliable_tracking_malloc
#define FREE_FN   tracking_free
#include "../verstable.h"

// Redefine max load factor as it is reused below.
#define MAX_LOAD 0.95

int main()
{
  srand( (unsigned int)std::time( nullptr ) );

  // Map.
  for( int test = 0; test < N_TESTS; ++test )
  {
    std::cout << "Map test " << test << "... ";
    std::unordered_map<int, int> stl_map;
    integer_map our_map;
    integer_map_init( &our_map );

    for( int op = 0; op < N_OPS; ++op )
    {
      switch( rand() % 7 )
      {
        case 0: // Insert.
        {
          integer_map_itr itr;
          int key = rand() % ( N_OPS / 10 );
          int val = rand();
          UNTIL_SUCCESS( !integer_map_is_end( itr = integer_map_insert( &our_map, key, val ) ) );

          ALWAYS_ASSERT( itr.data->key == key );
          ALWAYS_ASSERT( itr.data->val == val );

          stl_map[ key ] = val;
        }
        break;
        case 1: // Get or insert.
        {
          integer_map_itr itr;
          int key = rand() % ( N_OPS / 10 );
          int val = rand();
          size_t original_size = integer_map_size( &our_map );
          UNTIL_SUCCESS( !integer_map_is_end( itr = integer_map_get_or_insert( &our_map, key, val ) ) );

          ALWAYS_ASSERT( itr.data->key == key );

          if( integer_map_size( &our_map ) > original_size )
          {
            ALWAYS_ASSERT( itr.data->val == val );

            stl_map[ key ] = val;
          }
          else
            ALWAYS_ASSERT( itr.data->val == stl_map.find( key )->second );
        }
        break;
        case 2: // Get.
        {
          int key = rand() % ( N_OPS / 10 );
          integer_map_itr itr = integer_map_get( &our_map, key );
          if( !integer_map_is_end( itr ) )
            ALWAYS_ASSERT( itr.data->val == stl_map.find( key )->second );
          else
            ALWAYS_ASSERT( stl_map.find( key ) == stl_map.end() );
        }
        break;
        case 3: // Erase and erase itr.
        {
          if( rand() % 2 )
          {
            int key = rand() % ( N_OPS / 10 );
            ALWAYS_ASSERT( integer_map_erase( &our_map, key ) == (bool)stl_map.erase( key ) );
          }
          else
          {
            int key = rand() % ( N_OPS / 10 );
            integer_map_itr itr = integer_map_get( &our_map, key );
            if( !integer_map_is_end( itr ) )
              integer_map_erase_itr( &our_map, itr );
            
            stl_map.erase( key );
          }
        }
        break;
        case 4: // Reserve.
        {
          if( rand() % 2 ) // Reserve above current capacity.
            UNTIL_SUCCESS( integer_map_reserve( &our_map, integer_map_bucket_count( &our_map ) ) );
          else if( integer_map_bucket_count( &our_map ) * MAX_LOAD  >= 5 ) // Reserve below current capacity.
            UNTIL_SUCCESS( integer_map_reserve( &our_map, (size_t)( integer_map_bucket_count( &our_map ) * MAX_LOAD - 5
             ) ) );
        }
        break;
        case 5: // Shrink.
        {
          UNTIL_SUCCESS( integer_map_shrink( &our_map ) );
        }
        break;
        case 6: // Clone.
        {
          integer_map clone;
          UNTIL_SUCCESS( integer_map_init_clone( &clone, &our_map ) );
          integer_map_cleanup( &our_map );
          our_map = clone;
        }
        break;
      }
    }

    // Check our_map against unordered_map.
    ALWAYS_ASSERT( integer_map_size( &our_map ) == stl_map.size() );
    for(
      integer_map_itr itr = integer_map_first( &our_map );
      !integer_map_is_end( itr );
      itr = integer_map_next( itr )
    )
      ALWAYS_ASSERT( itr.data->val == stl_map.find( itr.data->key )->second );

    // Check unordered_map against our_map.
    for( auto i = stl_map.begin(); i != stl_map.end(); ++i )
      ALWAYS_ASSERT( integer_map_get( &our_map, i->first ).data->val == i->second );

    std::cout << "Done. Final size: " << integer_map_size( &our_map ) << "\n";
    integer_map_cleanup( &our_map );
  }

  // Set.
  for( int test = 0; test < N_TESTS; ++test )
  {
    std::cout << "Set test " << test << "... ";
    std::unordered_set<int> stl_set;
    integer_set our_set;
    integer_set_init( &our_set );

    for( int op = 0; op < N_OPS; ++op )
    {
      switch( rand() % 7 )
      {
        case 0: // Insert.
        {
          integer_set_itr itr;
          int key = rand() % ( N_OPS / 10 );
          UNTIL_SUCCESS( !integer_set_is_end( itr = integer_set_insert( &our_set, key ) ) );

          ALWAYS_ASSERT( itr.data->key == key );

          stl_set.insert( key );
        }
        break;
        case 1: // Get or insert.
        {
          integer_set_itr itr;
          int key = rand() % ( N_OPS / 10 );
          size_t original_size = integer_set_size( &our_set );
          UNTIL_SUCCESS( !integer_set_is_end( itr = integer_set_get_or_insert( &our_set, key ) ) );

          ALWAYS_ASSERT( itr.data->key == key );

          if( integer_set_size( &our_set ) > original_size )
          {
            ALWAYS_ASSERT( itr.data->key == key );

            stl_set.insert( key );
          }
          else
            ALWAYS_ASSERT( itr.data->key == *stl_set.find( key ) );
        }
        break;
        case 2: // Get.
        {
          int key = rand() % ( N_OPS / 10 );
          integer_set_itr itr = integer_set_get( &our_set, key );
          if( !integer_set_is_end( itr ) )
            ALWAYS_ASSERT( itr.data->key == *stl_set.find( key ) );
          else
            ALWAYS_ASSERT( stl_set.find( key ) == stl_set.end() );
        }
        break;
        case 3: // Erase and erase itr.
        {
          if( rand() % 2 )
          {
            int key = rand() % ( N_OPS / 10 );
            ALWAYS_ASSERT( integer_set_erase( &our_set, key ) == (bool)stl_set.erase( key ) );
          }
          else
          {
            int key = rand() % ( N_OPS / 10 );
            integer_set_itr itr = integer_set_get( &our_set, key );
            if( !integer_set_is_end( itr ) )
              integer_set_erase_itr( &our_set, itr );
            
            stl_set.erase( key );
          }
        }
        break;
        case 4: // Reserve.
        {
          if( rand() % 2 ) // Reserve above current capacity.
            UNTIL_SUCCESS( integer_set_reserve( &our_set, integer_set_bucket_count( &our_set ) ) );
          else if( integer_set_bucket_count( &our_set ) * MAX_LOAD  >= 5 ) // Reserve below current capacity.
            UNTIL_SUCCESS( integer_set_reserve( &our_set, (size_t)( integer_set_bucket_count( &our_set ) * MAX_LOAD - 5
              ) ) );
        }
        break;
        case 5: // Shrink.
        {
          UNTIL_SUCCESS( integer_set_shrink( &our_set ) );
        }
        break;
        case 6: // Clone.
        {
          integer_set clone;
          UNTIL_SUCCESS( integer_set_init_clone( &clone, &our_set ) );
          integer_set_cleanup( &our_set );
          our_set = clone;
        }
        break;
      }
    }

    // Check our_set against unordered_map.
    ALWAYS_ASSERT( integer_set_size( &our_set ) == stl_set.size() );
    for(
      integer_set_itr itr = integer_set_first( &our_set );
      !integer_set_is_end( itr );
      itr = integer_set_next( itr )
    )
      ALWAYS_ASSERT( itr.data->key == *stl_set.find( itr.data->key ) );

    // Check unordered_set against our_set.
    for( auto i = stl_set.begin(); i != stl_set.end(); ++i )
      ALWAYS_ASSERT( integer_set_get( &our_set, *i ).data->key == *i );

    std::cout << "Done. Final size: " << integer_set_size( &our_set ) << "\n";
    integer_set_cleanup( &our_set );
  }

  ALWAYS_ASSERT( oustanding_allocs.empty() );
  std::cout << "All done.\nSimulated allocation failures: " << simulated_alloc_failures << "\n";
}
