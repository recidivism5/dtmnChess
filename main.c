typedef int i32;
typedef unsigned int u32;
#define BACKGROUND_COLOR 0x0000ff
#define START_WIDTH 640
#define START_HEIGHT 480
i32 fbWidth = START_WIDTH, fbHeight = START_HEIGHT;
i32 fbPixelCount = START_WIDTH*START_HEIGHT;
u32 *frameBuffer;
char title[] = "swag";
void draw(void);
void char_input(char c);
#if _WIN32
#include <windows.h>
#include <dwmapi.h>
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
struct BMI {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[3];
} bmi;
HDC hdc;
LONG WINAPI WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
    BOOL t = TRUE;
    switch (msg){
    case WM_CREATE:
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &t, sizeof(t));
        break;
    case WM_SHOWWINDOW: { //https://stackoverflow.com/questions/69715610/how-to-initialize-the-background-color-of-win32-app-to-something-other-than-whit
        if (!GetLayeredWindowAttributes(hwnd, NULL, NULL, NULL)){
            SetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);
            DefWindowProc(hwnd, WM_ERASEBKGND, (WPARAM)GetDC(hwnd), lparam);
            SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
            AnimateWindow(hwnd, 200, AW_ACTIVATE|AW_BLEND);
            return 0;
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    case WM_SIZE: {
        int width = LOWORD(lparam);
        int height = HIWORD(lparam);
        if ((width != fbWidth) || (height != fbHeight)){
            fbWidth = LOWORD(lparam);
            fbHeight = HIWORD(lparam);
            if (fbWidth*fbHeight > fbPixelCount){
                while (fbWidth*fbHeight > fbPixelCount){
                    fbPixelCount *= 2;
                }
                free(frameBuffer);
                frameBuffer = malloc(fbPixelCount*4);
            }
            draw();
        }
        return 0;
    }
    case WM_PAINT:
        bmi.bmiHeader.biWidth = fbWidth;
        bmi.bmiHeader.biHeight = -fbHeight;
        StretchDIBits(hdc, 0,0, fbWidth,fbHeight, 0,0,fbWidth,fbHeight,frameBuffer, &bmi, DIB_RGB_COLORS, SRCCOPY);
        ValidateRect(hwnd, 0);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}
