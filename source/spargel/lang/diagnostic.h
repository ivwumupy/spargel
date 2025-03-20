#pragma once

namespace spargel::lang {

    enum class DiagnosticLevel {
        note,
        warning,
        error,
    };

    class DiagnosticArgument {
    };

    class Diagnostic {
    };

    class DiagnosticConsumer {
    public:
        virtual ~DiagnosticConsumer() = default;
    };

}
