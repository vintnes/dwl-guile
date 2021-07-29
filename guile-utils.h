#pragma once

#define GUILE_RGB_COLOR_LENGTH  4
#define GUILE_MAX_LIST_LENGTH   500
#define GUILE_MAX_TAGS          100

typedef enum { GUILE_PROC_ARRANGE, GUILE_PROC_ACTION } GuileProcType;
typedef struct {
        SCM proc;
        void *args;
} GuileProcData;

static inline SCM
get_value(SCM alist, const char* key)
{
        return scm_assoc_ref(alist, scm_from_locale_string(key));
}

static inline char*
get_value_string(SCM alist, const char* key)
{
        SCM value = get_value(alist, key);
        if (scm_is_string(value))
                return scm_to_locale_string(value);
        return NULL;
}

static inline int
get_value_int(SCM alist, const char* key)
{
        SCM value = get_value(alist, key);
        if (scm_is_bool(value))
                return scm_is_true(value) ? 1 : 0;
        return scm_to_int(value);
}

static inline unsigned int
get_value_unsigned_int(SCM alist, const char* key, int max)
{
        return scm_to_unsigned_integer(get_value(alist, key), 0, max);
}

static inline float
get_value_float(SCM alist, const char* key)
{
        SCM value = get_value(alist, key);
        if (scm_is_bool(value))
                return scm_is_true(value) ? 1 : 0;
        return (float)scm_to_double(value);
}


static inline scm_t_bits *
get_value_proc_pointer(SCM alist, const char *key)
{
        scm_t_bits *proc = NULL;
        SCM value = get_value(alist, key);
        if (scm_is_false(value))
                return proc;
        SCM eval = scm_primitive_eval(value);
        /* SCM_UNPACK_POINTER is only allowed on expressions where SCM_IMP is 0 */
        if (SCM_IMP(eval) == 1)
                BARF("guile: invalid callback procedure. SCM_IMP(proc) = 1");
        if (scm_procedure_p(eval) == SCM_BOOL_T) {
                proc = SCM_UNPACK_POINTER(eval);
                scm_gc_protect_object(eval);
        }
        return proc;
}

static inline SCM
get_variable(const char *name)
{
        return scm_variable_ref(scm_c_lookup(name));
}

static inline unsigned int
get_list_length(SCM list)
{
        return scm_to_unsigned_integer(scm_length(list), 0, GUILE_MAX_LIST_LENGTH);
}

static inline SCM
get_list_item(SCM list, unsigned int index)
{
        return scm_list_ref(list, scm_from_unsigned_integer(index));
}

static inline uint32_t
get_value_modifiers(SCM alist, const char *key)
{
        SCM modifiers = get_value(alist, key);
        uint32_t mod = 0;
        unsigned int i = 0, length = get_list_length(modifiers);
        for (; i < length; i++) {
                SCM item = get_list_item(modifiers, i);
                SCM eval = scm_primitive_eval(item);
                mod |= scm_to_uint32(eval);
        }
        return mod;
}

static inline unsigned int
get_value_tag(SCM tag, unsigned int tags)
{
        unsigned int target_tag = scm_to_unsigned_integer(tag, 1, tags) - 1;
        return (1 << (target_tag));
}

static inline void *
guile_iterate_list(SCM list, size_t elem_size, int append_null,
        void (*iterator)(unsigned int, SCM, void*), unsigned int *length_var)
{
        unsigned int i = 0, length = 0;
        length = get_list_length(list);
        void *allocated = calloc(append_null ? length + 1 : length, elem_size);
        for (; i < length; i++) {
                SCM item = get_list_item(list, i);
                (*iterator)(i, item, allocated);
        }
        if (append_null)
                ((void**)allocated)[i] = NULL;
        if (length_var)
                *length_var = length;
        return allocated;
}

static inline void*
guile_call_action(void *data)
{
        return scm_call_0(((GuileProcData*)data)->proc);
}

static inline void*
guile_call_arrange(void *data)
{
        GuileProcData *proc_data = (GuileProcData*)data;
        SCM mon = scm_from_pointer(proc_data->args, NULL);
        return scm_call_1(proc_data->proc, mon);
}

static inline void
guile_call(GuileProcType type, scm_t_bits *proc_ptr, void *data)
{
        if (proc_ptr == NULL) {
                fprintf(stderr, "guile: could not call proc that is NULL");
                return;
        }
        SCM proc = SCM_PACK_POINTER(proc_ptr);
        GuileProcData proc_data = {.proc = proc, .args = data};
        void *(*func)(void*) = NULL;
        switch (type) {
            case GUILE_PROC_ARRANGE:
                func = &guile_call_arrange;
                break;
            case GUILE_PROC_ACTION:
            default:
                func = &guile_call_action;
        }
        scm_c_with_continuation_barrier(func, &proc_data);
}
