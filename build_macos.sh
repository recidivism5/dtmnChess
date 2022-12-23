if [ "$#" -ne 2 ]; then
    echo "Usage: ./build_macos.sh <input obj-c file> <output app name>"
    exit
fi
mkdir $2.app
mkdir $2.app/Contents
mkdir $2.app/Contents/MacOS
clang -framework Cocoa -x objective-c -Os $1 -o $2.app/Contents/MacOS/$2
echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<plist version=\"1.0\">
<dict>
	<key>CFBundleExecutable</key>
	<string>$2</string>
</dict>
</plist>
" > $2.app/Contents/Info.plist