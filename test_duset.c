#import <stdio.h>
#import <string.h>
#import "duset.h"
#import "asserts.h"

void test_adoption_consistency() {
  ASSERT_EQ(1, 1, "equal");
  ASSERT_TRUE(1==1, "true");
  ASSERT_INEQ(1, 2, "ineq");
}

int main() {
  test_adoption_consistency();
  return 0;
}
