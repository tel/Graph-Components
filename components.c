#include "components.h"

void print_id(struct cco_node *node) {
  fprintf(stdout, "%i\n", node->id);
}

int main(int argc, char *argv[]) {

  if (argc != 2) {
    fprintf(stderr, "Must provide a database name.\n");
    exit(1);
  }

  // Initialize a database connection
  sqlite3 *db;
  CALL_SQLITE( open(argv[1], &db) );

  // Create the initial duset
  cco_duset *duset = create_duset();

  // Prepare a SQL statement to grab fdedges
  const char *sql = "SELECT left, right FROM fdedges WHERE id < 100;";
  sqlite3_stmt *stmt;
  CALL_SQLITE( prepare_v2(db, sql, strlen(sql) + 1, &stmt, NULL) );

  int res, x, y;
  do {
    // Step the database
    res = sqlite3_step(stmt);

    if (res == SQLITE_ROW) {
      // We've got a row
      x = sqlite3_column_int(stmt, 0);
      y = sqlite3_column_int(stmt, 1);
      duset_connect(duset, x, y);
    }
    
  } while (res == SQLITE_ROW);

  // Finalize the fdedges statement
  sqlite3_finalize(stmt);

  // Prepare a SQL statement to grab foedges
  sql = "SELECT left, right FROM foedges WHERE id < 100;";
  CALL_SQLITE( prepare_v2(db, sql, strlen(sql) + 1, &stmt, NULL) );

  do {
    // Step the database
    res = sqlite3_step(stmt);
    
    if (res == SQLITE_ROW) {
      // We've got a row
      x = sqlite3_column_int(stmt, 0);
      y = sqlite3_column_int(stmt, 1);
      duset_connect(duset, x, y);
    }

  } while (res == SQLITE_ROW);

  // Finalize the fdedges statement
  sqlite3_finalize(stmt);

  fprintf(stderr, "|R| = %i\n", HASH_CNT(hh_roots, duset->roots));
  
  // Find the largest connected component
  struct cco_node *biggest, *current_node, *tmp;
  int max_count = 0;
  int count;
  HASH_ITER(hh_roots, duset->roots, current_node, tmp) {
  	if (current_node != NULL) {
		count = count_children(current_node);
	} else {
		count = 0;
	}
    if (count > max_count) {
      max_count = count;
      biggest = current_node;
    }
  }

  fprintf(stderr, "Biggest cluster: %i\n", max_count);

  each_child(biggest, print_id);

  // Destroy the duset
  free_duset(duset);

  // Close the database connection
  sqlite3_close(db);

  // End
  return 0;
}
