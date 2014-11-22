struct traceobj_arg {
  int running;
  int keep_remains;
  VALUE newobj_trace;
  VALUE freeobj_trace;
  st_table *object_table; /* obj (VALUE) -> allocation_info */
  st_table *str_table;  /* cstr -> refcount */
  struct traceobj_arg *prev_traceobj_arg;
};

static const char *
make_unique_str(st_table *tbl, const char *str, long len)
{
  if (!str) {
    return NULL;
  }
  else {
    st_data_t n;
    char *result;

    if (st_lookup(tbl, (st_data_t)str, &n)) {
      st_insert(tbl, (st_data_t)str, n+1);
      st_get_key(tbl, (st_data_t)str, (st_data_t *)&result);
    }
    else {
      result = (char *)ruby_xmalloc(len+1);
      strncpy(result, str, len);
      result[len] = 0;
      st_add_direct(tbl, (st_data_t)result, 1);
    }
    return result;
  }
}

static void
delete_unique_str(st_table *tbl, const char *str)
{
  if (str) {
    st_data_t n;

    st_lookup(tbl, (st_data_t)str, &n);
    if (n == 1) {
      st_delete(tbl, (st_data_t *)&str, 0);
      ruby_xfree((char *)str);
    }
    else {
      st_insert(tbl, (st_data_t)str, n-1);
    }
  }
}

static void
newobj_i(VALUE tpval, void *data)
{
  struct traceobj_arg *arg = (struct traceobj_arg *)data;
  rb_trace_arg_t *tparg = rb_tracearg_from_tracepoint(tpval);
  VALUE obj = rb_tracearg_object(tparg);
  VALUE path = rb_tracearg_path(tparg);
  VALUE line = rb_tracearg_lineno(tparg);
  VALUE mid = rb_tracearg_method_id(tparg);
  VALUE klass = rb_tracearg_defined_class(tparg);
  struct allocation_info *info;
  const char *path_cstr = RTEST(path) ? make_unique_str(arg->str_table, RSTRING_PTR(path), RSTRING_LEN(path)) : 0;
  VALUE class_path = (RTEST(klass) && !OBJ_FROZEN(klass)) ? rb_class_path_cached(klass) : Qnil;
  const char *class_path_cstr = RTEST(class_path) ? make_unique_str(arg->str_table, RSTRING_PTR(class_path), RSTRING_LEN(class_path)) : 0;

  if (st_lookup(arg->object_table, (st_data_t)obj, (st_data_t *)&info)) {
    if (arg->keep_remains) {
      if (info->living) {
        /* do nothing. there is possibility to keep living if FREEOBJ events while suppressing tracing */
      }
    }
    /* reuse info */
    delete_unique_str(arg->str_table, info->path);
    delete_unique_str(arg->str_table, info->class_path);
  }
  else {
    info = (struct allocation_info *)ruby_xmalloc(sizeof(struct allocation_info));
  }
  info->living = 1;
  info->flags = RBASIC(obj)->flags;
  info->klass = RBASIC_CLASS(obj);

  info->path = path_cstr;
  info->line = NUM2INT(line);
  info->mid = mid;
  info->class_path = class_path_cstr;
  info->generation = rb_gc_count();
  st_insert(arg->object_table, (st_data_t)obj, (st_data_t)info);
}

static void
freeobj_i(VALUE tpval, void *data)
{
  struct traceobj_arg *arg = (struct traceobj_arg *)data;
  rb_trace_arg_t *tparg = rb_tracearg_from_tracepoint(tpval);
  VALUE obj = rb_tracearg_object(tparg);
  struct allocation_info *info;

  if (st_lookup(arg->object_table, (st_data_t)obj, (st_data_t *)&info)) {
    if (arg->keep_remains) {
      info->living = 0;
    }
    else {
      st_delete(arg->object_table, (st_data_t *)&obj, (st_data_t *)&info);
      delete_unique_str(arg->str_table, info->path);
      delete_unique_str(arg->str_table, info->class_path);
      ruby_xfree(info);
    }
  }
}

static int
free_keys_i(st_data_t key, st_data_t value, void *data)
{
  ruby_xfree((void *)key);
  return ST_CONTINUE;
}

static int
free_values_i(st_data_t key, st_data_t value, void *data)
{
  ruby_xfree((void *)value);
  return ST_CONTINUE;
}

static struct traceobj_arg *tmp_trace_arg; /* TODO: Do not use global variables */
static int tmp_keep_remains;         /* TODO: Do not use global variables */

static struct traceobj_arg *
get_traceobj_arg(void)
{
  if (tmp_trace_arg == 0) {
    tmp_trace_arg = ALLOC_N(struct traceobj_arg, 1);
    tmp_trace_arg->running = 0;
    tmp_trace_arg->keep_remains = tmp_keep_remains;
    tmp_trace_arg->newobj_trace = 0;
    tmp_trace_arg->freeobj_trace = 0;
    tmp_trace_arg->object_table = st_init_numtable();
    tmp_trace_arg->str_table = st_init_strtable();
  }
  return tmp_trace_arg;
}

