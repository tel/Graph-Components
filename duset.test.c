#import <stdio.h>
#import <string.h>
#import "duset.h"
#import "asserts.h"

void test_adoption_consistency() { 
  BEGIN_GROUP();
  struct cco_duset *duset = create_duset();
  ASSERT_NOT_NULL(duset, "Duset created");
  DID("Locate({1, 2, 3, 4, 5})");
  duset_insert(duset, 1);
  duset_insert(duset, 2);
  duset_insert(duset, 3);
  duset_insert(duset, 4);
  duset_insert(duset, 5);
  ASSERT_EQ(5, cco_count_nodes(duset), "Node loading count is 5");
  ASSERT_EQ(5, cco_count_roots(duset), "Root loading count is 5");
  struct cco_node *node1 = cco_locate(duset, 1);
  struct cco_node *node2 = cco_locate(duset, 2);
  struct cco_node *node3 = cco_locate(duset, 3);
  struct cco_node *node4 = cco_locate(duset, 4);
  struct cco_node *node5 = cco_locate(duset, 5);
  ASSERT_NULL(node1->children, "No children, node 1")
  ASSERT_NULL(node2->children, "No children, node 2")
  ASSERT_NULL(node3->children, "No children, node 3")
  ASSERT_NULL(node4->children, "No children, node 4")
  ASSERT_NULL(node5->children, "No children, node 5")
  END_GROUP("Creation stability");

  NEW_GROUP();
  ASSERT_EQ(node1, cco_find(node1), "Root finding: parent(node1) = node1");
  ASSERT_EQ(count_children(node1), 0, "Child counting: |children(node1)| = 0");
  ASSERT_EQ(node2, cco_find(node2), "Root finding: parent(node2) = node2");
  ASSERT_EQ(count_children(node2), 0, "Child counting: |children(node2)| = 0");
  ASSERT_EQ(node3, cco_find(node3), "Root finding: parent(node3) = node3");
  ASSERT_EQ(count_children(node3), 0, "Child counting: |children(node3)| = 0");
  ASSERT_EQ(node4, cco_find(node4), "Root finding: parent(node4) = node4");
  ASSERT_EQ(count_children(node4), 0, "Child counting: |children(node4)| = 0");
  ASSERT_EQ(node5, cco_find(node5), "Root finding: parent(node5) = node5");
  ASSERT_EQ(count_children(node5), 0, "Child counting: |children(node5)| = 0");
  END_GROUP("Duset root search");

  NEW_GROUP();
  ASSERT_EQ(count_children(node2), 0,
  	    "Child counting: |children(node2)| = 0");

  duset_connect(duset, 1, 2);
  DID("Merge(Locate(1), Locate(2))");
  ASSERT_EQ(5, cco_count_nodes(duset), "Node count stable under merge");
  ASSERT_EQ(4, cco_count_roots(duset), "Root count reduced by merge");
  ASSERT_EQ(node2->children->node, node1,
  	    "Child pointer consistency: car(children(node2)) = node1");
  ASSERT_EQ(node1->parent, node2,
  	    "Parent pointer consistency: parent(node1) = node2");
  ASSERT_EQ(node2, cco_find(node1),
  	    "Parent finding consistency: find(node1) = node2");

  ASSERT_EQ(count_children(node2), 1,
  	    "Child counting: |children(node2)| = 1");

  duset_connect(duset, 1, 3);
  DID("Merge(Locate(1), Locate(3))");
  ASSERT_EQ(3, cco_count_roots(duset), "Root count reduced by merge");
  ASSERT_EQ(node2->children->node, node3,
  	    "Child pointer consistency: car(children(node2)) = node3");
  ASSERT_EQ(node2->children->next->node, node1,
  	    "Child pointer consistency: car(cdr(children(node2))) = node1");
  ASSERT_EQ(node3->parent, node2,
  	    "Parent pointer consistency: parent(node3) = node2");
  ASSERT_EQ(node2, cco_find(node3),
  	    "Parent finding consistency: find(node3) = node2");

  ASSERT_EQ(count_children(node2), 2,
  	    "Child counting: |children(node2)| = 2");

  duset_connect(duset, 4, 5);
  DID("Merge(Locate(4), Locate(5))");
  ASSERT_EQ(2, cco_count_roots(duset), "Root count reduced by merge");
  ASSERT_EQ(node4->parent, node5,
  	    "Parent pointer consistency: parent(node4) = node5");
  ASSERT_EQ(node5, cco_find(node4),
  	    "Parent finding consistency: find(node4) = node5");

  ASSERT_EQ(count_children(node2), 2,
  	    "Child counting: |children(node2)| = 2");
  ASSERT_EQ(count_children(node5), 1,
  	    "Child counting: |children(node2)| = 2");

  duset_connect(duset, 3, 5);
  DID("Merge(Locate(3), Locate(5))");
  ASSERT_EQ(1, cco_count_roots(duset), "Root count reduced by merge");
  ASSERT_EQ(node2->parent, node5,
  	    "Parent pointer consistency: parent(node2) = node5");
  ASSERT_EQ(node5, cco_find(node3),
  	    "Parent finding consistency: find(node3) = node5");
  ASSERT_EQ(node5, cco_find(node4),
  	    "Parent finding consistency: find(node4) = node5");
  ASSERT_EQ(node3->parent, node5,
  	    "Path compression: parent(node3) = node5");
  ASSERT_NEQ(node1->parent, node5,
  	     "Path compression hasn't occured yet: parent(node1) = node5");

  cco_find(node1);
  DID("Find(Locate(1))");
  ASSERT_EQ(node1->parent, node5,
  	    "Path compression occured: parent(node1) = node5");


  END_GROUP("Duset merging stability");

  NEW_GROUP();
  duset_connect(duset, 1, 4);
  DID("Merge(Locate(1), Locate(4))");
  ASSERT_EQ(cco_find(node1), node5,
	    "Path connection maintained: find(node1) = node5");
  ASSERT_EQ(cco_find(node4), node5,
	    "Path connection maintained: find(node4) = node5");
  ASSERT_EQ(node1->parent, node5,
  	    "Path compression performed: parent(node1) = node5");
  ASSERT_EQ(node4->parent, node5,
  	    "Path compression performed: parent(node1) = node5");

  duset_connect(duset, 1, 1);
  DID("Merge(Locate(1), Locate(1))");
  ASSERT_EQ(cco_find(node1), node5,
  	    "Path connection maintained: find(node1) = node5");
  ASSERT_EQ(node1->parent, node5,
  	    "Path compression performed: parent(node1) = node5");

  END_GROUP("Pathological behavior management.");

  NEW_GROUP();
  free_duset(duset);
  END_GROUP("Memory freeing stability.");
}

int main() {
  test_adoption_consistency();
  return 0;
}
