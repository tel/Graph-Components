#ifndef CCOMP_H

#import <stdlib.h>
#import <assert.h>

#import "uthash.h"

typedef struct cco_cons {
  struct cco_node *node;
  struct cco_cons *next;
} cco_cons;

typedef struct cco_node {
  // Node structure. Does disjoint-set operations and also can descend
  // into its subtree via `children`.
  int id;
  int rank;
  struct cco_node *parent;
  struct cco_cons *children;
  UT_hash_handle hh_lookup;
  UT_hash_handle hh_roots;
} cco_node;

typedef struct cco_duset {
  // Hashes to index a disjoint-union set
  struct cco_node *lookup;
  struct cco_node *roots;
} cco_duset;

// Create a base disjoint union set. Allocates memory.
struct cco_duset *create_duset(void);

// Iterates through the entire tree freeing the set of cco_nodes
void free_duset(struct cco_duset *duset);

// Interface function: insert an id into the duset
void duset_insert(struct cco_duset *duset, int id);

// Interface function: insert an edge into the duset, merging the ids
void duset_connect(struct cco_duset *duset, int x_id, int y_id);

// Interface function: count the children of some node
int count_children(struct cco_node *node);

// Interface function: map over the children of this node
void each_child(struct cco_node *node,
		void (*callback)(struct cco_node *node));

#endif
#define CCOMP_H
