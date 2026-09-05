// Restore the manually verified initial Squirrel anchor on this exact input.
// @category VirtualCity
import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.SourceType;

public class SeedVirtualCity extends GhidraScript {
    @Override
    public void run() throws Exception {
        String expected = "bdc3bc9f075a813d12078fd2eaaaa1131ba74616b11aa1444cdd5a5b83e08695";
        if (!expected.equalsIgnoreCase(currentProgram.getExecutableSHA256())) {
            throw new IllegalStateException("Input hash mismatch; refusing address-based annotations");
        }
        var address = toAddr(0x00485f40L);
        var function = getFunctionAt(address);
        if (function == null) throw new IllegalStateException("No function at " + address);
        function.setName("sq_base_register", SourceType.USER_DEFINED);
        setPlateComment(address,
            "Squirrel 2.2.3 sq_base_register; matched official sqbaselib.cpp:248. " +
            "SQChar=1 (004860b7), SQInteger=4 (00486108/0048610d), SQFloat=4 (0048615a). " +
            "VM pointer arrives in EAX (00485f45): retain LTCG custom ABI; do not force cdecl. " +
            "See docs/toolchain.md and analysis/sq_base_register.asm.json.");
        println("Seeded sq_base_register at " + address);
    }
}
