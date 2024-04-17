from itertools import count
from pathlib import Path
from typing import Final

import pytest
from pytest import TempPathFactory

from .utils import TEST_DATA_DIR, kompile, run


@pytest.fixture(scope='session')
def definition_dir(tmp_path_factory: TempPathFactory) -> Path:
    main_file = TEST_DATA_DIR / 'ecdsa-test.k'
    definition = main_file.read_text()
    output_dir = tmp_path_factory.mktemp('ecdsa-kompiled')
    kompile(definition, output_dir=output_dir, main_module='ECDSA-TEST', syntax_module='ECDSA-TEST')
    return output_dir


TEST_DATA: Final = (
    (
        'addr1',
        '#unparseData(#addrFromPrivateKey("0x2b8ba85ff7f2c7fa62cddb20ae40ef2553b9fdbff086f60d1647ecaab15af867"), 20)',
        '"0xd308979bf428e38b458a99da348e28933a4cddb1"',
    ),
    (
        'addr2',
        '#unparseData(#addrFromPrivateKey("0x0000000000000000000000000000000000000000000000000000000000000001"), 20)',
        '"0x7e5f4552091a69125d5dfcb7b8c2659029395bdf"',
    ),
    (
        'addr3',
        '#unparseData(#addrFromPrivateKey("0x0000000000000000000000000000000000000000000000000000000000000002"), 20)',
        '"0x2b5ad5c4795c026514f8317c7a215e218dccd6cf"',
    ),
    (
        'sender',
        r"""
            #sender(
                b"\xc8\xc7\x95s/\xa0F\xc4H\xd6\xa9\x1c\x19\x11\xcaZ\x87\xcb-\x17\x13\xcb\x87\x08\xb9gF\x01\xda\xe5\x13\r",
                27,
                b"\xba\xf8\xa8\xf5\x93%\x06L\xb7\x94A\xe4\xe7\x83P\xce\xdc\xb7\xf7c\x17\xc4\xf8\xd9\xed2\x10%\xdbR\x91k",
                b"L\xc9\x88\xdb\x9c\xecW\xed\xa4V\xbb$\"\x01\xbbA\xc7#\xf2B\x7f\xf7P\x12?\x11\xafku\x95G\xe2"
            )
        """,
        '966588469268559010541288244128342317224451555083'
    ),
)


@pytest.mark.parametrize(
    'test_id,pgm,output',
    TEST_DATA,
    ids=[test_id for test_id, *_ in TEST_DATA],
)
def test_ecdsa(definition_dir: Path, test_id: str, pgm: str, output: str) -> None:
    # Given
    expected = f'<k>\n  {output} ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual
