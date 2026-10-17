#!/usr/bin/env python3
"""PreToolUse hook: deny assistant edits to the student's exercise work.

Protected (relative to the repository root):
    exercises/<name>/src/**
    exercises/<name>/include/**
    exercises/<name>/tests/student_*

Set LEARN_AUTHOR_MODE=1 in the environment that launches Claude Code to author exercises.
"""

from __future__ import annotations

import json
import os
import re
import sys
from pathlib import Path

PROTECTED_PATTERN = re.compile(r"^exercises/[^/]+/(src/|include/|tests/student_)")

DENY_REASON = (
    "This is the student's own exercise code, and the learning contract (CLAUDE.md) says the student writes it. "
    "Help with the teacher skill instead (graded hints, no solution unless explicitly asked and confirmed). "
    "To author exercise material, relaunch with LEARN_AUTHOR_MODE=1."
)


def is_protected(file_path: str, project_dir: Path) -> bool:
    """Whether `file_path` (absolute or relative to `project_dir`) is student work."""
    path = Path(file_path)
    if not path.is_absolute():
        path = project_dir / path
    try:
        relative = path.resolve().relative_to(project_dir.resolve())
    except ValueError:
        return False
    return PROTECTED_PATTERN.match(relative.as_posix()) is not None


def decide(event: dict[str, object], project_dir: Path, author_mode: bool) -> dict[str, object] | None:
    """The hook response for a PreToolUse event, or None to let the call proceed."""
    if author_mode:
        return None
    tool_input = event.get("tool_input")
    if not isinstance(tool_input, dict):
        return None
    file_path = tool_input.get("file_path") or tool_input.get("notebook_path")
    if not isinstance(file_path, str) or not is_protected(file_path, project_dir):
        return None
    return {
        "hookSpecificOutput": {
            "hookEventName": "PreToolUse",
            "permissionDecision": "deny",
            "permissionDecisionReason": DENY_REASON,
        }
    }


def main() -> int:
    event = json.load(sys.stdin)
    project_dir = Path(os.environ.get("CLAUDE_PROJECT_DIR") or str(event.get("cwd") or Path.cwd()))
    author_mode = os.environ.get("LEARN_AUTHOR_MODE") == "1"
    response = decide(event, project_dir, author_mode)
    if response is not None:
        json.dump(response, sys.stdout)
    return 0


if __name__ == "__main__":
    sys.exit(main())
