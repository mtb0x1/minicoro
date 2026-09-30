#include "minicoro.c"

#define MCO_WASM_DEMO_MAX_COROUTINES 64

static mco_coro* mco_wasm_demo_coroutines[MCO_WASM_DEMO_MAX_COROUTINES];
static size_t mco_wasm_demo_count = 0;

static void mco_wasm_demo_entry(mco_coro* co) {
  (void)co;
}

int mco_demo_create_many(size_t count) {
  size_t i;
  mco_result res;
  if(count == 0 || count > MCO_WASM_DEMO_MAX_COROUTINES || mco_wasm_demo_count != 0) {
    return MCO_INVALID_ARGUMENTS;
  }
  for(i = 0; i < count; i++) {
    mco_desc desc = mco_desc_init(mco_wasm_demo_entry, 32768);
    res = mco_create(&mco_wasm_demo_coroutines[i], &desc);
    if(res != MCO_SUCCESS) {
      while(i > 0) {
        i--;
        mco_destroy(mco_wasm_demo_coroutines[i]);
        mco_wasm_demo_coroutines[i] = NULL;
      }
      return res;
    }
  }
  mco_wasm_demo_count = count;
  return MCO_SUCCESS;
}

int mco_demo_suspended_count(void) {
  size_t i;
  int count = 0;
  for(i = 0; i < mco_wasm_demo_count; i++) {
    if(mco_status(mco_wasm_demo_coroutines[i]) == MCO_SUSPENDED) {
      count++;
    }
  }
  return count;
}

int mco_demo_destroy_all(void) {
  mco_result result = MCO_SUCCESS;
  while(mco_wasm_demo_count > 0) {
    mco_wasm_demo_count--;
    if(mco_wasm_demo_coroutines[mco_wasm_demo_count]) {
      mco_result res = mco_destroy(mco_wasm_demo_coroutines[mco_wasm_demo_count]);
      if(result == MCO_SUCCESS && res != MCO_SUCCESS) {
        result = res;
      }
      mco_wasm_demo_coroutines[mco_wasm_demo_count] = NULL;
    }
  }
  return result;
}