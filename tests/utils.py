import subprocess
from pathlib import Path
from typing import Final

PROJECT_DIR: Final = Path(__file__).parents[1]
SOURCE_DIR : Final = PROJECT_DIR / 'plugin-c'
BUILD_DIR: Final = PROJECT_DIR / 'build'


def hex2bytes(s: str) -> str:
    byte_str = ''.join(fr'\x{byte:02x}' for byte in bytes.fromhex(s))
    return f'b"{byte_str}"'


def run(definition_dir: Path, pgm: str) -> str:
    args = ['krun', '--definition', str(definition_dir), f'-cPGM={pgm}']
    proc_res = subprocess.run(args, stdout=subprocess.PIPE, check=True, text=True)
    return proc_res.stdout.rstrip()
