/* object_tracing.c */
struct allocation_info {
  /* all of information don't need marking. */
  int living;
  VALUE flags;
  VALUE klass;

  /* allocation info */
  const char *path;
  unsigned long line;
  const char *class_path;
  VALUE mid;
  size_t generation;
};
extern struct allocation_info *objspace_lookup_allocation_info(VALUE obj);
