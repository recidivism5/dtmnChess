#include <stdio.h>
#include <stdint.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int32_t i32;
typedef int bool;
#define TRUE 1
#define FALSE 0
#define BACKGROUND_COLOR 0
#define WHITE 0x00ffffff
#define RED 0x00990000
#define BROWN 0x00160e09
#define LIGHT_BROWN 0x0024170e
#define SHADOW 0
#define BOARD_GREEN 0x00006400
#define BOARD_WHITE 0x00b4b4b4
#define PIECE_WIDTH 16
#define CELL_WIDTH 20
#define WIDTH (CELL_WIDTH*8 + CELL_WIDTH*4)
#define HEIGHT (CELL_WIDTH*8)
#define SCALE 4
#define WND_WIDTH (SCALE*WIDTH)
#define WND_HEIGHT (SCALE*HEIGHT)
u32 frameBuffer[WIDTH*HEIGHT];
#define FAT(x,y) ((y)*WIDTH + (x))
char title[] = "dtmnChess";
unsigned char font[]={0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x5e,0x0,0x0,0x0,0x0,0x0,0x6,0x0,0x6,0x0,0x0,0x14,0x3e,0x14,0x3e,0x14,0x0,0x4c,0xfb,0x52,0x20,0x0,0x0,0x62,0x10,0x8,0x46,0x0,0x40,0xac,0xb2,0x4c,0xa0,0x90,0x0,0x0,0x0,0x6,0x0,0x0,0x0,0x3c,0x42,0x81,0x81,0x0,0x0,0x81,0x81,0x42,0x3c,0x0,0x0,0x0,0xa,0x4,0xa,0x0,0x0,0x10,0x10,0x7c,0x10,0x10,0x0,0x0,0xa0,0x60,0x0,0x0,0x0,0x8,0x8,0x8,0x8,0x0,0x0,0x0,0x60,0x60,0x0,0x0,0x0,0xc0,0x30,0xc,0x3,0x0,0x0,0x7e,0xe1,0x99,0x87,0x7e,0x0,0x84,0x82,0xff,0x80,0x80,0x0,0xc6,0xa1,0x91,0x89,0x86,0x0,0x46,0x81,0x89,0x89,0x76,0x0,0x30,0x2c,0x22,0xff,0x20,0x0,0x9f,0x91,0x89,0x89,0x71,0x0,0x7e,0xa1,0x91,0x91,0x66,0x0,0x1,0x1,0xe1,0x19,0x7,0x0,0x76,0x89,0x89,0x89,0x76,0x0,0x8e,0x91,0x91,0x91,0x7e,0x0,0x0,0x66,0x66,0x0,0x0,0x0,0x0,0xa6,0x66,0x0,0x0,0x0,0x10,0x28,0x44,0x0,0x0,0x0,0x24,0x24,0x24,0x24,0x0,0x0,0x0,0x44,0x28,0x10,0x0,0x0,0x2,0x1,0x99,0x9,0x6,0x3e,0x41,0x4d,0x5d,0x51,0x5e,0x0,0x78,0x16,0x11,0x16,0x78,0x0,0x7f,0x49,0x49,0x49,0x36,0x0,0x3e,0x41,0x41,0x41,0x22,0x0,0x7f,0x41,0x41,0x41,0x3e,0x0,0x7f,0x49,0x49,0x49,0x49,0x0,0x7f,0x9,0x9,0x9,0x1,0x0,0x3e,0x41,0x51,0x51,0x72,0x0,0x7f,0x8,0x8,0x8,0x7f,0x0,0x41,0x41,0x7f,0x41,0x41,0x0,0x31,0x41,0x41,0x3f,0x1,0x0,0x7f,0x8,0x14,0x22,0x41,0x0,0x7f,0x40,0x40,0x40,0x40,0x0,0x7f,0x2,0x4,0x2,0x7f,0x0,0x7f,0x3,0x1c,0x60,0x7f,0x0,0x3e,0x41,0x41,0x41,0x3e,0x0,0x7f,0x9,0x9,0x9,0x6,0x0,0x3e,0x41,0x31,0x61,0x5e,0x0,0x7f,0x11,0x31,0x51,0x4e,0x0,0x26,0x49,0x49,0x49,0x32,0x0,0x1,0x1,0x7f,0x1,0x1,0x0,0x3f,0x40,0x40,0x40,0x3f,0x0,0x1f,0x20,0x40,0x20,0x1f,0x0,0x3f,0x40,0x30,0x40,0x3f,0x0,0x63,0x14,0x8,0x14,0x63,0x0,0x3,0x4,0x78,0x4,0x3,0x0,0x61,0x51,0x49,0x45,0x43,0x0,0xff,0x81,0x81,0x81,0x81,0x0,0x3,0xc,0x30,0xc0,0x0,0x0,0x81,0x81,0x81,0x81,0xff,0x0,0x4,0x2,0x1,0x2,0x4,0x80,0x80,0x80,0x80,0x80,0x80,0x0,0x0,0x1,0x2,0x4,0x0,0x0,0x0,0x20,0x54,0x54,0x78,0x0,0x7f,0x48,0x48,0x30,0x0,0x0,0x38,0x44,0x44,0x44,0x28,0x0,0x0,0x30,0x48,0x48,0x7f,0x0,0x38,0x54,0x54,0x54,0x58,0x0,0x8,0x7e,0x9,0x9,0x2,0x0,0x40,0x98,0xa4,0xa4,0xf8,0x0,0x7f,0x8,0x8,0x70,0x0,0x0,0x0,0x0,0x7a,0x0,0x0,0x0,0x40,0x80,0x80,0x7a,0x0,0x0,0x7f,0x10,0x28,0x44,0x0,0x0,0x0,0x3f,0x40,0x0,0x0,0x0,0x78,0x4,0x8,0x4,0x78,0x0,0x0,0x7c,0x4,0x4,0x78,0x0,0x38,0x44,0x44,0x44,0x38,0x0,0x0,0xfc,0x24,0x24,0x18,0x0,0x0,0x18,0x24,0x24,0xfc,0x0,0x7c,0x8,0x4,0x4,0x8,0x0,0x8,0x54,0x54,0x54,0x20,0x0,0x4,0x3f,0x44,0x24,0x0,0x0,0x0,0x3c,0x40,0x40,0x7c,0x0,0xc,0x30,0x40,0x30,0xc,0x0,0x1c,0x60,0x10,0x60,0x1c,0x0,0x44,0x28,0x10,0x28,0x44,0x0,0x40,0x8c,0x90,0x90,0x7c,0x0,0x0,0x44,0x64,0x54,0x4c,0x0,0x10,0x56,0xa9,0x81,0x0,0x0,0x0,0x0,0xff,0x0,0x0,0x0,0x81,0xa9,0x56,0x10,0x0,0x0,0x10,0x8,0x10,0x8,0x0,};
void drawString(int x, int y, char *str){
    char col;
    int i,j;
    while (*str){
        for (i = 0; i < 6; i++){
            col = font[6*(*str - 32) + i];
            for (j = 0; j < 8; j++) if (col & (1<<j)) frameBuffer[(y+j)*WIDTH + x] = WHITE;
            x++;
        }
        str++;
    }
}
u16 pawn[]={0x00,0x00,0x00,0x3C0,0x7E0,0x7E0,0x7E0,0x3C0,0x180,0x180,0x3C0,0x3C0,0x7E0,0x1FF8,0x3FFC,0x3FFC,};
u16 bishop[]={0x00,0x00,0x3C0,0x180,0x3C0,0x3E0,0x1E0,0xCF0,0xEF0,0xFF0,0xFF0,0xFF0,0x7E0,0x3C0,0x3FFC,0x3FFC,};
u16 rook[]={0x00,0x00,0x1248,0x1FF8,0xFF0,0x7E0,0x7E0,0x7E0,0x7E0,0x7E0,0xFF0,0xFF0,0xFF0,0xFF0,0x3FFC,0x3FFC,};
u16 knight[]={0x00,0x20,0x60,0x3F0,0xFF8,0xFD8,0x1FFC,0x1FFE,0x1E7E,0x1E2E,0x1F00,0xFC0,0xFE0,0xFF0,0x3FFC,0x3FFC,};
u16 queen[]={0x00,0x00,0x420,0xE70,0x420,0x660,0x27E4,0x77EE,0x27E4,0x3FFC,0x1FF8,0x1FF8,0xFF0,0x7E0,0x3FFC,0x3FFC,};
u16 king[]={0x00,0x180,0x180,0x7E0,0x7E0,0x180,0xDB0,0x1BD8,0x318C,0x318C,0x318C,0x1998,0xDB0,0xFF0,0x3FFC,0x3FFC,};
typedef struct Cell {
    u16 *piece;
    bool side;
}Cell;
Cell board[8*8];
#define BAT(x,y) ((y)*8 + (x))
bool side = 0;
u32 pieceColors[2] = {WHITE, RED};
void setCell(int x, int y, u16 *piece, bool side){
    board[y*8 + x].piece = piece;
    board[y*8 + x].side = side;
}
void setRow(int y, bool side){
    setCell(0,y, rook, side);
    setCell(1,y, knight, side);
    setCell(2,y, bishop, side);
    setCell(3,y, queen, side);
    setCell(4,y, king, side);
    setCell(5,y, bishop, side);
    setCell(6,y, knight, side);
    setCell(7,y, rook, side);
}
void setBoard(){
    setRow(0, 0);
    setRow(7, 1);
    for (int x = 0; x < 8; x++){
        setCell(x, 1, pawn, 0);
        setCell(x, 6, pawn, 1);
    }
}
void drawSquare(int x, int y, u32 color){
     for (int j = 0; j < CELL_WIDTH; j++)
        for (int i = 0; i < CELL_WIDTH; i++)
            frameBuffer[(y+j)*WIDTH + x+i] = color;
}
void drawPiece(u16 *piece, u32 color, int x, int y){
    for (int j = 0; j < PIECE_WIDTH; j++)
        for (int i = 0; i < PIECE_WIDTH; i++)
            if (piece[j] & (1<<i)){
                frameBuffer[(y+j)*WIDTH + x+i] = color;
                frameBuffer[(y+j+1)*WIDTH + x+i+1] = SHADOW;
            }
}
void drawPieceOnCell(u16 *piece, u32 color, int x, int y){
    drawPiece(piece, color, x*CELL_WIDTH+(CELL_WIDTH-PIECE_WIDTH)/2, y*CELL_WIDTH+(CELL_WIDTH-PIECE_WIDTH)/2);
}
void init(){
    setBoard();
}
bool moveLegal(int x, int y, int tx, int ty){
    Cell *start = board + BAT(x,y);
    Cell *target = board + BAT(tx,ty);
    if (!start->piece) return FALSE;
    if (start->piece == pawn){
        if ((tx == x) &&
        (side ? ty < y : ty > y) &&
        (abs(ty-y) <= (y==6 || (y==1) ? 2 : 1)) &&
        (!target->piece)) return TRUE;
    }
    return FALSE;
}
bool mouseMove(int x, int y){
    return FALSE;
}
Cell *selectedCell;
char mousePos[32];
bool mouseLeftDown(int x, int y){
    int cx = side ? 7-x/CELL_WIDTH : x/CELL_WIDTH,
        cy = side ? y/CELL_WIDTH : 7-y/CELL_WIDTH;
    if (board[BAT(cx,cy)].piece && (side==board[BAT(cx,cy)].side)) selectedCell = board + BAT(cx,cy);
    else if (selectedCell){
        int x = (selectedCell-board) % 8,
            y = (selectedCell-board) / 8;
        if (moveLegal(x,y, cx,cy)){
            board[BAT(cx,cy)] = *selectedCell;
            selectedCell->piece = NULL;
            selectedCell = NULL;
        }
    }
    sprintf(mousePos, "%d,%d", cx, cy);
    return TRUE;
}
bool mouseRightDown(int x, int y){
    return FALSE;
}
void fillRect(int x, int y, int width, int height, u32 color){
    for (int j = 0; j < height; j++){
        for (int i = 0; i < width; i++){
            frameBuffer[FAT(x+i,y+j)] = color;
        }
    }
}
#define DRAW_BUTTON(index, str) drawString(8*CELL_WIDTH+4, (index)*10+4, str);
char name[9];
void draw(){
    for (int i = 0; i < (WIDTH*HEIGHT); i++) frameBuffer[i] = BROWN;
    for (int y = 0; y < 8; y++){
        for (int x = 0; x < 8; x++){
            int scrY = side ? y : 7-y,
                scrX = side ? 7-x : x;
            drawSquare(scrX*CELL_WIDTH,scrY*CELL_WIDTH, (scrX%2)^(scrY%2) ? BOARD_GREEN : BOARD_WHITE);
            Cell c = board[BAT(x,y)];
            if (c.piece) drawPieceOnCell(c.piece, pieceColors[c.side], scrX, scrY);
        }
    }
    drawString(0,0, mousePos);
    //max chars on row is 12
    DRAW_BUTTON(0, "Time:");
    DRAW_BUTTON(1, "<   1 min  >");
    DRAW_BUTTON(2, "CPU lvl:");
    DRAW_BUTTON(3, "<    10    >");
    DRAW_BUTTON(5, "Play CPU");
    DRAW_BUTTON(7, "Name?...");
    DRAW_BUTTON(8, "Play Human");
}
void charInput(char c){

}
#if _WIN32
#undef UNICODE
#include <windows.h>
#include <dwmapi.h>
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
struct BMI {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[3];
} bmi;
HDC hdc;
#define UPDATE draw(); StretchDIBits(hdc, 0,0, WND_WIDTH,WND_HEIGHT, 0,0,WIDTH,HEIGHT,frameBuffer, &bmi, DIB_RGB_COLORS, SRCCOPY);
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
    case WM_MOUSEMOVE:
        if (mouseMove(GET_X_LPARAM(lparam)/SCALE, GET_Y_LPARAM(lparam)/SCALE)){UPDATE}
        return 0;
    case WM_LBUTTONDOWN:
        if (mouseLeftDown(GET_X_LPARAM(lparam)/SCALE, GET_Y_LPARAM(lparam)/SCALE)){UPDATE}
        return 0;
    case WM_RBUTTONDOWN:
        if (mouseRightDown(GET_X_LPARAM(lparam)/SCALE, GET_Y_LPARAM(lparam)/SCALE)){UPDATE}
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
RECT wr;
int APIENTRY WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow){
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_BITFIELDS;
    bmi.bmiHeader.biWidth = WIDTH;
    bmi.bmiHeader.biHeight = -HEIGHT;
    bmi.bmiColors[0].rgbRed = 0xff;
    bmi.bmiColors[1].rgbGreen = 0xff;
    bmi.bmiColors[2].rgbBlue = 0xff;
    init();
    wc.hInstance = hCurrentInst;
    wc.hIcon = LoadIconA(0,IDI_APPLICATION);
    wc.hCursor = LoadCursorA(0,IDC_ARROW);
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(BACKGROUND_COLOR);
    RegisterClassA(&wc);
    wr.right = WND_WIDTH;
    wr.bottom = WND_HEIGHT;
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW^WS_THICKFRAME, FALSE);
    wnd = CreateWindowExA(0,title,title,WS_VISIBLE|WS_OVERLAPPEDWINDOW^WS_THICKFRAME,CW_USEDEFAULT,CW_USEDEFAULT,wr.right-wr.left,wr.bottom-wr.top,NULL,NULL,wc.hInstance,NULL);
    hdc = GetDC(wnd);
    UPDATE
    while (GetMessageA(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
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
    CGContextSetInterpolationQuality(context, 1);
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider = CGDataProviderCreateWithData(0x0,
        frameBuffer,
        sizeof(frameBuffer),
        0x0
    );
    CGImageRef img = CGImageCreate(WIDTH
        , HEIGHT
        , 8
        , 32
        , WIDTH * 4
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
        CGRectMake(0, 0, WND_WIDTH, WND_HEIGHT),
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

            charInput((char)codepoint);
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
- (BOOL)windowShouldClose:(NSWindow *) wnd {
    exit(0);
}
- (void)keyDown:(NSEvent *)event {
    [self.contentView interpretKeyEvents:@[event]];
}
@end

id applicationName;
id window;
int main(){
    init();
    draw();
    @autoreleasepool{
        [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    applicationName = [[NSProcessInfo processInfo] processName];
    window = [[FBWindow alloc] initWithContentRect:NSMakeRect(0, 0, WND_WIDTH, WND_HEIGHT)
        styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable
        backing:NSBackingStoreBuffered defer:NO];
    [window center];
    [window setTitle: applicationName];
    NSAppearance* appearance = [NSAppearance appearanceNamed:NSAppearanceNameVibrantDark];
    [window setAppearance:appearance];
    [window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
    while (1){
        NSEvent* event;
        @autoreleasepool {
            do {
                event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
                if (event) {
                    [NSApp sendEvent:event];
                }
            } while (event);
        }
        [[window contentView] setNeedsDisplay:YES];
        //usleep(1000);
    }
    }
    return 0;
}
#endif