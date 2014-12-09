/* Note (LukasFittl): Copy of ObjectSpace._id2ref from Ruby 2.1 source,
 * directly callable without the VM overhead, and more foregiving for internal objects.
 */

#include <id2ref.h>

#if SIZEOF_LONG == SIZEOF_VOIDP
# define nonspecial_obj_id(obj) (VALUE)((SIGNED_VALUE)(obj)|FIXNUM_FLAG)
# define obj_id_to_ref(objid) ((objid) ^ FIXNUM_FLAG) /* unset FIXNUM_FLAG */
#elif SIZEOF_LONG_LONG == SIZEOF_VOIDP
# define nonspecial_obj_id(obj) LL2NUM((SIGNED_VALUE)(obj) / 2)
# define obj_id_to_ref(objid) (FIXNUM_P(objid) ? \
   ((objid) ^ FIXNUM_FLAG) : (NUM2PTR(objid) << 1))
#else
# error not supported
#endif

VALUE
oh_id2ref(VALUE self, VALUE objid)
{
#if SIZEOF_LONG == SIZEOF_VOIDP
#define NUM2PTR(x) NUM2ULONG(x)
#elif SIZEOF_LONG_LONG == SIZEOF_VOIDP
#define NUM2PTR(x) NUM2ULL(x)
#endif
  VALUE ptr;
  void *p0;

  ptr = NUM2PTR(objid);
  p0 = (void *)ptr;

  if (ptr == Qtrue) return Qtrue;
  if (ptr == Qfalse) return Qfalse;
  if (ptr == Qnil) return Qnil;
  if (FIXNUM_P(ptr)) return (VALUE)ptr;
  if (FLONUM_P(ptr)) return (VALUE)ptr;
  ptr = obj_id_to_ref(objid);

  if ((ptr % rb_intern("RVALUE_SIZE")) == (4 << 2)) {
    ID symid = ptr / rb_intern("RVALUE_SIZE");
    if (rb_id2name(symid) == 0) return Qundef;
    return ID2SYM(symid);
  }

  return (VALUE)ptr;
}
