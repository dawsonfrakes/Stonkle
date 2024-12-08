#import "AppKit/AppKit.h"

static BOOL platform_wants_close;

@interface StonkleApplicationDelegate : NSObject <NSApplicationDelegate>
@end

@implementation StonkleApplicationDelegate
@end

@interface StonkleWindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation StonkleWindowDelegate
-(BOOL)windowShouldClose:(NSWindow*)sender {
  platform_wants_close = YES;
  return true;
}
@end

extern "C" [[noreturn]] void start() {
  [NSApplication sharedApplication];
  [NSApp setDelegate:[StonkleApplicationDelegate new]];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

  NSWindow* window = [[NSWindow alloc]
    initWithContentRect:CGRectMake(0, 0, 600, 400)
    styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable
    backing:NSBackingStoreBuffered
    defer:NO];
  [window setDelegate:[StonkleWindowDelegate new]];
  [window setTitle:@"Stonkle"];
  [window setAcceptsMouseMovedEvents:YES];
  [window makeKeyAndOrderFront:nil];

  NSWindowController* controller = [[NSWindowController alloc] initWithWindow:window];
  [controller setWindowFrameAutosaveName:@"position"];

  NSMenu* menubar = [NSMenu new];
  [NSApp setMenu:menubar];

  NSMenuItem* appmenuitem = [NSMenuItem new];
  [menubar addItem:appmenuitem];

  NSMenu* appmenu = [NSMenu new];
  [appmenuitem setSubmenu:appmenu];

  NSMenuItem* quitbutton = [[NSMenuItem alloc] initWithTitle:@"Quit Stonkle" action:@selector(terminate:) keyEquivalent:@"q"];
  [appmenu addItem:quitbutton];

  NSMenuItem* windowsmenuitem = [NSMenuItem new];
  [windowsmenuitem setTitle:@"Window"];
  [menubar addItem:windowsmenuitem];

  NSMenu* windowsmenu = [NSMenu new];
  [windowsmenuitem setSubmenu:windowsmenu];
  [NSApp setWindowsMenu:windowsmenu];

  [NSApp finishLaunching];
  [NSApp activate];

  while (!platform_wants_close) {
    NSEvent* ev;
    while ((ev = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES])) {
      switch ([ev type]) {
        default: {
          // [NSApp updateWindows]; // :maybe_unnecessary
          [NSApp sendEvent:ev];
        }
      }
      [ev release];
    }

    [[window contentView] setNeedsDisplay:YES];
  }

  [NSApp terminate:nil];
  __builtin_unreachable();
}
