#ifndef LATINO_BASIC_DIAGNOSTICCATEGORIES_H
#define LATINO_BASIC_DIAGNOSTICCATEGORIES_H

namespace latino {
    namespace diag {
        enum {
            #define GET_CATEGORY_TABLE
            #define CATEGORY(X, ENUM) ENUM,
            #include "latino/Basic/DiagnosticGroup.inc"
            #undef CATEGORY
            #undef GET_CATEGORY_TABLE
            DiagCat_NUM_CATEGORIES
        };
    } /* namespace diag */
} /* namespace latino */

#endif /* LATINO_BASIC_DIAGNOSTICCATEGORIES_H */