#import "duset.h"
#import <stdio.h>

// Create a base disjoint union set. Allocates memory.
struct cco_duset *create_duset() {
  struct cco_duset *duset = malloc(sizeof(struct cco_duset));
  return duset;
}

// Iterates through the entire tree freeing the set of cco_nodes
void free_duset(struct cco_duset *duset) {
  struct cco_node *current_node, *tmp;

  // Walk through the full id hash freeing each node
  HASH_ITER(hh_lookup, duset->lookup, current_node, tmp) {
    HASH_DELETE(hh_lookup, duset->lookup, current_node);
    free(current_node);
  }
}

// Returns the node associated with a particular id. Iff doesn't
// already exist, it is created within its own singleton group.
struct cco_node *cco_locate(struct cco_duset *duset, int id) {
  struct cco_node *node;

  // Lookup the id in the hash to see if it exists
  HASH_FIND(hh_lookup, duset->lookup, &id, sizeof(int), node);

  if (node == NULL) {
    // Create a new node, insert it into lookup AND roots
    node = malloc(sizeof(struct cco_node));
    node->id = id;
    node->rank = 0;
    node->children = NULL;
    node->parent = node;

    // Insert it both into the full lookup table and the roots
    HASH_ADD(hh_lookup, duset->lookup, id, sizeof(int), node);
    HASH_ADD(hh_roots, duset->roots, id, sizeof(int), node);
  }
  return node;
}

int cco_delete_child(struct cco_node *node, struct cco_cons **cell_p) {
  // Get the cell pointer cell_p points to
  struct cco_cons *cell = *cell_p;
  if (cell != NULL) {
    if (cell->node == node) {
      // If the head is to be deleted, then just shift the pointer forward
      *cell_p = cell->next;
    } else {
      cco_delete_child(node, &cell->next);
    }
  } else {
    // We've reached the end of the list, what?
    fprintf(stdout, "WARNING: Tried to delete from an empty list. What.");
    return 1;
  }
}

int cco_count_children_in_list(struct cco_cons *cell, int acc) {
  if (cell == NULL) {
    return acc;
  } else {
    return 
      cco_count_children_in_list(cell->next, 
				 cco_count_children1(cell->node, acc));
  }
}

int cco_count_children1(struct cco_node *node, int acc) {
  if (node->children == NULL) {
    return acc+1;
  } else {
    return cco_count_children_in_list(node->children, 1+acc);
  }
}

int count_children(struct cco_node *node) {
  return cco_count_children1(node, 0);
}

void cco_each_cell(struct cco_cons *cell,
		   void (*callback)(struct cco_node *node)) {
  // Recurse into this node
  each_child(cell->node, callback);

  // Recurse down the list
  if (cell->next != NULL) {
    cco_each_cell(cell->next, callback);
  }
}

void each_child(struct cco_node *node,
		void (*callback)(struct cco_node *node)) {
  // Call it here
  callback(node);
  // Descend into the children
  if (node->children != NULL) {
    cco_each_cell(node->children, callback);
  }
}

// Traverse upward to find the root of a particular node
struct cco_node *cco_find(struct cco_node *node) {
  struct cco_node *curr_parent, *new_parent;

  // If we're not already at a root
  curr_parent = node->parent;
  if (node != curr_parent) {

    // 1. Recurse upward to find the new parent
    new_parent = cco_find(node->parent);

    // 2. Compress the path for further lookups
    node->parent = new_parent;

    // 3. Remove this node from the old parent's children
    //    (Only needs to happen when this node isn't a root and isn't
    //     already compressed.)
    if (curr_parent != new_parent) {
      // Delete this node from the old parent's children
      cco_delete_child(node, &curr_parent->children);
      // Prepend to the new parent's children list
      struct cco_cons *new_children = malloc(sizeof(struct cco_cons));
      new_children->next = new_parent->children;
      new_children->node = node;
      new_parent->children = new_children;
    }

  } else {
    // This is a root node, so the link is cyclic!
    new_parent = curr_parent;
  }

