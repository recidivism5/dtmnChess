with open("font_6x8.bin", "rb") as f:
    with open("font_6x8.h", "w") as of:
        of.write("unsigned char font[]={")
        for b in f.read():
            of.write(hex(b)+',')
        of.write("};")