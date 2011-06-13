#include <stdio.h>
#include <stdlib.h>
#include <libc.h>

int main(int argc, char **argv) {

  if ( (argc-1) % 3 != 0 ) {
    fprintf(stderr, "Impromper number of arguments. Syntax is \n\
merge_posts [filename f0 n_frames] ... \n\
");
    exit(1);
  }

  /* The number of files to merge */
  int N = div(argc, 3).quot;
  int framesize = 45;
  size_t floatsize = 4;

  int i;
  FILE *fp;
  const char* filename;
  int frame0, dur;
  float *data, *silence;
  for (i = 0; i < N; i++) {
    /* Open/merge loop */

    /* Read the merge description */
    filename = argv[1+(3*i)];
    frame0   = atoi(argv[2+(3*i)]);
    dur      = atoi(argv[3+(3*i)]);
    
    /* Open and read file */
    fp = fopen(filename, "r");
    if (fp == NULL) { 
      fprintf(stderr, "File opening error at path: %s\n", filename); 
      exit(1);
    }
    
    data = malloc(floatsize*framesize*dur);
    silence = malloc(floatsize*framesize*20); // 10 silent frames
    
    /* Seek, read, write */
    fseek(fp, framesize*floatsize*frame0, SEEK_SET);
    fread(data, floatsize, dur*framesize, fp);
    fwrite(data, floatsize, dur*framesize, stdout);

    /* Write some silence to prevent overlapped matches */
    if (i != (N-1)) {
      fwrite(silence, floatsize, 20*framesize, stdout);
    }
    
    free(data);
    fclose(fp); // end fopen
  }

  return 0;

}
