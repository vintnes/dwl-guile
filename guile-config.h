#include <stdio.h>
#include <libguile.h>
#include <xkbcommon/xkbcommon.h>
#include <wlr/types/wlr_keyboard.h>
#include <wayland-client-protocol.h>
#include <linux/input-event-codes.h>

#define GUILE_RGB_COLOR_LENGTH  4
#define GUILE_MAX_LIST_LENGTH   500
#define GUILE_MAX_TAGS          100

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
static Button *buttons          = NULL;
static struct xkb_rule_names *xkb_rules = NULL;

static unsigned int numrules    = 0;
static unsigned int nummonrules = 0;
static unsigned int numbuttons  = 0;
/* static Key *keys                = NULL; */

#define MODKEY WLR_MODIFIER_ALT
#define TAGKEYS(KEY,SKEY,TAG) \
	{ MODKEY,                    KEY,            view,            {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_CTRL,  KEY,            toggleview,      {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_SHIFT, SKEY,           tag,             {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT,SKEY,toggletag, {.ui = 1 << TAG} }

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }
static Key keys[] = {
	/* Note that Shift changes certain key codes: c -> C, 2 -> at, etc. */
	/* modifier                  key                 function        argument */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Return,     spawn,          {.v = termcmd} }, */
	{ MODKEY,                    XKB_KEY_j,          focusstack,     {.i = +1} },
	{ MODKEY,                    XKB_KEY_k,          focusstack,     {.i = -1} },
	{ MODKEY,                    XKB_KEY_i,          incnmaster,     {.i = +1} },
	{ MODKEY,                    XKB_KEY_d,          incnmaster,     {.i = -1} },
	{ MODKEY,                    XKB_KEY_h,          setmfact,       {.f = -0.05} },
	{ MODKEY,                    XKB_KEY_l,          setmfact,       {.f = +0.05} },
	{ MODKEY,                    XKB_KEY_Return,     zoom,           {0} },
	{ MODKEY,                    XKB_KEY_Tab,        view,           {0} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_C,          killclient,     {0} },
	/* { MODKEY,                    XKB_KEY_t,          setlayout,      {.v = &layouts[0]} }, */
	/* { MODKEY,                    XKB_KEY_f,          setlayout,      {.v = &layouts[1]} }, */
	/* { MODKEY,                    XKB_KEY_m,          setlayout,      {.v = &layouts[2]} }, */
	{ MODKEY,                    XKB_KEY_space,      setlayout,      {0} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_space,      togglefloating, {0} },
	{ MODKEY, 					 XKB_KEY_e,    		togglefullscreen, {0} },
	{ MODKEY,                    XKB_KEY_0,          view,           {.ui = ~0} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_parenright, tag,            {.ui = ~0} },
	{ MODKEY,                    XKB_KEY_comma,      focusmon,       {.i = WLR_DIRECTION_LEFT} },
	{ MODKEY,                    XKB_KEY_period,     focusmon,       {.i = WLR_DIRECTION_RIGHT} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_less,       tagmon,         {.i = WLR_DIRECTION_LEFT} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_greater,    tagmon,         {.i = WLR_DIRECTION_RIGHT} },
	TAGKEYS(          XKB_KEY_1, XKB_KEY_exclam,                     0),
	TAGKEYS(          XKB_KEY_2, XKB_KEY_at,                         1),
	TAGKEYS(          XKB_KEY_3, XKB_KEY_numbersign,                 2),
	TAGKEYS(          XKB_KEY_4, XKB_KEY_dollar,                     3),
	TAGKEYS(          XKB_KEY_5, XKB_KEY_percent,                    4),
	TAGKEYS(          XKB_KEY_6, XKB_KEY_caret,                      5),
	TAGKEYS(          XKB_KEY_7, XKB_KEY_ampersand,                  6),
	TAGKEYS(          XKB_KEY_8, XKB_KEY_asterisk,                   7),
	TAGKEYS(          XKB_KEY_9, XKB_KEY_parenleft,                  8),
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Q,          quit,           {0} },
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_Terminate_Server, quit, {0} },
#define CHVT(n) { WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_XF86Switch_VT_##n, chvt, {.ui = (n)} }
	CHVT(1), CHVT(2), CHVT(3), CHVT(4), CHVT(5), CHVT(6),
	CHVT(7), CHVT(8), CHVT(9), CHVT(10), CHVT(11), CHVT(12),
};

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
        SCM value = get_value(alist, key);
        scm_t_bits *proc = NULL;
        if (scm_procedure_p(value) == SCM_BOOL_T)
                proc = SCM_UNPACK_POINTER(value);
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

