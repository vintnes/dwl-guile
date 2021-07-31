static double default_alpha = 1.0;

static inline SCM
dscm_binding_changealpha(SCM value)
{
        Arg a = {.f = scm_to_double(value)};
        changealpha(&a);
        return SCM_BOOL_T;
}

static inline void
patch_alpha_register()
{
        scm_c_define_gsubr("dwl:change-alpha", 1, 0, 0, &dscm_binding_changealpha);
}

static inline void
patch_alpha_config_parse()
{
        default_alpha = dscm_alist_get_double(config, "default-alpha");
}
