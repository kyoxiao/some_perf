#ifndef SHM_RING_BUFFER_H
#define SHM_RING_BUFFER_H
#ifndef __cplusplus
#  include <stdatomic.h>
#else
#  include <atomic>
#  define _Atomic(X) std::atomic< X >
#endif
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string>
#include <memory.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <bits/stdc++.h>
#include "tscn.h"

using namespace std;
using std::string;
using std::atomic;

#define DEFAULT_MODIFIER 1000
#define SRB_NAME "shm_ring_buffer"
#define SRB_CAP 100
#define MAX_SHM_NAME_LEN 40
#define TSC_GHZ 3.0924892759999145

template <typename T>
class ShmRingBuffer {
public:
  std::string _shm_name;
  bool _master;
  size_t _capacity;
  size_t _shm_size;
  bool _hasts;

  typedef struct _ShmRingBufferHeader {
    char shm_name[MAX_SHM_NAME_LEN];
    size_t _capacity;
    int64_t px_modifier;
    int64_t sz_modifier;
    int64_t min_sz;
    std::atomic<int> nextEntry;
    std::atomic<int> lastRead;
    std::atomic<int> lastWrite;
  } ShmRingBufferHeader;

  ShmRingBufferHeader* _hdr;

  ShmRingBuffer(size_t cap = SRB_CAP, bool master = false, const char * name = SRB_NAME, bool usets = false):
     _shm_name(name),_master(master),_capacity(cap),_hasts(usets)
  {
    _shm_size = sizeof(struct _ShmRingBufferHeader) + _capacity * sizeof(T);
    init(cap, master, name);
  }

  ~ShmRingBuffer() {
    if(_hdr)
    {
      munmap((void *)_hdr, _shm_size);
      _data = nullptr;
    }
  }

  T * _data;

  TSCNS tn;

  ShmRingBuffer(const ShmRingBuffer<T> &);
  ShmRingBuffer<T>& operator=(const ShmRingBuffer<T>&);

  int32_t init(size_t cap, bool master, const char * path);

  int32_t create();
  int32_t open();
  int32_t clear();

  size_t capacity() const;
  int32_t last_write() const;
  int64_t getTimeNS();

  void setPxModifier(int64_t m);
  void setSzModifier(int64_t m);
  void setMinSize(int64_t m);
  int64_t getPxModifier();
  int64_t getSzModifier();
  int64_t getMinSize();

  T* read(int32_t idx);
  T* read_last();
  T* get_node(int32_t idx);
  T* get_next_write();
  void set_next_write(T* d);
  int32_t write(const T& e);

  inline int32_t getLastReadIdx();
  int32_t getNextReadIdx();
  int32_t getLastWriteIdx();
  int32_t getUnreadNum();
  void setLastReadIdx(int32_t idx);
  void setLastWriteIdx(int32_t idx);

  void printAll();
};

template <typename T> inline size_t
ShmRingBuffer<T>::capacity() const
{
  assert(_hdr != NULL);

  return _hdr->_capacity;
}

template <typename T> inline int
ShmRingBuffer<T>::last_write() const
{
  assert(_hdr != NULL);

  return _hdr->lastWrite;
}

