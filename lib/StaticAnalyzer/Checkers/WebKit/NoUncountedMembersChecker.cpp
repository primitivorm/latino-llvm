//=======- NoUncountedMembersChecker.cpp -------------------------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ASTUtils.h"
#include "DiagOutputUtils.h"
#include "PtrTypesSemantics.h"
#include "latino/AST/CXXInheritance.h"
#include "latino/AST/Decl.h"
#include "latino/AST/DeclCXX.h"
#include "latino/AST/RecursiveASTVisitor.h"
#include "latino/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "latino/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "latino/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "latino/StaticAnalyzer/Core/Checker.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/Support/Casting.h"

using namespace latino;
using namespace ento;

namespace {

class NoUncountedMemberChecker
    : public Checker<check::ASTDecl<TranslationUnitDecl>> {
private:
  BugType Bug;
  mutable BugReporter *BR;

public:
  NoUncountedMemberChecker()
      : Bug(this,
            "Member variable is a raw-poiner/reference to reference-countable "
            "type",
            "WebKit coding guidelines") {}

  void checkASTDecl(const TranslationUnitDecl *TUD, AnalysisManager &MGR,
                    BugReporter &BRArg) const {
    BR = &BRArg;

    // The calls to checkAST* from AnalysisConsumer don't
    // visit template instantiations or lambda classes. We
    // want to visit those, so we make our own RecursiveASTVisitor.
    struct LocalVisitor : public RecursiveASTVisitor<LocalVisitor> {
      const NoUncountedMemberChecker *Checker;
      explicit LocalVisitor(const NoUncountedMemberChecker *Checker)
          : Checker(Checker) {
        assert(Checker);
      }

      bool shouldVisitTemplateInstantiations() const { return true; }
      bool shouldVisitImplicitCode() const { return false; }

      bool VisitRecordDecl(const RecordDecl *RD) {
        Checker->visitRecordDecl(RD);
        return true;
      }
    };

    LocalVisitor visitor(this);
    visitor.TraverseDecl(const_cast<TranslationUnitDecl *>(TUD));
  }

  void visitRecordDecl(const RecordDecl *RD) const {
    if (shouldSkipDecl(RD))
      return;

    for (auto Member : RD->fields()) {
      const Type *MemberType = Member->getType().getTypePtrOrNull();
      if (!MemberType)
        continue;

      if (auto *MemberCXXRD = MemberType->getPointeeCXXRecordDecl()) {
        // If we don't see the definition we just don't know.
        if (MemberCXXRD->hasDefinition() && isRefCountable(MemberCXXRD))
          reportBug(Member, MemberType, MemberCXXRD, RD);
      }
    }
  }

  bool shouldSkipDecl(const RecordDecl *RD) const {
    if (!RD->isThisDeclarationADefinition())
      return true;

    if (RD->isImplicit())
      return true;

    if (RD->isLambda())
      return true;

    // If the construct doesn't have a source file, then it's not something
    // we want to diagnose.
    const auto RDLocation = RD->getLocation();
    if (!RDLocation.isValid())
      return true;

    const auto Kind = RD->getTagKind();
    // FIMXE: Should we check union members too?
    if (Kind != TTK_Struct && Kind != TTK_Class)
      return true;

    // Ignore CXXRecords that come from system headers.
    if (BR->getSourceManager().isInSystemHeader(RDLocation))
      return true;

    // Ref-counted smartpointers actually have raw-pointer to uncounted type as
    // a member but we trust them to handle it correctly.
    auto CXXRD = llvm::dyn_cast_or_null<CXXRecordDecl>(RD);
    if (CXXRD)
      return isRefCounted(CXXRD);

    return false;
  }

  void reportBug(const FieldDecl *Member, const Type *MemberType,
                 const CXXRecordDecl *MemberCXXRD,
                 const RecordDecl *ClassCXXRD) const {
    assert(Member);
    assert(MemberType);
    assert(MemberCXXRD);

    SmallString<100> Buf;
    llvm::raw_svector_ostream Os(Buf);

    Os << "Member variable ";
    printQuotedName(Os, Member);
    Os << " in ";
    printQuotedQualifiedName(Os, ClassCXXRD);
    Os << " is a "
       << (isa<PointerType>(MemberType) ? "raw pointer" : "reference")
       << " to ref-countable type ";
    printQuotedQualifiedName(Os, MemberCXXRD);
    Os << "; member variables must be ref-counted.";

    PathDiagnosticLocation BSLoc(Member->getSourceRange().getBegin(),
                                 BR->getSourceManager());
    auto Report = std::make_unique<BasicBugReport>(Bug, Os.str(), BSLoc);
    Report->addRange(Member->getSourceRange());
    BR->emitReport(std::move(Report));
  }
};
} // namespace

void ento::registerNoUncountedMemberChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<NoUncountedMemberChecker>();
}

bool ento::shouldRegisterNoUncountedMemberChecker(
    const CheckerManager &Mgr) {
  return true;
}
