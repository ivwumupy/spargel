#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclGroup.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

#include <memory>

namespace {
    llvm::cl::OptionCategory MyToolCategory("spargel-reflect options");

    class PrintConsumer : public clang::ASTConsumer, public clang::RecursiveASTVisitor<PrintConsumer> {
    public:
        PrintConsumer([[maybe_unused]] clang::CompilerInstance& Instance) {}

        bool HandleTopLevelDecl(clang::DeclGroupRef DG) override {
            for (auto const* D : DG) {
                if (auto const* ND = dyn_cast<clang::NamedDecl>(D))
                    llvm::errs() << "top-level-decl: \"" << ND->getNameAsString() << "\"\n";
            }
            return true;
        }

        void HandleTranslationUnit(clang::ASTContext& ctx) override {
            llvm::errs() << "processing translation unit" << "\n";
            TraverseDecl(ctx.getTranslationUnitDecl());
        }

        bool VisitFunctionDecl(clang::FunctionDecl const* decl) {
            llvm::errs() << "visiting function decl\n";
            decl->dump();
            for (auto* attr : decl->specific_attrs<clang::AnnotateAttr>()) {
                llvm::errs() << attr->getSpelling() << "\n";
            }
            return true;
        }
    };

    class PrintAction : public clang::ASTFrontendAction {
    public:
        // Required.
        std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
            clang::CompilerInstance& CI, [[maybe_unused]] llvm::StringRef InFile) override {
            return std::make_unique<PrintConsumer>(CI);
        }
    };
}  // namespace

int main(int argc, char const* argv[]) {
    auto ExpectedParser = clang::tooling::CommonOptionsParser::create(argc, argv, MyToolCategory);
    if (!ExpectedParser) {
        // Fail gracefully for unsupported options.
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    auto& OptionsParser = ExpectedParser.get();
    clang::tooling::ClangTool Tool(OptionsParser.getCompilations(),
                                   OptionsParser.getSourcePathList());
    return Tool.run(clang::tooling::newFrontendActionFactory<PrintAction>().get());
}
