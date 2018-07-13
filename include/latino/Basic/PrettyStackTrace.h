#ifndef LATINO_BASIC_PRETTYSTACKTRACE_H
#define LATINO_BASIC_PRETTYSTACKTRACE_H

#include "latino/Basic/SourceLocation.h"
#include "llvm/Support/PrettyStackTrace.h"

namespace latino {
    /// If a crash happens while one of these objects are live, the message
    /// is printed out along with the specified source location
    class PrettyStackTraceLoc : public llvm::PrettyStackTraceEntry {
        SourceManager &SM;
        SourceLocation Loc;
        const char *Message;
        public:
        PrettyStackTraceLoc(SourceManager &sm, SourceLocation L, const char *Msg)
        : SM(sm), Loc(L), Message(Msg) {}
        
        void print(raw_ostream &OS) const override;

    }; /* PrettyStackTraceLoc */
} /* namespace latino */

#endif /* LATINO_BASIC_PRETTYSTACKTRACE_H */ 