set(LLVM_TARGET_DEFINITIONS Options.td)
tablegen(LLVM Options.inc -gen-opt-parser-defs)
# TODO: proman Revisar
add_public_tablegen_target(LatinoDriverOptions)
