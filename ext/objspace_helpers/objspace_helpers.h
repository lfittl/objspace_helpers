#ifndef RUBY_OBJSPACE_HELPERS
#define RUBY_OBJSPACE_HELPERS

#include <ruby.h>
#include <ruby_private/gc.h>

#define PTR2FIX(v) INT2FIX((void *)v)
#define FIX2PTR(v) ((VALUE)NUM2SIZET(v))

#endif
