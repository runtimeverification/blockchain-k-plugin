from __future__ import annotations

import json
from pathlib import Path
from typing import TYPE_CHECKING, Final

import pytest

from .utils import hex2bytes, run

if TYPE_CHECKING:
    pass


# EIP-7951 test vectors URL
# wget 'https://raw.githubusercontent.com/ethereum/EIPs/d386b29b5a31bd5cfd8d21bbf4e8a0c87734085e/assets/eip-7951/test-vectors.json'
TEST_VECTORS_FILE: Final = Path(__file__).parent / 'test-data' / 'test-vectors.json'


def load_test_vectors() -> list[tuple[str, str, str]]:
    """Load and parse EIP-7951 test vectors.

    Returns:
        List of (test_name, input_hex, expected_hex) tuples
    """
    with TEST_VECTORS_FILE.open('r') as f:
        test_data = json.load(f)

    test_vectors = []
    for test in test_data:
        test_name = test['Name']
        input_hex = test['Input']
        expected_hex = test['Expected']

        if expected_hex:
            expected_output = f'b"\\x{"\\x".join(expected_hex[i:i+2] for i in range(0, len(expected_hex), 2))}"'
        else:
            expected_output = 'b""'

        test_vectors.append((test_name, input_hex, expected_output))

    return test_vectors


P256VERIFY_TEST_DATA = load_test_vectors()


@pytest.mark.parametrize(
    'test_id,input_hex,expected_output',
    P256VERIFY_TEST_DATA,
    ids=[test_id for test_id, *_ in P256VERIFY_TEST_DATA],
)
def test_p256verify_hook(definition_dir: Path, test_id: str, input_hex: str, expected_output: str) -> None:
    # Given
    pgm = f'P256Verify({hex2bytes(input_hex)})'
    expected = f'<k>\n  {expected_output} ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual
