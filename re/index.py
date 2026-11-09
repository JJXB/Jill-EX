import re, os, glob, collections, json

D = r"C:\Users\jjxb_\GIT\Jill-EX\re\export\decomp"
funcs = []
for p in glob.glob(os.path.join(D, "*.c")):
    t = open(p, encoding="utf-8", errors="replace").read()
    m = re.match(r"// (\S+) @ (\S+)", t)
    name, addr = m.group(1), m.group(2)
    strs = re.findall(r"s_([A-Za-z0-9_.!?+\-/ '\x27]{3,80}?)_2cff_[0-9a-f]{4}", t)
    calls = re.findall(r"\b(FUN_[0-9a-f]{4}_[0-9a-f]{4}|func_0x[0-9a-f]+)", t)
    ports = re.findall(r"out\(([^,]+),", t)
    consts = re.findall(r"0x[0-9a-f]{3,4}\b", t)
    funcs.append(dict(file=os.path.basename(p), name=name, addr=addr,
                      size=os.path.getsize(p), strings=sorted(set(strs)),
                      calls=sorted(set(calls)), consts=collections.Counter(consts)))

funcs.sort(key=lambda f: -f["size"])
with open(r"C:\Users\jjxb_\GIT\Jill-EX\re\export\index.txt", "w", encoding="utf-8") as fh:
    for f in funcs:
        f2 = dict(f); del f2["consts"]
        f2["top_consts"] = [c for c, n in f["consts"].most_common(8)]
        f2["nlines"] = 0
        fh.write(json.dumps(f2) + "\n")

# quick human summary of functions with strings
for f in funcs:
    if f["strings"]:
        print(f["addr"], f["name"], "|", " ; ".join(f["strings"])[:200])
