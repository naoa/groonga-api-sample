#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <groonga.h>
#include <time.h>

#define MAX_VALUE_BYTES 16

int
main(int argc, char **argv)
{
  grn_ctx ctx;
  grn_id id;
  
  if (grn_init()) {
    fprintf(stderr, "grn_init() failed\n");
    return -1;
  }

  if (grn_ctx_init(&ctx, 0)) {
    fprintf(stderr, "grn_ctx_init() failed\n");
    return -1;
  }

  grn_hash *hash = NULL;
  const char *hash_path = "test.grn_hash";

  hash = grn_hash_create(&ctx, hash_path,
                         GRN_TABLE_MAX_KEY_SIZE,
                         MAX_VALUE_BYTES,
                         GRN_OBJ_TABLE_HASH_KEY | GRN_OBJ_KEY_VAR_SIZE);
  if(hash == NULL) {
    hash = grn_hash_open(&ctx, hash_path);
  }

  char *add_key = "key";
  void *got_value;
  void *value_buf = NULL;
  int added = 0;

  id = grn_hash_add(&ctx, hash, add_key, strlen(add_key), &value_buf, &added);
  if (added) {
    memcpy(value_buf, "value", strlen("value"));
  }

  grn_hash_get(&ctx, hash, add_key, strlen(add_key), &got_value);
  printf("got_value=%s\n", (char *)got_value);

  grn_hash_delete(&ctx, hash, add_key, strlen(add_key), NULL);
  got_value = "";
  grn_hash_get_value(&ctx, hash, id, &got_value);
  printf("got_value=%s\n", (char *)got_value);

  if (grn_hash_close(&ctx, hash)) {
    fprintf(stderr, "grn_hash_close() failed\n");
    return -1;
  }
  
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
