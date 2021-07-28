static inline SCM
guile_proc_monocle(SCM monitor)
{
        Monitor *m = (Monitor*)scm_to_pointer(monitor);
        monocle(m);
        return SCM_BOOL_T;
}

static inline SCM
guile_proc_tile(SCM monitor)
{
        Monitor *m = (Monitor*)scm_to_pointer(monitor);
        tile(m);
        return SCM_BOOL_T;
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
        return SCM_BOOL_T;
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
        return SCM_BOOL_T;
}

static inline SCM
guile_proc_spawn_terminal()
{
        Arg a = {.v = termcmd};
        spawn(&a);
        return SCM_BOOL_T;
}

static inline SCM
guile_proc_focusstack(SCM value)
{
        Arg a = {.i = scm_to_int(value)};
        focusstack(&a);
        return SCM_BOOL_T;
}

static inline SCM
guile_proc_setmfact(SCM value)
{
        Arg a = {.f = scm_to_double(value)};
        setmfact(&a);
        return SCM_BOOL_T;
}

/* static inline SCM */
/* guile_proc_togglegaps() */
/* { */
/*         togglegaps(NULL); */
/*         return SCM_BOOL_T; */
/* } */

static inline SCM
guile_proc_togglefloating()
{
        togglefloating(NULL);
        return SCM_BOOL_T;
}

static inline SCM
guile_proc_zoom()
{
        zoom(NULL);
        return SCM_BOOL_T;
}

static inline SCM
guile_proc_setlayout(SCM value)
{
        char *id = scm_to_locale_string(value);

        Layout *layout = NULL;
        for (int i = 0; i < numlayouts; i++) {
            if (strcmp(layouts[i].id, id) == 0)
                layout = &layouts[i];
        }
        if (layout == NULL)
            return SCM_BOOL_F;
        Arg a = {.v = layout};

        setlayout(&a);
        return SCM_BOOL_T;
}

static inline SCM
guile_proc_togglefullscreen()
{
        togglefullscreen(NULL);
        return SCM_BOOL_T;
}

static inline SCM
guile_proc_focusmon(SCM value)
{
        SCM eval = scm_primitive_eval(value);
        Arg a = {.i = scm_to_int(eval)};
        focusmon(&a);
        return SCM_BOOL_T;
}

static inline SCM
guile_proc_tagmon(SCM value)
{
        SCM eval = scm_primitive_eval(value);
        Arg a = {.i = scm_to_int(eval)};
        tagmon(&a);
        return SCM_BOOL_T;
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
        return SCM_BOOL_T;
}

static inline SCM
guile_proc_view(SCM value)
{
        Arg a = {.ui = get_value_tag(value, numtags)};
        view(&a);
        return SCM_BOOL_T;
}

static inline SCM
guile_proc_toggleview(SCM value)
{
        Arg a = {.ui = get_value_tag(value, numtags)};
        toggleview(&a);
        return SCM_BOOL_T;
}

static inline SCM
guile_proc_tag(SCM value)
{
        Arg a = {.ui = get_value_tag(value, numtags)};
        tag(&a);
        return SCM_BOOL_T;
}

static inline SCM
guile_proc_toggletag(SCM value)
{
        Arg a = {.ui = get_value_tag(value, numtags)};
        toggletag(&a);
        return SCM_BOOL_T;
}

static inline SCM
guile_proc_moveresize(SCM cursor)
{
        Arg a = {.ui = scm_to_unsigned_integer(cursor, 0, CurResize)};
        moveresize(&a);
        return SCM_BOOL_T;
}

static inline SCM
guile_proc_killclient()
{
        killclient(NULL);
        return SCM_BOOL_T;
}

static inline SCM
guile_proc_quit()
{
        quit(NULL);
        return SCM_BOOL_T;
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
        /* TODO: add bindings for other mouse buttons */
        scm_c_define("DIRECTION-LEFT", scm_from_int(WLR_DIRECTION_LEFT));
        scm_c_define("DIRECTION-RIGHT", scm_from_int(WLR_DIRECTION_RIGHT));
        scm_c_define("MOUSE-LEFT", scm_from_int(BTN_LEFT));
        scm_c_define("MOUSE-MIDDLE", scm_from_int(BTN_MIDDLE));
        scm_c_define("MOUSE-RIGHT", scm_from_int(BTN_RIGHT));
        scm_c_define("CURSOR-NORMAL", scm_from_int(CurNormal));
        scm_c_define("CURSOR-MOVE", scm_from_int(CurMove));
        scm_c_define("CURSOR-RESIZE", scm_from_int(CurResize));
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
        /* The following bindings are a one-to-one translation
           of the user-callable functions in dwl. Each scheme function
           has the same name as the corresponding dwl function, but
           with a dash ('-') between words. Instead of accepting a
           `Arg` struct as argument, each scheme function takes
           in each parameter separately. */
        scm_c_define_gsubr("dwl:chvt", 1, 0, 0, &guile_proc_chvt);
        scm_c_define_gsubr("dwl:quit", 0, 0, 0, &guile_proc_quit);
        scm_c_define_gsubr("dwl:killclient", 0, 0, 0, &guile_proc_killclient);
        scm_c_define_gsubr("dwl:tile", 1, 0, 0, &guile_proc_tile);
        scm_c_define_gsubr("dwl:monocle", 1, 0, 0, &guile_proc_monocle);
        scm_c_define_gsubr("dwl:spawn", 1, 0, 0, &guile_proc_spawn);
        scm_c_define_gsubr("dwl:view", 1, 0, 0, &guile_proc_view);
        scm_c_define_gsubr("dwl:toggle-view", 1, 0, 0, &guile_proc_toggleview);
        scm_c_define_gsubr("dwl:tag", 1, 0, 0, &guile_proc_tag);
        scm_c_define_gsubr("dwl:toggle-tag", 1, 0, 0, &guile_proc_toggletag);
        scm_c_define_gsubr("dwl:focus-stack", 1, 0, 0, &guile_proc_focusstack);
        scm_c_define_gsubr("dwl:set-master-factor", 1, 0, 0, &guile_proc_setmfact);
        scm_c_define_gsubr("dwl:zoom", 0, 0, 0, &guile_proc_zoom);
        scm_c_define_gsubr("dwl:set-layout", 1, 0, 0, &guile_proc_setlayout);
        scm_c_define_gsubr("dwl:toggle-fullscreen", 0, 0, 0, &guile_proc_togglefullscreen);
        scm_c_define_gsubr("dwl:toggle-floating", 0, 0, 0, &guile_proc_togglefloating);
        scm_c_define_gsubr("dwl:focus-monitor", 1, 0, 0, &guile_proc_focusmon);
        scm_c_define_gsubr("dwl:tag-monitor", 1, 0, 0, &guile_proc_tagmon);
        scm_c_define_gsubr("dwl:move-resize", 1, 0, 0, &guile_proc_moveresize);

        /* Custom helper bindings. These bindings corresponds to functions
           that are not present in dwl by default. They serve as utilites
           for making certain actions simpler, e.g. spawning your terminal
           or spawing a generic shell command. */
        scm_c_define_gsubr("dwl:shcmd", 1, 0, 0, &guile_proc_shcmd);
        scm_c_define_gsubr("dwl:spawn-menu", 0, 0, 0, &guile_proc_spawn_menu);
        scm_c_define_gsubr("dwl:spawn-terminal", 0, 0, 0, &guile_proc_spawn_terminal);
}
