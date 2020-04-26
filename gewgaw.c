#include <stdlib.h>
#include <unistd.h>
#include <objc/objc.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

/* -- options ------------------------------------------------------------- */

typedef struct Pane_tag
{
    int screen;
    CGRect area;
    id label;
}
Pane;

#define MAX_PANES 256
size_t pane_count = 0;
Pane panes[MAX_PANES];

void parse_arg(char *arg)
{
    char label[512];
    if (pane_count >= MAX_PANES)
    {
        fprintf(stderr, "gewgaw: too many panes specified.\n");
        exit(1);
    }
    if (sscanf(arg, "%d%*c%lf%*c%lf%*c%lf%*c%lf%*c%[^\n]",
               &panes[pane_count].screen,
               &panes[pane_count].area.origin.x,
               &panes[pane_count].area.origin.y,
               &panes[pane_count].area.size.width,
               &panes[pane_count].area.size.height,
               label) < 6)
    {
        fprintf(stderr, "gewgaw: unable to parse `%s'.\n", arg);
        exit(1);
    }
    panes[pane_count].label = objc_msgSend((id) objc_getClass("NSString"), sel_getUid("stringWithUTF8String:"), label);
    ++pane_count;
}

void parse_args(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "")) != -1)
    {
        switch (opt)
        {
        case '?':
            exit(1);
        }
    }

    for ( ; optind < argc; ++optind)
        parse_arg(argv[optind]);
}

/* -- Overlay ------------------------------------------------------------- */

Class OverlayClass;

typedef struct Overlay_tag
{
    Class isa;
}
Overlay;

id Overlay_initWithScreen(Overlay *self, SEL _cmd, id screen)
{
    CGRect rect = {};
    objc_msgSend_stret(&rect, screen, sel_getUid("frame"));
    rect.origin.x = 0;
    rect.origin.y = 0;
    struct objc_super super =
    {
        (id)self,
        objc_getClass("NSWindow")
    };
    if ((self = (Overlay *)objc_msgSendSuper(&super,
                                             sel_getUid("initWithContentRect:styleMask:backing:defer:screen:"),
                                             rect,
                                             0, /* borderless */
                                             2, /* buffered */
                                             NO,
                                             screen)))
    {
        id clear = objc_msgSend((id) objc_getClass("NSColor"), sel_getUid("clearColor"));
        objc_msgSend((id) self, sel_getUid("setBackgroundColor:"), clear);
        objc_msgSend((id) self, sel_getUid("setOpaque:"), NO);
    }
    return (id)self;
}

const char *find_key_name(CGKeyCode code)
{
    static struct key_name_tag {
        CGKeyCode code;
        const char *name;
    } key_names[] = {
        { kVK_Return,        "return"    },
        { kVK_Tab,           "tab"       },
        { kVK_Space,         "space"     },
        { kVK_Delete,        "backspace" },
        { kVK_Escape,        "escape"    },
        { kVK_ForwardDelete, "delete"    },
        { kVK_Home,          "home"      },
        { kVK_End,           "end"       },
        { kVK_PageUp,        "pageup"    },
        { kVK_PageDown,      "pagedown"  },
        { kVK_Help,          "insert"    },
        { kVK_LeftArrow,     "left"      },
        { kVK_RightArrow,    "right"     },
        { kVK_UpArrow,       "up"        },
        { kVK_DownArrow,     "down"      },
        { kVK_F1,            "f1"        },
        { kVK_F2,            "f2"        },
        { kVK_F3,            "f3"        },
        { kVK_F4,            "f4"        },
        { kVK_F5,            "f5"        },
        { kVK_F6,            "f6"        },
        { kVK_F7,            "f7"        },
        { kVK_F8,            "f8"        },
        { kVK_F9,            "f9"        },
        { kVK_F10,           "f10"       },
        { kVK_F11,           "f11"       },
        { kVK_F12,           "f12"       },
        { kVK_F13,           "f13"       },
        { kVK_F14,           "f14"       },
        { kVK_F15,           "f15"       },
        { kVK_F16,           "f16"       },
        { kVK_F17,           "f17"       },
        { kVK_F18,           "f18"       },
        { kVK_F19,           "f19"       },
        { kVK_F20,           "f20"       },
    };
    for (int i = 0; i < sizeof(key_names)/sizeof(key_names[0]); ++i)
    {
        if (key_names[i].code == code)
            return key_names[i].name;
    }
    return NULL;
}

