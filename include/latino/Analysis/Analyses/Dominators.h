//- Dominators.h - Implementation of dominators tree for Clang CFG -*- C++ -*-//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the dominators tree functionality for Clang CFGs.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_ANALYSIS_ANALYSES_DOMINATORS_H
#define LLVM_LATINO_ANALYSIS_ANALYSES_DOMINATORS_H

#include "latino/Analysis/AnalysisDeclContext.h"
#include "latino/Analysis/CFG.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/iterator.h"
#include "llvm/Support/GenericIteratedDominanceFrontier.h"
#include "llvm/Support/GenericDomTree.h"
#include "llvm/Support/GenericDomTreeConstruction.h"
#include "llvm/Support/raw_ostream.h"

// FIXME: There is no good reason for the domtree to require a print method
// which accepts an LLVM Module, so remove this (and the method's argument that
// needs it) when that is fixed.

namespace llvm {

class Module;

} // namespace llvm

namespace latino {

using DomTreeNode = llvm::DomTreeNodeBase<CFGBlock>;

/// Dominator tree builder for Clang's CFG based on llvm::DominatorTreeBase.
template <bool IsPostDom>
class CFGDominatorTreeImpl : public ManagedAnalysis {
  virtual void anchor();

public:
  using DominatorTreeBase = llvm::DominatorTreeBase<CFGBlock, IsPostDom>;

  CFGDominatorTreeImpl() = default;

  CFGDominatorTreeImpl(CFG *cfg) {
    buildDominatorTree(cfg);
  }

  ~CFGDominatorTreeImpl() override = default;

  DominatorTreeBase &getBase() { return DT; }

  CFG *getCFG() { return cfg; }

  /// \returns the root CFGBlock of the dominators tree.
  CFGBlock *getRoot() const {
    return DT.getRoot();
  }

  /// \returns the root DomTreeNode, which is the wrapper for CFGBlock.
  DomTreeNode *getRootNode() {
    return DT.getRootNode();
  }

  /// Compares two dominator trees.
  /// \returns false if the other dominator tree matches this dominator tree,
  /// false otherwise.
  bool compare(CFGDominatorTreeImpl &Other) const {
    DomTreeNode *R = getRootNode();
    DomTreeNode *OtherR = Other.getRootNode();

    if (!R || !OtherR || R->getBlock() != OtherR->getBlock())
      return true;

    if (DT.compare(Other.getBase()))
      return true;

    return false;
  }

  /// Builds the dominator tree for a given CFG.
  void buildDominatorTree(CFG *cfg) {
    assert(cfg);
    this->cfg = cfg;
    DT.recalculate(*cfg);
  }

  /// Dumps immediate dominators for each block.
  void dump() {
    llvm::errs() << "Immediate " << (IsPostDom ? "post " : "")
                 << "dominance tree (Node#,IDom#):\n";
    for (CFG::const_iterator I = cfg->begin(),
        E = cfg->end(); I != E; ++I) {

      assert(*I &&
             "LLVM's Dominator tree builder uses nullpointers to signify the "
             "virtual root!");

      DomTreeNode *IDom = DT.getNode(*I)->getIDom();
      if (IDom && IDom->getBlock())
        llvm::errs() << "(" << (*I)->getBlockID()
                     << ","
                     << IDom->getBlock()->getBlockID()
                     << ")\n";
      else {
        bool IsEntryBlock = *I == &(*I)->getParent()->getEntry();
        bool IsExitBlock = *I == &(*I)->getParent()->getExit();

        bool IsDomTreeRoot = !IDom && !IsPostDom && IsEntryBlock;
        bool IsPostDomTreeRoot =
            IDom && !IDom->getBlock() && IsPostDom && IsExitBlock;

        assert((IsDomTreeRoot || IsPostDomTreeRoot) &&
               "If the immediate dominator node is nullptr, the CFG block "
               "should be the exit point (since it's the root of the dominator "
               "tree), or if the CFG block it refers to is a nullpointer, it "
               "must be the entry block (since it's the root of the post "
               "dominator tree)");

        (void)IsDomTreeRoot;
        (void)IsPostDomTreeRoot;

        llvm::errs() << "(" << (*I)->getBlockID()
                     << "," << (*I)->getBlockID() << ")\n";
      }
    }
  }

  /// Tests whether \p A dominates \p B.
  /// Note a block always dominates itself.
  bool dominates(const CFGBlock *A, const CFGBlock *B) const {
    return DT.dominates(A, B);
  }

  /// Tests whether \p A properly dominates \p B.
  /// \returns false if \p A is the same block as \p B, otherwise whether A
  /// dominates B.
  bool properlyDominates(const CFGBlock *A, const CFGBlock *B) const {
    return DT.properlyDominates(A, B);
  }

