n = "dtmnChess"
mkdir $n.app
mkdir $n.app/Contents
mkdir $n.app/Contents/MacOS
clang -framework Cocoa -x objective-c -Os $n.c -o $n.app/Contents/MacOS/$n
echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<plist version=\"1.0\">
<dict>
	<key>CFBundleExecutable</key>
	<string>$n</string>
</dict>
</plist>
" > $n.app/Contents/Info.plist