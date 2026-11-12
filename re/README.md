# Reverse-engineering scratch space

The scripts here are development tools for analysing **your own** copy of the
original executables.  Nothing in this folder is needed to build or run
Jill-EX.

Artifacts produced by these tools are derived from Epic's copyrighted binaries:

```text
img*.bin     extracted executable images
proj/        Ghidra project (contains the imported executable)
export/      decompiled C, symbol/data dumps
*.txt        disassembly and string listings
*.asm        disassembly listings
*.png        rendered game art
```

They are deliberately untracked (see `/.gitignore`) and must not be
redistributed.  Generate them locally from a legally obtained copy of the
games.