WNDCLASSA wc = {0,WindowProc,0,0,NULL,NULL,NULL,NULL,NULL,title};
HWND wnd;
MSG msg;
int APIENTRY WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow){
    frameBuffer = malloc(fbPixelCount*4);
    draw();
    wc.hInstance = hCurrentInst;
    wc.hIcon = LoadIconA(0,IDI_APPLICATION);
    wc.hCursor = LoadCursorA(0,IDC_ARROW);
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(BACKGROUND_COLOR);
    RegisterClassA(&wc);
    wnd = CreateWindowExA(0,title,title,WS_VISIBLE|WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,16,16,fbWidth,fbHeight,NULL,NULL,wc.hInstance,NULL);
    hdc = GetDC(wnd);
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_BITFIELDS;
    bmi.bmiHeader.biWidth = fbWidth;
    bmi.bmiHeader.biHeight = -fbHeight;
    bmi.bmiColors[0].rgbRed = 0xff;
    bmi.bmiColors[1].rgbGreen = 0xff;
    bmi.bmiColors[2].rgbBlue = 0xff;
    while (GetMessageA(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
        InvalidateRect(wnd, 0, TRUE);
        SendMessage(wnd, WM_PAINT, 0, 0);
    }
    return msg.wParam;
}
#elif __APPLE__
#import <Cocoa/Cocoa.h>
@interface FBView : NSView<NSTextInputClient>
@end
@implementation FBView
- (void)drawRect:(NSRect)rect {
    CGContextRef context = [[NSGraphicsContext currentContext] CGContext];
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider = CGDataProviderCreateWithData(0x0,
        frameBuffer,
        fbPixelCount*4,
        0x0
    );
    CGImageRef img = CGImageCreate(fbWidth
        , fbHeight
        , 8
        , 32
        , fbWidth * 4
        , space
        , kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little
        , provider
        , 0x0
        , false
        , kCGRenderingIntentDefault
    );
    CGColorSpaceRelease(space);
    CGDataProviderRelease(provider);
    CGContextDrawImage(context,
        CGRectMake(0, 0, fbWidth, fbHeight),
        img
    );
    CGImageRelease(img);
}
#pragma mark NSTextInputClient
// Invokes the action specified by the given selector.
- (void)doCommandBySelector:(nonnull SEL)selector {
}
// Returns an attributed string derived from the given range in the receiver's text storage.
- (nullable NSAttributedString *)attributedSubstringForProposedRange:(NSRange)range actualRange:(nullable NSRangePointer)actualRange {
    return nil;
}
// Inserts the given string into the receiver, replacing the specified content.
- (void)insertText:(nonnull id)string replacementRange:(NSRange)replacementRange {
    NSString    *characters;
    NSUInteger  codepoint;

    if ([string isKindOfClass:[NSAttributedString class]])
        characters = [string string];
    else
        characters = (NSString*) string;

    NSRange range = NSMakeRange(0, [characters length]);
    while (range.length) {
        codepoint = 0;
        if ([characters getBytes:&codepoint
            maxLength:sizeof(codepoint)
            usedLength:NULL
            encoding:NSUTF32StringEncoding // NSUTF8StringEncoding
            options:0
            range:range
            remainingRange:&range]) {

            if ((codepoint & 0xff00) == 0xf700)
                continue;

            char_input((char)codepoint);
        }
    }
}
// Returns the index of the character whose bounding rectangle includes the given point.
- (NSUInteger)characterIndexForPoint:(NSPoint)point {
    return 0;
}
// Returns the first logical boundary rectangle for characters in the given range.
- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(nullable NSRangePointer)actualRange {
    return NSMakeRect(0.0, 0.0, 0.0, 0.0);
}
// [Handling Marked Text]
static const NSRange kEmptyRange = { NSNotFound, 0 };
// Returns a Boolean value indicating whether the receiver has marked text.
- (BOOL)hasMarkedText {
    return false;
}
// Returns the range of the marked text.
- (NSRange)markedRange {
    return kEmptyRange;
}
// Returns the range of selected text.
- (NSRange)selectedRange {
    return kEmptyRange;
}
// Replaces a specified range in the receiver�s text storage with the given string and sets the selection.
- (void)setMarkedText:(nonnull id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange {
}
// Unmarks the marked text.
- (void)unmarkText {
}
// Returns an array of attribute names recognized by the receiver.
- (nonnull NSArray<NSString *> *)validAttributesForMarkedText {
    return [NSArray array];
}
@end

@interface FBWindow : NSWindow<NSWindowDelegate>
@end
@implementation FBWindow
- (id)initWithContentRect:(NSRect)contentRect
styleMask:(NSWindowStyleMask)windowStyle
backing:(NSBackingStoreType)bufferingType
defer:(BOOL)deferCreation
{
    self = [super
        initWithContentRect:contentRect
        styleMask:windowStyle
        backing:bufferingType
        defer:deferCreation];

    if (self){
        self.delegate = self;
        NSRect bounds = [self frame];
        bounds.origin = NSZeroPoint;
        FBView *frameView = [[FBView alloc] initWithFrame:bounds];
        [super setContentView:frameView];
    }
    return self;
}
- (void)windowDidResize:(NSNotification *)notification {
    CGSize size = [self contentRectForFrameRect:[self frame]].size;
    fbWidth = (int)size.width;
    fbHeight = (int)size.height;
    if (fbWidth*fbHeight > fbPixelCount){
        while (fbWidth*fbHeight > fbPixelCount){
            fbPixelCount *= 2;
        }
        free(frameBuffer);
        frameBuffer = malloc(fbPixelCount*4);
    }
    draw();
}
- (BOOL)windowShouldClose:(NSWindow *) wnd {
    exit(0);
}
- (void)keyDown:(NSEvent *)event {
    //short int key_code = g_keycodes[[event keyCode] & 0x1ff];
    //window_data->key_status[key_code] = true;
    //kCall(keyboard_func, key_code, window_data->mod_keys, true);
    [self.contentView interpretKeyEvents:@[event]];
}
@end

id applicationName;
id window;
static void update_events(){
    NSEvent* event;
    @autoreleasepool {
        do {
            event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
            if (event) {
                [NSApp sendEvent:event];
            }
        } while (event);
    }
}
void update(){
    update_events();
    [[window contentView] setNeedsDisplay:YES];
}
int main(){
    frameBuffer = malloc(fbPixelCount*4);
    draw();
    @autoreleasepool{
        [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    applicationName = [[NSProcessInfo processInfo] processName];
    window = [[FBWindow alloc] initWithContentRect:NSMakeRect(0, 0, fbWidth, fbHeight)
        styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskResizable|NSWindowStyleMaskMiniaturizable
        backing:NSBackingStoreBuffered defer:NO];
    [window center];
    [window setTitle: applicationName];
    NSAppearance* appearance = [NSAppearance appearanceNamed:NSAppearanceNameVibrantDark];
    [window setAppearance:appearance];
    [window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
    while (1){
        update();
        //usleep(1000);
    }
    }
    return 0;
}
#endif
unsigned char font[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x5e,0x0,0x0,0x0,0x0,0x0,0x6,0x0,0x6,0x0,0x0,0x14,0x3e,0x14,0x3e,0x14,0x0,0x4c,0xfb,0x52,0x20,0x0,0x0,0x62,0x10,0x8,0x46,0x0,0x40,0xac,0xb2,0x4c,0xa0,0x90,0x0,0x0,0x0,0x6,0x0,0x0,0x0,0x3c,0x42,0x81,0x81,0x0,0x0,0x81,0x81,0x42,0x3c,0x0,0x0,0x0,0xa,0x4,0xa,0x0,0x0,0x10,0x10,0x7c,0x10,0x10,0x0,0x0,0xa0,0x60,0x0,0x0,0x0,0x8,0x8,0x8,0x8,0x0,0x0,0x0,0x60,0x60,0x0,0x0,0x0,0xc0,0x30,0xc,0x3,0x0,0x0,0x7e,0xe1,0x99,0x87,0x7e,0x0,0x84,0x82,0xff,0x80,0x80,0x0,0xc6,0xa1,0x91,0x89,0x86,0x0,0x46,0x81,0x89,0x89,0x76,0x0,0x30,0x2c,0x22,0xff,0x20,0x0,0x9f,0x91,0x89,0x89,0x71,0x0,0x7e,0xa1,0x91,0x91,0x66,0x0,0x1,0x1,0xe1,0x19,0x7,0x0,0x76,0x89,0x89,0x89,0x76,0x0,0x8e,0x91,0x91,0x91,0x7e,0x0,0x0,0x66,0x66,0x0,0x0,0x0,0x0,0xa6,0x66,0x0,0x0,0x0,0x10,0x28,0x44,0x0,0x0,0x0,0x24,0x24,0x24,0x24,0x0,0x0,0x0,0x44,0x28,0x10,0x0,0x0,0x2,0x1,0x99,0x9,0x6,0x3e,0x41,0x4d,0x5d,0x51,0x5e,0x0,0x78,0x16,0x11,0x16,0x78,0x0,0x7f,0x49,0x49,0x49,0x36,0x0,0x3e,0x41,0x41,0x41,0x22,0x0,0x7f,0x41,0x41,0x41,0x3e,0x0,0x7f,0x49,0x49,0x49,0x49,0x0,0x7f,0x9,0x9,0x9,0x1,0x0,0x3e,0x41,0x51,0x51,0x72,0x0,0x7f,0x8,0x8,0x8,0x7f,0x0,0x41,0x41,0x7f,0x41,0x41,0x0,0x31,0x41,0x41,0x3f,0x1,0x0,0x7f,0x8,0x14,0x22,0x41,0x0,0x7f,0x40,0x40,0x40,0x40,0x0,0x7f,0x2,0x4,0x2,0x7f,0x0,0x7f,0x3,0x1c,0x60,0x7f,0x0,0x3e,0x41,0x41,0x41,0x3e,0x0,0x7f,0x9,0x9,0x9,0x6,0x0,0x3e,0x41,0x31,0x61,0x5e,0x0,0x7f,0x11,0x31,0x51,0x4e,0x0,0x26,0x49,0x49,0x49,0x32,0x0,0x1,0x1,0x7f,0x1,0x1,0x0,0x3f,0x40,0x40,0x40,0x3f,0x0,0x1f,0x20,0x40,0x20,0x1f,0x0,0x3f,0x40,0x30,0x40,0x3f,0x0,0x63,0x14,0x8,0x14,0x63,0x0,0x3,0x4,0x78,0x4,0x3,0x0,0x61,0x51,0x49,0x45,0x43,0x0,0xff,0x81,0x81,0x81,0x81,0x0,0x3,0xc,0x30,0xc0,0x0,0x0,0x81,0x81,0x81,0x81,0xff,0x0,0x4,0x2,0x1,0x2,0x4,0x80,0x80,0x80,0x80,0x80,0x80,0x0,0x0,0x1,0x2,0x4,0x0,0x0,0x0,0x20,0x54,0x54,0x78,0x0,0x7f,0x48,0x48,0x30,0x0,0x0,0x38,0x44,0x44,0x44,0x28,0x0,0x0,0x30,0x48,0x48,0x7f,0x0,0x38,0x54,0x54,0x54,0x58,0x0,0x8,0x7e,0x9,0x9,0x2,0x0,0x40,0x98,0xa4,0xa4,0xf8,0x0,0x7f,0x8,0x8,0x70,0x0,0x0,0x0,0x0,0x7a,0x0,0x0,0x0,0x40,0x80,0x80,0x7a,0x0,0x0,0x7f,0x10,0x28,0x44,0x0,0x0,0x0,0x3f,0x40,0x0,0x0,0x0,0x78,0x4,0x8,0x4,0x78,0x0,0x0,0x7c,0x4,0x4,0x78,0x0,0x38,0x44,0x44,0x44,0x38,0x0,0x0,0xfc,0x24,0x24,0x18,0x0,0x0,0x18,0x24,0x24,0xfc,0x0,0x7c,0x8,0x4,0x4,0x8,0x0,0x8,0x54,0x54,0x54,0x20,0x0,0x4,0x3f,0x44,0x24,0x0,0x0,0x0,0x3c,0x40,0x40,0x7c,0x0,0xc,0x30,0x40,0x30,0xc,0x0,0x1c,0x60,0x10,0x60,0x1c,0x0,0x44,0x28,0x10,0x28,0x44,0x0,0x40,0x8c,0x90,0x90,0x7c,0x0,0x0,0x44,0x64,0x54,0x4c,0x0,0x10,0x56,0xa9,0x81,0x0,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x81,0xa9,0x56,0x10,0x0,0x0,0x10,0x8,0x10,0x8,0x0,};
u32 g_x = 0, g_y = 9;
#define WHITE 0x00ffffff
void draw_char(char c){
    char col;
    int i,j;
    for (i = 0; i < 6; i++){
        col = font[6*(c - 32) + i];
        for (j = 0; j < 8; j++){
            frameBuffer[(g_y+j)*fbWidth + g_x] = col & (1<<j) ? WHITE : 0;
        }
        g_x++;
    }
}
void draw_string_g(char *str){
    while (*str){
        draw_char(*str);
        str++;
    }
}
void newline(void){
    g_x = 0;
    g_y += 9;
}
void draw_string(int x, int y, char *str){
    char col;
    int i,j;
    while (*str){
        for (i = 0; i < 6; i++){
            col = font[6*(*str - 32) + i];
            for (j = 0; j < 8; j++){
                frameBuffer[(y+j)*fbWidth + x] = col & (1<<j) ? WHITE : 0;
            }
            x++;
        }
        str++;
    }
}
void draw(){
    for (int i = 0; i < fbPixelCount; i++){
        frameBuffer[i] = 0;
    }
    draw_string(0,0, "fart");
}
void char_input(char c){
    draw_char(c);
}