/* See LICENSE file for copyright and license details. */


// volume control from: https://gist.github.com/palopezv/efd34059af6126ad970940bcc6a90f2e
// volume status with: //  xsetroot -name "Volume: $(pactl list sinks | grep '^[[:space:]]Volume:' | head -n $(( $SINK + 1 )) | tail -n 1 | sed -e 's,.* \([0-9][0-9]*\)%.*,\1,')%"
#include <X11/XF86keysym.h>

 
/* volume keys*/
static const char *upvol[] = { "/usr/bin/pactl", "set-sink-volume", "0", "+5%", NULL };
static const char *downvol[] = { "/usr/bin/pactl", "set-sink-volume", "0", "-5%", NULL };
static const char *mutevol[] = { "/usr/bin/pactl", "set-sink-mute", "0", "toggle", NULL };

/* backlight */
static const char *brightnessup[] = {"xbacklight", "-inc", "5", NULL };
static const char *brightnessdown[] = {"xbacklight", "-dec", "5", NULL };

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "fira code nerd font:size=16" };
static const char dmenufont[]       = "fira code nerd font:size=16";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";

static const unsigned int gappih = 25; //inner horizontal gaps
static const unsigned int gappiv = 40; //inner vertical gaps
static const unsigned int gappoh = 40; //outer horizontal gaps
static const unsigned int gappov = 40; //outer vertical gaps
static const unsigned int smartgaps = 0;

//colors from https://github.com/plasmoduck/themes/blob/master/gruvbox-hard/colors-wal-dwm.h
static const char norm_fg[] = "#D5C4A1";
static const char norm_bg[] = "#1d2021";
static const char norm_border[] = "#665c54";

static const char sel_fg[] = "#FE8019";
static const char sel_bg[] = "#1d2021";
static const char sel_border[] = "#fbf1c7";

static const char urg_fg[] = "#fbf1c7";
static const char urg_bg[] = "#fb4934";
static const char urg_border[] = "#fb4934";


static const char *colors[][3]      = {
    /*               fg           bg          border                         */
    [SchemeNorm] =   { norm_fg,   norm_bg,    norm_border }, 		     // unfocused wins
    [SchemeSel]  =   { sel_fg,    sel_bg,     sel_border },		     // the focused win
};

/* tagging */
static const char *tags[] = { "一", "二", "三", "四", "五", "六", "七", "八", "九" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class     instance  title           tags mask  isfloating  isterminal  noswallow  monitor */
	{ "Gimp",    NULL,     NULL,           0,         1,          0,           0,        -1 },
	{ "Firefox", NULL,     NULL,           1 << 8,    0,          0,          -1,        -1 },
	{ "kitty",   NULL,     NULL,           0,         0,          1,           0,        -1 },
	{ NULL,      NULL,     "Event Tester", 0,         0,          0,           1,        -1 }, /* xev */
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "瓦",      tile },    /* first entry is default */
	{ "浮",      NULL },    /* no layout function means floating behavior */
	{ "M",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
//static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", norm_bg, "-sf", col_gray4, NULL };
static const char *dmenucmd[] = {"dmenu_run", "-bw", "2", "-l", "30", NULL};
static const char *termcmd[]  = { "kitty", NULL };
static const char *browsercmd[]  = { "firefox", NULL };
static const char scratchpadname[] = "scratchpad";
static const char *scratchpadcmd[] = { "st", "-t", scratchpadname, "-g", "60x18", NULL }; //120x34
static const char *screenshotcmd[] = { "scrot", "~/Pictures/Screenshots/'screenshot-%Y-%m-%d.png'", "-q", "100", NULL };
static const char *systemdsuspendslock[] = {"/usr/local/bin/slock", NULL};
static const char *duckselectfirefox[] = {"/home/max/scripts/duckselectfirefox.sh", NULL};
static const char *firefoxbookmarks[] = {"/home/max/scripts/bookmarks.sh", NULL};
//static const char *scratchpadcmd[] = { "kitty", "--name", scratchpadname, NULL };

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_space,      spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY|ShiftMask,             XK_f, spawn,          {.v = browsercmd } },
	{ MODKEY,                       XK_p,  togglescratch,  {.v = scratchpadcmd } },
	{ MODKEY,                       XK_Print,      spawn,      {.v = screenshotcmd} },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY,             XK_q,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_z,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_z,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
	{ 0,                       XF86XK_AudioLowerVolume, spawn, {.v = downvol } },
	{ 0,                       XF86XK_AudioMute, spawn, {.v = mutevol } },
	{ 0,                       XF86XK_AudioRaiseVolume, spawn, {.v = upvol } },
  { 0, XF86XK_MonBrightnessUp, spawn, {.v = brightnessup} },
  { 0, XF86XK_MonBrightnessDown, spawn, {.v = brightnessdown} },
	{ MODKEY|ShiftMask,             XK_l,      spawn,           {.v = systemdsuspendslock} },
	{ MODKEY|ShiftMask,             XK_u,      spawn,           {.v = duckselectfirefox} },
	{ MODKEY|ShiftMask,             XK_b,      spawn,           {.v = firefoxbookmarks} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

