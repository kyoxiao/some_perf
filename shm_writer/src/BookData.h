#ifndef BOOK_DATA_H
#define BOOK_DATA_H
#ifndef __cplusplus
#  include <stdatomic.h>
#else
#  include <atomic>
#  define _Atomic(X) std::atomic< X >
#endif
#define LEVEL_SIZE 5
#define SYMBOL_SIZE 24 
#define BID_IDX 0
#define ASK_IDX 1
#define BIDSIZE_IDX 2
#define ASKSIZE_IDX 3
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cstring>
using std::atomic;

struct BookData {
  int64_t timestamp;
  int32_t idx;
  int64_t quote[LEVEL_SIZE * 4];
  char instrument[SYMBOL_SIZE];
  int64_t oi;
  int64_t tp;
  int64_t tn;
  int64_t tv;

  void init()
  {
    oi = 0;
    tp = 0;
    tn = 0;
    tv = 0;
    for(uint32_t i = 0; i < LEVEL_SIZE; i++)
    {
      quote[BID_IDX * LEVEL_SIZE + i] = 0;
      quote[ASK_IDX * LEVEL_SIZE + i] = 0;
      quote[BIDSIZE_IDX * LEVEL_SIZE + i] = 0;
      quote[ASKSIZE_IDX * LEVEL_SIZE + i] = 0;
    }
  }

  std::string toString() const
  {
    std::ostringstream oss;
    static const int32_t W = 12;
    oss       << std::string(instrument) << " "
              << idx << " "
              << timestamp << std::endl;
    oss       << std::left
              << std::setw(4) << tv << "@"
              << std::setw(8) << tp << " ("
              << std::setw(10) << tn << ")"
              << std::endl;
    oss       << std::left
              << std::setw(3) << " "
              << std::setw(W) << "askSize"
              << std::setw(W) << "ask"
              << std::setw(W) << "bid"
              << std::setw(W) << "bidSize"
              << std::endl;

    for(uint32_t i = 0; i < LEVEL_SIZE; i++)
    {
      oss       << std::left
                << std::setw(3) << i
                << std::setw(W) << quote[ASKSIZE_IDX * LEVEL_SIZE + i]
                << std::setw(W) << quote[ASK_IDX * LEVEL_SIZE + i]
                << std::setw(W) << quote[BID_IDX * LEVEL_SIZE + i]
                << std::setw(W) << quote[BIDSIZE_IDX * LEVEL_SIZE + i]
                << std::endl;
    }
    return oss.str();
  }

  inline __attribute__((always_inline)) void write_symbol(const char* sym)
  {
    std::memcpy(instrument, sym, SYMBOL_SIZE);
  }

  inline __attribute__((always_inline)) void write_symbol(const char* sym, size_t len)
  {
    std::memcpy(instrument, sym, len);
  }

  inline __attribute__((always_inline)) void write_trade(int64_t i, int64_t p, int64_t n, int64_t v)
  {
    oi = i;
    tp = p;
    tn = n;
    tv = v;
  }

  inline __attribute__((always_inline)) void write_quote(int64_t ap, int64_t as, int64_t bp, int64_t bs, int32_t l)
  {
    quote[ASK_IDX * LEVEL_SIZE + l] = ap;
    quote[ASKSIZE_IDX * LEVEL_SIZE + l] = as;
    quote[BID_IDX * LEVEL_SIZE + l] = bp;
    quote[BIDSIZE_IDX * LEVEL_SIZE + l] = bs;
  }
};

#endif