void Overlay_keyDown(Overlay *self, SEL _cmd, id event)
{
    CGKeyCode keyCode = (CGKeyCode) objc_msgSend(event, sel_getUid("keyCode"));
    const char *name = find_key_name(keyCode);
    if (name)
        printf("%s\n", name);
    else
    {
        const char *chars = (const char*)objc_msgSend(objc_msgSend(event, sel_getUid("characters")), sel_getUid("UTF8String"));
        if (!strcmp(chars, ""))
            printf("0x%02X\n", keyCode);
        else
            printf("%s\n", chars);
    }

    id application = objc_msgSend((id) objc_getClass("NSApplication"), sel_getUid("sharedApplication"));
    objc_msgSend(application, sel_getUid("terminate:"), (id)self);
}

void Overlay_addPane_label(Overlay *self, SEL _cmd, CGRect frame, id label)
{
    id pane = objc_msgSend(objc_msgSend((id) objc_getClass("NSTextField"), sel_getUid("alloc")), sel_getUid("init"));

    objc_msgSend(pane, sel_getUid("setFrame:"), frame);
    objc_msgSend(pane, sel_getUid("setStringValue:"), label);
    objc_msgSend(pane, sel_getUid("setEditable:"), NO);
    objc_msgSend(pane, sel_getUid("setBezeled:"), NO);
    objc_msgSend(pane, sel_getUid("setBackgroundColor:"),
                 objc_msgSend(objc_msgSend((id) objc_getClass("NSColor"), sel_getUid("cyanColor")),
                              sel_getUid("colorWithAlphaComponent:"),
                              (CGFloat)0.15));
    objc_msgSend(pane, sel_getUid("setFont:"), objc_msgSend((id) objc_getClass("NSFont"), sel_getUid("systemFontOfSize:"), (CGFloat) 25));
    objc_msgSend(pane, sel_getUid("setBordered:"), YES);
    objc_msgSend(pane, sel_getUid("setTextColor:"), objc_msgSend((id) objc_getClass("NSColor"), sel_getUid("whiteColor")));

    id contentView = objc_msgSend((id)self, sel_getUid("contentView"));
    objc_msgSend(contentView, sel_getUid("addSubview:"), pane);
}

BOOL Overlay_canBecomeKeyWindow(Overlay *self, SEL _cmd)
{
    return YES;
}

BOOL Overlay_canBecomeMainWindow(Overlay *self, SEL _cmd)
{
    return YES;
}

void register_OverlayClass()
{
    OverlayClass = objc_allocateClassPair(objc_getClass("NSWindow"), "Overlay", 0);
    class_addMethod(OverlayClass, sel_getUid("initWithScreen:"), (IMP) Overlay_initWithScreen, "@@:@");
    class_addMethod(OverlayClass, sel_getUid("keyDown:"), (IMP) Overlay_keyDown, "v@:@");
    class_addMethod(OverlayClass, sel_getUid("addPane:label:"), (IMP) Overlay_addPane_label, "v@:{CGRect={CGPoint=dd}{CGSize=dd}}@");;
    class_addMethod(OverlayClass, sel_getUid("canBecomeKeyWindow"), (IMP) Overlay_canBecomeKeyWindow, "c@:");
    class_addMethod(OverlayClass, sel_getUid("canBecomeMainWindow"), (IMP) Overlay_canBecomeMainWindow, "c@:");
    objc_registerClassPair(OverlayClass);
}

