from __future__ import annotations

class IndentContext:
    def __init__(self, tracker: IndentTracker):
        self.tracker = tracker
    def __enter__(self):
        self.tracker.indent_level += 1
    def __exit__(self, exc_type, exc_value, traceback):
        self.tracker.indent_level -= 1

class IndentTracker:
    def __init__(self):
        self.indent_level = 0
    def indent(self) -> IndentContext:
        return IndentContext(self)
