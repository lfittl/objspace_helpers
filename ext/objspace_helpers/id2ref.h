#ifndef RUBY_ID2REF
#define RUBY_ID2REF

#include <ruby.h>
#include <ruby_private/gc.h>

VALUE oh_id2ref(VALUE self, VALUE objid);

#endif
