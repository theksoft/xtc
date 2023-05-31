#ifndef _XTC_LCLHP_HXX_
#define _XTC_LCLHP_HXX_

#include "xtc_lclhp.h"
#include "xtc_gnrhp.hxx"
#include <cstring>

namespace xtc {
  namespace xlh {

  template <size_t Length>
  class Allocator : public xtc::Allocator {
  protected:
    char _memory[Length];   /**< Memory pool */
    xlh_heap_t _heap;       /**< C local heap wrapped in the allocator definition */

  public:

    Allocator(xtc_protect_t *protect = NULL) {
      memset(_memory, 0, Length);
      xlh_init(&_heap, _memory, Length, protect);
    }

    virtual ~Allocator() {
      xlh_end(&_heap, NULL);
      memset(_memory, 0, Length);
    }

    virtual void* allocate(size_t size) {
      return xlh_alloc(&_heap, size);
    }

    virtual void free(void *ptr) {
      xlh_free(&_heap, ptr);
    }

    virtual size_t count() {
      return xlh_count(&_heap);
    }

    size_t maxFreeBlk() {
      return xlh_max_free_blk(&_heap);
    }

    void freeStats(xlh_stats_t& stats) {
      xlh_free_stats(&_heap, &stats);
    }

    void allocatedStats(xlh_stats_t& stats) {
      xlh_allocated_stats(&_heap, &stats);
    }

  };

}}

#endif
