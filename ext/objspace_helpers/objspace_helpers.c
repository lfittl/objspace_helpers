#include <objspace_info.h>

void Init_objspace_helpers(void);

static int
collect_addresses(void *vstart, void *vend, size_t stride, void *data)
{
  VALUE v = (VALUE)vstart;
  for (; v != (VALUE)vend; v += stride) {
    if (RBASIC(v)->flags)
      rb_ary_push((VALUE) data, INT2FIX((void *)v));
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
  static const char filename[] = "rubyobj";
  VALUE obj = Qnil, opts = Qnil;

  VALUE hash = rb_hash_new();

  for (size_t i = 0; i < RARRAY_LEN(ary); i += 1) {
    VALUE obj_addr = rb_ary_entry(ary, i);
    VALUE obj = FIX2PTR(obj_addr);
    VALUE info_hash = rb_hash_new();
    objspace_info(obj, info_hash);
    rb_hash_aset(hash, obj_addr, info_hash);
  }

  return hash;
}

static VALUE oh_address_of_obj(VALUE self, VALUE obj)
{
  return PTR2FIX(obj);
}

static VALUE oh_obj_for_address(VALUE self, VALUE address)
{
  return FIX2PTR(address);
}

void Init_objspace_helpers(void)
{
  VALUE cObjspaceHelpers;

  cObjspaceHelpers = rb_const_get(rb_cObject, rb_intern("ObjspaceHelpers"));

  rb_define_singleton_method(cObjspaceHelpers, "_dump_addresses", oh_dump_addresses, 0);
  rb_define_singleton_method(cObjspaceHelpers, "_addresses_to_info", oh_addresses_to_info, 1);
  rb_define_singleton_method(cObjspaceHelpers, "_address_of_obj", oh_address_of_obj, 1);
  rb_define_singleton_method(cObjspaceHelpers, "_obj_for_address", oh_obj_for_address, 1);
}