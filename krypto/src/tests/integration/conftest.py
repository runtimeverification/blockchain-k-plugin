from __future__ import annotations

import os
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

    return Path(PROJECT_DIR)


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
            ],
        }
        args = {**default_args, **kwargs}
        return kompile(**args)

    return _krypto_kompile


@pytest.fixture(scope='session')
def definition_dir(krypto_kompile: Callable[..., Path]) -> Path:
    definition = """
        requires "plugin/krypto.md"

        module TEST
            imports BOOL
            imports BYTES
            imports INT
            imports K-EQUAL
            imports STRING
            imports KRYPTO
            syntax Pgm ::= Bool | Bytes | String | G1Point | G2Point
            configuration <k> $PGM:Pgm </k>

            // Passes the same Bytes term to Blake2Compress and to the check that
            // follows it, so that a hook mutating its argument in place is observable.
            syntax Bool ::= blake2NoAlias    ( Bytes )           [function]
                          | blake2NoAliasAux ( String , Bytes )  [function]
            rule blake2NoAlias(B) => blake2NoAliasAux(Blake2Compress(B), B)
            rule blake2NoAliasAux(OUT, B) => lengthString(OUT) ==Int 128 andBool B ==K padRightBytes(b"", 213, 0)
        endmodule
    """
    return krypto_kompile(definition=definition, main_module='TEST', syntax_module='TEST')
