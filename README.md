# Benchmarks of Single Producer Multiple Consumer implementations

Comparison between several implementations of data exchange between a single producer and multiple consumers.

# Disclaimer

This code is written for educational purposes. It is not meant for production.

# Overview

A Single Producer Multiple Consumer (SPMC) design is often used in audio processing and other applications that process a stream of sensor data in soft real time systems. The performance of SPMC data structures is usually defined by the mechanism used to synchronize data between the producer and consumer threads. This repository compares several implementations of SPMC:

- _Baseline_ solution measures the time required to write and read data without any synchronization. To avoid undefined behaviour two memory buffers are used: one for reading and one for writing.

- _Exclusive locks_ using `std::mutex`. This is a practical, but suboptimal solution as only one thread has access to data at any given time. The producer (writer) and consumers (readers) have equal priority at obtaining the mutex.
- _Shared locks_ using `std::shared_mutex` which is an improvement of the previous solution. The writer has exclusive access to the memory while multiple consumers share the lock which enables concurrent read access.
- _SeqLocks_, a lock-free solution commonly used in financial applications. Synchronization is achieved with atomic counters. There is no blocking of the producer (writer) thread, while the readers check if the data is being written and retry if this is the case. It should be noted that this mechanism is incomplete and unless the data itself is atomic, race conditions still occur as the producer can potentially write into a section of memory which is being read by a consumer.
- _ZeroMQ inprocess_ provides an alternative mechanism for exchanging data between several threads.

As an additional optimization, the underlying data structure is implemented as a ring buffer.

# Methodology

Most benchmarks of lock-free data structures focus on the performance of the synchronization mechanism by itself. While this approach is valid for many applications especially in finance, it provides little insight into an overall performance of memory-bound systems where large volumes of data are being exchanged, for instance audio.

In this repository the benchmarks take into account the size of the data and the number of consumers. The length of the ring buffer measured in data blocks is another parameter.