  /// \returns the nearest common dominator CFG block for CFG block \p A and \p
  /// B. If there is no such block then return NULL.
  CFGBlock *findNearestCommonDominator(CFGBlock *A, CFGBlock *B) {
    return DT.findNearestCommonDominator(A, B);
  }

  const CFGBlock *findNearestCommonDominator(const CFGBlock *A,
                                             const CFGBlock *B) {
    return DT.findNearestCommonDominator(A, B);
  }

  /// Update the dominator tree information when a node's immediate dominator
  /// changes.
  void changeImmediateDominator(CFGBlock *N, CFGBlock *NewIDom) {
    DT.changeImmediateDominator(N, NewIDom);
  }

  /// Tests whether \p A is reachable from the entry block.
  bool isReachableFromEntry(const CFGBlock *A) {
    return DT.isReachableFromEntry(A);
  }

  /// Releases the memory held by the dominator tree.
  virtual void releaseMemory() { DT.reset(); }

  /// Converts the dominator tree to human readable form.
  virtual void print(raw_ostream &OS, const llvm::Module* M= nullptr) const {
    DT.print(OS);
  }

private:
  CFG *cfg;
  DominatorTreeBase DT;
};

using CFGDomTree = CFGDominatorTreeImpl</*IsPostDom*/ false>;
using CFGPostDomTree = CFGDominatorTreeImpl</*IsPostDom*/ true>;

template<> void CFGDominatorTreeImpl<true>::anchor();
template<> void CFGDominatorTreeImpl<false>::anchor();

} // end of namespace latino

namespace llvm {
namespace IDFCalculatorDetail {

/// Specialize ChildrenGetterTy to skip nullpointer successors.
template <bool IsPostDom>
struct ChildrenGetterTy<latino::CFGBlock, IsPostDom> {
  using NodeRef = typename GraphTraits<latino::CFGBlock>::NodeRef;
  using ChildrenTy = SmallVector<NodeRef, 8>;

  ChildrenTy get(const NodeRef &N) {
    using OrderedNodeTy =
        typename IDFCalculatorBase<latino::CFGBlock, IsPostDom>::OrderedNodeTy;

    auto Children = children<OrderedNodeTy>(N);
    ChildrenTy Ret{Children.begin(), Children.end()};
    Ret.erase(std::remove(Ret.begin(), Ret.end(), nullptr), Ret.end());
    return Ret;
  }
};

} // end of namespace IDFCalculatorDetail
} // end of namespace llvm

namespace latino {

class ControlDependencyCalculator : public ManagedAnalysis {
  using IDFCalculator = llvm::IDFCalculatorBase<CFGBlock, /*IsPostDom=*/true>;
  using CFGBlockVector = llvm::SmallVector<CFGBlock *, 4>;
  using CFGBlockSet = llvm::SmallPtrSet<CFGBlock *, 4>;

  CFGPostDomTree PostDomTree;
  IDFCalculator IDFCalc;

  llvm::DenseMap<CFGBlock *, CFGBlockVector> ControlDepenencyMap;

public:
  ControlDependencyCalculator(CFG *cfg)
    : PostDomTree(cfg), IDFCalc(PostDomTree.getBase()) {}

  const CFGPostDomTree &getCFGPostDomTree() const { return PostDomTree; }

  // Lazily retrieves the set of control dependencies to \p A.
  const CFGBlockVector &getControlDependencies(CFGBlock *A) {
    auto It = ControlDepenencyMap.find(A);
    if (It == ControlDepenencyMap.end()) {
      CFGBlockSet DefiningBlock = {A};
      IDFCalc.setDefiningBlocks(DefiningBlock);

      CFGBlockVector ControlDependencies;
      IDFCalc.calculate(ControlDependencies);

      It = ControlDepenencyMap.insert({A, ControlDependencies}).first;
    }

    assert(It != ControlDepenencyMap.end());
    return It->second;
  }

  /// Whether \p A is control dependent on \p B.
  bool isControlDependent(CFGBlock *A, CFGBlock *B) {
    return llvm::is_contained(getControlDependencies(A), B);
  }

  // Dumps immediate control dependencies for each block.
  LLVM_DUMP_METHOD void dump() {
    CFG *cfg = PostDomTree.getCFG();
    llvm::errs() << "Control dependencies (Node#,Dependency#):\n";
    for (CFGBlock *BB : *cfg) {

      assert(BB &&
             "LLVM's Dominator tree builder uses nullpointers to signify the "
             "virtual root!");

      for (CFGBlock *isControlDependency : getControlDependencies(BB))
        llvm::errs() << "(" << BB->getBlockID()
                     << ","
                     << isControlDependency->getBlockID()
                     << ")\n";
    }
  }
};

} // namespace latino

