#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <groonga.h>

grn_obj *
table_create(grn_ctx *ctx, char *table_name)
{
  grn_obj *table, *key_type;

  key_type = grn_ctx_at(ctx, GRN_DB_SHORT_TEXT);
  table = grn_table_create(ctx, table_name, strlen(table_name),
                           NULL,
                           GRN_OBJ_TABLE_HASH_KEY|GRN_OBJ_PERSISTENT,
                           key_type, NULL);
  if (table == NULL) {
    table = grn_ctx_get(ctx, table_name, strlen(table_name));
  }
  return table;
}

grn_obj *
column_create(grn_ctx *ctx, grn_obj *table, char *column_name)
{
  grn_obj *column, *value_type;

  value_type = grn_ctx_at(ctx, GRN_DB_TEXT);
  column = grn_column_create(ctx, table, column_name, strlen(column_name),
                             NULL,
                             GRN_OBJ_PERSISTENT|GRN_OBJ_COLUMN_SCALAR,
                             value_type);
  if (column == NULL) {
    column = grn_obj_column(ctx, table, column_name, strlen(column_name));
  }
  return column;
}

grn_obj *
lexicon_create(grn_ctx *ctx, grn_obj *target_table, grn_obj *target_column,
               char *lexicon_name)
{
  grn_obj *key_type;
  grn_obj *lexicon, *tokenizer, *normalizer, *index_column;
  grn_id column_id;

  grn_obj bulk;

  key_type = grn_ctx_at(ctx, GRN_DB_SHORT_TEXT);
  lexicon = grn_table_create(ctx, "lexicon", strlen("lexicon"),
                             NULL,
                             GRN_OBJ_TABLE_PAT_KEY|GRN_OBJ_PERSISTENT,
                             key_type, NULL);
  if (lexicon == NULL) {
    lexicon = grn_ctx_get(ctx, lexicon_name, strlen(lexicon_name));
  }

  tokenizer = grn_ctx_get(ctx, "TokenBigram", strlen("TokenBigram"));
  if (grn_obj_set_info(ctx, lexicon, GRN_INFO_DEFAULT_TOKENIZER, tokenizer)) {
    fprintf(stderr, "grn_obj_set_info() set tokenizer failed\n");
    return NULL;
  }

  normalizer = grn_ctx_get(ctx, "NormalizerAuto", strlen("NormalizerAuto"));
  if (grn_obj_set_info(ctx, lexicon, GRN_INFO_NORMALIZER, normalizer)) {
    fprintf(stderr, "grn_obj_set_info() set normalizer failed\n");
    return NULL;
  }

  index_column = grn_column_create(ctx, lexicon, "index", strlen("index"),
                                   NULL,
                                   GRN_OBJ_PERSISTENT|GRN_OBJ_WITH_POSITION|GRN_OBJ_COLUMN_INDEX,
                                   target_table);
  if (index_column == NULL) {
    index_column = grn_obj_column(ctx, lexicon, "index", strlen("index"));
  }

  column_id = grn_obj_id(ctx, target_column);
  GRN_UINT32_INIT(&bulk, 0);
  GRN_UINT32_SET(ctx, &bulk, column_id);
  grn_obj_set_info(ctx, index_column, GRN_INFO_SOURCE, &bulk);
  grn_obj_close(ctx, &bulk);

  return lexicon;
}
  
grn_id
record_insert(grn_ctx *ctx, grn_obj *table, grn_obj *column, char *key, char *record)
{
  grn_id id;
  grn_rc rc;
  int added;
  grn_obj value;

  id = grn_table_add(ctx, table, key, strlen(key), &added);

  GRN_TEXT_INIT(&value, 0);
  GRN_TEXT_PUT(ctx, &value, record, strlen(record));
  rc = grn_obj_set_value(ctx, column, id, &value, GRN_OBJ_SET);
  if (rc == GRN_INVALID_ARGUMENT) {
    id = -1;
  }
  grn_obj_unlink(ctx, &value);

  return id;
}

int
record_print(grn_ctx *ctx, grn_obj *column, grn_id id)
{
  grn_obj bulk;
  GRN_TEXT_INIT(&bulk, 0);
  GRN_BULK_REWIND(&bulk);
  grn_obj_get_value(ctx, column, id, &bulk);
  printf("%s\n", GRN_BULK_HEAD(&bulk));
  grn_obj_unlink(ctx, &bulk);

  return 0;
}

grn_obj *
table_select_by_filter(grn_ctx *ctx, grn_obj *table, char *filter)
{
  grn_obj *v, *cond;
  grn_obj *result = NULL;

  GRN_EXPR_CREATE_FOR_QUERY(ctx, table, cond, v);
  grn_expr_parse(ctx, cond,
                 filter,
                 strlen(filter),
                 NULL,
                 GRN_OP_MATCH,
                 GRN_OP_AND,
                 GRN_EXPR_SYNTAX_SCRIPT);

  result = grn_table_create(ctx, NULL, 0, NULL,
                            GRN_TABLE_HASH_KEY|GRN_OBJ_WITH_SUBREC,
                            table, NULL);
  if (result) {
    grn_table_select(ctx, table, cond, result, GRN_OP_OR);
  }
  printf("hits: %d\n", grn_table_size(ctx, result));

  return result;
}

int
column_print(grn_ctx *ctx, grn_obj *table, char *column_name)
{
  grn_table_cursor *cur;
  grn_obj *column;
  grn_obj buf;

  column = grn_obj_column(ctx, table, column_name, strlen(column_name));
  GRN_TEXT_INIT(&buf, 0);
  if ((cur = grn_table_cursor_open(ctx, table, NULL, 0, NULL, 0, 0, -1,
                                   GRN_CURSOR_BY_ID))) {
    grn_id id;
    while ((id = grn_table_cursor_next(ctx, cur)) != GRN_ID_NIL) {
      GRN_BULK_REWIND(&buf);
      grn_obj_get_value(ctx, column, id, &buf);
      printf("%s\n", GRN_TEXT_VALUE(&buf));
    }
  }
  grn_obj_unlink(ctx, &buf);
  return 0;
}

int
main(int argc, char **argv)
{
  grn_ctx ctx;
  grn_obj *db, *table, *column;
  grn_id id;
  grn_obj *result;
  
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
  
  table = table_create(&ctx, "data");
  column = column_create(&ctx, table, "column");

  id = record_insert(&ctx, table, column, "groonga", "groonga world");
  printf("add record:\n");
  record_print(&ctx, column, id);

  lexicon_create(&ctx, table, column, "lexicon");

  result = table_select_by_filter(&ctx, table, "column @ \"groonga\"");
  printf("hit records:\n");
  column_print(&ctx, result, "column");

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
