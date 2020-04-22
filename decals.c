#include <stdlib.h>
#include <objc/objc.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

/* -- AppDelegate --------------------------------------------------------- */

Class AppDelegateClass;

typedef struct AppDelegate_tag
{
    Class isa;
}
AppDelegate;

void AppDelegate_applicationDidFinishLaunching(AppDelegate *self, SEL _cmd, id notification)
{
    printf("hello, world!\n");

    CGRect rect = {
        .origin = {100,100},
        .size = {200,200}
    };
    id window = objc_msgSend(objc_msgSend((id) objc_getClass("NSWindow"), sel_getUid("alloc")),
                             sel_getUid("initWithContentRect:styleMask:backing:defer:"),
                             rect,
                             (1 << 0) | (1 << 1), /* titled, closable */
                             2, /* buffered */
                             NO);

    objc_msgSend(window, sel_getUid("makeKeyAndOrderFront:"), Nil);
}

void AppDelegate_applicationWillTerminate(AppDelegate *self, SEL _cmd, id notification)
{
}

void init_AppDelegateClass()
{
    AppDelegateClass = objc_allocateClassPair(objc_getClass("NSObject"), "AppDelegate", 0);
    class_addProtocol(AppDelegateClass, objc_getProtocol("NSApplicationDelegate"));
    class_addMethod(AppDelegateClass, sel_getUid("applicationDidFinishLaunching:"), (IMP) AppDelegate_applicationDidFinishLaunching, "v@:@");
    class_addMethod(AppDelegateClass, sel_getUid("applicationWillTerminate:"), (IMP) AppDelegate_applicationWillTerminate, "v@:@");
    objc_registerClassPair(AppDelegateClass);
}

/* -- main() -------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    init_AppDelegateClass();

    id application = objc_msgSend((id) objc_getClass("NSApplication"), sel_getUid("sharedApplication"));
    id delegate = objc_msgSend(objc_msgSend((id) AppDelegateClass, sel_getUid("alloc")), sel_getUid("init"));
    objc_msgSend(application, sel_getUid("setDelegate:"), delegate);

    objc_msgSend(application, sel_getUid("run"));
    exit(0);
}
