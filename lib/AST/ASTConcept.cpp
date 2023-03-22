//===--- ASTConcept.cpp - Concepts Related AST Data Structures --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file defines AST data structures related to concepts.
///
//===----------------------------------------------------------------------===//

#include "latino/AST/ASTConcept.h"
#include "latino/AST/ASTContext.h"
#include "latino/AST/Decl.h"
#include "latino/AST/TemplateBase.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/FoldingSet.h"
using namespace latino;

ASTConstraintSatisfaction::ASTConstraintSatisfaction(const ASTContext &C,
    const ConstraintSatisfaction &Satisfaction):
    NumRecords{Satisfaction.Details.size()},
    IsSatisfied{Satisfaction.IsSatisfied} {
  for (unsigned I = 0; I < NumRecords; ++I) {
    auto &Detail = Satisfaction.Details[I];
    if (Detail.second.is<Expr *>())
      new (getTrailingObjects<UnsatisfiedConstraintRecord>() + I)
         UnsatisfiedConstraintRecord{Detail.first,
                                     UnsatisfiedConstraintRecord::second_type(
                                         Detail.second.get<Expr *>())};
    else {
      auto &SubstitutionDiagnostic =
          *Detail.second.get<std::pair<SourceLocation, StringRef> *>();
      unsigned MessageSize = SubstitutionDiagnostic.second.size();
      char *Mem = new (C) char[MessageSize];
      memcpy(Mem, SubstitutionDiagnostic.second.data(), MessageSize);
      auto *NewSubstDiag = new (C) std::pair<SourceLocation, StringRef>(
          SubstitutionDiagnostic.first, StringRef(Mem, MessageSize));
      new (getTrailingObjects<UnsatisfiedConstraintRecord>() + I)
         UnsatisfiedConstraintRecord{Detail.first,
                                     UnsatisfiedConstraintRecord::second_type(
                                         NewSubstDiag)};
    }
  }
}


ASTConstraintSatisfaction *
ASTConstraintSatisfaction::Create(const ASTContext &C,
                                  const ConstraintSatisfaction &Satisfaction) {
  std::size_t size =
      totalSizeToAlloc<UnsatisfiedConstraintRecord>(
          Satisfaction.Details.size());
  void *Mem = C.Allocate(size, alignof(ASTConstraintSatisfaction));
  return new (Mem) ASTConstraintSatisfaction(C, Satisfaction);
}

void ConstraintSatisfaction::Profile(
    llvm::FoldingSetNodeID &ID, const ASTContext &C,
    const NamedDecl *ConstraintOwner, ArrayRef<TemplateArgument> TemplateArgs) {
  ID.AddPointer(ConstraintOwner);
  ID.AddInteger(TemplateArgs.size());
  for (auto &Arg : TemplateArgs)
    Arg.Profile(ID, C);
}
