#include <stdlib.h>
#include <unistd.h>
#include <objc/objc.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

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

    struct objc_super super =
    {
        (id)self,
        objc_getClass("NSWindow")
    };
    if ((self = (Overlay *)objc_msgSendSuper(&super,
                                             sel_getUid("initWithContentRect:styleMask:backing:defer:screen:"),
                                             rect,
                                             (1 << 0) | (1 << 1), /* titled, closable */
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

void Overlay_keyDown(Overlay *self, SEL _cmd, id event)
{
    const char *s = objc_msgSend(objc_msgSend(event, sel_getUid("characters")), sel_getUid("UTF8String"));
    printf("%s\n", s);

    id application = objc_msgSend((id) objc_getClass("NSApplication"), sel_getUid("sharedApplication"));
    objc_msgSend(application, sel_getUid("terminate:"), (id)self);
}

void register_OverlayClass()
{
    OverlayClass = objc_allocateClassPair(objc_getClass("NSWindow"), "Overlay", 0);
    class_addMethod(OverlayClass, sel_getUid("initWithScreen:"), (IMP) Overlay_initWithScreen, "@@:@");
    class_addMethod(OverlayClass, sel_getUid("keyDown:"), (IMP) Overlay_keyDown, "v@:@");
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

void parse_options(int argc, char *argv[])
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
}

int main(int argc, char *argv[])
{
    register_OverlayClass();
    register_AppDelegateClass();

    id application = objc_msgSend((id) objc_getClass("NSApplication"), sel_getUid("sharedApplication"));

    /* Regular activation policy */
    objc_msgSend(application, sel_getUid("setActivationPolicy:"), 0);

    id delegate = objc_msgSend(objc_msgSend((id) AppDelegateClass, sel_getUid("alloc")), sel_getUid("init"));
    objc_msgSend(application, sel_getUid("setDelegate:"), delegate);

    parse_options(argc, argv);

    objc_msgSend(application, sel_getUid("run"));
    exit(0);
}
