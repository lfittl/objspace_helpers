#ifndef RUBY_OBJSPACE_INFO
#define RUBY_OBJSPACE_INFO

#include <ruby.h>
#include <ruby/io.h>

#include <ruby_private/internal_defs.h>
#include <ruby_private/objspace.h>

#include <objspace_helpers.h>

void
objspace_info(VALUE obj, VALUE target_hash);

#endif
