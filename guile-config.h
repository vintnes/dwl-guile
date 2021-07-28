#pragma once

/* Config variable definitions. */
/* These will be automatically set from the guile config. */
static int repeat_rate          = 25;
static int repeat_delay         = 600;
static int sloppyfocus          = 1;
static int tap_to_click         = 1;
static int natural_scrolling    = 1;
static unsigned int borderpx    = 1;
static float *rootcolor         = NULL;
static float *bordercolor       = NULL;
static float *focuscolor        = NULL;
static char **tags              = NULL;
static char **termcmd           = NULL;
static char **menucmd           = NULL;
static Layout *layouts          = NULL;
static MonitorRule *monrules    = NULL;
static Rule *rules              = NULL;
static Key *keys                = NULL;
static Button *buttons          = NULL;
static struct xkb_rule_names *xkb_rules = NULL;

static unsigned int numtags     = 0;
static unsigned int numkeys     = 0;
static unsigned int numrules    = 0;
static unsigned int numlayouts  = 0;
static unsigned int nummonrules = 0;
static unsigned int numbuttons  = 0;
static unsigned int TAGMASK     = 0;

static inline void
guile_parse_color(unsigned int index, SCM value, void *data)
{
        ((float*)data)[index] = (float)scm_to_double(value);
}

static inline void
guile_parse_string(unsigned int index, SCM str, void *data)
{
        ((char**)data)[index] = scm_to_locale_string(str);
}

static inline void
guile_parse_layout(unsigned int index, SCM layout, void *data)
{
        ((Layout*)data)[index] = (Layout){
                .id = get_value_string(layout, "id"),
                .symbol = get_value_string(layout, "symbol"),
                .arrange = get_value_proc_pointer(layout, "arrange")
        };
}

static inline void
guile_parse_monitor_rule(unsigned int index, SCM rule, void *data)
{
        SCM transform = get_value(rule, "transform");
        SCM eval = scm_primitive_eval(transform);
        ((MonitorRule*)data)[index] = (MonitorRule){
                .name = get_value_string(rule, "name"),
                .mfact = get_value_float(rule, "master-factor"),
                .nmaster = get_value_int(rule, "masters"),
                .scale = get_value_float(rule, "scale"),
                .lt = &layouts[get_value_int(rule, "layout")],
                .rr = (enum wl_output_transform)scm_to_int(eval),
                .x = get_value_int(rule, "x"),
                .y = get_value_int(rule, "y"),
        };
}

static inline void
guile_parse_rule(unsigned int index, SCM rule, void *data)
{
         ((Rule*)data)[index] = (Rule){
                .id = get_value_string(rule, "id"),
                .title = get_value_string(rule, "title"),
                .tags = get_value_unsigned_int(rule, "tag", GUILE_MAX_TAGS),
                .isfloating = get_value_int(rule, "floating"),
                .monitor = get_value_int(rule, "monitor")
         };
}

static inline void
guile_parse_key(unsigned int index, SCM key, void *data)
{
        xkb_keycode_t keycode = get_value_unsigned_int(key, "key", -1);
        /* Should we use `xkb_keycode_is_legal_x11`? */
        if (!xkb_keycode_is_legal_x11(keycode)
                || !xkb_keycode_is_legal_ext(keycode))
                BARF("guile: keycode '%d' is not a legal keycode\n", keycode);
        ((Key*)data)[index] = (Key){
                .mod = get_value_modifiers(key, "modifiers"),
                .keycode = keycode,
                .func = get_value_proc_pointer(key, "action")
        };
}

static inline void
guile_parse_button(unsigned int index, SCM button, void *data)
{
        SCM symbol = get_value(button, "button");
        SCM eval = scm_primitive_eval(symbol);
        ((Button*)data)[index] = (Button){
                .mod = (unsigned int)get_value_modifiers(button, "modifiers"),
                .button = scm_to_unsigned_integer(eval, 0, -1),
                .func = get_value_proc_pointer(button, "action")
        };
}

