import re, collections, os

c = collections.Counter()
for line in open(r"C:\Users\jjxb_\GIT\Jill-EX\re\kdigest.txt"):
    if "calls:" in line:
        for m in re.finditer(r"(FUN_[0-9a-f]{4}_[0-9a-f]{4})(?: x(\d+))?", line):
            c[m.group(1)] += int(m.group(2) or 1)

DEC = r"C:\Users\jjxb_\GIT\Jill-EX\re\export\decomp"
for f, n in c.most_common(45):
    seg, off = f.split("_")[1], f.split("_")[2]
    p = os.path.join(DEC, "%s_%s_%s.c" % (seg, off, f))
    print("%4d  %s  %s" % (n, f, "ok" if os.path.exists(p) else "MISSING"))
print("distinct:", len(c))
