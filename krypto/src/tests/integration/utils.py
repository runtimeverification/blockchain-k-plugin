from __future__ import annotations

from pathlib import Path
from typing import TYPE_CHECKING

from pyk.ktool.krun import _krun

if TYPE_CHECKING:
    from typing import Final


TEST_DIR: Final = Path(__file__).parent
TEST_DATA_DIR: Final = TEST_DIR / 'test-data'
PROJECT_DIR: Final = TEST_DIR.parents[3]
BUILD_DIR: Final = PROJECT_DIR / 'build'


def hex2bytes(s: str) -> str:
    byte_str = ''.join(fr'\x{byte:02x}' for byte in bytes.fromhex(s))
    return f'b"{byte_str}"'  # noqa: B907


def run(definition_dir: Path, pgm: str) -> str:
    proc_res = _krun(definition_dir=definition_dir, cmap={'PGM': pgm})
    return proc_res.stdout.rstrip()
