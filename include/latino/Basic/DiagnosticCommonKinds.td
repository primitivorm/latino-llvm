let Component = "Common" in {
// Targets
def err_target_unknown_triple : Error<
  "unknown target triple '%0', please use -triple or -arch">;

// Source manager
def err_cannot_open_file : Error<"cannot open file '%0': %1">, DefaultFatal;

// TransformActions
def warn_mt_message : Warning<"[rewriter] %0">;

}
