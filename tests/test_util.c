#include <stdbool.h>

bool same(float diff) {
  return  -0.001 < diff && diff < 0.001;
}

#define ASSERT_EQUAL_NUM(want, got) { \
  if (!same(want - got)) { \
    printf("want %.2f, got %.2f\n", (float) want, (float)got); \
    assert(same(want - got)); \
  }}