/* -- AppDelegate --------------------------------------------------------- */

Class AppDelegateClass;

typedef struct AppDelegate_tag
{
    Class isa;
    id overlays;
}
AppDelegate;

id AppDelegate_init(AppDelegate *self, SEL _cmd)
{
    struct objc_super super =
    {
        (id)self,
        objc_getClass("NSObject")
    };
    if ((self = (AppDelegate *)objc_msgSendSuper(&super, sel_getUid("init"))))
    {
        self->overlays = objc_msgSend(objc_msgSend((id) objc_getClass("NSMutableArray"), sel_getUid("alloc")), sel_getUid("init"));
    }
    return (id)self;
}

void AppDelegate_applicationDidFinishLaunching(AppDelegate *self, SEL _cmd, id notification)
{
    id screens = objc_msgSend((id) objc_getClass("NSScreen"), sel_getUid("screens"));
    const int count = (int)objc_msgSend(screens, sel_getUid("count"));
    for (int i = 0; i < count; i++)
    {
        id screen = objc_msgSend(screens, sel_getUid("objectAtIndex:"), i);
        id overlay = objc_msgSend(objc_msgSend((id) objc_getClass("Overlay"), sel_getUid("alloc")),
                                  sel_getUid("initWithScreen:"),
                                  screen);
        objc_msgSend(self->overlays, sel_getUid("addObject:"), overlay);

        CGRect screen_rect;
        objc_msgSend_stret(&screen_rect, screen, sel_getUid("frame"));
        for (int j = 0; j < pane_count; j++)
        {
            if (panes[j].screen == 1+i)
            {
                CGRect inverted = panes[j].area;
                inverted.origin.y = screen_rect.size.height - inverted.size.height - inverted.origin.y;
                objc_msgSend(overlay, sel_getUid("addPane:label:"), inverted, panes[j].label);
            }
        }

        objc_msgSend(overlay, sel_getUid("makeKeyAndOrderFront:"), Nil);
    }
}

BOOL AppDelegate_applicationShouldTerminateAfterLastWindowClosed(AppDelegate *self, SEL _cmd, id application)
{
    return YES;
}

void AppDelegate_applicationWillTerminate(AppDelegate *self, SEL _cmd, id notification)
{
}

void register_AppDelegateClass()
{
    AppDelegateClass = objc_allocateClassPair(objc_getClass("NSObject"), "AppDelegate", 0);
    class_addIvar(AppDelegateClass, "overlays", sizeof(id), 0, "@");
    class_addMethod(AppDelegateClass, sel_getUid("init"), (IMP) AppDelegate_init, "@@:");
    class_addMethod(AppDelegateClass, sel_getUid("applicationDidFinishLaunching:"), (IMP) AppDelegate_applicationDidFinishLaunching, "v@:@");
    class_addMethod(AppDelegateClass, sel_getUid("applicationShouldTerminateAfterLastWindowClosed:"), (IMP) AppDelegate_applicationShouldTerminateAfterLastWindowClosed, "v@:@");
    class_addMethod(AppDelegateClass, sel_getUid("applicationWillTerminate:"), (IMP) AppDelegate_applicationWillTerminate, "v@:@");
    objc_registerClassPair(AppDelegateClass);
}

/* -- main() -------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    register_OverlayClass();
    register_AppDelegateClass();

    id application = objc_msgSend((id) objc_getClass("NSApplication"), sel_getUid("sharedApplication"));

    /* Regular activation policy */
    objc_msgSend(application, sel_getUid("setActivationPolicy:"), 0);

    id delegate = objc_msgSend(objc_msgSend((id) AppDelegateClass, sel_getUid("alloc")), sel_getUid("init"));
    objc_msgSend(application, sel_getUid("setDelegate:"), delegate);

    parse_args(argc, argv);

    objc_msgSend(application, sel_getUid("run"));
    exit(0);
}
