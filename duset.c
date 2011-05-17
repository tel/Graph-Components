#import "duset.h"
#import <stdio.h>

// Create a base disjoint union set. Allocates memory.
struct cco_duset *create_duset() {
  struct cco_duset *duset = malloc(sizeof(struct cco_duset));
  if (duset == NULL) {
    fprintf(stderr, "Memory error: could not allocate for a duset.");
    exit(1);
  }
  return duset;
}

// Iterates down a child list freeing the cons nodes
void cco_free_node(struct cco_node *node) {
  struct cco_cons *child = node->children;
  struct cco_cons *new_child;
  while (child != NULL) {
    new_child = child->next;
    free(child);
    child = new_child;
  }
  free(node);
}

// Iterates through the entire tree freeing the set of cco_nodes
void free_duset(struct cco_duset *duset) {
  struct cco_node *current_node, *tmp;

  // Free up the roots hash
  HASH_CLEAR(hh_roots, duset->roots);

  // Walk through the full id hash freeing each node
  HASH_ITER(hh_lookup, duset->lookup, current_node, tmp) {
    HASH_DELETE(hh_lookup, duset->lookup, current_node);
    cco_free_node(current_node);
  }

  // Finally clear the duset itself
  free(duset);
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
    if (node == NULL) {
      fprintf(stderr, "Memory error: could not allocate for a duset node.");
      exit(1);
    }
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

struct cco_cons *cco_make_cons(struct cco_node *node, struct cco_cons *cell) {
  struct cco_cons *new_cell = malloc(sizeof(struct cco_cons));
  if (new_cell == NULL) {
    fprintf(stderr, "Memory error: could not allocate a new child ll-cell.");
    exit(1);
  }
  new_cell->node = node;
  new_cell->next = cell;
  return new_cell;
}

struct cco_cons *cco_remove_child(struct cco_cons *cell, struct cco_node *child) {
  // Get the cell pointer cell_p points to
  if (cell != NULL) {
    if (cell->node == child) {
      struct cco_cons *next = cell->next;
      // This cell is gone
      free(cell);
      return next;
    } else {
      cell->next = cco_remove_child(cell->next, child);
      return cell;
    }
  } else {
    return NULL;
  }
}

count_t count_nodes(struct cco_node *node) {
  count_t count = 1;
  struct cco_cons *cell = node->children;
  while (cell != NULL) {
    count += count_nodes(cell->node);
    cell = cell->next;
  }
  return count;
}

count_t count_children(struct cco_node *node) {
  return (count_nodes(node) - 1);
}

count_t cco_count_roots(struct cco_duset *duset) {
  count_t cnt = HASH_CNT(hh_roots, duset->roots);
  return cnt; 
}

count_t cco_count_nodes(struct cco_duset *duset) {
  count_t cnt = HASH_CNT(hh_lookup, duset->lookup);
  return cnt;
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

// Consistency ensuring operation! We have to make sure that the
// child's old parent no longer claims it.
void cco_adopt_child(struct cco_node *parent, struct cco_node *child) {
  struct cco_node *old_parent = child->parent;
  if (old_parent != parent) {
    // We'll need to perform an adoption
    old_parent->children = cco_remove_child(old_parent->children, child);
    parent->children = cco_make_cons(child, parent->children);
    child->parent = parent;
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

    // 2. Compress the node
    //    (Only needs to happen when this node isn't a root and isn't
    //     already compressed.)
    if (curr_parent != new_parent) {
      cco_adopt_child(new_parent, node);
    }
  } else {
    new_parent = curr_parent;
  }
  return new_parent;
}

// Merge the groups containing nodes `x` and `y`.
void cco_merge(struct cco_duset *duset, 
	       struct cco_node *x, 
	       struct cco_node *y) {
  struct cco_node *x_head = cco_find(x);
  struct cco_node *y_head = cco_find(y);
  if (x_head != y_head) {
    if (x_head->rank > y_head->rank) {
      // Put y_head beneath x_head
      cco_adopt_child(x_head, y_head);
      // Delete y_head from roots
      HASH_DELETE(hh_roots, duset->roots, y_head);
    } else { 
      // Put x_head beneath y_head
      cco_adopt_child(y_head, x_head);
      // Delete x_head from roots
      HASH_DELETE(hh_roots, duset->roots, x_head);
      // If they were equal sized before, now y_head is larger
      if (x_head->rank == y_head->rank) {
	y_head->rank++;
      }
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
