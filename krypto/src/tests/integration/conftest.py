from __future__ import annotations

from typing import TYPE_CHECKING

import pytest

from .utils import BUILD_DIR, PROJECT_DIR

if TYPE_CHECKING:
    from collections.abc import Callable
    from pathlib import Path
    from typing import Any

    from pyk.testing import Kompiler


@pytest.fixture(scope='session')
def krypto_kompile(kompile: Kompiler) -> Callable[..., Path]:
    def _krypto_kompile(**kwargs: Any) -> Path:
        default_args = {
            'include_dirs': [PROJECT_DIR],
            'md_selector': 'k | libcrypto-extra',
            'hook_namespaces': ['KRYPTO'],
            'ccopts': [
                '-std=c++17',
                '-lssl',
                '-lcrypto',
                '-lsecp256k1',
                '-lprocps',
                str(BUILD_DIR / 'libff/lib/libff.a'),
                str(BUILD_DIR / 'libcryptopp/lib/libcryptopp.a'),
                str(BUILD_DIR / 'blake2/lib/blake2.a'),
                str(BUILD_DIR / 'plugin/lib/plugin.a'),
            ],
        }
        args = {**default_args, **kwargs}
        return kompile(**args)

    return _krypto_kompile
