/******************************************************************************
 * Copyright (c) 2017 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Philipp Schubert and others
 *****************************************************************************/

#include <phasar/PhasarClang/RandomChangeVisitor.h>
using namespace psr;

namespace psr {

RandomChangeVisitor::RandomChangeVisitor(clang::Rewriter &R) : RW(R) {}

bool RandomChangeVisitor::VisitStmt(clang::Stmt *S) {
  // Only care about If statements.
  if (clang::isa<clang::IfStmt>(S)) {
    clang::IfStmt *IfStatement = clang::cast<clang::IfStmt>(S);
    clang::Stmt *Then = IfStatement->getThen();
    RW.InsertText(Then->getLocStart(), "// the 'if' part\n", true, true);
    clang::Stmt *Else = IfStatement->getElse();
    if (Else)
      RW.InsertText(Else->getLocStart(), "// the 'else' part\n", true, true);
  }
  return true;
}

bool RandomChangeVisitor::VisitFunctionDecl(clang::FunctionDecl *F) {
  // Only function definitions (with bodies), not declarations.
  if (F->hasBody()) {
    clang::Stmt *FuncBody = F->getBody();
    // Type name as string
    clang::QualType QT = F->getReturnType();
    std::string TypeStr = QT.getAsString();
    // Function name
    clang::DeclarationName DeclName = F->getNameInfo().getName();
    std::string FuncName = DeclName.getAsString();
    // Add comment before
    std::stringstream SSBefore;
    SSBefore << "// Begin function " << FuncName << " returning " << TypeStr
             << "\n";
    clang::SourceLocation ST = F->getSourceRange().getBegin();
    RW.InsertText(ST, SSBefore.str(), true, true);
    // And after
    std::stringstream SSAfter;
    SSAfter << "\n// End function " << FuncName;
    ST = FuncBody->getLocEnd().getLocWithOffset(1);
    RW.InsertText(ST, SSAfter.str(), true, true);
  }

  return true;
}

bool RandomChangeVisitor::VisitTypeDecl(clang::TypeDecl *T) {
  llvm::outs() << "Found TypeDecl:\n";
  T->dump();
  return true;
}

bool RandomChangeVisitor::VisitVarDecl(clang::VarDecl *V) {
  if (V->isLocalVarDecl()) {
    llvm::outs() << "Found local VarDecl: " << V->getName() << '\n';
    if (V->getName() == "x") {
      llvm::outs() << "The DeclName:\n";
      V->getDeclName().dump();
      // Use DeclRefExpr to find uses of the VarDecl 'V'!
      // --> use the VisitStmt function!

      auto Start = V->getLocStart();
      // setting it to Start enables the use of auto for different input
      // decls we use
      auto End = Start;
      End = Start.getLocWithOffset(V->getDeclName().getAsString().size());
      // gets the range of the total parameter - including type and argument
      auto range = clang::CharSourceRange::getTokenRange(Start, End);
      // get the stringPtr from the range and convert to std::string
      std::string s = std::string(clang::Lexer::getSourceText(
          range, RW.getSourceMgr(), RW.getLangOpts()));
      // offset gives us the length
      int offset = clang::Lexer::MeasureTokenLength(End, RW.getSourceMgr(),
                                                    RW.getLangOpts());
      // replace the text with the text sent
      RW.ReplaceText(Start, s.length() - offset, "newX");
    }
  }
  return true;
}
} // namespace psr