namespace llvm {

/// Clang's CFG contains nullpointers for unreachable succesors, e.g. when an
/// if statement's condition is always false, it's 'then' branch is represented
/// with a nullptr. This however will result in a nullpointer derefernece for
/// dominator tree calculation.
///
/// To circumvent this, let's just crudely specialize the children getters
/// used in LLVM's dominator tree builder.
namespace DomTreeBuilder {

using ClangCFGDomChildrenGetter =
SemiNCAInfo<latino::CFGDomTree::DominatorTreeBase>::ChildrenGetter<
                                                             /*Inverse=*/false>;

template <>
template <>
inline ClangCFGDomChildrenGetter::ResultTy ClangCFGDomChildrenGetter::Get(
    latino::CFGBlock *N, std::integral_constant<bool, /*Inverse=*/false>) {
  auto RChildren = reverse(children<NodePtr>(N));
  ResultTy Ret(RChildren.begin(), RChildren.end());
  Ret.erase(std::remove(Ret.begin(), Ret.end(), nullptr), Ret.end());
  return Ret;
}

using ClangCFGDomReverseChildrenGetter =
SemiNCAInfo<latino::CFGDomTree::DominatorTreeBase>::ChildrenGetter<
                                                              /*Inverse=*/true>;

template <>
template <>
inline ClangCFGDomReverseChildrenGetter::ResultTy
ClangCFGDomReverseChildrenGetter::Get(
    latino::CFGBlock *N, std::integral_constant<bool, /*Inverse=*/true>) {
  auto IChildren = inverse_children<NodePtr>(N);
  ResultTy Ret(IChildren.begin(), IChildren.end());
  Ret.erase(std::remove(Ret.begin(), Ret.end(), nullptr), Ret.end());
  return Ret;
}

using ClangCFGPostDomChildrenGetter =
SemiNCAInfo<latino::CFGPostDomTree::DominatorTreeBase>::ChildrenGetter<
                                                             /*Inverse=*/false>;

template <>
template <>
inline ClangCFGPostDomChildrenGetter::ResultTy
ClangCFGPostDomChildrenGetter::Get(
    latino::CFGBlock *N, std::integral_constant<bool, /*Inverse=*/false>) {
  auto RChildren = reverse(children<NodePtr>(N));
  ResultTy Ret(RChildren.begin(), RChildren.end());
  Ret.erase(std::remove(Ret.begin(), Ret.end(), nullptr), Ret.end());
  return Ret;
}

using ClangCFGPostDomReverseChildrenGetter =
SemiNCAInfo<latino::CFGPostDomTree::DominatorTreeBase>::ChildrenGetter<
                                                              /*Inverse=*/true>;

template <>
template <>
inline ClangCFGPostDomReverseChildrenGetter::ResultTy
ClangCFGPostDomReverseChildrenGetter::Get(
    latino::CFGBlock *N, std::integral_constant<bool, /*Inverse=*/true>) {
  auto IChildren = inverse_children<NodePtr>(N);
  ResultTy Ret(IChildren.begin(), IChildren.end());
  Ret.erase(std::remove(Ret.begin(), Ret.end(), nullptr), Ret.end());
  return Ret;
}

} // end of namespace DomTreeBuilder

//===-------------------------------------
/// DominatorTree GraphTraits specialization so the DominatorTree can be
/// iterable by generic graph iterators.
///
template <> struct GraphTraits<latino::DomTreeNode *> {
  using NodeRef = ::latino::DomTreeNode *;
  using ChildIteratorType = ::latino::DomTreeNode::const_iterator;

  static NodeRef getEntryNode(NodeRef N) { return N; }
  static ChildIteratorType child_begin(NodeRef N) { return N->begin(); }
  static ChildIteratorType child_end(NodeRef N) { return N->end(); }

  using nodes_iterator =
      llvm::pointer_iterator<df_iterator<::latino::DomTreeNode *>>;

  static nodes_iterator nodes_begin(::latino::DomTreeNode *N) {
    return nodes_iterator(df_begin(getEntryNode(N)));
  }

  static nodes_iterator nodes_end(::latino::DomTreeNode *N) {
    return nodes_iterator(df_end(getEntryNode(N)));
  }
};

template <> struct GraphTraits<latino::CFGDomTree *>
    : public GraphTraits<latino::DomTreeNode *> {
  static NodeRef getEntryNode(latino::CFGDomTree *DT) {
    return DT->getRootNode();
  }

  static nodes_iterator nodes_begin(latino::CFGDomTree *N) {
    return nodes_iterator(df_begin(getEntryNode(N)));
  }

  static nodes_iterator nodes_end(latino::CFGDomTree *N) {
    return nodes_iterator(df_end(getEntryNode(N)));
  }
};

} // namespace llvm

#endif // LLVM_LATINO_ANALYSIS_ANALYSES_DOMINATORS_H
