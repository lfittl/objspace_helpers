#include <objspace_helpers.h>
#include <objspace_info.h>

void Init_objspace_helpers(void);

static int
collect_addresses(void *vstart, void *vend, size_t stride, void *data)
{
  VALUE v = (VALUE)vstart;
  for (; v != (VALUE)vend; v += stride) {
    if (RBASIC(v)->flags)
      rb_ary_push((VALUE) data, rb_obj_id(v));
  }
  return 0;
}

static VALUE oh_dump_addresses(VALUE self)
{
  VALUE ary = rb_ary_new();
  rb_objspace_each_objects(collect_addresses, (void *)ary);
  return ary;
}

static VALUE oh_addresses_to_info(VALUE self, VALUE ary)
{
  VALUE hash, obj_addr, info_hash;
  long i;

  hash = rb_hash_new();

  for (i = 0; i < RARRAY_LEN(ary); i += 1) {
    obj_addr = rb_ary_entry(ary, i);
    info_hash = rb_hash_new();
    objspace_info(oh_id2ref(self, obj_addr), info_hash);
    rb_hash_aset(hash, obj_addr, info_hash);
  }

  return hash;
}

static void
reachable_object_i(VALUE ref, void *data)
{
  rb_ary_push((VALUE)data, rb_obj_id(ref));
}

static VALUE oh_addresses_to_references(VALUE self, VALUE addresses)
{
  VALUE hash, obj_addr, references;
  long i;

  hash = rb_hash_new();

  for (i = 0; i < RARRAY_LEN(addresses); i += 1) {
    obj_addr = rb_ary_entry(addresses, i);

    references = rb_ary_new();
    rb_objspace_reachable_objects_from(oh_id2ref(self, obj_addr), reachable_object_i, (void*)references);

    rb_hash_aset(hash, obj_addr, references);
  }

  return hash;
}

void Init_objspace_helpers(void)
{
  VALUE cObjspaceHelpers;

  cObjspaceHelpers = rb_const_get(rb_cObject, rb_intern("ObjspaceHelpers"));

  rb_define_singleton_method(cObjspaceHelpers, "_dump_addresses", oh_dump_addresses, 0);
  rb_define_singleton_method(cObjspaceHelpers, "_addresses_to_info", oh_addresses_to_info, 1);
  rb_define_singleton_method(cObjspaceHelpers, "_addresses_to_references", oh_addresses_to_references, 1);
  rb_define_singleton_method(cObjspaceHelpers, "_id2ref", oh_id2ref, 1);
}
