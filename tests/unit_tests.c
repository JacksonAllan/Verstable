/*

Verstable - tests/unit_tests.c - v2.0.0

This file tests Verstable sets and maps.
It aims to cover the full functionality, via the C11 generic API, and to check corner cases.

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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Assert macro that is not disabled by NDEBUG.
#define ALWAYS_ASSERT( xp )                                                                               \
( (xp) ? (void)0 : ( fprintf( stderr, "Assertion failed at line %d: %s\n", __LINE__, #xp ), exit( 0 ) ) ) \

// Disable this macro to turn off failing realloc.
#define SIMULATE_ALLOC_FAILURES

// If realloc can fail, then we need a macro to repeat each call until it succeeds.
#ifdef SIMULATE_ALLOC_FAILURES
#define UNTIL_SUCCESS( xp ) while( !(xp) )
#else
#define UNTIL_SUCCESS( xp ) xp
#endif

// Custom malloc and free functions that track the number of outstanding allocations.
// If SIMULATE_ALLOC_FAILURES is defined above, the malloc function will also sporadically fail.

size_t simulated_alloc_failures = 0;
size_t oustanding_allocs = 0;

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

  ++oustanding_allocs;

  return ptr;
}

void tracking_free( void *ptr, size_t size )
{
  if( ptr )
    --oustanding_allocs;

  free( ptr );
}

// Custom malloc and free functions that make use of the ctx member.

typedef struct
{
  size_t id; // Used to check that the ctx member is correctly set and conveyed during rehashing.
  size_t alloc_size; // Used to check that the size that Verstable passes into the free function is the same as the size
                     // that was used for allocation.
} context;

void *unreliable_tracking_malloc_with_ctx( size_t size, context *ctx )
{
  void *ptr = unreliable_tracking_malloc( size );
  if( !ptr )
    return NULL;

  ctx->alloc_size = size;

  return ptr;
}

void tracking_free_with_ctx( void *ptr, size_t size, context *ctx )
{
  ALWAYS_ASSERT( ctx->alloc_size == size );
  tracking_free( ptr, size );
}

// Destructor function and array to track for which keys it has been called.

bool dtor_called[ 100 ];

void dtor( uint64_t key_or_val )
{
  dtor_called[ key_or_val ] = true;
}

void check_dtors_arr()
{
  for( size_t i = 0; i < 100; ++i )
  {
    ALWAYS_ASSERT( dtor_called[ i ] );
    dtor_called[ i ] = false;
  }
}

// Max load factor.
// Set to 1.0 to test correct handling of rehashing due to displacement limit violation.
#define GLOBAL_MAX_LOAD 0.95

// Instantiate hash table templates.

#define NAME      integer_map
#define KEY_TY    uint64_t
#define VAL_TY    uint64_t
#define MAX_LOAD  GLOBAL_MAX_LOAD
#define MALLOC_FN unreliable_tracking_malloc
#define FREE_FN   tracking_free
#include "../verstable.h"

#define NAME        integer_dtors_map
#define KEY_TY      uint64_t
#define VAL_TY      uint64_t
#define KEY_DTOR_FN dtor
#define VAL_DTOR_FN dtor
#define MAX_LOAD    GLOBAL_MAX_LOAD
#define MALLOC_FN   unreliable_tracking_malloc
#define FREE_FN     tracking_free
#include "../verstable.h"

#define NAME      string_map
#define KEY_TY    char *
#define VAL_TY    char *
#define MAX_LOAD  GLOBAL_MAX_LOAD
#define MALLOC_FN unreliable_tracking_malloc
#define FREE_FN   tracking_free
#include "../verstable.h"

#define NAME      integer_set
#define KEY_TY    uint64_t
#define MAX_LOAD  GLOBAL_MAX_LOAD
#define MALLOC_FN unreliable_tracking_malloc
#define FREE_FN   tracking_free
#include "../verstable.h"

#define NAME        integer_dtors_set
#define KEY_TY      uint64_t
#define KEY_DTOR_FN dtor
#define MAX_LOAD    GLOBAL_MAX_LOAD
#define MALLOC_FN   unreliable_tracking_malloc
#define FREE_FN     tracking_free
#include "../verstable.h"

#define NAME      string_set
#define KEY_TY    char *
#define MAX_LOAD  GLOBAL_MAX_LOAD
#define MALLOC_FN unreliable_tracking_malloc
#define FREE_FN   tracking_free
#include "../verstable.h"

#define NAME      integer_map_with_ctx
#define KEY_TY    uint64_t
#define VAL_TY    uint64_t
#define CTX_TY    context
#define MAX_LOAD  GLOBAL_MAX_LOAD
#define MALLOC_FN unreliable_tracking_malloc_with_ctx
#define FREE_FN   tracking_free_with_ctx
#include "../verstable.h"

#define NAME      integer_set_with_ctx
#define KEY_TY    uint64_t
#define CTX_TY    context
#define MAX_LOAD  GLOBAL_MAX_LOAD
#define MALLOC_FN unreliable_tracking_malloc_with_ctx
#define FREE_FN   tracking_free_with_ctx
#include "../verstable.h"

// Unit tests.

void test_map_reserve( void )
{
  integer_map our_map;
  vt_init( &our_map );

  // Reserve zero with placeholder.
  UNTIL_SUCCESS( vt_reserve( &our_map, 0 ) );
  ALWAYS_ASSERT( our_map.metadata == vt_placeholder_metadata_buffer );

  // Reserve up from placeholder.
  UNTIL_SUCCESS( vt_reserve( &our_map, 30 ) );
  ALWAYS_ASSERT( 30 <= vt_bucket_count( &our_map ) * GLOBAL_MAX_LOAD );

  // Reserve same capacity.
  size_t bucket_count = vt_bucket_count( &our_map );
  UNTIL_SUCCESS( vt_reserve( &our_map, 30 ) );
  ALWAYS_ASSERT( vt_bucket_count( &our_map ) == bucket_count );

  // Reserve up from non-placeholder.
  UNTIL_SUCCESS( vt_reserve( &our_map, 60 ) );
  ALWAYS_ASSERT( 60 <= vt_bucket_count( &our_map ) * GLOBAL_MAX_LOAD );

  // Reserve lower capacity.
  bucket_count = vt_bucket_count( &our_map );
  UNTIL_SUCCESS( vt_reserve( &our_map, 30 ) );
  ALWAYS_ASSERT( vt_bucket_count( &our_map ) == bucket_count );

  // Test validity through use.
  for( uint64_t i = 0; i < 60; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_map, i, i + 1 ) ) );

  // Check.
  ALWAYS_ASSERT( vt_size( &our_map ) == 60 );
  for( uint64_t i = 0; i < 60; ++i )
  {
    integer_map_itr itr = vt_get( &our_map, i );
    ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->val == i + 1 );
  }

  vt_cleanup( &our_map );
}

void test_map_shrink( void )
{
  integer_map our_map;
  vt_init( &our_map );

  // Test placeholder.
  UNTIL_SUCCESS( vt_shrink( &our_map ) );
  ALWAYS_ASSERT( vt_size( &our_map ) == 0 );
  ALWAYS_ASSERT( vt_bucket_count( &our_map ) == 0 );

  // Test restoration of placeholder.
  UNTIL_SUCCESS( vt_reserve( &our_map, 30 ) );
  UNTIL_SUCCESS( vt_shrink( &our_map ) );
  ALWAYS_ASSERT( vt_size( &our_map ) == 0 );
  ALWAYS_ASSERT( vt_bucket_count( &our_map ) == 0 );
  ALWAYS_ASSERT( our_map.metadata == vt_placeholder_metadata_buffer );

  // Test shrink same size.
  UNTIL_SUCCESS( vt_reserve( &our_map, 30 ) );
  for( uint64_t i = 0; i < 30; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_map, i, i + 1 ) ) );

  ALWAYS_ASSERT( vt_size( &our_map ) == 30 );
  void *buckets_ptr = (void *)our_map.buckets;
  size_t bucket_count = vt_bucket_count( &our_map );
  UNTIL_SUCCESS( vt_shrink( &our_map ) );
  ALWAYS_ASSERT( (void *)our_map.buckets == buckets_ptr );
  ALWAYS_ASSERT( vt_bucket_count( &our_map ) == bucket_count );

  // Test shrink down.
  UNTIL_SUCCESS( vt_reserve( &our_map, 500 ) );
  ALWAYS_ASSERT( vt_size( &our_map ) == 30 );
  ALWAYS_ASSERT( 500 <= vt_bucket_count( &our_map ) * GLOBAL_MAX_LOAD );
  UNTIL_SUCCESS( vt_shrink( &our_map ) );
  ALWAYS_ASSERT( vt_size( &our_map ) == 30 );
  ALWAYS_ASSERT( vt_bucket_count( &our_map ) == bucket_count );

  // Check.
  for( uint64_t i = 0; i < 30; ++i )
  {
    integer_map_itr itr = vt_get( &our_map, i );
    ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->val == i + 1 );
  }

  vt_cleanup( &our_map ); 
}

void test_map_insert( void )
{
  integer_map our_map;
  vt_init( &our_map );

  // Insert new.
  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_map_itr itr;
    UNTIL_SUCCESS( !vt_is_end( itr = vt_insert( &our_map, i, i + 1 ) ) );
    ALWAYS_ASSERT( itr.data->val == i + 1 );
  }

  // Insert existing.
  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_map_itr itr;
    UNTIL_SUCCESS( !vt_is_end( itr = vt_insert( &our_map, i, i + 2 ) ) );
    ALWAYS_ASSERT( itr.data->val == i + 2 );
  }

  // Check.
  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_map_itr itr;
    itr = vt_get( &our_map, i );
    ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->val == i + 2 );
  }

  vt_cleanup( &our_map );
}

void test_map_get_or_insert( void )
{
  integer_map our_map;
  vt_init( &our_map );

  // Test insert.
  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_map_itr itr;
    UNTIL_SUCCESS( !vt_is_end( itr = vt_get_or_insert( &our_map, i, i + 1 ) ) );
    ALWAYS_ASSERT( itr.data->val == i + 1 );
  }

  ALWAYS_ASSERT( vt_size( &our_map ) == 100 );
  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_map_itr itr = vt_get( &our_map, i );
    ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->val == i + 1 );
  }

  // Test get.
  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_map_itr itr_1 = vt_get( &our_map, i );
    ALWAYS_ASSERT( !vt_is_end( itr_1 ) );
    integer_map_itr itr_2;
    UNTIL_SUCCESS( !vt_is_end( itr_2 = vt_get_or_insert( &our_map, i, i + 1 ) ) );
    ALWAYS_ASSERT( itr_2.data == itr_1.data && itr_2.data->val == i + 1 );
  }

  ALWAYS_ASSERT( vt_size( &our_map ) == 100 );

  vt_cleanup( &our_map );
}

void test_map_get( void )
{
  integer_map our_map;
  vt_init( &our_map );

  // Test empty.
  for( uint64_t i = 0; i < 100; ++i )
    ALWAYS_ASSERT( vt_is_end( vt_get( &our_map, i ) ) );

  // Test get existing.
  for( uint64_t i = 0; i < 100; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_map, i, i + 1 ) ) );

  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_map_itr itr = vt_get( &our_map, i );
    ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->val == i + 1 );
  }

  // Test get non-existing.
  for( uint64_t i = 100; i < 200; ++i )
    ALWAYS_ASSERT( vt_is_end( vt_get( &our_map, i ) ) );

  vt_cleanup( &our_map );
}

void test_map_erase( void )
{
  integer_map our_map;
  vt_init( &our_map );

  // Test erase existing.
  for( uint64_t i = 0; i < 100; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_map, i, i + 1 ) ) );

  ALWAYS_ASSERT( vt_size( &our_map ) == 100 );

  for( uint64_t i = 0; i < 100; i += 2 )
    ALWAYS_ASSERT( vt_erase( &our_map, i ) );

  // Test erase non-existing.
  for( uint64_t i = 0; i < 100; i += 2 )
    ALWAYS_ASSERT( !vt_erase( &our_map, i ) );

  // Check.
  ALWAYS_ASSERT( vt_size( &our_map ) == 50 );
  for( uint64_t i = 0; i < 100; ++i )
  {
    if( i % 2 == 0 )
      ALWAYS_ASSERT( vt_is_end( vt_get( &our_map, i ) ) );
    else
    {
      integer_map_itr itr = vt_get( &our_map, i );
      ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->val == i + 1 );
    }
  }

  vt_cleanup( &our_map );
}

void test_map_erase_itr( void )
{
  integer_map our_map;
  vt_init( &our_map );

  // In this instance, the key count and order of insert have been carefully chosen to cause skipped or repeat-visted
  // keys if vt_erase_itr does not correctly handle the case of another key being moved to the bucket of the erased key.
  for( int i = 119; i >= 0; --i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_map, i, i + 1 ) ) );

  ALWAYS_ASSERT( vt_size( &our_map ) == 120 );

  // Test with iterator from get.
  for( uint64_t i = 0; i < 120; i += 4 )
    vt_erase_itr( &our_map, vt_get( &our_map, i ) );

  // Check.
  ALWAYS_ASSERT( vt_size( &our_map ) == 90 );
  for( uint64_t i = 0; i < 120; ++i )
  {
    if( i % 4 == 0 )
      ALWAYS_ASSERT( vt_is_end( vt_get( &our_map, i ) ) );
    else
    {
      integer_map_itr itr = vt_get( &our_map, i );
      ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->val == i + 1 );
    }
  }

  // Test deletion while iterating.
  integer_map_itr itr = vt_first( &our_map );
  size_t n_iterations = 0;
  while( !vt_is_end( itr ) )
  {
    ++n_iterations;

    if( itr.data->key % 2 == 0 )
      itr = vt_erase_itr( &our_map, itr );
    else
      itr = vt_next( itr );
  }

  ALWAYS_ASSERT( n_iterations == 90 );
  ALWAYS_ASSERT( vt_size( &our_map ) == 60 );

  for( uint64_t i = 0; i < 120; ++i )
  {
    if( i % 2 == 0 )
      ALWAYS_ASSERT( vt_is_end( vt_get( &our_map, i ) ) );
    else
    {
      integer_map_itr itr = vt_get( &our_map, i );
      ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->val == i + 1 );
    }
  }

  vt_cleanup( &our_map );
}

void test_map_clear( void )
{
  integer_map our_map;
  vt_init( &our_map );

  // Test empty.
  vt_clear( &our_map );
  ALWAYS_ASSERT( vt_size( &our_map ) == 0 );

  // Test non-empty;
  for( uint64_t i = 0; i < 100; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_map, i, i + 1 ) ) );

  vt_clear( &our_map );
  ALWAYS_ASSERT( vt_size( &our_map ) == 0 );
  for( uint64_t i = 0; i < 100; ++i )
    ALWAYS_ASSERT( vt_is_end( vt_get( &our_map, i ) ) );

  // Test reuse.
  for( uint64_t i = 0; i < 100; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_map, i, i + 1 ) ) );

  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_map_itr itr = vt_get( &our_map, i );
    ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->val == i + 1 );
  }

  vt_cleanup( &our_map );
}

void test_map_cleanup( void )
{
  integer_map our_map;
  vt_init( &our_map );

  // Empty.
  vt_cleanup( &our_map );
  ALWAYS_ASSERT( our_map.metadata == vt_placeholder_metadata_buffer );

  // Non-empty.
  for( uint64_t i = 0; i < 100; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_map, i, i + 1 ) ) );

  ALWAYS_ASSERT( vt_size( &our_map ) == 100 );
  vt_cleanup( &our_map );
  ALWAYS_ASSERT( vt_size( &our_map ) == 0 );
  ALWAYS_ASSERT( our_map.metadata == vt_placeholder_metadata_buffer );

  // Test use.
  for( uint64_t i = 0; i < 100; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_map, i, i + 1 ) ) );
  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_map_itr itr = vt_get( &our_map, i );
    ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->val == i + 1 );
  }

  vt_cleanup( &our_map );
}

void test_map_init_clone( void )
{
  integer_map src_map;
  vt_init( &src_map );

  // Test init_clone placeholder.
  integer_map empty_map;
  UNTIL_SUCCESS( vt_init_clone( &empty_map, &src_map ) );
  ALWAYS_ASSERT( empty_map.metadata == vt_placeholder_metadata_buffer );

  // Test init_clone non-placeholder.
  integer_map our_map;
  for( uint64_t i = 0; i < 10; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &src_map, i, i + 1 ) ) );
  UNTIL_SUCCESS( vt_init_clone( &our_map, &src_map ) );

  // Check.
  ALWAYS_ASSERT( vt_size( &our_map ) == 10 );
  for( uint64_t i = 0; i < 10; ++i )
  {
    integer_map_itr itr = vt_get( &our_map, i );
    ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->val == i + 1 );
  }

  vt_cleanup( &src_map );
  vt_cleanup( &empty_map );
  vt_cleanup( &our_map );
}

void test_map_iteration( void )
{
  integer_map our_map;
  vt_init( &our_map );

  // Empty.

  // Test fist.
  ALWAYS_ASSERT( vt_is_end( vt_first( &our_map ) ) );

  size_t n_iterations = 0;

  for(
    integer_map_itr itr = vt_first( &our_map );
    !vt_is_end( itr );
    itr = vt_next( itr )
  )
    ++n_iterations;

  ALWAYS_ASSERT( n_iterations == 0 );

  // Non-empty.
  for( uint64_t i = 0; i < 30; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_map, i, i + 1 ) ) );

  for(
    integer_map_itr itr = vt_first( &our_map );
    !vt_is_end( itr );
    itr = vt_next( itr )
  )
    ++n_iterations;

  ALWAYS_ASSERT( n_iterations == 30 );

  vt_cleanup( &our_map );
}

void test_map_dtors( void )
{
  integer_dtors_map our_map;
  integer_dtors_map_init( &our_map );

  // Test erase and clear.

  for( uint64_t i = 0; i < 50; ++i )
    UNTIL_SUCCESS( !integer_dtors_map_is_end( integer_dtors_map_insert( &our_map, i, i + 50 ) ) );

  for( uint64_t i = 0; i < 50; i += 2 )
    integer_dtors_map_erase( &our_map, i );

  integer_dtors_map_clear( &our_map );

  check_dtors_arr();

  // Test replace.

  for( uint64_t i = 0; i < 50; ++i )
    UNTIL_SUCCESS( !integer_dtors_map_is_end( integer_dtors_map_insert( &our_map, i, i + 50 ) ) );

  for( uint64_t i = 0; i < 50; ++i )
    UNTIL_SUCCESS( !integer_dtors_map_is_end( integer_dtors_map_insert( &our_map, i, i + 50 ) ) );

  check_dtors_arr();
  integer_dtors_map_clear( &our_map );

  // Test cleanup.

  for( uint64_t i = 0; i < 50; ++i )
    UNTIL_SUCCESS( !integer_dtors_map_is_end( integer_dtors_map_insert( &our_map, i, i + 50 ) ) );

  integer_dtors_map_cleanup( &our_map );
  check_dtors_arr();
}

// Strings are a special case that warrant seperate testing.
void test_map_strings( void )
{
  string_map our_map;
  vt_init( &our_map );

  string_map_itr itr;

  // String literals.
  UNTIL_SUCCESS( !vt_is_end( itr = vt_insert( &our_map, "This", "is" ) ) );
  ALWAYS_ASSERT( strcmp( itr.data->val, "is" ) == 0 );
  UNTIL_SUCCESS( !vt_is_end( itr = vt_get_or_insert( &our_map, "a", "test" ) ) );
  ALWAYS_ASSERT( strcmp( itr.data->val, "test" ) == 0 );

  // Other strings.
  char str_1[] = "of";
  char str_2[] = "maps";
  char str_3[] = "with";
  char str_4[] = "strings.";

  UNTIL_SUCCESS( !vt_is_end( itr = vt_insert( &our_map, str_1, str_2 ) ) );
  ALWAYS_ASSERT( strcmp( itr.data->val, str_2 ) == 0 );
  UNTIL_SUCCESS( !vt_is_end( itr = vt_get_or_insert( &our_map, str_3, str_4 ) ) );
  ALWAYS_ASSERT( strcmp( itr.data->val, str_4 ) == 0 );

  // Check.
  ALWAYS_ASSERT( vt_size( &our_map ) == 4 );
  ALWAYS_ASSERT( strcmp( vt_get( &our_map, "This" ).data->val, "is" ) == 0 );
  ALWAYS_ASSERT( strcmp( vt_get( &our_map, "a" ).data->val, "test" ) == 0 );
  UNTIL_SUCCESS( !vt_is_end( itr = vt_insert( &our_map, str_1, str_2 ) ) );
  ALWAYS_ASSERT( strcmp( itr.data->val, str_2 ) == 0 );
  UNTIL_SUCCESS( !vt_is_end( itr = vt_insert( &our_map, str_3, str_4 ) ) );
  ALWAYS_ASSERT( strcmp( itr.data->val, str_4 ) == 0 );
  ALWAYS_ASSERT( vt_size( &our_map ) == 4 );

  // Erase.
  vt_erase( &our_map, "This" );
  vt_erase( &our_map, str_1 );
  ALWAYS_ASSERT( vt_size( &our_map ) == 2 );

  // Iteration.
  for(
    itr = vt_first( &our_map );
    !vt_is_end( itr );
    itr = vt_next( itr )
  )
    ALWAYS_ASSERT( strcmp( itr.data->val, "test" ) == 0 || strcmp( itr.data->val, str_4 ) == 0 );

  vt_cleanup( &our_map );
}

void test_map_with_ctx( void )
{
  integer_map_with_ctx our_maps[ 10 ];
  integer_map_with_ctx our_map_clones[ 10 ];

  for( size_t i = 0; i < 10; ++i )
  {
    // Initializing the ctx member on init.
    context ctx = { i, 0 };
    vt_init( &our_maps[ i ], ctx );
    ALWAYS_ASSERT( our_maps[ i ].ctx.id == i );

    // Conveying ctx during rehashes.

    for( size_t j = 0; j < 100; ++j )
      UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_maps[ i ], j, 0 ) ) );

    for( size_t j = 0; j < 50; ++j )
      vt_erase( &our_maps[ i ], j );

    UNTIL_SUCCESS( vt_shrink( &our_maps[ i ] ) );
    
    ALWAYS_ASSERT( our_maps[ i ].ctx.id == i );

    // Initializing the ctx member on init_clone.

    UNTIL_SUCCESS( vt_init_clone( &our_map_clones[ i ], &our_maps[ i ], our_maps[ i ].ctx ) );
    ALWAYS_ASSERT( our_map_clones[ i ].ctx.id == i );

    for( size_t j = 50; j < 100; ++j )
      UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_map_clones[ i ], j, 0 ) ) );

    vt_cleanup( &our_maps[ i ] );
    vt_cleanup( &our_map_clones[ i ] );
  }
}

// Set tests.

void test_set_reserve( void )
{
  integer_set our_set;
  vt_init( &our_set );

  // Reserve zero with placeholder.
  UNTIL_SUCCESS( vt_reserve( &our_set, 0 ) );
  ALWAYS_ASSERT( our_set.metadata == vt_placeholder_metadata_buffer );

  // Reserve up from placeholder.
  UNTIL_SUCCESS( vt_reserve( &our_set, 30 ) );
  ALWAYS_ASSERT( 30 <= vt_bucket_count( &our_set ) * GLOBAL_MAX_LOAD );

  // Reserve same capacity.
  size_t bucket_count = vt_bucket_count( &our_set );
  UNTIL_SUCCESS( vt_reserve( &our_set, 30 ) );
  ALWAYS_ASSERT( vt_bucket_count( &our_set ) == bucket_count );

  // Reserve up from non-placeholder.
  UNTIL_SUCCESS( vt_reserve( &our_set, 60 ) );
  ALWAYS_ASSERT( 60 <= vt_bucket_count( &our_set ) * GLOBAL_MAX_LOAD );

  // Reserve lower capacity.
  bucket_count = vt_bucket_count( &our_set );
  UNTIL_SUCCESS( vt_reserve( &our_set, 30 ) );
  ALWAYS_ASSERT( vt_bucket_count( &our_set ) == bucket_count );

  // Test validity through use.
  for( uint64_t i = 0; i < 60; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_set, i ) ) );

  // Check.
  ALWAYS_ASSERT( vt_size( &our_set ) == 60 );
  for( uint64_t i = 0; i < 60; ++i )
  {
    integer_set_itr itr = vt_get( &our_set, i );
    ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->key == i );
  }

  vt_cleanup( &our_set );
}

void test_set_shrink( void )
{
  integer_set our_set;
  vt_init( &our_set );

  // Test placeholder.
  UNTIL_SUCCESS( vt_shrink( &our_set ) );
  ALWAYS_ASSERT( vt_size( &our_set ) == 0 );
  ALWAYS_ASSERT( vt_bucket_count( &our_set ) == 0 );

  // Test restoration of placeholder.
  UNTIL_SUCCESS( vt_reserve( &our_set, 30 ) );
  UNTIL_SUCCESS( vt_shrink( &our_set ) );
  ALWAYS_ASSERT( vt_size( &our_set ) == 0 );
  ALWAYS_ASSERT( vt_bucket_count( &our_set ) == 0 );
  ALWAYS_ASSERT( our_set.metadata == vt_placeholder_metadata_buffer );

  // Test shrink same size.
  UNTIL_SUCCESS( vt_reserve( &our_set, 30 ) );
  for( uint64_t i = 0; i < 30; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_set, i ) ) );

  ALWAYS_ASSERT( vt_size( &our_set ) == 30 );
  void *buckets_ptr = (void *)our_set.buckets;
  size_t bucket_count = vt_bucket_count( &our_set );
  UNTIL_SUCCESS( vt_shrink( &our_set ) );
  ALWAYS_ASSERT( (void *)our_set.buckets == buckets_ptr );
  ALWAYS_ASSERT( vt_bucket_count( &our_set ) == bucket_count );

  // Test shrink down.
  UNTIL_SUCCESS( vt_reserve( &our_set, 500 ) );
  ALWAYS_ASSERT( vt_size( &our_set ) == 30 );
  ALWAYS_ASSERT( 500 <= vt_bucket_count( &our_set ) * GLOBAL_MAX_LOAD );
  UNTIL_SUCCESS( vt_shrink( &our_set ) );
  ALWAYS_ASSERT( vt_size( &our_set ) == 30 );
  ALWAYS_ASSERT( vt_bucket_count( &our_set ) == bucket_count );

  // Check.
  for( uint64_t i = 0; i < 30; ++i )
  {
    integer_set_itr itr = vt_get( &our_set, i );
    ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->key == i );
  }

  vt_cleanup( &our_set ); 
}

void test_set_insert( void )
{
  integer_set our_set;
  vt_init( &our_set );

  // Insert new.
  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_set_itr itr;
    UNTIL_SUCCESS( !vt_is_end( itr = vt_insert( &our_set, i ) ) );
    ALWAYS_ASSERT( itr.data->key == i );
  }

  // Insert existing.
  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_set_itr itr;
    UNTIL_SUCCESS( !vt_is_end( itr = vt_insert( &our_set, i) ) );
    ALWAYS_ASSERT( itr.data->key == i );
  }

  // Check.
  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_set_itr itr;
    itr = vt_get( &our_set, i );
    ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->key == i );
  }

  vt_cleanup( &our_set );
}

void test_set_get_or_insert( void )
{
  integer_set our_set;
  vt_init( &our_set );

  // Test insert.
  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_set_itr itr;
    UNTIL_SUCCESS( !vt_is_end( itr = vt_get_or_insert( &our_set, i ) ) );
    ALWAYS_ASSERT( itr.data->key == i );
  }

  ALWAYS_ASSERT( vt_size( &our_set ) == 100 );
  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_set_itr itr = vt_get( &our_set, i );
    ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->key == i );
  }

  // Test get.
  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_set_itr itr_1 = vt_get( &our_set, i );
    ALWAYS_ASSERT( !vt_is_end( itr_1 ) );
    integer_set_itr itr_2;
    UNTIL_SUCCESS( !vt_is_end( itr_2 = vt_get_or_insert( &our_set, i ) ) );
    ALWAYS_ASSERT( itr_2.data == itr_1.data && itr_2.data->key == i );
  }

  ALWAYS_ASSERT( vt_size( &our_set ) == 100 );

  vt_cleanup( &our_set );
}

void test_set_get( void )
{
  integer_set our_set;
  vt_init( &our_set );

  // Test empty.
  for( uint64_t i = 0; i < 100; ++i )
    ALWAYS_ASSERT( vt_is_end( vt_get( &our_set, i ) ) );

  // Test get existing.
  for( uint64_t i = 0; i < 100; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_set, i ) ) );

  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_set_itr itr = vt_get( &our_set, i );
    ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->key == i );
  }

  // Test get non-existing.
  for( uint64_t i = 100; i < 200; ++i )
    ALWAYS_ASSERT( vt_is_end( vt_get( &our_set, i ) ) );

  vt_cleanup( &our_set );
}

void test_set_erase( void )
{
  integer_set our_set;
  vt_init( &our_set );

  // Test erase existing.
  for( uint64_t i = 0; i < 100; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_set, i ) ) );

  ALWAYS_ASSERT( vt_size( &our_set ) == 100 );

  for( uint64_t i = 0; i < 100; i += 2 )
    ALWAYS_ASSERT( vt_erase( &our_set, i ) );

  // Test erase non-existing.
  for( uint64_t i = 0; i < 100; i += 2 )
    ALWAYS_ASSERT( !vt_erase( &our_set, i ) );

  // Check.
  ALWAYS_ASSERT( vt_size( &our_set ) == 50 );
  for( uint64_t i = 0; i < 100; ++i )
  {
    if( i % 2 == 0 )
      ALWAYS_ASSERT( vt_is_end( vt_get( &our_set, i ) ) );
    else
    {
      integer_set_itr itr = vt_get( &our_set, i );
      ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->key == i );
    }
  }

  vt_cleanup( &our_set );
}

void test_set_erase_itr( void )
{
  integer_set our_set;
  vt_init( &our_set );

  // In this instance, the key count and order of insert have been carefully chosen to cause skipped or repeat-visted
  // keys if vt_erase_itr does not correctly handle the case of another key being moved to the bucket of the erased key.
  for( int i = 119; i >= 0; --i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_set, i ) ) );

  ALWAYS_ASSERT( vt_size( &our_set ) == 120 );

  // Test with iterator from get.
  for( uint64_t i = 0; i < 120; i += 4 )
    vt_erase_itr( &our_set, vt_get( &our_set, i ) );

  // Check.
  ALWAYS_ASSERT( vt_size( &our_set ) == 90 );
  for( uint64_t i = 0; i < 120; ++i )
  {
    if( i % 4 == 0 )
      ALWAYS_ASSERT( vt_is_end( vt_get( &our_set, i ) ) );
    else
    {
      integer_set_itr itr = vt_get( &our_set, i );
      ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->key == i );
    }
  }

  // Test deletion while iterating.
  integer_set_itr itr = vt_first( &our_set );
  size_t n_iterations = 0;
  while( !vt_is_end( itr ) )
  {
    ++n_iterations;

    if( itr.data->key % 2 == 0 )
      itr = vt_erase_itr( &our_set, itr );
    else
      itr = vt_next( itr );
  }

  ALWAYS_ASSERT( n_iterations == 90 );
  ALWAYS_ASSERT( vt_size( &our_set ) == 60 );

  for( uint64_t i = 0; i < 120; ++i )
  {
    if( i % 2 == 0 )
      ALWAYS_ASSERT( vt_is_end( vt_get( &our_set, i ) ) );
    else
    {
      integer_set_itr itr = vt_get( &our_set, i );
      ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->key == i );
    }
  }

  vt_cleanup( &our_set );
}

void test_set_clear( void )
{
  integer_set our_set;
  vt_init( &our_set );

  // Test empty.
  vt_clear( &our_set );
  ALWAYS_ASSERT( vt_size( &our_set ) == 0 );

  // Test non-empty;
  for( uint64_t i = 0; i < 100; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_set, i ) ) );

  vt_clear( &our_set );
  ALWAYS_ASSERT( vt_size( &our_set ) == 0 );
  for( uint64_t i = 0; i < 100; ++i )
    ALWAYS_ASSERT( vt_is_end( vt_get( &our_set, i ) ) );

  // Test reuse.
  for( uint64_t i = 0; i < 100; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_set, i ) ) );

  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_set_itr itr = vt_get( &our_set, i );
    ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->key == i );
  }

  vt_cleanup( &our_set );
}

void test_set_cleanup( void )
{
  integer_set our_set;
  vt_init( &our_set );

  // Empty.
  vt_cleanup( &our_set );
  ALWAYS_ASSERT( our_set.metadata == vt_placeholder_metadata_buffer );

  // Non-empty.
  for( uint64_t i = 0; i < 100; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_set, i ) ) );

  ALWAYS_ASSERT( vt_size( &our_set ) == 100 );
  vt_cleanup( &our_set );
  ALWAYS_ASSERT( vt_size( &our_set ) == 0 );
  ALWAYS_ASSERT( our_set.metadata == vt_placeholder_metadata_buffer );

  // Test use.
  for( uint64_t i = 0; i < 100; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_set, i ) ) );
  for( uint64_t i = 0; i < 100; ++i )
  {
    integer_set_itr itr = vt_get( &our_set, i );
    ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->key == i );
  }

  vt_cleanup( &our_set );
}

void test_set_init_clone( void )
{
  integer_set src_set;
  vt_init( &src_set );

  // Test init_clone placeholder.
  integer_set empty_set;
  UNTIL_SUCCESS( vt_init_clone( &empty_set, &src_set ) );
  ALWAYS_ASSERT( empty_set.metadata == vt_placeholder_metadata_buffer );

  // Test init_clone non-placeholder.
  integer_set our_set;
  for( uint64_t i = 0; i < 10; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &src_set, i ) ) );
  UNTIL_SUCCESS( vt_init_clone( &our_set, &src_set ) );

  // Check.
  ALWAYS_ASSERT( vt_size( &our_set ) == 10 );
  for( uint64_t i = 0; i < 10; ++i )
  {
    integer_set_itr itr = vt_get( &our_set, i );
    ALWAYS_ASSERT( !vt_is_end( itr ) && itr.data->key == i );
  }

  vt_cleanup( &src_set );
  vt_cleanup( &empty_set );
  vt_cleanup( &our_set );
}

void test_set_iteration( void )
{
  integer_set our_set;
  vt_init( &our_set );

  // Empty.

  // Test fist.
  ALWAYS_ASSERT( vt_is_end( vt_first( &our_set ) ) );

  size_t n_iterations = 0;

  for(
    integer_set_itr itr = vt_first( &our_set );
    !vt_is_end( itr );
    itr = vt_next( itr )
  )
    ++n_iterations;

  ALWAYS_ASSERT( n_iterations == 0 );

  // Non-empty.
  for( uint64_t i = 0; i < 30; ++i )
    UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_set, i ) ) );

  for(
    integer_set_itr itr = vt_first( &our_set );
    !vt_is_end( itr );
    itr = vt_next( itr )
  )
    ++n_iterations;

  ALWAYS_ASSERT( n_iterations == 30 );

  vt_cleanup( &our_set );
}

void test_set_dtors( void )
{
  integer_dtors_set our_set;
  integer_dtors_set_init( &our_set );

  // Test erase and clear.

  for( uint64_t i = 0; i < 100; ++i )
    UNTIL_SUCCESS( !integer_dtors_set_is_end( integer_dtors_set_insert( &our_set, i ) ) );

  for( uint64_t i = 0; i < 100; i += 2 ) // TODO: Fix in CC
    integer_dtors_set_erase( &our_set, i );

  integer_dtors_set_clear( &our_set );

  check_dtors_arr();

  // Test replace.

  for( uint64_t i = 0; i < 100; ++i )
    UNTIL_SUCCESS( !integer_dtors_set_is_end( integer_dtors_set_insert( &our_set, i ) ) );

  for( uint64_t i = 0; i < 100; ++i )
    UNTIL_SUCCESS( !integer_dtors_set_is_end( integer_dtors_set_insert( &our_set, i ) ) );

  check_dtors_arr();
  integer_dtors_set_clear( &our_set );

  // Test cleanup.

  for( uint64_t i = 0; i < 100; ++i )
    UNTIL_SUCCESS( !integer_dtors_set_is_end( integer_dtors_set_insert( &our_set, i ) ) );

  integer_dtors_set_cleanup( &our_set );
  check_dtors_arr();
}

void test_set_strings( void )
{
  string_set our_set;
  vt_init( &our_set );

  string_set_itr itr;

  // String literals.
  UNTIL_SUCCESS( !vt_is_end( itr = vt_insert( &our_set, "This" ) ) );
  ALWAYS_ASSERT( strcmp( itr.data->key, "This" ) == 0 );
  UNTIL_SUCCESS( !vt_is_end( itr = vt_insert( &our_set, "is" ) ) );
  ALWAYS_ASSERT( strcmp( itr.data->key, "is" ) == 0 );
  UNTIL_SUCCESS( !vt_is_end( itr = vt_insert( &our_set, "a" ) ) );
  ALWAYS_ASSERT( strcmp( itr.data->key, "a" ) == 0 );
  UNTIL_SUCCESS( !vt_is_end( itr = vt_insert( &our_set, "test" ) ) );
  ALWAYS_ASSERT( strcmp( itr.data->key, "test" ) == 0 );

  // Other strings.
  char str_1[] = "of";
  char str_2[] = "sets";
  char str_3[] = "with";
  char str_4[] = "strings";

  UNTIL_SUCCESS( !vt_is_end( itr = vt_insert( &our_set, str_1 ) ) );
  ALWAYS_ASSERT( strcmp( itr.data->key, str_1 ) == 0 );
  UNTIL_SUCCESS( !vt_is_end( itr = vt_insert( &our_set, str_2 ) ) );
  ALWAYS_ASSERT( strcmp( itr.data->key, str_2 ) == 0 );
  UNTIL_SUCCESS( !vt_is_end( itr = vt_insert( &our_set, str_3 ) ) );
  ALWAYS_ASSERT( strcmp( itr.data->key, str_3 ) == 0 );
  UNTIL_SUCCESS( !vt_is_end( itr = vt_insert( &our_set, str_4 ) ) );
  ALWAYS_ASSERT( strcmp( itr.data->key, str_4 ) == 0 );

  // Check.
  ALWAYS_ASSERT( vt_size( &our_set ) == 8 );
  ALWAYS_ASSERT( strcmp( vt_get( &our_set, "This" ).data->key, "This" ) == 0 );
  ALWAYS_ASSERT( strcmp( vt_get( &our_set, "is" ).data->key, "is" ) == 0 );
  ALWAYS_ASSERT( strcmp( vt_get( &our_set, "a" ).data->key, "a" ) == 0 );
  ALWAYS_ASSERT( strcmp( vt_get( &our_set, "test" ).data->key, "test" ) == 0 );
  ALWAYS_ASSERT( strcmp( vt_get( &our_set, "of" ).data->key, str_1 ) == 0 );
  ALWAYS_ASSERT( strcmp( vt_get( &our_set, "sets" ).data->key, str_2 ) == 0 );
  ALWAYS_ASSERT( strcmp( vt_get( &our_set, "with" ).data->key, str_3 ) == 0 );
  ALWAYS_ASSERT( strcmp( vt_get( &our_set, "strings" ).data->key, str_4 ) == 0 );

  vt_cleanup( &our_set );
}

void test_set_with_ctx( void )
{
  integer_set_with_ctx our_sets[ 10 ];
  integer_set_with_ctx our_set_clones[ 10 ];

  for( size_t i = 0; i < 10; ++i )
  {
    // Initializing the ctx member on init.
    context ctx = { i, 0 };
    vt_init( &our_sets[ i ], ctx );
    ALWAYS_ASSERT( our_sets[ i ].ctx.id == i );

    // Conveying ctx during rehashes.

    for( size_t j = 0; j < 100; ++j )
      UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_sets[ i ], j ) ) );

    for( size_t j = 0; j < 50; ++j )
      vt_erase( &our_sets[ i ], j );

    UNTIL_SUCCESS( vt_shrink( &our_sets[ i ] ) );
    
    ALWAYS_ASSERT( our_sets[ i ].ctx.id == i );

    // Initializing the ctx member on init_clone.

    UNTIL_SUCCESS( vt_init_clone( &our_set_clones[ i ], &our_sets[ i ], our_sets[ i ].ctx ) );
    ALWAYS_ASSERT( our_set_clones[ i ].ctx.id == i );

    for( size_t j = 50; j < 100; ++j )
      UNTIL_SUCCESS( !vt_is_end( vt_insert( &our_set_clones[ i ], j ) ) );

    vt_cleanup( &our_sets[ i ] );
    vt_cleanup( &our_set_clones[ i ] );
  }
}

int main( void )
{
  srand( (unsigned int)time( NULL ) );

  // Repeat 1000 times since realloc failures are random.
  for( int i = 0; i < 1000; ++i )
  {
    // init, bucket_count, and size tested implicitly.

    // Map.
    test_map_reserve();
    test_map_shrink();
    test_map_insert();
    test_map_get_or_insert();
    test_map_get();
    test_map_erase();
    test_map_erase_itr();
    test_map_clear();
    test_map_cleanup();
    test_map_init_clone();
    test_map_iteration();
    test_map_dtors();
    test_map_strings();
    test_map_with_ctx();

    // Set.
    test_set_reserve();
    test_set_shrink();
    test_set_insert();
    test_set_get_or_insert();
    test_set_get();
    test_set_erase();
    test_set_erase_itr();
    test_set_clear();
    test_set_cleanup();
    test_set_init_clone();
    test_set_iteration();
    test_set_dtors();
    test_set_strings();
    test_set_with_ctx();
  }

  ALWAYS_ASSERT( oustanding_allocs == 0 );
  printf( "All done.\n" );
  printf( "Simulated realloc failures: %zu\n", simulated_alloc_failures );
}