  return curr_parent;
}

// Merge the groups containing nodes `x` and `y`.
void cco_merge(struct cco_duset *duset, 
	       struct cco_node *x, 
	       struct cco_node *y) {
  struct cco_node *x_head = cco_find(x);
  struct cco_node *y_head = cco_find(y);
  if (x_head->rank > y_head->rank) {
    // Put y_head beneath x_head
    y_head->parent = x_head; // parent
    struct cco_cons *new_children = malloc(sizeof(struct cco_cons));
    new_children->next = x_head->children;
    new_children->node = y_head;
    x_head->children = new_children; //child
    // Delete y_head from roots
    HASH_DELETE(hh_roots, duset->roots, y_head);
  } else { 
    // Put x_head beneath y_head
    x_head->parent = y_head; // parent
    struct cco_cons *new_children = malloc(sizeof(struct cco_cons));
    new_children->next = y_head->children;
    new_children->node = x_head;
    y_head->children = new_children; //child
    // Delete x_head from roots
    HASH_DELETE(hh_roots, duset->roots, x_head);
    // If they were equal sized before, now y_head is larger
    if (x_head->rank == y_head->rank) {
      y_head->rank++;
    }
  }
}

// Interface function: insert an id into the duset
void duset_insert(struct cco_duset *duset, int id) {
  struct cco_node *node = cco_locate(duset, id);
}

// Interface function: insert an edge into the duset, merging the ids
void duset_connect(struct cco_duset *duset, int x_id, int y_id) {
  struct cco_node *x = cco_locate(duset, x_id);
  struct cco_node *y = cco_locate(duset, y_id);
  cco_merge(duset, x, y);
}

int test() { 
  // Allocate a duset
  struct cco_duset *duset = create_duset();

  // Insert a node
  struct cco_node *node1 = cco_locate(duset, 1);
  assert(HASH_CNT(hh_lookup, duset->lookup) == 1);
  assert(HASH_CNT(hh_roots, duset->roots) == 1);

  // Find that node without a new insert
  struct cco_node *node1_2 = cco_locate(duset, 1);
  assert(HASH_CNT(hh_lookup, duset->lookup) == 1);
  assert(HASH_CNT(hh_roots, duset->roots) == 1);
  
  // They are the same node, right?
  assert(node1 == node1_2);

  /* // Insert another node, increase the count  */
  struct cco_node *node2 = cco_locate(duset, 2);

  // Ensure the count increased
  assert(HASH_CNT(hh_lookup, duset->lookup) == 2);
  assert(HASH_CNT(hh_roots, duset->roots) == 2);

  // Build a list
  struct cco_cons *cons1 = malloc(sizeof(struct cco_cons));
  cons1->node = node1;
  struct cco_cons *cons2 = malloc(sizeof(struct cco_cons));
  cons2->node = node2;
  cons1->next = cons2;

  // Test deletion
  cco_delete_child(node1, &cons1);
  assert(cons1->node == node2);

  free(cons1);

  // Test a merger
  cco_merge(duset, node1, node2);
  assert(node1->parent == node2 || node2->parent == node1);
  assert(node1->rank == 1 || node2->rank == 1);
  assert(node1->children != NULL ? node1->children->node == node2 : 1);
  assert(node2->children != NULL ? node2->children->node == node1 : 1);

  // Free the duset
  free_duset(duset);

  return 0;
}

void test1() {
  cco_duset *duset = create_duset();
  duset_connect(duset, 1, 2);
  duset_connect(duset, 2, 3);
  duset_connect(duset, 3, 4);
  duset_connect(duset, 4, 5);
  duset_connect(duset, 5, 6);
  duset_connect(duset, 6, 7);
  duset_connect(duset, 10, 20);
  duset_connect(duset, 11, 21);
  duset_connect(duset, 21, 7);
  fprintf(stdout, "|R| = %i\n", HASH_CNT(hh_roots, duset->roots));
  free_duset(duset);
}

/* int main() { */
/*   test1(); */
/*   return 0; */
/* } */
