static inline SCM
dscm_binding_incrgaps(SCM value)
{
        Arg a = {.i = scm_to_int(value)};
        incrgaps(&a);
        return SCM_BOOL_T;
}

static inline SCM
dscm_binding_incrigaps(SCM value)
{
        Arg a = {.i = scm_to_int(value)};
        incrigaps(&a);
        return SCM_BOOL_T;
}
static inline SCM
dscm_binding_incrogaps(SCM value)
{
        Arg a = {.i = scm_to_int(value)};
        incrogaps(&a);
        return SCM_BOOL_T;
}

static inline SCM
dscm_binding_incrohgaps(SCM value)
{
        Arg a = {.i = scm_to_int(value)};
        incrohgaps(&a);
        return SCM_BOOL_T;
}

static inline SCM
dscm_binding_incrovgaps(SCM value)
{
        Arg a = {.i = scm_to_int(value)};
        incrovgaps(&a);
        return SCM_BOOL_T;
}

static inline SCM
dscm_binding_incrihgaps(SCM value)
{
        Arg a = {.i = scm_to_int(value)};
        incrihgaps(&a);
        return SCM_BOOL_T;
}

static inline SCM
dscm_binding_incrivgaps(SCM value)
{
        Arg a = {.i = scm_to_int(value)};
        incrivgaps(&a);
        return SCM_BOOL_T;
}

static inline SCM
dscm_binding_togglegaps()
{
        togglegaps(NULL);
        return SCM_BOOL_T;
}

static inline SCM
dscm_binding_defaultgaps()
{
        defaultgaps(NULL);
        return SCM_BOOL_T;
}

static inline void
patch_vanitygaps_register()
{
        scm_c_define_gsubr("dwl:toggle-gaps", 0, 0, 0, &dscm_binding_togglegaps);
        scm_c_define_gsubr("dwl:default-gaps", 0, 0, 0, &dscm_binding_defaultgaps);
        scm_c_define_gsubr("dwl:gaps", 1, 0, 0, &dscm_binding_incrgaps);
        scm_c_define_gsubr("dwl:gaps-inner", 1, 0, 0, &dscm_binding_incrigaps);
        scm_c_define_gsubr("dwl:gaps-outer", 1, 0, 0, &dscm_binding_incrogaps);
        scm_c_define_gsubr("dwl:gaps-inner-horizontal", 1, 0, 0, &dscm_binding_incrihgaps);
        scm_c_define_gsubr("dwl:gaps-inner-vertical", 1, 0, 0, &dscm_binding_incrivgaps);
        scm_c_define_gsubr("dwl:gaps-outer-horizontal", 1, 0, 0, &dscm_binding_incrohgaps);
        scm_c_define_gsubr("dwl:gaps-outer-vertical", 1, 0, 0, &dscm_binding_incrovgaps);
}

static inline void
patch_vanitygaps_config_parse()
{
        gappih = dscm_alist_get_unsigned_int(config, "gaps-horizontal-inner", -1);
        gappiv = dscm_alist_get_unsigned_int(config, "gaps-vertical-inner", -1);
        gappoh = dscm_alist_get_unsigned_int(config, "gaps-horizontal-outer", -1);
        gappov = dscm_alist_get_unsigned_int(config, "gaps-vertical-outer", -1);
        smartgaps = dscm_alist_get_int(config, "smart-gaps");
}
