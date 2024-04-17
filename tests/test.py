import subprocess
from pathlib import Path
from typing import Final

import pytest
from pytest import TempPathFactory


PROJECT_DIR: Final = Path(__file__).parents[1]
SOURCE_DIR : Final = PROJECT_DIR / 'plugin-c'
BUILD_DIR: Final = PROJECT_DIR / 'build'


def hex2bytes(s: str) -> str:
    byte_str = ''.join(fr'\x{byte:02x}' for byte in bytes.fromhex(s))
    return f'b"{byte_str}"'


x01_32B: Final = hex2bytes(31 * '00' + '01')


@pytest.fixture(scope='session')
def definition_dir(tmp_path_factory: TempPathFactory) -> Path:
    definition = """
        requires "plugin/krypto.md"

        module TEST
            imports KRYPTO
            syntax Pgm ::= Bool | Bytes | String | G1Point
            configuration <k> $PGM:Pgm </k>
        endmodule
    """

    output_dir = tmp_path_factory.mktemp('kompiled')
    main_file = output_dir / 'test.k'
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
        '--syntax-module',
        'TEST',
        '-I',
        str(PROJECT_DIR),
        '--md-selector',
        'k | libcrypto-extra',
        '--hook-namespaces',
        'KRYPTO',
        '--warnings-to-errors',
    ] + [arg for ccopt in ccopts for arg in ['-ccopt', ccopt]]

    subprocess.run(args, check=True, text=True)

    return output_dir


def run(definition_dir: Path, pgm: str) -> str:
    args = ['krun', '--definition', str(definition_dir), f'-cPGM={pgm}']
    proc_res = subprocess.run(args, stdout=subprocess.PIPE, check=True, text=True)
    return proc_res.stdout.rstrip()


