#Removes all comments and all empty lines from C or C++ code
#usage: python ccruncher.py in.txt out.txt
#Modified from https://gist.github.com/ChunMinChang/88bfa5842396c1fbbc5b to replace multiple newlines with single newlines

import re
import sys

def commentRemover(text):
    def replacer(match):
        s = match.group(0)
        if s.startswith('/'):
            return ""
        else:
            return s
    pattern = re.compile(
        r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
        re.DOTALL | re.MULTILINE
    )
    def replacer2(match):
        return "{"
    pattern2 = re.compile(
        r'\s+{',
        re.DOTALL | re.MULTILINE
    )
    return re.sub(pattern2, replacer2, "".join(line.rstrip()+'\n' for line in re.sub(pattern, replacer, text).split('\n')))

open(sys.argv[2], "w").write(re.sub(r'\n\s*\n', '\n', commentRemover(open(sys.argv[1], "r").read())).strip())