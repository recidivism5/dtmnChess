import sys
with open(sys.argv[2],"w") as fout:
    with open(sys.argv[1],"rb") as fin:
        fout.write("u8 b[]={")
        for b in fin.read():
            fout.write(str(b)+",")
        fout.write("};")