/*
 * call-seq: trace_object_allocations_start
 *
 * Starts tracing object allocations.
 *
 */
static VALUE
trace_object_allocations_start(VALUE self)
{
  struct traceobj_arg *arg = get_traceobj_arg();

  if (arg->running++ > 0) {
    /* do nothing */
  }
  else {
    if (arg->newobj_trace == 0) {
      arg->newobj_trace = rb_tracepoint_new(0, RUBY_INTERNAL_EVENT_NEWOBJ, newobj_i, arg);
      arg->freeobj_trace = rb_tracepoint_new(0, RUBY_INTERNAL_EVENT_FREEOBJ, freeobj_i, arg);
    }
    rb_tracepoint_enable(arg->newobj_trace);
    rb_tracepoint_enable(arg->freeobj_trace);
  }

  return Qnil;
}

/*
 * call-seq: trace_object_allocations_stop
 *
 * Stop tracing object allocations.
 *
 * Note that if ::trace_object_allocations_start is called n-times, then
 * tracing will stop after calling ::trace_object_allocations_stop n-times.
 *
 */
static VALUE
trace_object_allocations_stop(VALUE self)
{
  struct traceobj_arg *arg = get_traceobj_arg();

  if (arg->running > 0) {
    arg->running--;
  }

  if (arg->running == 0) {
    rb_tracepoint_disable(arg->newobj_trace);
    rb_tracepoint_disable(arg->freeobj_trace);
    arg->newobj_trace = 0;
    arg->freeobj_trace = 0;
  }

  return Qnil;
}

/*
 * call-seq: trace_object_allocations_clear
 *
 * Clear recorded tracing information.
 *
 */
static VALUE
trace_object_allocations_clear(VALUE self)
{
  struct traceobj_arg *arg = get_traceobj_arg();

  /* clear tables */
  st_foreach(arg->object_table, free_values_i, 0);
  st_clear(arg->object_table);
  st_foreach(arg->str_table, free_keys_i, 0);
  st_clear(arg->str_table);

  /* do not touch TracePoints */

  return Qnil;
}

/*
 * call-seq: trace_object_allocations { block }
 *
 * Starts tracing object allocations from the ObjectSpace extension module.
 *
 * For example:
 *
 *    require 'objspace'
 *
 *    class C
 *      include ObjectSpace
 *
 *      def foo
 *      trace_object_allocations do
 *        obj = Object.new
 *        p "#{allocation_sourcefile(obj)}:#{allocation_sourceline(obj)}"
 *      end
 *      end
 *    end
 *
 *    C.new.foo #=> "objtrace.rb:8"
 *
 * This example has included the ObjectSpace module to make it easier to read,
 * but you can also use the ::trace_object_allocations notation (recommended).
 *
 * Note that this feature introduces a huge performance decrease and huge
 * memory consumption.
 */
static VALUE
trace_object_allocations(VALUE self)
{
  trace_object_allocations_start(self);
  return rb_ensure(rb_yield, Qnil, trace_object_allocations_stop, self);
}

int rb_bug_reporter_add(void (*func)(FILE *, void *), void *data);
static int object_allocations_reporter_registered = 0;

static int
object_allocations_reporter_i(st_data_t key, st_data_t val, st_data_t ptr)
{
  FILE *out = (FILE *)ptr;
  VALUE obj = (VALUE)key;
  struct allocation_info *info = (struct allocation_info *)val;

  fprintf(out, "-- %p (%s F: %p, ", (void *)obj, info->living ? "live" : "dead", (void *)info->flags);
  if (info->class_path) fprintf(out, "C: %s", info->class_path);
  else          fprintf(out, "C: %p", (void *)info->klass);
  fprintf(out, "@%s:%lu", info->path ? info->path : "", info->line);
  if (!NIL_P(info->mid)) fprintf(out, " (%s)", rb_id2name(SYM2ID(info->mid)));
  fprintf(out, ")\n");

  return ST_CONTINUE;
}

static void
object_allocations_reporter(FILE *out, void *ptr)
{
  fprintf(out, "== object_allocations_reporter: START\n");
  if (tmp_trace_arg) {
    st_foreach(tmp_trace_arg->object_table, object_allocations_reporter_i, (st_data_t)out);
  }
  fprintf(out, "== object_allocations_reporter: END\n");
}

static VALUE
trace_object_allocations_debug_start(VALUE self)
{
  tmp_keep_remains = 1;
  if (object_allocations_reporter_registered == 0) {
    object_allocations_reporter_registered = 1;
    rb_bug_reporter_add(object_allocations_reporter, 0);
  }

  return trace_object_allocations_start(self);
}

static struct allocation_info *
lookup_allocation_info(VALUE obj)
{
  if (tmp_trace_arg) {
     struct allocation_info *info;
    if (st_lookup(tmp_trace_arg->object_table, obj, (st_data_t *)&info)) {
      return info;
    }
  }
  return NULL;
}

struct allocation_info *
objspace_lookup_allocation_info(VALUE obj)
{
  return lookup_allocation_info(obj);
}
