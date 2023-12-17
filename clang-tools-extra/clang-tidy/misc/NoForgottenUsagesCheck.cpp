//===--- NoForgottenUsagesCheck.cpp - clang-tidy --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "NoForgottenUsagesCheck.h"
#include "../utils/OptionsUtils.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "llvm/ADT/STLExtras.h"
#include <string>

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace misc {

NoForgottenUsagesCheck::NoForgottenUsagesCheck(StringRef Name,
                                               ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context),
      Classes(utils::options::parseStringList(Options.get("Classes", ""))) {}

void NoForgottenUsagesCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(varDecl(hasAutomaticStorageDuration()).bind("var"), this);
}

void NoForgottenUsagesCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *MatchedDecl = Result.Nodes.getNodeAs<VarDecl>("var");
  // Non local var decls or referenced are skipped
  if (!MatchedDecl->isLocalVarDecl() || MatchedDecl->isReferenced()) {
    return;
  }
  // Otherwise check our types.
  const Type *DesurgaredType =
      MatchedDecl->getType()->getUnqualifiedDesugaredType();
  const CXXRecordDecl *RecordType = DesurgaredType->getAsCXXRecordDecl();
  if (RecordType == nullptr) {
    return;
  }
  // This is the qualified name of the class, for templates this ignores
  // template parameters.
  const std::string &QualifiedName = RecordType->getQualifiedNameAsString();
  bool Matches =
      llvm::any_of(Classes, [&QualifiedName](const StringRef &Class) {
        return Class == QualifiedName;
      });
  if (!Matches) {
    return;
  }
  // Print out our warning
  diag(MatchedDecl->getLocation(), "variable %0 with type %1 must be used")
      << MatchedDecl << RecordType;
}

void NoForgottenUsagesCheck::storeOptions(ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "Classes", utils::options::serializeStringList(Classes));
}

} // namespace misc
} // namespace tidy
} // namespace clang
