#include "components.h"

sqlite3 *db;

void print_id(struct cco_node *node) {
  fprintf(stdout, "%i\n", node->id);
}

void sql_lookup(struct cco_node *node) {
  char *format = "SELECT nodes.t0*80, (nodes.tf - nodes.t0)*80, documents.filename \
 FROM nodes, documents							\
 WHERE nodes.id = %i AND nodes.document_id = documents.id;\n";
  char *sql = malloc(strlen(format) + 10);
  sprintf(sql, format, node->id);

  sqlite3_stmt *stmt;
  CALL_SQLITE( prepare_v2(db, sql, strlen(sql) + 1, &stmt, NULL) );
  free(sql);

  int res = sqlite3_step(stmt);
  int s0, sf;
  char *filename = malloc(sizeof(char)*8);

  if (res == SQLITE_ROW) {
    s0 = sqlite3_column_int(stmt, 0);
    sf = sqlite3_column_int(stmt, 1);
    strncpy(filename, (const char *) sqlite3_column_text(stmt, 2), 7);
  }
  sqlite3_finalize(stmt);

  fprintf(stdout, "\"|sox %s.sph -p trim %is %is\" \\\n",
 	  filename, s0, sf);
  free(filename);
}

int main(int argc, char *argv[]) {

  if (argc != 2) {
    fprintf(stderr, "Must provide a database name.\n");
    exit(1);
  }

  // Initialize a database connection
  CALL_SQLITE( open(argv[1], &db) );

  // Create the initial duset
  cco_duset *duset = create_duset();

  // Prepare a SQL statement to grab fdedges
  const char *sql = "SELECT left, right FROM fdedges;";
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
  sql = "SELECT left, right FROM foedges;";
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
  
  // Find the largest connected component
  struct cco_node *biggest, *current_node, *tmp;
  count_t max_count = 0;
  count_t count = 0;

  HASH_ITER(hh_lookup, duset->lookup, current_node, tmp) {
    count += 1;
  }

  fprintf(stderr, "Seen nodes: %lld\n", count);

  count = 0;
  HASH_ITER(hh_roots, duset->roots, current_node, tmp) {
    count += 1;
  }

  fprintf(stderr, "Root nodes: %lld\n", count);

  HASH_ITER(hh_roots, duset->roots, current_node, tmp) {
    count = count_children(current_node) + 1;
    if (count > max_count) {
      max_count = count;
      biggest = current_node;
    }
    if (count > 1000) {
      fprintf(stdout, "sox --combine sequence \\\n");
      each_child(current_node, sql_lookup);
      fprintf(stdout, "~/work/common/keys/%i.wav\n\n", current_node->id);
    }
    /* fprintf(stdout, "%lld\n", count); */
  }

  /* fprintf(stderr, "Biggest cluster: %lld \n", max_count); */

  /* each_child(biggest, sql_lookup); */

  // Close the database connection
  sqlite3_close(db);

  // Destroy the duset
  free_duset(duset);

  // End
  return 0;
}
