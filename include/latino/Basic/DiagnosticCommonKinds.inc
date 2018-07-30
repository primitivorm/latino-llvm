#ifdef COMMONSTART
__COMMONSTART = DIAG_START_COMMON,
#undef COMMONSTART
#endif

DIAG(err_cannot_open_file, CLASS_ERROR, (unsigned)diag::Severity::Fatal, "cannot open file '%0': %1", 0, SFINAE_SubstitutionFailure, false, true, 0)
DIAG(err_target_unknown_triple, CLASS_ERROR, (unsigned)diag::Severity::Error, "unknown target triple '%0', please use -triple or -arch", 0, SFINAE_SubstitutionFailure, false, true, 0)
DIAG(warn_mt_message, CLASS_WARNING, (unsigned)diag::Severity::Warning, "[rewriter] %0", 0, SFINAE_Suppress, false, false, 0)
