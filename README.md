# Fastmap

**Fastmap** is a generic hash table that brings the speed and memory efficiency of state-of-the-art C++ hash tables such as Abseil, Boost, and Bytell to C.

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