static inline struct xkb_rule_names *
guile_parse_xkb_rules(SCM config)
{
        SCM xkb = get_value(config, "xkb-rules");
        struct xkb_rule_names *dest = calloc(1, sizeof(struct xkb_rule_names));
        *dest = (struct xkb_rule_names){
                .rules = get_value_string(xkb, "rules"),
                .model = get_value_string(xkb, "model"),
                .layout = get_value_string(xkb, "layouts"),
                .variant = get_value_string(xkb, "variants"),
                .options = get_value_string(xkb, "options"),
        };
        return dest;
}

static inline void
guile_parse_config(char *config_file)
{
        scm_c_primitive_load(config_file);
        SCM config = get_variable("config");

        sloppyfocus = get_value_int(config, "sloppy-focus");
        tap_to_click = get_value_int(config, "tap-to-click");
        natural_scrolling = get_value_int(config, "natural-scrolling");
        borderpx = get_value_unsigned_int(config, "border-px", 25);
        repeat_rate = get_value_unsigned_int(config, "repeat-rate", 5000);
        repeat_delay = get_value_unsigned_int(config, "repeat-delay", 5000);

        SCM colors = get_value(config, "colors");
        rootcolor = guile_iterate_list(get_value(colors, "root"),
                sizeof(float), 0, &guile_parse_color, NULL);
        bordercolor = guile_iterate_list(get_value(colors, "border"),
                sizeof(float), 0, &guile_parse_color, NULL);
        focuscolor = guile_iterate_list(get_value(colors, "focus"),
                sizeof(float), 0, &guile_parse_color, NULL);
        tags = guile_iterate_list(get_value(config, "tags"),
                sizeof(char*), 0, &guile_parse_string, &numtags);
        termcmd = guile_iterate_list(get_value(config, "terminal"),
                sizeof(char*), 1, &guile_parse_string, NULL);
        menucmd = guile_iterate_list(get_value(config, "menu"),
                sizeof(char*), 1, &guile_parse_string, NULL);
        layouts = guile_iterate_list(get_value(config, "layouts"),
                sizeof(Layout), 0, &guile_parse_layout, &numlayouts);
        rules = guile_iterate_list(get_value(config, "rules"),
                sizeof(Rule), 0, &guile_parse_rule, &numrules);
        monrules = guile_iterate_list(get_value(config, "monitor-rules"),
                sizeof(MonitorRule), 0, &guile_parse_monitor_rule, &nummonrules);
        keys = guile_iterate_list(get_value(config, "keys"),
                sizeof(Key), 0, &guile_parse_key, &numkeys);
        buttons = guile_iterate_list(get_value(config, "buttons"),
                sizeof(Button), 0, &guile_parse_button, &numbuttons);
        xkb_rules = guile_parse_xkb_rules(config);
        TAGMASK = ((1 << numtags) - 1);
}

static inline void
guile_cleanup()
{
        fprintf(stdout, "guile: starting cleanup\n");
        int i;
        char **str;
        for (i = 0; i < numtags; i++) free(tags[i]);
        for (str = termcmd; *str != NULL; str++) free(*str);
        for (str = menucmd; *str != NULL; str++) free(*str);
        for (i = 0; i < numlayouts; i++) free(layouts[i].symbol);
        for (i = 0; i < nummonrules; i++) free(monrules[i].name);
        for (i = 0; i < numrules; i++) {
                Rule r = rules[i];
                free(r.id);
                free(r.title);
        }
        free(layouts);
        free(monrules);
        /* TODO: iterate keys and call scm_gc_unprotect_object()
           on the packed scm_t_bits? */
        free(keys);
        free(buttons);
        free(rootcolor);
        free(bordercolor);
        free(focuscolor);
        free((char*)xkb_rules->rules);
        free((char*)xkb_rules->model);
        free((char*)xkb_rules->layout);
        free((char*)xkb_rules->variant);
        free((char*)xkb_rules->options);
        free(xkb_rules);
        /* TODO: run scm_gc()? */
}