HOOK_TEST_DATA: Final = (
    (
        'Keccak256',
        'Keccak256(b"foo")',
        '"41b1a0649752af1b28b3dc29a1556eee781e4a4c3a1f7f53f90fa834de098c4d"',
    ),
    (
        'Sha256',
        'Sha256(b"foo")',
        '"2c26b46b68ffc68ff99b453c1d30413413422d706483bfa0f98a5e886266e7ae"',
    ),
    (
        'Sha512',
        'Sha512(b"foo")',
        '"f7fbba6e0636f890e56fbbf3283e524c6fa3204ae298382d624741d0dc6638326e282c41be5e4254d8820772c5518a2c5a8c0c7f7eda19594a7eb539453e1ed7"',
    ),
    (
        'Sha512_256',
        'Sha512_256(b"foo")',
        '"d58042e6aa5a335e03ad576c6a9e43b41591bfd2077f72dec9df7930e492055d"',
    ),
    (
        'Sha3_256',
        'Sha3_256(b"foo")',
        '"76d3bc41c9f588f7fcd0d5bf4718f8f84b1c41b20882703100b9eb9413807c01"',
    ),
    (
        'RipEmd160',
        'RipEmd160(b"foo")',
        '"42cfa211018ea492fdee45ac637b7972a0ad6873"',
    ),
    (
        'Blake2Compress',
        f"Blake2Compress({hex2bytes(213 * '00')})",
        '"08c9bcf367e6096a3ba7ca8485ae67bb2bf894fe72f36e3cf1361d5f3af54fa5d182e6ad7f520e511f6c3e2b8c68059b6bbd41fbabd9831f79217e1319cde05b"',
    ),
    (
        'Keccak256raw',
        'Keccak256raw(b"foo")',
        r'b"A\xb1\xa0d\x97R\xaf\x1b(\xb3\xdc)\xa1Un\xeex\x1eJL:\x1f\x7fS\xf9\x0f\xa84\xde\t\x8cM"',
    ),
    (
        'Sha256raw',
        'Sha256raw(b"foo")',
        r'b",&\xb4kh\xff\xc6\x8f\xf9\x9bE<\x1d0A4\x13B-pd\x83\xbf\xa0\xf9\x8a^\x88bf\xe7\xae"',
    ),
    (
        'Sha512raw',
        'Sha512raw(b"foo")',
        r'b"\xf7\xfb\xban\x066\xf8\x90\xe5o\xbb\xf3(>RLo\xa3 J\xe2\x988-bGA\xd0\xdcf82n(,A\xbe^BT\xd8\x82\x07r\xc5Q\x8a,Z\x8c\f\x7f~\xda\x19YJ~\xb59E>\x1e\xd7"',
    ),
    (
        'Sha512_256raw',
        'Sha512_256raw(b"foo")',
        r'b"\xd5\x80B\xe6\xaaZ3^\x03\xadWlj\x9eC\xb4\x15\x91\xbf\xd2\x07\x7fr\xde\xc9\xdfy0\xe4\x92\x05]"',
    ),
    (
        'Sha3_256raw',
        'Sha3_256raw(b"foo")',
        r'b"v\xd3\xbcA\xc9\xf5\x88\xf7\xfc\xd0\xd5\xbfG\x18\xf8\xf8K\x1cA\xb2\x08\x82p1\x00\xb9\xeb\x94\x13\x80|\x01"',
    ),
    (
        'RipEmd160raw',
        'RipEmd160raw(b"foo")',
        r'b"B\xcf\xa2\x11\x01\x8e\xa4\x92\xfd\xeeE\xacc{yr\xa0\xadhs"',
    ),
    (
        'Blake2b256',
        'Blake2b256(b"foo")',
        '"b8fe9f7f6255a6fa08f668ab632a8d081ad87983c77cd274e48ce450f0b349fd"',
    ),
    (
        'Blake2b256raw',
        'Blake2b256raw(b"foo")',
        r'b"\xb8\xfe\x9f\x7fbU\xa6\xfa\x08\xf6h\xabc*\x8d\x08\x1a\xd8y\x83\xc7|\xd2t\xe4\x8c\xe4P\xf0\xb3I\xfd"',
    ),
    (
        'ED25519VerifyMessage-valid',
        f"""
            ED25519VerifyMessage(
                {hex2bytes('0B8E3CE25D39580B7D8F54E1A92AE7626AD727DEFFA280950E9FD1443BE5EC54')},
                b"foo",
                {hex2bytes('B3C0684B979C9B87F273A2DDFD16011737F6FAEA8A33B272FD2306C99CB81DF14C23F197022A063A9A697FF870E7BC9266335AAE8B1E782DCD2A076B81247308')}
            )
        """,
        'true',
    ),
    (
        'ED25519VerifyMessage-invalid',
        f"""
            ED25519VerifyMessage(
                {hex2bytes('0B8E3CE25D39580B7D8F54E1A92AE7626AD727DEFFA280950E9FD1443BE5EC54')},
                b"bar",
                {hex2bytes('B3C0684B979C9B87F273A2DDFD16011737F6FAEA8A33B272FD2306C99CB81DF14C23F197022A063A9A697FF870E7BC9266335AAE8B1E782DCD2A076B81247308')}
            )
        """,
        'false',
    ),
    (
        'ECDSARecover',
        f'ECDSARecover({x01_32B}, 27, {x01_32B}, {x01_32B})',
        r'b"\x87\x1e\f\x83n\xc6u\xe0s\x95\xae\xf5\x8dr\xb6F\xf7~\x0e3\xce\x16\"\xdf\xaa\x0f\xb8\xe11\xf3\xfe\x12\xa9\x8cB}\xc1\x05\xe9\x02\xe0I\xa4\xd6\xae\x80\x06\x08\xa6bY|W\\s\xbf\xe6\xafu\xfb\\3\x1d>"',
    ),
    (
        'ECDSASign',
        f"ECDSASign({x01_32B}, {x01_32B})",
        '"6673ffad2147741f04772b6f921f0ba6af0c1e77fc439e65c36dedf4092e88984c1a971652e0ada880120ef8025e709fff2080c4a39aae068d12eed009b68c8901"',
    ),
    (
        'ECDSAPubKey',
        f"ECDSAPubKey({x01_32B})",
        '"79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8"',
    ),
    (
        'BN128Add',
        'BN128Add((101, 2), (3, 404))',
        '( 4485220946665940611972191431348013054410072923527917218676804100027260014333 , 11698076881678876217810497786165132604203673270809061347218409299710221831261 )',
    ),
    (
        'BN128Mul',
        'BN128Mul((1, 2), 3)',
        '( 3353031288059533942658390886683067124040920775575537747144343083137631628272 , 19321533766552368860946552437480515441416830039777911637913418824951667761761 )',
    ),
    (
        'BN128AtePairing',
        'BN128AtePairing(ListItem((0, 0)) .List, ListItem((0x0, 0x0)) .List)',
        'true',
    ),
    (
        'isValidPoint-G1',
        'isValidPoint((0, 0))',
        'true',
    ),
    (
        'isValidPoint-G2',
        'isValidPoint((0x0, 0x0))',
        'true',
    ),
)


@pytest.mark.parametrize(
    'test_id,pgm,output',
    HOOK_TEST_DATA,
    ids=[test_id for test_id, *_ in HOOK_TEST_DATA],
)
def test_hook(definition_dir: Path, test_id: str, pgm: str, output: str) -> None:
    # Given
    expected = f'<k>\n  {output} ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual
