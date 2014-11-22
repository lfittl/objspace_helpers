/* internal.h */

#define STR_NOEMBED FL_USER1
#define STR_SHARED  FL_USER2 /* = ELTS_SHARED */
#define STR_ASSOC   FL_USER3
#define STR_SHARED_P(s) FL_ALL((s), STR_NOEMBED|ELTS_SHARED)
#define STR_ASSOC_P(s)  FL_ALL((s), STR_NOEMBED|STR_ASSOC)
#define STR_NOCAPA  (STR_NOEMBED|ELTS_SHARED|STR_ASSOC)
#define STR_NOCAPA_P(s) (FL_TEST((s),STR_NOEMBED) && FL_ANY((s),ELTS_SHARED|STR_ASSOC))
#define STR_EMBED_P(str) (!FL_TEST((str), STR_NOEMBED))
#define is_ascii_string(str) (rb_enc_str_coderange(str) == ENC_CODERANGE_7BIT)
#define is_broken_string(str) (rb_enc_str_coderange(str) == ENC_CODERANGE_BROKEN)

#define HASH_DELETED  FL_USER1
#define HASH_PROC_DEFAULT FL_USER2

size_t rb_obj_memsize_of(VALUE);
#define RB_OBJ_GC_FLAGS_MAX 5
size_t rb_obj_gc_flags(VALUE, ID[], size_t);
void rb_gc_mark_values(long n, const VALUE *values);


/* vm_core.h */

const char *ruby_node_name(int node);

/* debug.h */

VALUE rb_tracepoint_new(VALUE target_thread_not_supported_yet, rb_event_flag_t events, void (*func)(VALUE, void *), void *data);
VALUE rb_tracepoint_enable(VALUE tpval);
VALUE rb_tracepoint_disable(VALUE tpval);
VALUE rb_tracepoint_enabled_p(VALUE tpval);

typedef struct rb_trace_arg_struct rb_trace_arg_t;
rb_trace_arg_t *rb_tracearg_from_tracepoint(VALUE tpval);

rb_event_flag_t rb_tracearg_event_flag(rb_trace_arg_t *trace_arg);
VALUE rb_tracearg_event(rb_trace_arg_t *trace_arg);
VALUE rb_tracearg_lineno(rb_trace_arg_t *trace_arg);
VALUE rb_tracearg_path(rb_trace_arg_t *trace_arg);
VALUE rb_tracearg_method_id(rb_trace_arg_t *trace_arg);
VALUE rb_tracearg_defined_class(rb_trace_arg_t *trace_arg);
VALUE rb_tracearg_binding(rb_trace_arg_t *trace_arg);
VALUE rb_tracearg_self(rb_trace_arg_t *trace_arg);
VALUE rb_tracearg_return_value(rb_trace_arg_t *trace_arg);
VALUE rb_tracearg_raised_exception(rb_trace_arg_t *trace_arg);
VALUE rb_tracearg_object(rb_trace_arg_t *trace_arg);

/* node.h */

typedef struct RNode {
    VALUE flags;
    VALUE nd_reserved;		/* ex nd_file */
    union {
	struct RNode *node;
	ID id;
	VALUE value;
	VALUE (*cfunc)(ANYARGS);
	ID *tbl;
    } u1;
    union {
	struct RNode *node;
	ID id;
	long argc;
	VALUE value;
    } u2;
    union {
	struct RNode *node;
	ID id;
	long state;
	struct rb_global_entry *entry;
	struct rb_args_info *args;
	long cnt;
	VALUE value;
    } u3;
} NODE;

#define RNODE(obj)  (R_CAST(RNode)(obj))

#define NODE_FL_NEWLINE             (((VALUE)1)<<7)
#define NODE_FL_CREF_PUSHED_BY_EVAL (((VALUE)1)<<15)
#define NODE_FL_CREF_OMOD_SHARED    (((VALUE)1)<<16)

#define NODE_TYPESHIFT 8
#define NODE_TYPEMASK  (((VALUE)0x7f)<<NODE_TYPESHIFT)

#define nd_type(n) ((int) (((RNODE(n))->flags & NODE_TYPEMASK)>>NODE_TYPESHIFT))
