#include <objspace_info.h>

// FIXME: Link objspace library instead
#include <ruby_private/objspace_tracing.c>

static inline const char *
obj_type(VALUE obj)
{
  switch (BUILTIN_TYPE(obj)) {
  #define CASE_TYPE(type) case T_##type: return #type; break
  CASE_TYPE(NONE);
  CASE_TYPE(NIL);
  CASE_TYPE(OBJECT);
  CASE_TYPE(CLASS);
  CASE_TYPE(ICLASS);
  CASE_TYPE(MODULE);
  CASE_TYPE(FLOAT);
  CASE_TYPE(STRING);
  CASE_TYPE(REGEXP);
  CASE_TYPE(ARRAY);
  CASE_TYPE(HASH);
  CASE_TYPE(STRUCT);
  CASE_TYPE(BIGNUM);
  CASE_TYPE(FILE);
  CASE_TYPE(FIXNUM);
  CASE_TYPE(TRUE);
  CASE_TYPE(FALSE);
  CASE_TYPE(DATA);
  CASE_TYPE(MATCH);
  CASE_TYPE(SYMBOL);
  CASE_TYPE(RATIONAL);
  CASE_TYPE(COMPLEX);
  CASE_TYPE(UNDEF);
  CASE_TYPE(NODE);
  CASE_TYPE(ZOMBIE);
  #undef CASE_TYPE
  }
  return "UNKNOWN";
}

static void
reachable_object_i(VALUE ref, void *data)
{
  rb_ary_push((VALUE)data, PTR2FIX(ref));
}

#define HASH_SET(k,v) rb_hash_aset(target_hash, rb_str_new2(k), v)
#define HASH_SET_PTR(k,v) HASH_SET(k,INT2FIX((void *)v))
#define HASH_SET_INT(k,v) HASH_SET(k,INT2FIX(v))
#define HASH_SET_UINT(k,v) HASH_SET(k,UINT2NUM(v))
#define HASH_SET_ULONG(k,v) HASH_SET(k,ULONG2NUM(v))
#define HASH_SET_STR(k,v) HASH_SET(k,rb_str_new2(v))
#define HASH_SET_TRUE(k)  HASH_SET(k,Qtrue)

void
objspace_info(VALUE obj, VALUE target_hash)
{
  size_t memsize;
  struct allocation_info *ainfo;
  rb_io_t *fptr;
  ID flags[RB_OBJ_GC_FLAGS_MAX];
  size_t n, i;
  VALUE obj_klass, references;

  obj_klass = BUILTIN_TYPE(obj) == T_NODE ? 0 : RBASIC_CLASS(obj);
  references = rb_ary_new();

  if (obj == target_hash)
    return;

  HASH_SET_STR("type", obj_type(obj));

  if (obj_klass)
    HASH_SET_PTR("class", obj_klass);
  if (rb_obj_frozen_p(obj))
    HASH_SET_TRUE("frozen");

  switch (BUILTIN_TYPE(obj)) {
  case T_NODE:
    HASH_SET_STR("node_type", ruby_node_name(nd_type(obj)));
    break;

  case T_STRING:
    if (STR_EMBED_P(obj))
      HASH_SET_TRUE("embedded");
    if (STR_ASSOC_P(obj))
      HASH_SET_TRUE("associated");
    if (is_broken_string(obj))
      HASH_SET_TRUE("broken");
    if (FL_TEST(obj, RSTRING_FSTR))
      HASH_SET_TRUE("fstring");
    if (STR_SHARED_P(obj))
      HASH_SET_TRUE("shared");
    else {
      HASH_SET_INT("bytesize", RSTRING_LEN(obj));

      if (!STR_EMBED_P(obj) && !STR_NOCAPA_P(obj) && (long)rb_str_capacity(obj) != RSTRING_LEN(obj))
        HASH_SET_INT("capacity", rb_str_capacity(obj));

      if (is_ascii_string(obj))
        HASH_SET("value", obj);
    }

    if (!ENCODING_IS_ASCII8BIT(obj))
      HASH_SET_STR("encoding", rb_enc_name(rb_enc_from_index(ENCODING_GET(obj))));

    break;

  case T_HASH:
    HASH_SET_INT("size", RHASH_SIZE(obj));

    if (FL_TEST(obj, HASH_PROC_DEFAULT))
      HASH_SET_PTR("default", RHASH_IFNONE(obj));
      break;

  case T_ARRAY:
    HASH_SET_INT("length", RARRAY_LEN(obj));

    if (RARRAY_LEN(obj) > 0 && FL_TEST(obj, ELTS_SHARED))
      HASH_SET_TRUE("shared");
    if (RARRAY_LEN(obj) > 0 && FL_TEST(obj, RARRAY_EMBED_FLAG))
      HASH_SET_TRUE("embedded");

    break;

  case T_CLASS:
  case T_MODULE:
    if (obj_klass)
      HASH_SET_STR("name", rb_class2name(obj));
    break;

  case T_DATA:
    if (RTYPEDDATA_P(obj))
      HASH_SET_STR("struct", RTYPEDDATA_TYPE(obj)->wrap_struct_name);
    break;

  case T_FLOAT:
    HASH_SET("value", obj);
    break;

  case T_OBJECT:
    HASH_SET_INT("ivars", ROBJECT_NUMIV(obj));
    break;

  case T_FILE:
    fptr = RFILE(obj)->fptr;
    if (fptr)
      HASH_SET_INT("fd", fptr->fd);
    break;

  case T_ZOMBIE:
    return;
  }

  rb_objspace_reachable_objects_from(obj, reachable_object_i, (void*)references);
  rb_ary_delete(references, PTR2FIX(obj_klass));
  HASH_SET("references", references);

  if ((ainfo = objspace_lookup_allocation_info(obj))) {
    HASH_SET_STR("file", ainfo->path);
    HASH_SET_ULONG("line", ainfo->line);
    HASH_SET_UINT("generation", ainfo->generation);

    if (RTEST(ainfo->mid))
      HASH_SET_STR("method", rb_id2name(SYM2ID(ainfo->mid)));
  }

  if ((memsize = rb_obj_memsize_of(obj)) > 0)
    HASH_SET_UINT("memsize", memsize);

  if ((n = rb_obj_gc_flags(obj, flags, sizeof(flags))) > 0) {
    VALUE flags_hash = rb_hash_new();
    for (i=0; i<n; i++) {
      rb_hash_aset(flags_hash, rb_str_new2(rb_id2name(flags[i])), Qtrue);
    }
    HASH_SET("flags", flags_hash);
  }
}
