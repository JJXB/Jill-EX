p = r"C:\Users\jjxb_\GIT\Jill-EX\docs\jill1-engine.md"
d = open(p, encoding="utf-8").read()
m = {
    "\u2014": "-",
    "\u2192": "->",
    "\u00d7": "x",
    "\u2026": "...",
    "\u2212": "-",
    "\u2013": "-",
    "\u00a7": "#",
    "\u00b9": "1",
}
for k, v in m.items():
    d = d.replace(k, v)
open(p, "w", encoding="ascii", newline="\n").write(d)
raw = open(p, "rb").read()
print("ascii ok", all(b < 128 for b in raw), len(raw))
