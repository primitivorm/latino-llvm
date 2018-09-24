#include "latino/Basic/MemoryBufferCache.h"
#include "llvm/Support/MemoryBuffer.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace latino;

namespace {

std::unique_ptr<MemoryBuffer> getBuffer(int I) {
  SmallVector<char, 8> Bytes;
  raw_svector_ostream(Bytes) << "data:" << I;
  return MemoryBuffer::getBuffer(StringRef(Bytes.data(), Bytes.size()), "",
                                 /*RequiredNullTerminator=*/false);
}

TEST(MemoryBufferCacheTest, addBuffer) {
  auto B1 = getBuffer(1);
  auto B2 = getBuffer(2);
  auto B3 = getBuffer(3);
  auto *RawB1 = B1.get();
  auto *RawB2 = B2.get();
  auto *RawB3 = B3.get();

  // Add a few buffers.
  MemoryBufferCache Cache;
  EXPECT_EQ(RawB1, &Cache.addBuffer("1", std::move(B1)));
}
} // namespace
