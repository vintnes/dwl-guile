#include <stdio.h>
#include <libguile.h>
#include <xkbcommon/xkbcommon.h>
#include <wlr/types/wlr_keyboard.h>
#include <wayland-client-protocol.h>
#include <linux/input-event-codes.h>

#define GUILE_RGB_COLOR_LENGTH  4
#define GUILE_MAX_LIST_LENGTH   500

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
/* static Rule *rules              = NULL; */
/* static Layout *layouts          = NULL; */
/* static MonitorRule *monrules    = NULL; */
/* static char **termcmd           = NULL; */
/* static Key *keys                = NULL; */
/* static Button *buttons          = NULL; */
/* static struct xkb_rule_names *xkb_rules = NULL; */

static Rule rules[] = {
	/* app_id     title       tags mask     isfloating   monitor */
	/* examples:
	{ "Gimp",     NULL,       0,            1,           -1 },
	{ "firefox",  NULL,       1 << 8,       0,           -1 },
	*/
};
static Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};
static MonitorRule monrules[] = {
	/* name       mfact nmaster scale layout       rotate/reflect x y */
	/* example of a HiDPI laptop monitor:
	{ "eDP-1",    0.5,  1,      2,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL, 0, 0 },
	*/
	/* defaults */
	{ NULL,       0.55, 1,      1,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL, 0, 0 },
};
static struct xkb_rule_names xkb_rules = {
	/* can specify fields: rules, model, layout, variant, options */
	/* example:
	.options = "ctrl:nocaps",
	*/
};

#define MODKEY WLR_MODIFIER_ALT
#define TAGKEYS(KEY,SKEY,TAG) \
	{ MODKEY,                    KEY,            view,            {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_CTRL,  KEY,            toggleview,      {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_SHIFT, SKEY,           tag,             {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT,SKEY,toggletag, {.ui = 1 << TAG} }

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }
static char *termcmd[]  = { "alacritty", NULL };
static Key keys[] = {
	/* Note that Shift changes certain key codes: c -> C, 2 -> at, etc. */
	/* modifier                  key                 function        argument */
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Return,     spawn,          {.v = termcmd} },
	{ MODKEY,                    XKB_KEY_j,          focusstack,     {.i = +1} },
	{ MODKEY,                    XKB_KEY_k,          focusstack,     {.i = -1} },
	{ MODKEY,                    XKB_KEY_i,          incnmaster,     {.i = +1} },
	{ MODKEY,                    XKB_KEY_d,          incnmaster,     {.i = -1} },
	{ MODKEY,                    XKB_KEY_h,          setmfact,       {.f = -0.05} },
	{ MODKEY,                    XKB_KEY_l,          setmfact,       {.f = +0.05} },
	{ MODKEY,                    XKB_KEY_Return,     zoom,           {0} },
	{ MODKEY,                    XKB_KEY_Tab,        view,           {0} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_C,          killclient,     {0} },
	{ MODKEY,                    XKB_KEY_t,          setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                    XKB_KEY_f,          setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                    XKB_KEY_m,          setlayout,      {.v = &layouts[2]} },
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

static Button buttons[] = {
	{ MODKEY, BTN_LEFT,   moveresize,     {.ui = CurMove} },
	{ MODKEY, BTN_MIDDLE, togglefloating, {0} },
	{ MODKEY, BTN_RIGHT,  moveresize,     {.ui = CurResize} },
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

static inline unsigned int
get_list_length(SCM list)
{
        return scm_to_unsigned_integer(scm_length(list), 0, GUILE_MAX_LIST_LENGTH);
}

static inline void *
iterate_list(SCM alist, const char* key, size_t elem_size, void (*iterator)(unsigned int, SCM, void*))
{
        SCM list = get_value(alist, key);
        unsigned int length = get_list_length(list);
        void *allocated = calloc(length, elem_size);
        for (unsigned int i = 0; i < length; i++) {
                SCM item = scm_list_ref(list, scm_from_unsigned_integer(i));
                (*iterator)(i, item, allocated);
        }
        return allocated;
}

static inline SCM
get_variable(const char *name)
{
        return scm_variable_ref(scm_c_lookup(name));
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
guile_parse_tag(unsigned int index, SCM tag, void *data)
{
        ((char**)data)[index] = scm_to_locale_string(tag);
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
        rootcolor = iterate_list(colors, "root", sizeof(float),
                &guile_parse_color);
        bordercolor = iterate_list(colors, "border", sizeof(float),
                &guile_parse_color);
        focuscolor = iterate_list(colors, "focus", sizeof(float),
                &guile_parse_color);
        tags = iterate_list(config, "tags", sizeof(char*),
                &guile_parse_tag);
}
