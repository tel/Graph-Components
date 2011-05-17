#define ASSERT_EQ(a, b, msg)							\
  {										\
if ((a) == (b)) {								\
  fprintf(stderr, "[1;32m PASSED: [0;32m %s [0m \n", (msg));	        \
 } else {									\
  fprintf(stderr, "[1;31m FAILED: [0;31m %s [0m \n", (msg));	        \
 }										\
}										\

#define ASSERT_INEQ(a, b, msg)							\
  {										\
if ((a) != (b)) {								\
  fprintf(stderr, "[1;32m PASSED: [0;32m %s [0m \n", (msg));	        \
 } else {									\
  fprintf(stderr, "[1;31m FAILED: [0;31m %s [0m \n", (msg));	        \
 }										\
}										\

#define ASSERT_TRUE(a, msg) {ASSERT_EQ((a), 1, (msg))}
#define ASSERT_NULL(a, msg) {ASSERT_EQ((void *)(a), NULL, (msg))}
#define ASSERT_NOT_NULL(a, msg) {ASSERT_INEQ((void *)(a), NULL, (msg))}
