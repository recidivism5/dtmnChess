#include <stdint.h>
typedef int32_t i32;
typedef uint32_t u32;
typedef uint64_t u64;//king,queen,pawn,rook,bishop,knight, 6*64=384 lines = 384*8 bytes per line = 3072 bytes stb_image.h
#define BACKGROUND_COLOR 0x0000ff
#define WIDTH 512
#define HEIGHT 512
#define AT(x,y) ((x) + (y)*WIDTH)
#define FOR(var,val,count) for(int var = (val); var < count; var++)

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

i32 fbWidth = WIDTH, fbHeight = HEIGHT;
i32 fbPixelCount = WIDTH*HEIGHT;
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
- (void)doCommandBySelector:(nonnull SEL)selector {
}
- (nullable NSAttributedString *)attributedSubstringForProposedRange:(NSRange)range actualRange:(nullable NSRangePointer)actualRange {
    return nil;
}
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
            encoding:NSUTF32StringEncoding
            options:0
            range:range
            remainingRange:&range]) {

            if ((codepoint & 0xff00) == 0xf700)
                continue;

            char_input((char)codepoint);
        }
    }
}
- (NSUInteger)characterIndexForPoint:(NSPoint)point {
    return 0;
}
- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(nullable NSRangePointer)actualRange {
    return NSMakeRect(0.0, 0.0, 0.0, 0.0);
}
static const NSRange kEmptyRange = { NSNotFound, 0 };
- (BOOL)hasMarkedText {
    return false;
}
- (NSRange)markedRange {
    return kEmptyRange;
}
- (NSRange)selectedRange {
    return kEmptyRange;
}
- (void)setMarkedText:(nonnull id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange {
}
- (void)unmarkText {
}
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
#include "font_6x8.h"
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
    /*
    TODO: Make piece bitmaps
    TODO: Make image editor to make piece bitmaps
    */
    FOR(x,0,WIDTH) frameBuffer[x] = (x/(WIDTH/8))%2 ? 0 : WHITE;
    FOR(y,1,WIDTH)
        if ((y/(WIDTH/8))%2) memcpy(frameBuffer+AT(WIDTH/8,y), frameBuffer, (WIDTH*7/8)*sizeof(*frameBuffer));
        else memcpy(frameBuffer+AT(0,y), frameBuffer, WIDTH*sizeof(*frameBuffer));
    draw_string(WIDTH/2,WIDTH/2,"FEMTOCHESS");

    
    int x,y,n;
    x = 8;
    y = 8;
    n = 8;
    unsigned char *data = stbi_load("bishop.png", &x, &y,&n, 0);
    frameBuffer[x] = data;
}
void char_input(char c){
    draw_char(c);
}