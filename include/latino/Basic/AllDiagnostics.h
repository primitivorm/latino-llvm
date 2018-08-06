#ifndef LATINO_BASIC_ALLDIAGNOSTICS_H
#define LATINO_BASIC_ALLDIAGNOSTICS_H

namespace latino {
template <size_t SizeOfStr, typename FieldType> class StringSizerHelper {
  static_assert(SizeOfStr <= FieldType(~0U), "Field too small!");

public:
  enum { Size = SizeOfStr };
};
} /* namespace latino */

#define STR_SIZE(str, fieldTy)                                                 \
  latino::StringSizerHelper<sizeof(str) - 1, fieldTy>::Size

#endif /* LATINO_BASIC_ALLDIAGNOSTICS_H */
