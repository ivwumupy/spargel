#include <memory>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/DeclGroup.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

namespace {
    llvm::cl::OptionCategory MyToolCategory("spargel-reflect options");

    class PrintConsumer : public clang::ASTConsumer {
    public:
        PrintConsumer(clang::CompilerInstance &Instance) {}

        bool HandleTopLevelDecl(clang::DeclGroupRef DG) override {
            for (auto const* D : DG) {
                if (auto const* ND = dyn_cast<clang::NamedDecl>(D))
                llvm::errs() << "top-level-decl: \"" << ND->getNameAsString() << "\"\n";
            }
            return true;
        }
    };

    class PrintAction : public clang::ASTFrontendAction {
    public:
        // Required.
        std::unique_ptr<clang::ASTConsumer>
            CreateASTConsumer(clang::CompilerInstance &CI,
                              [[maybe_unused]] llvm::StringRef InFile) override {
            return std::make_unique<PrintConsumer>(CI);
        }
    };
} // namespace

int main(int argc, char const* argv[]) {
    auto ExpectedParser = clang::tooling::CommonOptionsParser::create(argc, argv, MyToolCategory);
    if (!ExpectedParser) {
        // Fail gracefully for unsupported options.
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    auto& OptionsParser = ExpectedParser.get();
    clang::tooling::ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    return Tool.run(clang::tooling::newFrontendActionFactory<PrintAction>().get());
}
