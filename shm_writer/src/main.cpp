#include <stdio.h>
#include <stdint.h>
#include <cstring>
#include <string.h>
#include "BookData.h"
#include "ShmRingBuffer.h"

int main(int argc, char** argv)
{
  ShmRingBuffer<BookData>* q = new ShmRingBuffer<BookData>(1024, true, "QuotePubTEST");
  int interval = 100;
  int i = 0;
  int64_t latency = 0;

  int RECORDS = 1000000000;
  double SCALINGFACTOR = 100.0;
  std::chrono::high_resolution_clock::time_point start, stop;
  double time_span;
  start = std::chrono::high_resolution_clock::now();
  BookData* b = nullptr;
  //b = q->get_next_write();

  for (int i = 0; i < RECORDS; i ++)
  {
    b = q->get_next_write();

    b->init();

    b->write_symbol("cu2007", 7);
    b->write_trade(112411,
                   42940 * SCALINGFACTOR,
                   34343434* SCALINGFACTOR,
                   633);

    b->write_quote(42945 * SCALINGFACTOR,
                   55,
                   42930 * SCALINGFACTOR,
                   33,
                   0);
    b->write_quote(42945 * SCALINGFACTOR,
                   55,
                   42930 * SCALINGFACTOR,
                   33,
                   1);
    b->write_quote(42945 * SCALINGFACTOR,
                   55,
                   42930 * SCALINGFACTOR,
                   33,
                   2);
    b->write_quote(42945 * SCALINGFACTOR,
                   55,
                   42930 * SCALINGFACTOR,
                   33,
                   3);
    b->write_quote(42945 * SCALINGFACTOR,
                   55,
                   42930 * SCALINGFACTOR,
                   33,
                   4);
  
    q->set_next_write(b);
  }
  stop = std::chrono::high_resolution_clock::now();

  time_span = std::chrono::duration_cast<std::chrono::duration<double>>(
                                                      stop - start).count();
  printf("The total time spent %d "
          "times took %0.2lf seconds (%0.2lf ns/message average)\r\n",
          RECORDS, time_span, (time_span/RECORDS)*1e9);
  
  return 0;
}
