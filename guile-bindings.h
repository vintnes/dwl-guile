static inline SCM
guile_proc_monocle(SCM monitor)
{
        Monitor *m = (Monitor*)scm_to_pointer(monitor);
        monocle(m);
        return SCM_UNSPECIFIED;
}

static inline SCM
guile_proc_tile(SCM monitor)
{
        Monitor *m = (Monitor*)scm_to_pointer(monitor);
        tile(m);
        return SCM_UNSPECIFIED;
}

static inline SCM
guile_proc_spawn(SCM args)
{
        if (scm_is_null(args))
                return SCM_BOOL_F;
        unsigned int i = 0, length = get_list_length(args);
        char *cmd_args[length + 1];
        for (; i < length; i++) {
                SCM arg_exp = get_list_item(args, i);
                char *arg = scm_to_locale_string(arg_exp);
                cmd_args[i] = arg;
                scm_dynwind_free(arg);
        }
        cmd_args[i] = NULL;
        Arg a = {.v = cmd_args};
        spawn(&a);
        return SCM_UNSPECIFIED;
}

static inline SCM
guile_proc_shcmd(SCM args)
{
        SCM extended = scm_list_3(scm_from_locale_string("/bin/sh"),
                scm_from_locale_string("-c"), args);
        return guile_proc_spawn(extended);
}

static inline SCM
guile_proc_spawn_menu()
{
        Arg a = {.v = menucmd};
        spawn(&a);
        return SCM_UNSPECIFIED;
}

static inline SCM
guile_proc_spawn_terminal()
{
        Arg a = {.v = termcmd};
        spawn(&a);
        return SCM_UNSPECIFIED;
}

static inline SCM
guile_proc_chvt(SCM tty)
{
        if (!scm_is_number(tty))
                return SCM_BOOL_F;
        int target_tty = scm_to_int(tty);
        if (target_tty <= 0 || target_tty > 12)
                return SCM_BOOL_F;
        Arg a = {.ui = (unsigned int)target_tty};
        chvt(&a);
        return SCM_UNSPECIFIED;
}

static inline SCM
guile_proc_quit()
{
        quit(NULL);
        return SCM_UNSPECIFIED;
}

static inline void
guile_register_constants()
{
        scm_c_define("SHIFT", scm_from_int(WLR_MODIFIER_SHIFT));
        scm_c_define("CAPS", scm_from_int(WLR_MODIFIER_CAPS));
        scm_c_define("CTRL", scm_from_int(WLR_MODIFIER_CTRL));
        scm_c_define("ALT", scm_from_int(WLR_MODIFIER_ALT));
        scm_c_define("MOD2", scm_from_int(WLR_MODIFIER_MOD2));
        scm_c_define("MOD3", scm_from_int(WLR_MODIFIER_MOD3));
        scm_c_define("SUPER", scm_from_int(WLR_MODIFIER_LOGO));
        scm_c_define("MOD5", scm_from_int(WLR_MODIFIER_MOD5));
        scm_c_define("MOUSE-LEFT", scm_from_int(BTN_LEFT));
        scm_c_define("MOUSE-MIDDLE", scm_from_int(BTN_MIDDLE));
        scm_c_define("MOUSE-RIGHT", scm_from_int(BTN_RIGHT));
        scm_c_define("TRANSFORM-NORMAL",
                scm_from_int(WL_OUTPUT_TRANSFORM_NORMAL));
        scm_c_define("TRANSFORM-ROTATE-90",
                scm_from_int(WL_OUTPUT_TRANSFORM_90));
        scm_c_define("TRANSFORM-ROTATE-180",
                scm_from_int(WL_OUTPUT_TRANSFORM_180));
        scm_c_define("TRANSFORM-ROTATE-270",
                scm_from_int(WL_OUTPUT_TRANSFORM_270));
        scm_c_define("TRANSFORM-FLIPPED",
                scm_from_int(WL_OUTPUT_TRANSFORM_FLIPPED));
        scm_c_define("TRANSFORM-FLIPPED-90",
                scm_from_int(WL_OUTPUT_TRANSFORM_FLIPPED_90));
        scm_c_define("TRANSFORM-FLIPPED-180",
                scm_from_int(WL_OUTPUT_TRANSFORM_FLIPPED_180));
        scm_c_define("TRANSFORM-FLIPPED-270",
                scm_from_int(WL_OUTPUT_TRANSFORM_FLIPPED_270));
}

static inline void
guile_register_procedures()
{
        scm_c_define_gsubr("chvt", 1, 0, 0, &guile_proc_chvt);
        scm_c_define_gsubr("quit", 0, 0, 0, &guile_proc_quit);
        scm_c_define_gsubr("tile", 1, 0, 0, &guile_proc_tile);
        scm_c_define_gsubr("monocle", 1, 0, 0, &guile_proc_monocle);
        scm_c_define_gsubr("spawn", 1, 0, 0, &guile_proc_spawn);
        scm_c_define_gsubr("spawn_menu", 0, 0, 0, &guile_proc_spawn_menu);
        scm_c_define_gsubr("spawn-terminal", 0, 0, 0, &guile_proc_spawn_terminal);
        scm_c_define_gsubr("shcmd", 1, 0, 0, &guile_proc_shcmd);
}
