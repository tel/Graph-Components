#include "make_keys.h"

char *filename_lookup(sqlite3 *db, int id) {
  char *format = "SELECT documents.filename FROM nodes, documents \
WHERE nodes.id = %i AND documents.id = nodes.document_id ;\n";
  char *sql = malloc(sizeof(char)*(strlen(format) + 10));

  sprintf(sql, format, id);

  sqlite3_stmt *stmt;
  CALL_SQLITE( prepare_v2(db, sql, strlen(sql) + 1, &stmt, NULL) );
  free(sql);

  int res = sqlite3_step(stmt);

  char *filename = malloc(sizeof(char)*22);
  if (res == SQLITE_ROW) {
    strncpy(filename, (const char *) sqlite3_column_text(stmt, 0), 21);
    filename[21] = '\0';	/* null terminate */
  }
  sqlite3_finalize(stmt);

  return filename;
}

key_interval key_interval_lookup(sqlite3 *db, int id) {
  char *format = "SELECT t0, tf FROM nodes WHERE nodes.id = %i;\n";
  char *sql = malloc(strlen(format) + 10);
  sprintf(sql, format, id);

  sqlite3_stmt *stmt;
  CALL_SQLITE( prepare_v2(db, sql, strlen(sql) + 1, &stmt, NULL) );
  free(sql);

  int res = sqlite3_step(stmt);
  key_interval out;

  if (res == SQLITE_ROW) {
    out.t0 = sqlite3_column_int(stmt, 0);
    out.tf = sqlite3_column_int(stmt, 1);
  } else {
    out.t0 = 0;
    out.tf = 0;
  }
  sqlite3_finalize(stmt);

  return out;
}

key_interval print_coverage(sqlite3 *db, cco_duset *foduset, int id) {
  struct cco_node *fo_head = cco_locate(foduset, id);

  // Get the ids within this cluster
  int n = count_nodes(fo_head);
  int *ids = malloc(sizeof(int)*n);
  count_t nfound = 0;
  cluster_ids(fo_head, ids, &nfound);

  // Key intervals for each id
  char *filename = filename_lookup(db, fo_head->id);
  key_interval interval = key_interval_lookup(db, fo_head->id);
  count_t maxframe = interval.tf;
  count_t minframe = interval.t0;

  count_t i;	    
  for (i = 0; i < nfound; i++) {
    interval = key_interval_lookup(db, ids[i]);
    if (minframe > interval.t0) minframe = interval.t0;
    if (maxframe < interval.tf) maxframe = interval.tf;
  }
            
  fprintf(stdout, "$1/%c%c/%s %lld %lld ", filename[3], filename[4],\
	  filename, minframe, maxframe-minframe);
	 
  // Output interval
  key_interval out;
  out.t0 = minframe;
  out.tf = maxframe;
   
  free(ids);
  free(filename);

  return out;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Must provide a valid database name.\n");
        exit(1);
    }   

    // Initialize the database and duset
    sqlite3 *db;
    CALL_SQLITE( open(argv[1], &db) );
    cco_duset *foduset = create_duset();
    cco_duset *fdduset = create_duset();


    // Build foverlap duset
    sqlite3_stmt *stmt;
    const char *sql = "SELECT left, right FROM foedges;";
    CALL_SQLITE( prepare_v2(db, sql, strlen(sql) + 1, &stmt, NULL) );

    int res, x, y;
    do {
        // Step the database
        res = sqlite3_step(stmt);

        if (res == SQLITE_ROW) {
            // We've got a row
            x = sqlite3_column_int(stmt, 0);
            y = sqlite3_column_int(stmt, 1);
            duset_connect(foduset, x, y);
        }
    } while (res == SQLITE_ROW);

    sqlite3_finalize(stmt);


    // Build fdoverlap superduset
    sql = "SELECT left, right FROM fdedges;";
    CALL_SQLITE( prepare_v2(db, sql, strlen(sql) + 1, &stmt, NULL) );

    cco_node *x_node, *y_node;
    do {
        // Step the database
        res = sqlite3_step(stmt);
        
        if (res == SQLITE_ROW) {
            // Lift the found row into node heads for the foduset
            x_node = cco_find(cco_locate(foduset, sqlite3_column_int(stmt, 0)));
            y_node = cco_find(cco_locate(foduset, sqlite3_column_int(stmt, 1)));
            x = x_node->id;
            y = y_node->id;
            // Link by the node head ids
            duset_connect(fdduset, x, y);
        }
    } while (res == SQLITE_ROW);

    // Finalize the fdedges statement
    sqlite3_finalize(stmt);


    // Extract meaningful clusters
    struct cco_node *current_node, *tmp;
    count_t count = 0;

    int *ids;
    count_t nfound;
    HASH_ITER(hh_roots, fdduset->roots, current_node, tmp) {
        count = count_children(current_node) + 1;
        if (count > 2 && count < 1000) { // prune pseudoterms by overlap-cluster count

            // Get the ids within this cluster
            int n = count_nodes(current_node);
            ids = malloc(sizeof(int)*n);
            nfound = 0;
            cluster_ids(current_node, ids, &nfound);

	    // Script to merge the posteriograms
	    fprintf(stdout, "merge_posts ");
	    count_t i;
	    key_interval *ivals = malloc(sizeof(key_interval)*nfound);
   	    for (i = 0; i < nfound; i++) {
	      fprintf(stdout, "\\\n");
	      ivals[i] = print_coverage(db, foduset, ids[i]);
	    }
	    fprintf(stdout, "> $3/%i_key.comb.binary \n\n", current_node->id);

	    // Print the interval matching info files
	    fprintf(stdout, "echo \"");
	    for (i = 0; i < nfound; i++) {
	      fprintf(stdout, "%i %i ", (int)(ivals[i].tf), (int)(ivals[i].t0));
	    }
	    fprintf(stdout, "\n\" > $3/%i_key.intervals \n\n", current_node->id);

	    // Script to merge the sph files
	    
	    
	    free(ivals);
            free(ids);
        }
    }
    

    // Close everything
    sqlite3_close(db);
    free_duset(foduset);
    free_duset(fdduset);

    return 0;
}
