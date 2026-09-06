// Replay verified annotations on the exact supported original.
// @category VirtualCity
import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.SourceType;

public class ReplayAnalysis extends GhidraScript {
    public void run() throws Exception {
        if (!"bdc3bc9f075a813d12078fd2eaaaa1131ba74616b11aa1444cdd5a5b83e08695"
                .equalsIgnoreCase(currentProgram.getExecutableSHA256())) {
            throw new IllegalStateException("Input hash mismatch; refusing address-based annotations");
        }

        var base = getFunctionAt(toAddr(0x00485f40L));
        if (base == null) throw new IllegalStateException("Missing function at 00485f40");
        if (base.getSymbol().getSource() == SourceType.DEFAULT || base.getName().equals("sq_base_register")) {
            base.setName("sq_base_register", SourceType.USER_DEFINED);
        }
        setPlateComment(base.getEntryPoint(),
            "Squirrel 2.2.3 sq_base_register; matched official sqbaselib.cpp:248. " +
            "SQChar=1, SQInteger=4, SQFloat=4. VM arrives in EAX after LTCG; do not force cdecl.");

        var lexer = getFunctionAt(toAddr(0x0047ef70L));
        if (lexer == null) throw new IllegalStateException("Missing function at 0047ef70");
        if (lexer.getSymbol().getSource() == SourceType.DEFAULT || lexer.getName().equals("SQLexer_LexBlockComment")) {
            lexer.setName("SQLexer_LexBlockComment", SourceType.USER_DEFINED);
        }
        lexer.addTag("VC1_VERIFIED");
        setPlateComment(lexer.getEntryPoint(),
            "SQLexer::LexBlockComment from Squirrel 2.2.3 sqlexer.cpp. " +
            "VC9 /GL /LTCG reccmp match 100%; actual lexer pointer is ESI.");

        String[] names = {"Color", "Vector3", "Vector2", "Vector4"};
        for (int i = 0; i < names.length; i++) {
            var address = toAddr(0x00423640L + 0x20L * i);
            var function = getFunctionAt(address);
            String name = "Vc1" + names[i] + "TypeOf";
            if (function == null) {
                disassemble(address);
                function = createFunction(address, name);
            }
            if (function == null) throw new IllegalStateException("Cannot create " + address);
            if (function.getSymbol().getSource() == SourceType.DEFAULT || function.getName().equals(name)) {
                function.setName(name, SourceType.USER_DEFINED);
            }
            function.addTag("VC1_VERIFIED");
            setPlateComment(address, "Algorithm: sq_pushstring(vm, \"" + names[i] + "\", -1). " +
                "Parameters: native callback VM on stack; callee uses LTCG ECX/EAX/stack ABI. " +
                "Returns: 1 Squirrel result. Descriptive name, not an original C++ symbol.");
        }

        var init = getFunctionAt(toAddr(0x004b7bd0L));
        if (init == null) throw new IllegalStateException("Missing function at 004b7bd0");
        if (init.getSymbol().getSource() == SourceType.DEFAULT) {
            init.setName("Vc1InitSizeClassPools", SourceType.USER_DEFINED);
        }
        init.addTag("VC1_INFERRED");
        setPlateComment(init.getEntryPoint(),
            "Algorithm: for i in 0..0x20 malloc 0x18, zero links, itemSize=i*4, growth=32. " +
            "Parameters: none. Returns: void. Called from 0x00463b40. Descriptive name.");

        var destroy = getFunctionAt(toAddr(0x004b7c60L));
        if (destroy == null) throw new IllegalStateException("Missing function at 004b7c60");
        if (destroy.getSymbol().getSource() == SourceType.DEFAULT) {
            destroy.setName("Vc1DestroySizeClassPools", SourceType.USER_DEFINED);
        }
        destroy.addTag("VC1_INFERRED");
        setPlateComment(destroy.getEntryPoint(),
            "Algorithm: for each pools[i], free block chain via trailer, restore growthCount from +0x14, free header. " +
            "Parameters: none. Returns: void. Called from 0x004c38b0. Descriptive name.");

        println("Replayed annotations");
    }
}
