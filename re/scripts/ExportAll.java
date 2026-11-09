// Export decompiled C, symbols, strings and memory map from a Ghidra program.
// @category RE
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.util.*;

public class ExportAll extends GhidraScript {
    File outDir;

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        outDir = new File(args.length > 0 ? args[0] : "re/export");
        outDir.mkdirs();
        new File(outDir, "decomp").mkdirs();

        Program prog = currentProgram;
        println("Exporting " + prog.getName() + " to " + outDir);

        // ---- memory map ----
        try (PrintWriter pw = new PrintWriter(new File(outDir, "memory.txt"))) {
            for (MemoryBlock b : prog.getMemory().getBlocks()) {
                pw.printf("%s %s %s size=0x%x r=%b w=%b x=%b vol=%b%n",
                    b.getName(), b.getStart(), b.getEnd(), b.getSize(),
                    b.isRead(), b.isWrite(), b.isExecute(), b.isVolatile());
            }
        }

        // ---- symbols ----
        try (PrintWriter pw = new PrintWriter(new File(outDir, "symbols.txt"))) {
            SymbolTable st = prog.getSymbolTable();
            for (Symbol s : st.getAllSymbols(true)) {
                pw.printf("%s %-40s %s%n", s.getAddress(), s.getName(), s.getSymbolType());
            }
        }

        // ---- defined data / strings ----
        try (PrintWriter pw = new PrintWriter(new File(outDir, "data.txt"))) {
            Listing lst = prog.getListing();
            DataIterator di = lst.getDefinedData(true);
            while (di.hasNext()) {
                Data d = di.next();
                String v = d.getValue() == null ? "" : d.getValue().toString();
                if (v.length() > 120) v = v.substring(0, 120);
                pw.printf("%s %-24s %s%n", d.getAddress(), d.getDataType().getName(), v.replace('\n', ' '));
            }
        }

        // ---- functions + decompiled C ----
        DecompInterface di = new DecompInterface();
        di.openProgram(prog);
        DecompileOptions opts = new DecompileOptions();
        di.setOptions(opts);

        FunctionManager fm = prog.getFunctionManager();
        int n = 0, ok = 0;
        try (PrintWriter pw = new PrintWriter(new File(outDir, "functions.txt"))) {
            for (Function f : fm.getFunctions(true)) {
                n++;
                pw.printf("%s %-40s size=%-6d params=%d ret=%s%n",
                    f.getEntryPoint(), f.getName(), f.getBody().getNumAddresses(),
                    f.getParameterCount(),
                    f.getReturnType() == null ? "?" : f.getReturnType().getName());
                try {
                    DecompileResults res = di.decompileFunction(f, 45, monitor);
                    if (res != null && res.decompileCompleted()) {
                        File out = new File(outDir, "decomp/" + f.getEntryPoint().toString().replace(':', '_') + "_" +
                            f.getName().replaceAll("[^A-Za-z0-9_.-]", "_") + ".c");
                        try (PrintWriter fw = new PrintWriter(out)) {
                            fw.println("// " + f.getName() + " @ " + f.getEntryPoint());
                            fw.println(res.getDecompiledFunction().getC());
                        }
                        ok++;
                    }
                } catch (Exception e) {
                    println("decomp fail " + f.getEntryPoint() + " " + e);
                }
            }
        }
        println("functions=" + n + " decompiled=" + ok);
    }
}
