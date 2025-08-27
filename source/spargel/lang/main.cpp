#include "spargel/lang/lexer.h"
#include "spargel/lang/parser.h"

namespace spargel::lang {
    namespace {
        void DriverMain() {
            Cursor c{R"(
                open cpp_interop
                @cpp.export
                func main(args: list[string_view]) {
                    return 0
                }
            )"};
            Parser p{c};
            p.parse();
            // while (true) {
            //     auto tok = c.nextToken();
            //     tok.dump();
            //     if (tok.kind == TokenKind::end_of_file) {
            //         break;
            //     }
            // }
        }
    }  // namespace
}  // namespace spargel::lang

int main() {
    spargel::lang::DriverMain();
    return 0;
}
