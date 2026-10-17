"""Tests for protect_student_paths.py. Run: python3 -m unittest discover -s .claude/hooks"""

from __future__ import annotations

import json
import os
import subprocess
import sys
import unittest
from pathlib import Path

HOOK = Path(__file__).with_name("protect_student_paths.py")
PROJECT_DIR = HOOK.parents[2]


def run_hook(tool_name: str, file_path: str, author_mode: bool = False) -> dict[str, object] | None:
    event = {"tool_name": tool_name, "tool_input": {"file_path": file_path}, "cwd": str(PROJECT_DIR)}
    env = {key: value for key, value in os.environ.items() if key != "LEARN_AUTHOR_MODE"}
    env["CLAUDE_PROJECT_DIR"] = str(PROJECT_DIR)
    if author_mode:
        env["LEARN_AUTHOR_MODE"] = "1"
    result = subprocess.run(
        [sys.executable, str(HOOK)], input=json.dumps(event), capture_output=True, text=True, env=env, check=True
    )
    return json.loads(result.stdout) if result.stdout.strip() else None


def is_denied(response: dict[str, object] | None) -> bool:
    if response is None:
        return False
    output = response["hookSpecificOutput"]
    assert isinstance(output, dict)
    return output["permissionDecision"] == "deny"


class ProtectStudentPathsTest(unittest.TestCase):
    def test_denies_student_source(self) -> None:
        path = PROJECT_DIR / "exercises/01-sensor-sync/src/odometryBuffer.cpp"
        response = run_hook("Edit", str(path))
        self.assertTrue(is_denied(response))
        assert response is not None
        self.assertIn("teacher", json.dumps(response))

    def test_denies_student_header_and_new_files(self) -> None:
        self.assertTrue(is_denied(run_hook("Write", str(PROJECT_DIR / "exercises/01-sensor-sync/include/x/new.h"))))
        self.assertTrue(is_denied(run_hook("Write", "exercises/02-pipeline/src/main.cpp")))

    def test_denies_student_tests(self) -> None:
        path = PROJECT_DIR / "exercises/01-sensor-sync/tests/student_odometryBufferTest.cpp"
        self.assertTrue(is_denied(run_hook("Edit", str(path))))

    def test_denies_path_traversal_into_student_code(self) -> None:
        path = PROJECT_DIR / "docs/../exercises/01-sensor-sync/src/odometryBuffer.cpp"
        self.assertTrue(is_denied(run_hook("Edit", str(path))))

    def test_allows_provided_tests_docs_and_reviews(self) -> None:
        for relative in (
            "exercises/01-sensor-sync/tests/odometryBufferSession1Test.cpp",
            "exercises/01-sensor-sync/reviews/session-1.md",
            "exercises/01-sensor-sync/README.md",
            "journal/progress.md",
            "docs/engineering-practices.md",
        ):
            with self.subTest(relative=relative):
                self.assertFalse(is_denied(run_hook("Edit", str(PROJECT_DIR / relative))))

    def test_allows_paths_outside_the_repository(self) -> None:
        self.assertFalse(is_denied(run_hook("Write", "/tmp/exercises/01-sensor-sync/src/x.cpp")))

    def test_author_mode_allows_student_paths(self) -> None:
        path = PROJECT_DIR / "exercises/01-sensor-sync/src/odometryBuffer.cpp"
        self.assertFalse(is_denied(run_hook("Edit", str(path), author_mode=True)))


if __name__ == "__main__":
    unittest.main()
