#define BEGIN_GROUP()     			\
  int grouptrue__ = 1;				\

#define NEW_GROUP()				\
  grouptrue__ = 1;				\

#define DID(msg) fprintf(stderr, "             >[0;33m %s [0m \n", (msg));

#define ASSERT_EQ(a, b, msg)							\
  {										\
if ((a) == (b)) {								\
  fprintf(stderr, " ->[1;32m PASSED: [0;32m %s [0m \n", (msg)); 	        \
 } else {									\
  fprintf(stderr, " ->[1;31m FAILED: [0;31m %s [0m \n", (msg));	        \
  grouptrue__ = 0;							        \
 }										\
}										\

#define ASSERT_NEQ(a, b, msg)							\
  {										\
if ((a) != (b)) {								\
  fprintf(stderr, " ->[1;32m PASSED: [0;32m %s [0m \n", (msg));	        \
 } else {									\
  fprintf(stderr, " ->[1;31m FAILED: [0;31m %s [0m \n", (msg));	        \
  grouptrue__ = 0;							        \
 }										\
}										\

#define END_GROUP(msg) {                                                        \
if (grouptrue__) {								\
  fprintf(stderr, "[1;32mGROUP PASS: [0;32m %s [0m \n\n", (msg));        \
 } else {									\
  fprintf(stderr, "[1;31mGROUP FAIL: [0;31m %s [0m \n\n", (msg));        \
 }										\
}                                                                               \

#define ASSERT_TRUE(a, msg) {ASSERT_EQ((a), 1, msg)}
#define ASSERT_NULL(a, msg) {ASSERT_EQ((void *)(a), NULL, msg)}
#define ASSERT_NOT_NULL(a, msg) {ASSERT_NEQ((void *)(a), NULL, msg)}
