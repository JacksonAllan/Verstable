# Fastmap

Fastmap is a generic hash table intended to bring the speed and memory efficiency of state-of-the-art C++ hash tables such as Abseil, Boost, and Folly to C. Its features include:

* Type safety.
* High performance almost immune to high load factors.
* No tombstones.
* Customizable hash, comparison, and destructor functions.
* Single header.
* C99 compatibility.
* Generic API for C11 and later.
* Distributed under the MIT license.

Benchmarks comparing Fastmap to the aforementioned hash tables and several Robin Hood hash tables can be found here.