template <typename T> inline int
ShmRingBuffer<T>::create()
{
  shm_unlink(_shm_name.c_str());

  umask(0); // To make sure the file creation
  int32_t fd = shm_open(_shm_name.c_str(), O_CREAT | O_EXCL | O_RDWR, 0777);

  if (fd < 0) {
    perror("shm_open()");
    return EXIT_FAILURE;
  }

  if (ftruncate(fd, _shm_size) < 0) {
    perror("fruncate()");
    return EXIT_FAILURE;
  }

  std::cout << "buffer size: " << _shm_size << std::endl;
  void* ptr = mmap(NULL, _shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  _hdr = reinterpret_cast<ShmRingBufferHeader*>(ptr);
  _hdr->_capacity = _capacity;
  _hdr->nextEntry = 0;
  _hdr->lastRead = 0;
  _hdr->lastWrite = 0;
  _hdr->px_modifier = DEFAULT_MODIFIER;
  _hdr->sz_modifier = DEFAULT_MODIFIER;

  std::copy(_shm_name.begin(), _shm_name.end(), _hdr->shm_name);
  _hdr->shm_name[_shm_name.size()] = '\0';

  _data = reinterpret_cast<T*>((char*)_hdr + sizeof(ShmRingBufferHeader));

  std::cout << "name: " << _shm_name << std::endl;
  std::cout << "mapped address: " << _hdr << std::endl;

  return EXIT_SUCCESS;
}

template <typename T> inline void
ShmRingBuffer<T>::setPxModifier(int64_t m)
{
  if(_hdr)
  {
    std::cout << "set px modifier: " << m << std::endl;
    _hdr->px_modifier = m;
  }
}

template <typename T> inline void
ShmRingBuffer<T>::setSzModifier(int64_t m)
{
  if(_hdr)
  {
    std::cout << "set sz modifier: " << m << std::endl;
    _hdr->sz_modifier = m;
  }
}

template <typename T> inline void
ShmRingBuffer<T>::setMinSize(int64_t m)
{
  if(_hdr)
  {
    std::cout << "set min size: " << m << std::endl;
    _hdr->min_sz = m;
  }
}

template <typename T> inline int64_t
ShmRingBuffer<T>::getPxModifier()
{
  if(_hdr)
  {
    return _hdr->px_modifier;
  }
  return DEFAULT_MODIFIER;
}

template <typename T> inline int64_t
ShmRingBuffer<T>::getSzModifier()
{
  if(_hdr)
  {
    return _hdr->sz_modifier;
  }
  return DEFAULT_MODIFIER;
}

template <typename T> inline int64_t
ShmRingBuffer<T>::getMinSize()
{
  if(_hdr)
  {
    return _hdr->min_sz;
  }
  return DEFAULT_MODIFIER;
}

template <typename T> inline int
ShmRingBuffer<T>::open()
{
  std::cout << "open: " << _shm_name << " size: " << _shm_size << std::endl;
  int32_t fd = shm_open(_shm_name.c_str(), O_RDWR, 0666);
  if (fd < 0) {
    perror("shm_open()");
    return EXIT_FAILURE;
  }

  if (ftruncate(fd, _shm_size) < 0) {
    perror("fruncate()");
    return EXIT_FAILURE;
  }

  void* ptr = mmap(NULL, _shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  _hdr = reinterpret_cast<ShmRingBufferHeader*>(ptr);
  _data = reinterpret_cast<T*>((char*)_hdr + sizeof(ShmRingBufferHeader));

  std::cout << "HEADER: " << std::endl;
  std::cout << "capacity: " << _hdr->_capacity << std::endl;
  std::cout << "px_modifier: " << _hdr->px_modifier << std::endl;
  std::cout << "sz_modifier: " << _hdr->sz_modifier << std::endl;

  return EXIT_SUCCESS;
}

template <typename T> inline int
ShmRingBuffer<T>::clear()
{
  _hdr->nextEntry = 0;
  _hdr->lastRead = 0;
  _hdr->lastWrite = 0;
  return EXIT_SUCCESS;
}

template <typename T> inline int
ShmRingBuffer<T>::getLastReadIdx()
{
  return atomic_load_explicit(&(_hdr->lastRead), std::memory_order_acquire);
}

template <typename T> inline int
ShmRingBuffer<T>::getNextReadIdx()
{
  return (atomic_load_explicit(&(_hdr->lastRead), std::memory_order_acquire) + 1) % _hdr->_capacity;
}

template <typename T> inline int
ShmRingBuffer<T>::getLastWriteIdx()
{
  return atomic_load_explicit(&(_hdr->lastWrite), std::memory_order_acquire);
}

template <typename T> inline int
ShmRingBuffer<T>::getUnreadNum()
{
  int32_t _lastW = atomic_load_explicit(&(_hdr->lastWrite), std::memory_order_relaxed);
  int32_t _lastR = atomic_load_explicit(&(_hdr->lastRead), std::memory_order_acquire);
  int32_t distance = _lastW - _lastR;
  return (distance >= 0) ? distance : (distance + _hdr->_capacity);
}

template <typename T> inline void
ShmRingBuffer<T>::setLastReadIdx(int32_t idx)
{
  //std::cout << "lastRead: " << idx << std::endl;
  atomic_store_explicit(&(_hdr->lastRead), idx, std::memory_order_release);
}

template <typename T> inline void
ShmRingBuffer<T>::setLastWriteIdx(int32_t idx)
{
  atomic_store_explicit(&(_hdr->lastWrite), idx, std::memory_order_release);
}

template<typename T> inline int
ShmRingBuffer<T>::init(size_t cap, bool master, const char* name)
{
  int32_t ret;
  if(master)
  {
    ret = create();
  } else {
    ret = open();
  }

  tn.init();
  int wait_sec = 1;
  std::cout << "waiting " << wait_sec << " secs for calibration"
      << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(wait_sec));
  double tsc_ghz = tn.calibrate();
  std::cout << std::setprecision(17) << "tsc_ghz: " << tsc_ghz << std::endl;
  std::cout << "offset: " << tn.rdoffset() << std::endl;

  return ret;
}

template<typename T> inline T*
ShmRingBuffer<T>::get_node(int32_t idx)
{
  T* _d = _data + idx;
  return _d;
}

template<typename T> inline T*
ShmRingBuffer<T>::read(int32_t idx)
{
  T* d = get_node(idx);
  return d;
}

template<typename T> inline T*
ShmRingBuffer<T>::read_last()
{
  int32_t idx = atomic_load_explicit(&(_hdr->lastWrite), std::memory_order_acquire);
  //std::cout << "read_last: last idx: " << idx << std::endl;
  T* d = get_node(idx);
  return d;
}

template<typename T> inline int
ShmRingBuffer<T>::write(const T& e)
{
  int32_t idx = atomic_load_explicit(&(_hdr->nextEntry), std::memory_order_acquire);
  T* d = get_node(idx);
  memcpy(d, &e, sizeof(e));
  d->idx = idx;
  atomic_store_explicit(&(_hdr->lastWrite), idx, std::memory_order_relaxed);
  int32_t _nextEntry = (idx + 1) % _hdr->_capacity;
  //std::cout << "Current idx: " << idx << ", Next idx: " << _nextEntry << std::endl;
  atomic_store_explicit(&(_hdr->nextEntry), _nextEntry, std::memory_order_release);

  return EXIT_SUCCESS;
}

template<typename T> inline T*
ShmRingBuffer<T>::get_next_write()
{
  int32_t idx = atomic_load_explicit(&(_hdr->nextEntry), std::memory_order_acquire);
  T* d = get_node(idx);

  return d;
}

template<typename T> inline void
ShmRingBuffer<T>::set_next_write(T* d)
{
  int32_t idx = d->idx;
  int32_t _nextEntry = (idx + 1) % _hdr->_capacity;
  // TODO we do not set time here to save time
  if (_hasts)
  {
    d->timestamp = getTimeNS();
  }
  //std::cout << "set_next_write:: Current idx: " << idx << ", Next idx: " << _nextEntry << std::endl;
  atomic_store_explicit(&(_hdr->lastWrite), idx, std::memory_order_relaxed);
  atomic_store_explicit(&(_hdr->nextEntry), _nextEntry, std::memory_order_release);
}

template<typename T> inline int64_t
ShmRingBuffer<T>::getTimeNS()
{
//return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  return tn.rdns();
}

template<typename T> inline void
ShmRingBuffer<T>::printAll()
{
  for(uint32_t i = 0; i < _capacity; i++)
  {
    T* d = get_node(i);
    std::cout << d->toString() << std::endl;
  }
}
#endif
