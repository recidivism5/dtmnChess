import sys
def toSigned(b):
    if b > 127:
        return (256-b) * (-1)
    else:
        return b
with open(sys.argv[2],"w") as fout:
    with open(sys.argv[1],"rb") as fin:
        fout.write("i8 b[]={")
        for b in fin.read():
            fout.write(str(toSigned(b))+",")
        fout.write("};")