static inline unsigned int
get_value_modifiers(SCM alist, const char *key)
{
        SCM modifiers = get_value(alist, key);
        unsigned int i = 0, mod = 0, length = get_list_length(modifiers);
        for (; i < length; i++) {
            SCM item = get_list_item(modifiers, i);
            SCM eval = scm_primitive_eval(item);
            mod |= scm_to_unsigned_integer(eval, 0, -1);
        }
        return mod;
}

static inline void *
iterate_list(SCM alist, const char* key, size_t elem_size, int append_null,
        void (*iterator)(unsigned int, SCM, void*), unsigned int *length_var)
{
        unsigned int i = 0, length = 0;
        SCM list = get_value(alist, key);
        length = get_list_length(list);
        void *allocated = calloc(append_null ? length + 1 : length, elem_size);
        for (; i < length; i++) {
                SCM item = get_list_item(list, i);
                (*iterator)(i, item, allocated);
        }
        if (append_null)
                ((void**)allocated)[i+1] = NULL;
        if (length_var)
                *length_var = length;
        return allocated;
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
        scm_t_bits *proc = get_value_proc_pointer(layout, "arrange");
        ((Layout*)data)[index] = (Layout){
                .symbol = get_value_string(layout, "symbol"),
                .arrange = proc
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
                .nmaster = get_value_int(rule, "number-of-masters"),
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
guile_parse_button(unsigned int index, SCM rule, void *data)
{
        SCM button = get_value(rule, "button");
        SCM eval = scm_primitive_eval(button);
        ((Button*)data)[index] = (Button){
                .mod = get_value_modifiers(rule, "modifiers"),
                .button = scm_to_unsigned_integer(eval, 0, -1),
                .func = get_value_proc_pointer(rule, "action")
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
guile_call_arrange(scm_t_bits *arrange, Monitor *m)
{
        SCM proc = SCM_PACK_POINTER(arrange);
        scm_call_1(proc, scm_from_pointer(m, NULL));
}

static inline void
guile_call_action(scm_t_bits *action)
{
        SCM proc = SCM_PACK_POINTER(action);
        scm_call_0(proc);
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
        rootcolor = iterate_list(colors, "root",
                sizeof(float), 0, &guile_parse_color, NULL);
        bordercolor = iterate_list(colors, "border",
                sizeof(float), 0, &guile_parse_color, NULL);
        focuscolor = iterate_list(colors, "focus",
                sizeof(float), 0, &guile_parse_color, NULL);
        tags = iterate_list(config, "tags",
                sizeof(char*), 0, &guile_parse_string, NULL);
        termcmd = iterate_list(config, "terminal",
                sizeof(char*), 1, &guile_parse_string, NULL);
        menucmd = iterate_list(config, "menu",
                sizeof(char*), 1, &guile_parse_string, NULL);
        layouts = iterate_list(config, "layouts",
                sizeof(Layout), 0, &guile_parse_layout, NULL);
        rules = iterate_list(config, "rules",
                sizeof(Rule), 0, &guile_parse_rule, &numrules);
        monrules = iterate_list(config, "monitor-rules",
                sizeof(MonitorRule), 0, &guile_parse_monitor_rule, &nummonrules);
        buttons = iterate_list(config, "buttons",
                sizeof(Button), 0, &guile_parse_button, &numbuttons);
        xkb_rules = guile_parse_xkb_rules(config);
}
