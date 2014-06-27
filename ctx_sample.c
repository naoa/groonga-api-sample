#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <groonga.h>
  
int
main(int argc, char **argv)
{
  grn_ctx ctx;
  grn_obj *db;
  
  const char *path = "test.grn";
  
  if (grn_init()) {
    fprintf(stderr, "grn_init() failed\n");
    return -1;
  }
  
  if (grn_ctx_init(&ctx, 0)) {
    fprintf(stderr, "grn_ctx_init() failed\n");
    return -1;
  }
  
  db = grn_db_open(&ctx, path);
  
  if (!db) { db = grn_db_create(&ctx, path, NULL); }
  
  if (!db) {
    fprintf(stderr, "db initialize failed\n");
    return -1;
  }
  
  grn_ctx_send(&ctx, "status", strlen("status"), GRN_CTX_QUIET); 
  
  char *result = NULL;
  unsigned int result_size = 0;
  int recv_flags;
  grn_ctx_recv(&ctx, &result, &result_size, &recv_flags);

  result[result_size] = '\0'; 
  
  printf("result:%s\n", result);
  
  if (grn_ctx_fin(&ctx)) {
    fprintf(stderr, "grn_ctx_fin() failed\n");
    return -1;
  }

  if (grn_fin()) {
    fprintf(stderr, "grn_fin() failed\n");
    return -1;
  }
  return 0;
}
