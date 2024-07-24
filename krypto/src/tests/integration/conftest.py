from __future__ import annotations

import os
import sys
from pathlib import Path
from typing import TYPE_CHECKING

import pytest

from .utils import BUILD_DIR, PROJECT_DIR

if TYPE_CHECKING:
    from collections.abc import Callable
    from typing import Any, Final

    from pyk.testing import Kompiler

K_PLUGIN_ROOT: Final = 'K_PLUGIN_ROOT'


def _source_dir() -> Path:
    if K_PLUGIN_ROOT in os.environ:
        return Path(os.environ[K_PLUGIN_ROOT]) / 'krypto/src'

    return Path(PROJECT_DIR) / 'plugin'


def _library_path() -> Path:
    return Path(os.environ.get(K_PLUGIN_ROOT, BUILD_DIR)) / 'krypto/lib/krypto.a'


@pytest.fixture(scope='session')
def krypto_kompile(kompile: Kompiler) -> Callable[..., Path]:
    def _krypto_kompile(**kwargs: Any) -> Path:
        default_args = {
            'include_dirs': [_source_dir()],
            'md_selector': 'k | libcrypto-extra',
            'hook_namespaces': ['KRYPTO'],
            'ccopts': [
                '-std=c++17',
                '-lssl',
                '-lsecp256k1',
                '-lcrypto',
                str(_library_path()),
            ]
            + (['-lprocps'] if sys.platform == 'linux' else []),
        }
        args = {**default_args, **kwargs}
        return kompile(**args)

    return _krypto_kompile
