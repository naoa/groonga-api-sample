#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <groonga.h>
  
int
main(int argc, char **argv)
{
  grn_ctx ctx;
  grn_obj *db, *table, *column, *key_type, *value_type;
  grn_id id;
  grn_rc rc;
  int added;
  grn_obj value, bulk;
  
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
  
  key_type = grn_ctx_at(&ctx, GRN_DB_SHORT_TEXT);
  table = grn_table_create(&ctx, "data", strlen("data"),
                           NULL,
                           GRN_OBJ_TABLE_HASH_KEY|GRN_OBJ_PERSISTENT,
                           key_type, NULL);
  if (table == NULL) {
    table = grn_ctx_get(&ctx, "data", strlen("data"));
  }

  value_type = grn_ctx_at(&ctx, GRN_DB_TEXT);
  column = grn_column_create(&ctx, table, "column", strlen("column"),
                             NULL,
                             GRN_OBJ_PERSISTENT|GRN_OBJ_COLUMN_SCALAR,
                             value_type);
  if (column == NULL) {
    column = grn_obj_column(&ctx, table, "column", strlen("column"));
  }

  id = grn_table_add(&ctx, table, "rec1", strlen("rec1"), &added);
  printf("id=%d, added=%d\n", id, added);
  
  GRN_TEXT_INIT(&value, 0);
  GRN_TEXT_PUT(&ctx, &value, "groonga world.", strlen("groonga world."));
  
  rc = grn_obj_set_value(&ctx, column, id, &value, GRN_OBJ_SET);
  printf("grn_obj_set_value: rc=%d\n", rc);
  
  GRN_TEXT_INIT(&bulk, 0);
  GRN_BULK_REWIND(&bulk);
  
  grn_obj_get_value(&ctx, column, id, &bulk);
  printf("grn_obj_get_value: bulk=%s\n", GRN_BULK_HEAD(&bulk));
  
  grn_obj_close(&ctx, &bulk);
  
  grn_obj *lexicon, *tokenizer, *normalizer, *index_column;
  grn_id column_id;
 
  key_type = grn_ctx_at(&ctx, GRN_DB_SHORT_TEXT);
  lexicon = grn_table_create(&ctx, "lexicon", strlen("lexicon"),
                             NULL,
                             GRN_OBJ_TABLE_PAT_KEY|GRN_OBJ_PERSISTENT,
                             key_type, NULL);
  if (lexicon == NULL) {
    lexicon = grn_ctx_get(&ctx, "lexicon", strlen("lexicon"));
  }

  tokenizer = grn_ctx_get(&ctx, "TokenBigram", strlen("TokenBigram"));
  if (grn_obj_set_info(&ctx, lexicon, GRN_INFO_DEFAULT_TOKENIZER, tokenizer)) {
    fprintf(stderr, "grn_obj_set_info() set tokenizer failed\n");
    return -1;
  }
 
  normalizer = grn_ctx_get(&ctx, "NormalizerAuto", strlen("NormalizerAuto"));
  if (grn_obj_set_info(&ctx, lexicon, GRN_INFO_NORMALIZER, normalizer)) {
    fprintf(stderr, "grn_obj_set_info() set normalizer failed\n");
    return -1;
  }

  index_column = grn_column_create(&ctx, lexicon, "index", strlen("index"),
                                   NULL,
                                   GRN_OBJ_PERSISTENT|GRN_OBJ_WITH_POSITION|GRN_OBJ_COLUMN_INDEX,
                                   table);
  if (index_column == NULL) {
    index_column = grn_obj_column(&ctx, lexicon, "index", strlen("index"));
  }
 
  column_id = grn_obj_id(&ctx, column);
  GRN_UINT32_INIT(&bulk, 0);
  GRN_UINT32_SET(&ctx, &bulk, column_id);
  grn_obj_set_info(&ctx, index_column, GRN_INFO_SOURCE, &bulk);
  grn_obj_close(&ctx, &bulk);

  grn_obj *v, *cond;
  grn_obj *result = NULL;
  char *filter = "column @ \"groonga\"";

  printf("filter = %s\n", filter);

  GRN_EXPR_CREATE_FOR_QUERY(&ctx, table, cond, v);
  grn_expr_parse(&ctx, cond,
                 filter,
                 strlen(filter),
                 NULL,
                 GRN_OP_MATCH,
                 GRN_OP_AND,
                 GRN_EXPR_SYNTAX_SCRIPT); 
 
  result = grn_table_create(&ctx, NULL, 0, NULL,
                            GRN_TABLE_HASH_KEY|GRN_OBJ_WITH_SUBREC,
                            table, NULL);
  if (result) {
    grn_table_select(&ctx, table, cond, result, GRN_OP_OR);
  }

  printf("hits: %d\n", grn_table_size(&ctx, result));

  grn_obj *result_column;
  result_column = grn_obj_column(&ctx, result, "_key", strlen("_key"));

  grn_table_cursor *cur;
  if ((cur = grn_table_cursor_open(&ctx, result, NULL, 0, NULL, 0, 0, -1,
                                   GRN_CURSOR_BY_ID))) {
    grn_id id;
    while ((id = grn_table_cursor_next(&ctx, cur)) != GRN_ID_NIL) {
      grn_obj buf;
      GRN_TEXT_INIT(&buf, 0);
      GRN_BULK_REWIND(&buf);
      grn_obj_get_value(&ctx, result_column, id, &buf);
      printf("hit record=%s\n", GRN_TEXT_VALUE(&buf));
      grn_obj_unlink(&ctx, &buf);
    }
  }

  if (grn_obj_close(&ctx, db)) {
    fprintf(stderr, "grn_obj_close() failed\n");
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
