import subprocess
from pathlib import Path
from typing import Final

TEST_DIR: Final = Path(__file__).parent
TEST_DATA_DIR: Final = TEST_DIR / 'test-data'
PROJECT_DIR: Final = TEST_DIR.parents[3]
SOURCE_DIR: Final = PROJECT_DIR / 'plugin-c'
BUILD_DIR: Final = PROJECT_DIR / 'build'


def hex2bytes(s: str) -> str:
    byte_str = ''.join(fr'\x{byte:02x}' for byte in bytes.fromhex(s))
    return f'b"{byte_str}"'  # noqa: B907


def kompile(
    definition: str,
    *,
    output_dir: Path,
    main_module: str,
    syntax_module: str,
) -> None:
    main_file = output_dir / 'definition.k'
    main_file.write_text(definition)

    ccopts = [
        '-std=c++17',
        '-lssl',
        '-lcrypto',
        '-lsecp256k1',
        '-lprocps',
        str(BUILD_DIR / 'libff/lib/libff.a'),
        str(BUILD_DIR / 'libcryptopp/lib/libcryptopp.a'),
        str(BUILD_DIR / 'blake2/lib/blake2.a'),
        str(SOURCE_DIR / 'crypto.cpp'),
        str(SOURCE_DIR / 'hash_ext.cpp'),
        str(SOURCE_DIR / 'plugin_util.cpp'),
        f"-I{BUILD_DIR / 'libff/include'}",
        f"-I{BUILD_DIR / 'libcryptopp/include'}",
    ]

    args = [
        'kompile',
        str(main_file),
        '--output-definition',
        str(output_dir),
        '--main-module',
        main_module,
        '--syntax-module',
        syntax_module,
        '-I',
        str(PROJECT_DIR),
        '--md-selector',
        'k | libcrypto-extra',
        '--hook-namespaces',
        'KRYPTO',
        '--warnings-to-errors',
    ] + [arg for ccopt in ccopts for arg in ['-ccopt', ccopt]]

    subprocess.run(args, check=True, text=True)


def run(definition_dir: Path, pgm: str) -> str:
    args = ['krun', '--definition', str(definition_dir), f'-cPGM={pgm}']
    proc_res = subprocess.run(args, stdout=subprocess.PIPE, check=True, text=True)
    return proc_res.stdout.rstrip()
