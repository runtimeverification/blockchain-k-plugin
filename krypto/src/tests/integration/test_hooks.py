from __future__ import annotations

from itertools import count
from typing import TYPE_CHECKING

import pytest

from .utils import hex2bytes, run

if TYPE_CHECKING:
    from collections.abc import Callable
    from pathlib import Path
    from typing import Final


x01_32B: Final = hex2bytes(31 * '00' + '01')  # noqa: N816


@pytest.fixture(scope='session')
def definition_dir(krypto_kompile: Callable[..., Path]) -> Path:
    definition = """
        requires "plugin/krypto.md"

        module TEST
            imports BOOL
            imports KRYPTO
            syntax Pgm ::= Bool | Bytes | String | G1Point | G2Point
            configuration <k> $PGM:Pgm </k>
        endmodule
    """
    return krypto_kompile(definition=definition, main_module='TEST', syntax_module='TEST')


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
        f'ECDSASign({x01_32B}, {x01_32B})',
        '"6673ffad2147741f04772b6f921f0ba6af0c1e77fc439e65c36dedf4092e88984c1a971652e0ada880120ef8025e709fff2080c4a39aae068d12eed009b68c8901"',
    ),
    (
        'ECDSAPubKey',
        f'ECDSAPubKey({x01_32B})',
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


ED25519_TEST_DATA: Final = (
    (
        'd75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a',
        '',
        'e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e065224901555fb8821590a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b',
    ),
    (
        '3d4017c3e843895a92b70aa74d1b7ebc9c982ccf2ec4968cc0cd55f12af4660c',
        '72',
        '92a009a9f0d4cab8720e820b5f642540a2b27b5416503f8fb3762223ebdb69da085ac1e43e15996e458f3613d0f11d8c387b2eaeb4302aeeb00d291612bb0c00',
    ),
    (
        'fc51cd8e6218a1a38da47ed00230f0580816ed13ba3303ac5deb911548908025',
        'af82',
        '6291d657deec24024827e69c3abe01a30ce548a284743a445e3680d7db5ac3ac18ff9b538d16f290ae67f760984dc6594a7c15e9716ed28dc027beceea1ec40a',
    ),
    (
        'e61a185bcef2613a6c7cb79763ce945d3b245d76114dd440bcf5f2dc1aa57057',
        'cbc77b',
        'd9868d52c2bebce5f3fa5a79891970f309cb6591e3e1702a70276fa97c24b3a8e58606c38c9758529da50ee31b8219cba45271c689afa60b0ea26c99db19b00c',
    ),
    (
        'c0dac102c4533186e25dc43128472353eaabdb878b152aeb8e001f92d90233a7',
        '5f4c8989',
        '124f6fc6b0d100842769e71bd530664d888df8507df6c56dedfdb509aeb93416e26b918d38aa06305df3095697c18b2aa832eaa52edc0ae49fbae5a85e150c07',
    ),
    (
        'e253af0766804b869bb1595be9765b534886bbaab8305bf50dbc7f899bfb5f01',
        '18b6bec097',
        'b2fc46ad47af464478c199e1f8be169f1be6327c7f9a0a6689371ca94caf04064a01b22aff1520abd58951341603faed768cf78ce97ae7b038abfe456aa17c09',
    ),
    (
        'fbcfbfa40505d7f2be444a33d185cc54e16d615260e1640b2b5087b83ee3643d',
        '89010d855972',
        '6ed629fc1d9ce9e1468755ff636d5a3f40a5d9c91afd93b79d241830f7e5fa29854b8f20cc6eecbb248dbd8d16d14e99752194e4904d09c74d639518839d2300',
    ),
    (
        '98a5e3a36e67aaba89888bf093de1ad963e774013b3902bfab356d8b90178a63',
        'b4a8f381e70e7a',
        '6e0af2fe55ae377a6b7a7278edfb419bd321e06d0df5e27037db8812e7e3529810fa5552f6c0020985ca17a0e02e036d7b222a24f99b77b75fdd16cb05568107',
    ),
    (
        'f81fb54a825fced95eb033afcd64314075abfb0abd20a970892503436f34b863',
        '4284abc51bb67235',
        'd6addec5afb0528ac17bb178d3e7f2887f9adbb1ad16e110545ef3bc57f9de2314a5c8388f723b8907be0f3ac90c6259bbe885ecc17645df3db7d488f805fa08',
    ),
    (
        'c1a49c66e617f9ef5ec66bc4c6564ca33de2a5fb5e1464062e6d6c6219155efd',
        '672bf8965d04bc5146',
        '2c76a04af2391c147082e33faacdbe56642a1e134bd388620b852b901a6bc16ff6c9cc9404c41dea12ed281da067a1513866f9d964f8bdd24953856c50042901',
    ),
    (
        '31b2524b8348f7ab1dfafa675cc538e9a84e3fe5819e27c12ad8bbc1a36e4dff',
        '33d7a786aded8c1bf691',
        '28e4598c415ae9de01f03f9f3fab4e919e8bf537dd2b0cdf6e79b9e6559c9409d9151a4c40f083193937627c369488259e99da5a9f0a87497fa6696a5dd6ce08',
    ),
    (
        '44b57ee30cdb55829d0a5d4f046baef078f1e97a7f21b62d75f8e96ea139c35f',
        '3486f68848a65a0eb5507d',
        '77d389e599630d934076329583cd4105a649a9292abc44cd28c40000c8e2f5ac7660a81c85b72af8452d7d25c070861dae91601c7803d656531650dd4e5c4100',
    ),
    (
        '6fe83693d011d111131c4f3fbaaa40a9d3d76b30012ff73bb0e39ec27ab18257',
        '5a8d9d0a22357e6655f9c785',
        '0f9ad9793033a2fa06614b277d37381e6d94f65ac2a5a94558d09ed6ce922258c1a567952e863ac94297aec3c0d0c8ddf71084e504860bb6ba27449b55adc40e',
    ),
    (
        'a2eb8c0501e30bae0cf842d2bde8dec7386f6b7fc3981b8c57c9792bb94cf2dd',
        'b87d3813e03f58cf19fd0b6395',
        'd8bb64aad8c9955a115a793addd24f7f2b077648714f49c4694ec995b330d09d640df310f447fd7b6cb5c14f9fe9f490bcf8cfadbfd2169c8ac20d3b8af49a0c',
    ),
    (
        'cf3af898467a5b7a52d33d53bc037e2642a8da996903fc252217e9c033e2f291',
        '55c7fa434f5ed8cdec2b7aeac173',
        '6ee3fe81e23c60eb2312b2006b3b25e6838e02106623f844c44edb8dafd66ab0671087fd195df5b8f58a1d6e52af42908053d55c7321010092748795ef94cf06',
    ),
    (
        'fd2a565723163e29f53c9de3d5e8fbe36a7ab66e1439ec4eae9c0a604af291a5',
        '0a688e79be24f866286d4646b5d81c',
        'f68d04847e5b249737899c014d31c805c5007a62c0a10d50bb1538c5f35503951fbc1e08682f2cc0c92efe8f4985dec61dcbd54d4b94a22547d24451271c8b00',
    ),
    (
        '34e5a8508c4743746962c066e4badea2201b8ab484de5c4f94476ccd2143955b',
        'c942fa7ac6b23ab7ff612fdc8e68ef39',
        '2a3d27dc40d0a8127949a3b7f908b3688f63b7f14f651aacd715940bdbe27a0809aac142f47ab0e1e44fa490ba87ce5392f33a891539caf1ef4c367cae54500c',
    ),
    (
        '0445e456dacc7d5b0bbed23c8200cdb74bdcb03e4c7b73f0a2b9b46eac5d4372',
        '7368724a5b0efb57d28d97622dbde725af',
        '3653ccb21219202b8436fb41a32ba2618c4a133431e6e63463ceb3b6106c4d56e1d2ba165ba76eaad3dc39bffb130f1de3d8e6427db5b71938db4e272bc3e20b',
    ),
    (
        '74d29127f199d86a8676aec33b4ce3f225ccb191f52c191ccd1e8cca65213a6b',
        'bd8e05033f3a8bcdcbf4beceb70901c82e31',
        'fbe929d743a03c17910575492f3092ee2a2bf14a60a3fcacec74a58c7334510fc262db582791322d6c8c41f1700adb80027ecabc14270b703444ae3ee7623e0a',
    ),
    (
        '5b96dca497875bf9664c5e75facf3f9bc54bae913d66ca15ee85f1491ca24d2c',
        '8171456f8b907189b1d779e26bc5afbb08c67a',
        '73bca64e9dd0db88138eedfafcea8f5436cfb74bfb0e7733cf349baa0c49775c56d5934e1d38e36f39b7c5beb0a836510c45126f8ec4b6810519905b0ca07c09',
    ),
    (
        '1ca281938529896535a7714e3584085b86ef9fec723f42819fc8dd5d8c00817f',
        '8ba6a4c9a15a244a9c26bb2a59b1026f21348b49',
        'a1adc2bc6a2d980662677e7fdff6424de7dba50f5795ca90fdf3e96e256f3285cac71d3360482e993d0294ba4ec7440c61affdf35fe83e6e04263937db93f105',
    ),
    (
        '7fae45dd0a05971026d410bc497af5be7d0827a82a145c203f625dfcb8b03ba8',
        '1d566a6232bbaab3e6d8804bb518a498ed0f904986',
        'bb61cf84de61862207c6a455258bc4db4e15eea0317ff88718b882a06b5cf6ec6fd20c5a269e5d5c805bafbcc579e2590af414c7c227273c102a10070cdfe80f',
    ),
    (
        '48359b850d23f0715d94bb8bb75e7e14322eaf14f06f28a805403fbda002fc85',
        '1b0afb0ac4ba9ab7b7172cddc9eb42bba1a64bce47d4',
        'b6dcd09989dfbac54322a3ce87876e1d62134da998c79d24b50bd7a6a797d86a0e14dc9d7491d6c14a673c652cfbec9f962a38c945da3b2f0879d0b68a921300',
    ),
    (
        'fdb30673402faf1c8033714f3517e47cc0f91fe70cf3836d6c23636e3fd2287c',
        '507c94c8820d2a5793cbf3442b3d71936f35fe3afef316',
        '7ef66e5e86f2360848e0014e94880ae2920ad8a3185a46b35d1e07dea8fa8ae4f6b843ba174d99fa7986654a0891c12a794455669375bf92af4cc2770b579e0c',
    ),
    (
        'b1d39801892027d58a8c64335163195893bfc1b61dbeca3260497e1f30371107',
        'd3d615a8472d9962bb70c5b5466a3d983a4811046e2a0ef5',
        '836afa764d9c48aa4770a4388b654e97b3c16f082967febca27f2fc47ddfd9244b03cfc729698acf5109704346b60b230f255430089ddc56912399d1122de70a',
    ),
    (
        'd0c846f97fe28585c0ee159015d64c56311c886eddcc185d296dbb165d2625d6',
        '6ada80b6fa84f7034920789e8536b82d5e4678059aed27f71c',
        '16e462a29a6dd498685a3718b3eed00cc1598601ee47820486032d6b9acc9bf89f57684e08d8c0f05589cda2882a05dc4c63f9d0431d6552710812433003bc08',
    ),
    (
        '2bf32ba142ba4622d8f3e29ecd85eea07b9c47be9d64412c9b510b27dd218b23',
        '82cb53c4d5a013bae5070759ec06c3c6955ab7a4050958ec328c',
        '881f5b8c5a030df0f75b6634b070dd27bd1ee3c08738ae349338b3ee6469bbf9760b13578a237d5182535ede121283027a90b5f865d63a6537dca07b44049a0f',
    ),
    (
        '94d23d977c33e49e5e4992c68f25ec99a27c41ce6b91f2bfa0cd8292fe962835',
        'a9a8cbb0ad585124e522abbfb40533bdd6f49347b55b18e8558cb0',
        '3acd39bec8c3cd2b44299722b5850a0400c1443590fd4861d59aae7496acb3df73fc3fdf7969ae5f50ba47dddc435246e5fd376f6b891cd4c2caf5d614b6170c',
    ),
    (
        '9d084aa8b97a6b9bafa496dbc6f76f3306a116c9d917e681520a0f914369427e',
        '5cb6f9aa59b80eca14f6a68fb40cf07b794e75171fba96262c1c6adc',
        'f5875423781b66216cb5e8998de5d9ffc29d1d67107054ace3374503a9c3ef811577f269de81296744bd706f1ac478caf09b54cdf871b3f802bd57f9a6cb9101',
    ),
    (
        '16cee8a3f2631834c88b670897ff0b08ce90cc147b4593b3f1f403727f7e7ad5',
        '32fe27994124202153b5c70d3813fdee9c2aa6e7dc743d4d535f1840a5',
        'd834197c1a3080614e0a5fa0aaaa808824f21c38d692e6ffbd200f7dfb3c8f44402a7382180b98ad0afc8eec1a02acecf3cb7fde627b9f18111f260ab1db9a07',
    ),
    (
        '23be323c562dfd71ce65f5bba56a74a3a6dfc36b573d2f94f635c7f9b4fd5a5b',
        'bb3172795710fe00054d3b5dfef8a11623582da68bf8e46d72d27cece2aa',
        '0f8fad1e6bde771b4f5420eac75c378bae6db5ac6650cd2bc210c1823b432b48e016b10595458ffab92f7a8989b293ceb8dfed6c243a2038fc06652aaaf16f02',
    ),
    (
        '3f60c7541afa76c019cf5aa82dcdb088ed9e4ed9780514aefb379dabc844f31a',
        '7cf34f75c3dac9a804d0fcd09eba9b29c9484e8a018fa9e073042df88e3c56',
        'be71ef4806cb041d885effd9e6b0fbb73d65d7cdec47a89c8a994892f4e55a568c4cc78d61f901e80dbb628b86a23ccd594e712b57fa94c2d67ec26634878507',
    ),
)


@pytest.mark.parametrize('pk,msg,sig', ED25519_TEST_DATA, ids=count())
def test_ed25519_verify(definition_dir: Path, pk: str, msg: str, sig: str) -> None:
    # Given
    pgm = f'ED25519VerifyMessage({hex2bytes(pk)}, {hex2bytes(msg)}, {hex2bytes(sig)})'
    expected = '<k>\n  true ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual


VERIFYKZGPROOF_TEST_DATA: Final = (
    (
        'c00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000',
        '0000000000000000000000000000000000000000000000000000000000000002',
        '0000000000000000000000000000000000000000000000000000000000000000',
        'c00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000',
        'true',
    ),
    (
        'a572cbea904d67468808c8eb50a9450c9721db309128012543902d0ac358a62ae28f75bb8f1c7c42c39a8c5529bf0f4e',
        '564c0a11a0f704f4fc3e8acfe0f8245f0ad1347b378fbf96e206da11a5d36306',
        '0000000000000000000000000000000000000000000000000000000000000002',
        'c00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000',
        'true',
    ),
    (
        '93efc82d2017e9c57834a1246463e64774e56183bb247c8fc9dd98c56817e878d97b05f5c8d900acf1fbbbca6f146556',
        '73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000000',
        '0000000000000000000000000000000000000000000000000000000000000000',
        '92c51ff81dd71dab71cefecd79e8274b4b7ba36a0f40e2dc086bc4061c7f63249877db23297212991fd63e07b7ebc348',
        'true',
    ),
    (
        '8f59a8d2a1a625a17f3fea0fe5eb8c896db3764f3185481bc22f91b4aaffcca25f26936857bc3a7c2539ea8ec3a952b7',
        '73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000000',
        '1522a4a7f34e1ea350ae07c29c96c7e79655aa926122e95fe69fcbd932ca49e9',
        'a62ad71d14c5719385c0686f1871430475bf3a00f0aa3f7b8dd99a9abc2160744faf0070725e00b60ad9a026a15b1a8c',
        'true',
    ),
    (
        'a572cbea904d67468808c8eb50a9450c9721db309128012543902d0ac358a62ae28f75bb8f1c7c42c39a8c5529bf0f4e',
        '564c0a11a0f704f4fc3e8acfe0f8245f0ad1347b378fbf96e206da11a5d36306',
        '0000000000000000000000000000000000000000000000000000000000000002',
        'c00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000',
        'true',
    ),
    (
        'c00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000',
        '0000000000000000000000000000000000000000000000000000000000000002',
        '0000000000000000000000000000000000000000000000000000000000000000',
        '97f1d3a73197d7942695638c4fa9ac0fc3688c4f9774b905a14e3a3f171bac586c55e83ff97a1aeffb3af00adb22c6bb',
        'false',
    ),
    (
        'a572cbea904d67468808c8eb50a9450c9721db309128012543902d0ac358a62ae28f75bb8f1c7c42c39a8c5529bf0f4e',
        '564c0a11a0f704f4fc3e8acfe0f8245f0ad1347b378fbf96e206da11a5d36306',
        '0000000000000000000000000000000000000000000000000000000000000002',
        '97f1d3a73197d7942695638c4fa9ac0fc3688c4f9774b905a14e3a3f171bac586c55e83ff97a1aeffb3af00adb22c6bb',
        'false',
    ),
    (
        'a572cbea904d67468808c8eb50a9450c9721db309128012543902d0ac358a62ae28f75bb8f1c7c42c39a8c5529bf0f4e',
        '0000000000000000000000000000000000000000000000000000000000000001',
        '0000000000000000000000000000000000000000000000000000000000000002',
        '97f1d3a73197d7942695638c4fa9ac0fc3688c4f9774b905a14e3a3f171bac586c55e83ff97a1aeffb3af00adb22c6bb',
        'false',
    ),
    (
        'a421e229565952cfff4ef3517100a97da1d4fe57956fa50a442f92af03b1bf37adacc8ad4ed209b31287ea5bb94d9d06',
        '73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000000',
        '304962b3598a0adf33189fdfd9789feab1096ff40006900400000003fffffffc',
        'c00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000',
        'false',
    ),
)


@pytest.mark.parametrize('com,z,y,pr,output', VERIFYKZGPROOF_TEST_DATA, ids=count())
def test_verify_kzg_proof(definition_dir: Path, com: str, z: str, y: str, pr: str, output: str) -> None:
    # Given
    pgm = f'verifyKZGProof({hex2bytes(com)}, {hex2bytes(z)}, {hex2bytes(y)}, {hex2bytes(pr)})'
    expected = f'<k>\n  {output} ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual


VERIFYKZGPROOF_REGRESSION_TEST_DATA: Final = (
    (
        '93efc82d2017e9c57834a1246463e64774e56183bb247c8fc9dd98c56817e878d97b05f5c8d900acf1fbbbca6f146556',
        '73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000000',
        '0000000000000000000000000000000000000000000000000000000000000000',
        '92c51ff81dd71dab71cefecd79e8274b4b7ba36a0f40e2dc086bc4061c7f63249877db23297212991fd63e07b7ebc348',
    ),
)


@pytest.mark.parametrize('com,z,y,pr', VERIFYKZGPROOF_REGRESSION_TEST_DATA, ids=count())
def test_verify_kzg_proof_regression(definition_dir: Path, com: str, z: str, y: str, pr: str) -> None:
    # Given
    pgm = f'verifyKZGProof({hex2bytes(com)}, {hex2bytes(z)}, {hex2bytes(y)}, {hex2bytes(pr)}) andBool verifyKZGProof({hex2bytes(com)}, {hex2bytes(z)}, {hex2bytes(y)}, {hex2bytes(pr)})'
    expected = '<k>\n  true ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual


BLS12_X: Final = -0xD201000000010000
BLS12_Q: Final = BLS12_X**4 - BLS12_X**2 + 1
BLS12_P: Final = (BLS12_X - 1) ** 2 * BLS12_Q // 3 + BLS12_X

BLS12_G1_INFTY: Final = (0, 0)
BLS12_G1: Final = (
    0x17F1D3A73197D7942695638C4FA9AC0FC3688C4F9774B905A14E3A3F171BAC586C55E83FF97A1AEFFB3AF00ADB22C6BB,
    0x8B3F481E3AAA0F1A09E30ED741D8AE4FCF5E095D5D00AF600DB18CB2C04B3EDD03CC744A2888AE40CAA232946C5E7E1,
)
BLS12_2G1: Final = (
    0x572CBEA904D67468808C8EB50A9450C9721DB309128012543902D0AC358A62AE28F75BB8F1C7C42C39A8C5529BF0F4E,
    0x166A9D8CABC673A322FDA673779D8E3822BA3ECB8670E461F73BB9021D5FD76A4C56D9D4CD16BD1BBA86881979749D28,
)
BLS12_P1: Final = (  # random point in G1
    0x112B98340EEE2777CC3C14163DEA3EC97977AC3DC5C70DA32E6E87578F44912E902CCEF9EFE28D4A78B8999DFBCA9426,
    0x186B28D92356C4DFEC4B5201AD099DBDEDE3781F8998DDF929B4CD7756192185CA7B8F4EF7088F813270AC3D48868A21,
)
BLS12_2P1: Final = (
    0x15222CDDBABDD764C4BEE0B3720322A65FF4712C86FC4B1588D0C209210A0884FA9468E855D261C483091B2BF7DE6A63,
    0x9F9EDB99BC3B75D7489735C98B16AB78B9386C5F7A1F76C7E96AC6EB5BBDE30DBCA31A74EC6E0F0B12229EECEA33C39,
)
BLS12_P1_PLUS_G1: Final = (
    0xA40300CE2DEC9888B60690E9A41D3004FDA4886854573974FAB73B046D3147BA5B7A5BDE85279FFEDE1B45B3918D82D,
    0x6D3D887E9F53B9EC4EB6CEDF5607226754B07C01ACE7834F57F3E7315FAEFB739E59018E22C492006190FBA4A870025,
)

BLS12_P1_NOT_IN_SUBGROUP: Final = (0, 2)
BLS12_2P1_NOT_IN_SUBGROUP: Final = (0, BLS12_P - 2)

BLS12_G1_POINT_ZERO_FP = (
    0x11A9A0372B8F332D5C30DE9AD14E50372A73FA4C45D5F2FA5097F2D6FB93BCAC592F2E1711AC43DB0519870C7D0EA415,
    0x92C0F994164A0719F51C24BA3788DE240FF926B55F58C445116E8BC6A47CD63392FD4E8E22BDF9FEAA96EE773222133,
)


BLS12_G2_INFTY: Final = ((0, 0), (0, 0))
BLS12_P2: Final = (
    (
        0x103121A2CEAAE586D240843A398967325F8EB5A93E8FEA99B62B9F88D8556C80DD726A4B30E84A36EEABAF3592937F27,
        0x86B990F3DA2AEAC0A36143B7D7C824428215140DB1BB859338764CB58458F081D92664F9053B50B3FBD2E4723121B68,
    ),
    (
        0xF9E7BA9A86A8F7624AA2B42DCC8772E1AF4AE115685E60ABC2C9B90242167ACEF3D0BE4050BF935EED7C3B6FC7BA77E,
        0xD22C3652D0DC6F0FC9316E14268477C2049EF772E852108D269D9C38DBA1D4802E8DAE479818184C08F9A569D878451,
    ),
)
BLS12_G2: Final = (
    (
        0x24AA2B2F08F0A91260805272DC51051C6E47AD4FA403B02B4510B647AE3D1770BAC0326A805BBEFD48056C8C121BDB8,
        0x13E02B6052719F607DACD3A088274F65596BD0D09920B61AB5DA61BBDC7F5049334CF11213945D57E5AC7D055D042B7E,
    ),
    (
        0xCE5D527727D6E118CC9CDC6DA2E351AADFD9BAA8CBDD3A76D429A695160D12C923AC9CC3BACA289E193548608B82801,
        0x606C4A02EA734CC32ACD2B02BC28B99CB3E287E85A763AF267492AB572E99AB3F370D275CEC1DA1AAA9075FF05F79BE,
    ),
)
BLS12_P2_PLUS_G2: Final = (
    (
        0xB54A8A7B08BD6827ED9A797DE216B8C9057B3A9CA93E2F88E7F04F19ACCC42DA90D883632B9CA4DC38D013F71EDE4DB,
        0x77EBA4EECF0BD764DCE8ED5F45040DD8F3B3427CB35230509482C14651713282946306247866DFE39A8E33016FCBE52,
    ),
    (
        0x14E60A76A29EF85CBD69F251B9F29147B67CFE3ED2823D3F9776B3A0EFD2731941D47436DC6D2B58D9E65F8438BAD073,
        0x1586C3C910D95754FEF7A732DF78E279C3D37431C6A2B77E67A00C7C130A8FCD4D19F159CBEB997A178108FFFFFCBD20,
    ),
)
BLS12_2G2: Final = (
    (
        0x1638533957D540A9D2370F17CC7ED5863BC0B995B8825E0EE1EA1E1E4D00DBAE81F14B0BF3611B78C952AACAB827A053,
        0x0A4EDEF9C1ED7F729F520E47730A124FD70662A904BA1074728114D1031E1572C6C886F6B57EC72A6178288C47C33577,
    ),
    (
        0x0468FB440D82B0630AEB8DCA2B5256789A66DA69BF91009CBFE6BD221E47AA8AE88DECE9764BF3BD999D95D71E4C9899,
        0x0F6D4552FA65DD2638B361543F887136A43253D9C66C411697003F7A13C308F5422E1AA0A59C8967ACDEFD8B6E36CCF3,
    ),
)
BLS12_2P2: Final = (
    (
        0x0B76FCBB604082A4F2D19858A7BEFD6053FA181C5119A612DFEC83832537F644E02454F2B70D40985EBB08042D1620D4,
        0x19A4A02C0AE51365D964C73BE7BABB719DB1C69E0DDBF9A8A335B5BED3B0A4B070D2D5DF01D2DA4A3F1E56AAE2EC106D,
    ),
    (
        0x0D18322F821AC72D3CA92F92B000483CF5B7D9E5D06873A44071C4E7E81EFD904F210208FE0B9B4824F01C65BC7E6208,
        0x04E563D53609A2D1E216AAAEE5FBC14EF460160DB8D1FDC5E1BD4E8B54CD2F39ABF6F925969FA405EFB9E700B01C7085,
    ),
)
BLS12_P2_NOT_IN_SUBGROUP: Final = (
    (1, 1),
    (
        0x17FAA6201231304F270B858DAD9462089F2A5B83388E4B10773ABC1EEF6D193B9FCE4E8EA2D9D28E3C3A315AA7DE14CA,
        0xCC12449BE6AC4E7F367E7242250427C4FB4C39325D3164AD397C1837A90F0EA1A534757DF374DD6569345EB41ED76E,
    ),
)
BLS12_2P2_NOT_IN_SUBGROUP: Final = (
    (
        0x919F97860ECC3E933E3477FCAC0E2E4FCC35A6E886E935C97511685232456263DEF6665F143CCCCB44C733333331553,
        0x18B4376B50398178FA8D78ED2654B0FFD2A487BE4DBE6B69086E61B283F4E9D58389CCCB8EDC99995718A66666661555,
    ),
    (
        0x26898F699C4B07A405AB4183A10B47F923D1C0FDA1018682DD2CCC88968C1B90D44534D6B9270CF57F8DC6D4891678A,
        0x3270414330EAD5EC92219A03A24DFA059DBCBE610868BE1851CC13DAC447F60B40D41113FD007D3307B19ADD4B0F061,
    ),
)

BLS12_G2_POINT_ZERO_FP = (
    (
        0x18320896EC9EEF9D5E619848DC29CE266F413D02DD31D9B9D44EC0C79CD61F18B075DDBA6D7BD20B7FF27A4B324BFCE,
        0xA67D12118B5A35BB02D2E86B3EBFA7E23410DB93DE39FB06D7025FA95E96FFA428A7A27C3AE4DD4B40BD251AC658892,
    ),
    (
        0x260E03644D1A2C321256B3246BAD2B895CAD13890CBE6F85DF55106A0D334604FB143C7A042D878006271865BC35941,
        0x4C69777A43F0BDA07679D5805E63F18CF4E0E7C6112AC7F70266D199B4F76AE27C6269A3CEEBDAE30806E9A76AADF5C,
    ),
)


def bls12_neg_g1(p: tuple[int, int]) -> tuple[int, int]:
    return (p[0], BLS12_P - p[1])


def bls12_neg_g2(p: tuple[tuple[int, int], tuple[int, int]]) -> tuple[tuple[int, int], tuple[int, int]]:
    return (p[0], (BLS12_P - p[1][0], BLS12_P - p[1][1]))


# See https://github.com/ethereum/execution-spec-tests/blob/b48d1dc81233af6e4d6c7c84e60e8eaa4067a288/tests/prague/eip2537_bls_12_381_precompiles/test_bls12_g1add.py
VERIFYBLS12G1ADD_TEST_DATA: Final = (
    (BLS12_G1, BLS12_G1_INFTY, BLS12_G1, 'generator_plus_inf'),
    (BLS12_G1_INFTY, BLS12_G1, BLS12_G1, 'inf_plus_generator'),
    (BLS12_G1_INFTY, BLS12_G1_INFTY, BLS12_G1_INFTY, 'inf_plus_inf'),
    (BLS12_P1, BLS12_G1_INFTY, BLS12_P1, 'point_plus_inf'),
    (BLS12_G1_INFTY, BLS12_P1, BLS12_P1, 'inf_plus_point'),
    (BLS12_P1, bls12_neg_g1(BLS12_P1), BLS12_G1_INFTY, 'point_plus_neg_point'),
    (BLS12_G1, bls12_neg_g1(BLS12_G1), BLS12_G1_INFTY, 'generator_plus_neg_point'),
    (BLS12_P1, BLS12_G1, BLS12_P1_PLUS_G1, 'commutative_check_a'),
    (BLS12_G1, BLS12_P1, BLS12_P1_PLUS_G1, 'commutative_check_b'),
    (BLS12_P1, BLS12_P1, BLS12_2P1, 'point_doubling'),
    (bls12_neg_g1(BLS12_P1), bls12_neg_g1(BLS12_G1), bls12_neg_g1(BLS12_P1_PLUS_G1), 'negation_of_sum'),
    (BLS12_P1_NOT_IN_SUBGROUP, BLS12_P1_NOT_IN_SUBGROUP, BLS12_2P1_NOT_IN_SUBGROUP, 'non_sub_plus_non_sub'),
    (BLS12_P1_NOT_IN_SUBGROUP, BLS12_2P1_NOT_IN_SUBGROUP, BLS12_G1_INFTY, 'non_sub_order_3_to_inf'),
    (BLS12_P1_NOT_IN_SUBGROUP, BLS12_G1_INFTY, BLS12_P1_NOT_IN_SUBGROUP, 'non_sub_PLUS_inf'),
)


@pytest.mark.parametrize('first,second,result,explanation', VERIFYBLS12G1ADD_TEST_DATA, ids=count())
def test_verify_bls12g1_add(
    definition_dir: Path, first: tuple[int, int], second: tuple[int, int], result: tuple[int, int], explanation: str
) -> None:
    # Given
    pgm = f'BLS12G1Add({first}, {second})'
    expected = f'<k>\n  ( {result[0]} , {result[1]} ) ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual


# https://github.com/ethereum/execution-spec-tests/blob/b48d1dc81233af6e4d6c7c84e60e8eaa4067a288/tests/prague/eip2537_bls_12_381_precompiles/test_bls12_g1mul.py
VERIFYBLS12G1MUL_TEST_DATA: Final = (
    (BLS12_G1_INFTY, 0, BLS12_G1_INFTY, 'zero_times_inf'),
    (BLS12_G1_INFTY, 1, BLS12_G1_INFTY, 'one_times_inf'),
    (BLS12_G1_INFTY, 2, BLS12_G1_INFTY, 'two_times_inf'),
    (BLS12_G1_INFTY, BLS12_Q, BLS12_G1_INFTY, 'q_times_inf'),
    (BLS12_G1_INFTY, 2**256 - 1, BLS12_G1_INFTY, 'max_scalar_times_inf'),
    (BLS12_G1, 0, BLS12_G1_INFTY, 'zero_times_generator'),
    (BLS12_G1, 1, BLS12_G1, 'one_times_generator'),
    (BLS12_G1, BLS12_Q, BLS12_G1_INFTY, 'q_times_generator'),
    (BLS12_P1, 0, BLS12_G1_INFTY, 'zero_times_point'),
    (BLS12_P1, 1, BLS12_P1, 'one_times_point'),
    (BLS12_P1, BLS12_Q - 1, bls12_neg_g1(BLS12_P1), 'q_minus_1_times_point'),
    (BLS12_P1, BLS12_Q, BLS12_G1_INFTY, 'q_times_point'),
    (BLS12_P1, BLS12_Q + 1, BLS12_P1, 'q_plus_1_times_point'),
    (BLS12_P1, 2 * BLS12_Q, BLS12_G1_INFTY, '2q_times_point'),
    (BLS12_P1, 2**256 // BLS12_Q * BLS12_Q, BLS12_G1_INFTY, 'large_multiple_of_q_times_point'),
    (
        BLS12_P1,
        2**256 - 1,
        (
            0x3DA1F13DDEF2B8B5A46CD543CE56C0A90B8B3B0D6D43DEC95836A5FD2BACD6AA8F692601F870CF22E05DDA5E83F460B,
            0x18D64F3C0E9785365CBDB375795454A8A4FA26F30B9C4F6E33CA078EB5C29B7AEA478B076C619BC1ED22B14C95569B2D,
        ),
        'max_scalar_times_point',
    ),
)


@pytest.mark.parametrize('first,second,result,explanation', VERIFYBLS12G1MUL_TEST_DATA, ids=count())
def test_verify_bls12g1_mul(
    definition_dir: Path, first: tuple[int, int], second: int, result: tuple[int, int], explanation: str
) -> None:
    # Given
    pgm = f'BLS12G1Mul({first}, {second})'
    expected = f'<k>\n  ( {result[0]} , {result[1]} ) ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual


# https://github.com/ethereum/execution-spec-tests/blob/b48d1dc81233af6e4d6c7c84e60e8eaa4067a288/tests/prague/eip2537_bls_12_381_precompiles/test_bls12_g2add.py
VERIFYBLS12G2ADD_TEST_DATA: Final = (
    (BLS12_G2, BLS12_G2_INFTY, BLS12_G2, 'generator_plus_inf'),
    (BLS12_G2_INFTY, BLS12_G2, BLS12_G2, 'inf_plus_generator'),
    (BLS12_G2_INFTY, BLS12_G2_INFTY, BLS12_G2_INFTY, 'inf_plus_inf'),
    (BLS12_P2, BLS12_G2_INFTY, BLS12_P2, 'inf_plus_point'),
    (BLS12_P2, bls12_neg_g2(BLS12_P2), BLS12_G2_INFTY, 'point_plus_neg_point'),
    (BLS12_G2, bls12_neg_g2(BLS12_G2), BLS12_G2_INFTY, 'generator_plus_neg_point'),
    (BLS12_P2, BLS12_G2, BLS12_P2_PLUS_G2, 'commutative_check_a'),
    (BLS12_G2, BLS12_P2, BLS12_P2_PLUS_G2, 'commutative_check_b'),
    (BLS12_P2, BLS12_P2, BLS12_2P2, 'point_doubling'),
    (bls12_neg_g2(BLS12_P2), bls12_neg_g2(BLS12_G2), bls12_neg_g2(BLS12_P2_PLUS_G2), 'negation_of_sum'),
    (BLS12_P2_NOT_IN_SUBGROUP, BLS12_P2_NOT_IN_SUBGROUP, BLS12_2P2_NOT_IN_SUBGROUP, 'non_sub_plus_non_sub'),
    (BLS12_P2_NOT_IN_SUBGROUP, BLS12_G2_INFTY, BLS12_P2_NOT_IN_SUBGROUP, 'non_sub_plus_inf'),
    (BLS12_P2_NOT_IN_SUBGROUP, bls12_neg_g2(BLS12_P2_NOT_IN_SUBGROUP), BLS12_G2_INFTY, 'non_sub_plus_neg_non_sub'),
)


@pytest.mark.parametrize('first,second,result,explanation', VERIFYBLS12G2ADD_TEST_DATA, ids=count())
def test_verify_bls12g2_add(
    definition_dir: Path,
    first: tuple[tuple[int, int], tuple[int, int]],
    second: tuple[tuple[int, int], tuple[int, int]],
    result: tuple[tuple[int, int], tuple[int, int]],
    explanation: str,
) -> None:
    # Given
    pgm = f'BLS12G2Add( ( {first[0][0]} x {first[0][1]} , {first[1][0]} x {first[1][1]} ), ( {second[0][0]} x {second[0][1]} , {second[1][0]} x {second[1][1]} ) )'
    expected = f'<k>\n  ( {result[0][0]} x {result[0][1]} , {result[1][0]} x {result[1][1]} ) ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual


# https://github.com/ethereum/execution-spec-tests/blob/b48d1dc81233af6e4d6c7c84e60e8eaa4067a288/tests/prague/eip2537_bls_12_381_precompiles/test_bls12_g2mul.py
VERIFYBLS12G2MUL_TEST_DATA: Final = (
    (BLS12_G2_INFTY, 0, BLS12_G2_INFTY, 'zero_times_inf'),
    (BLS12_G2_INFTY, 1, BLS12_G2_INFTY, 'one_times_inf'),
    (BLS12_G2_INFTY, 2, BLS12_G2_INFTY, 'two_times_inf'),
    (BLS12_G2_INFTY, BLS12_Q, BLS12_G2_INFTY, 'q_times_inf'),
    (BLS12_G2_INFTY, 2**256 - 1, BLS12_G2_INFTY, 'max_scalar_times_inf'),
    (BLS12_G2, 0, BLS12_G2_INFTY, 'zero_times_generator'),
    (BLS12_G2, 1, BLS12_G2, 'one_times_generator'),
    (BLS12_P2, 0, BLS12_G2_INFTY, 'zero_times_point'),
    (BLS12_P2, 1, BLS12_P2, 'one_times_point'),
    (
        BLS12_P2,
        2**256 - 1,
        (
            (
                0x2663E1C3431E174CA80E5A84489569462E13B52DA27E7720AF5567941603475F1F9BC0102E13B92A0A21D96B94E9B22,
                0x6A80D056486365020A6B53E2680B2D72D8A93561FC2F72B960936BB16F509C1A39C4E4174A7C9219E3D7EF130317C05,
            ),
            (
                0xC49EAD39E9EB7E36E8BC25824299661D5B6D0E200BBC527ECCB946134726BF5DBD861E8E6EC946260B82ED26AFE15FB,
                0x5397DAD1357CF8333189821B737172B18099ECF7EE8BDB4B3F05EBCCDF40E1782A6C71436D5ACE0843D7F361CBC6DB2,
            ),
        ),
        'max_scalar_times_point',
    ),
    (BLS12_P2, BLS12_Q - 1, bls12_neg_g2(BLS12_P2), 'q_minus_1_times_point'),
    (BLS12_P2, BLS12_Q, BLS12_G2_INFTY, 'q_times_point'),
    (BLS12_P2, BLS12_Q + 1, BLS12_P2, 'q_plus_1_times_point'),
    (BLS12_P2, 2 * BLS12_Q, BLS12_G2_INFTY, '2q_times_point'),
    (BLS12_P2, 2**256 // BLS12_Q * BLS12_Q, BLS12_G2_INFTY, 'large_multiple_of_q_times_point'),
)


@pytest.mark.parametrize('first,second,result,explanation', VERIFYBLS12G2MUL_TEST_DATA, ids=count())
def test_verify_bls12g2_mul(
    definition_dir: Path,
    first: tuple[tuple[int, int], tuple[int, int]],
    second: int,
    result: tuple[tuple[int, int], tuple[int, int]],
    explanation: str,
) -> None:
    # Given
    pgm = f'BLS12G2Mul(( {first[0][0]} x {first[0][1]} , {first[1][0]} x {first[1][1]} ), {second})'
    expected = f'<k>\n  ( {result[0][0]} x {result[0][1]} , {result[1][0]} x {result[1][1]} ) ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual


# https://github.com/ethereum/execution-spec-tests/blob/b48d1dc81233af6e4d6c7c84e60e8eaa4067a288/tests/prague/eip2537_bls_12_381_precompiles/test_bls12_pairing.py
VERIFYBLS12PAIRING_TEST_DATA: Final = (
    ('generator_with_inf_g2', [BLS12_P1], [BLS12_G2_INFTY], True),
    ('inf_g1_with_generator', [BLS12_G1_INFTY], [BLS12_P2], True),
    ('inf_pair', [BLS12_G1_INFTY], [BLS12_G2_INFTY], True),
    ('multi_inf_pair', [BLS12_G1_INFTY] * 10, [BLS12_G2_INFTY] * 10, True),
    ('g1_g2_and_inverse', [BLS12_G1, BLS12_G1], [BLS12_G2, bls12_neg_g2(BLS12_G2)], True),
    (
        'full_sign_cancellation',
        [BLS12_G1, BLS12_G1, bls12_neg_g1(BLS12_G1), bls12_neg_g1(BLS12_G1)],
        [BLS12_G2, bls12_neg_g2(BLS12_G2), BLS12_G2, bls12_neg_g2(BLS12_G2)],
        True,
    ),
    (
        'large_input_with_cancellation',
        [BLS12_G1_INFTY] * 10 + [BLS12_G1, BLS12_G1],
        [BLS12_G2_INFTY] * 10 + [BLS12_G2, bls12_neg_g2(BLS12_G2)],
        True,
    ),
    ('negated_both_pairs', [BLS12_G1, bls12_neg_g1(BLS12_G1)], [BLS12_G2, bls12_neg_g2(BLS12_G2)], False),
    ('multi_inf_g1_neg_g2', [BLS12_G1_INFTY, BLS12_G1], [BLS12_G2_INFTY, bls12_neg_g2(BLS12_G2)], False),
    ('g1_neg_g2_multi_inf', [BLS12_G1, BLS12_G1_INFTY], [bls12_neg_g2(BLS12_G2), BLS12_G2_INFTY], False),
    ('single_generator_pair', [BLS12_G1], [BLS12_G2], False),
    ('inf_plus_generator_pair', [BLS12_G1_INFTY, BLS12_G1], [BLS12_G2_INFTY, BLS12_G2], False),
    (
        'partial_sign_cancellation',
        [BLS12_G1, BLS12_G1, bls12_neg_g1(BLS12_G1)],
        [BLS12_G2, bls12_neg_g2(BLS12_G2), BLS12_G2],
        False,
    ),
)


@pytest.mark.parametrize(
    'id, first,second,result', VERIFYBLS12PAIRING_TEST_DATA, ids=[id for id, *_ in VERIFYBLS12PAIRING_TEST_DATA]
)
def test_verify_bls12pairing(
    definition_dir: Path,
    id: str,
    first: list[tuple[int, int]],
    second: list[tuple[tuple[int, int], tuple[int, int]]],
    result: bool,
) -> None:
    # Given
    first_list = [f'ListItem(({x[0]} , {x[1]}))' for x in first]
    second_list = [f'ListItem(({x[0][0]} x {x[0][1]} , {x[1][0]} x {x[1][1]}))' for x in second]
    first_str = ' '.join(first_list)
    second_str = ' '.join(second_list)
    pgm = f'BLS12PairingCheck( {first_str} , {second_str} )'

    result_str = 'true' if result else 'false'
    expected = f'<k>\n  {result_str} ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual


# https://github.com/ethereum/execution-spec-tests/blob/b48d1dc81233af6e4d6c7c84e60e8eaa4067a288/tests/prague/eip2537_bls_12_381_precompiles/test_bls12_map_fp_to_g1.py
VERIFYBLS12FPTOG1_TEST_DATA: Final = (
    ('fp_0', 0, BLS12_G1_POINT_ZERO_FP),
    (
        'fp_p_minus_1',
        BLS12_P - 1,
        (
            0x1073311196F8EF19477219CCEE3A48035FF432295AA9419EED45D186027D88B90832E14C4F0E2AA4D15F54D1C3ED0F93,
            0x16B3A3B2E3DDDF6A11459DDAF657FDE21C4F10282A56029D9B55AB3CE1F41E1CF39AD27E0EA35823C7D3250E81FF3D66,
        ),
    ),
)


@pytest.mark.parametrize(
    'id,first,result', VERIFYBLS12FPTOG1_TEST_DATA, ids=[id for id, *_ in VERIFYBLS12FPTOG1_TEST_DATA]
)
def test_verify_bls12fptog1(definition_dir: Path, id: str, first: int, result: tuple[int, int]) -> None:
    # Given
    pgm = f'BLS12MapFpToG1({first})'
    expected = f'<k>\n  ( {result[0]} , {result[1]} ) ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual


# https://github.com/ethereum/execution-spec-tests/blob/b48d1dc81233af6e4d6c7c84e60e8eaa4067a288/tests/prague/eip2537_bls_12_381_precompiles/test_bls12_map_fp2_to_g2.py
VERIFYBLS12FP2TOG2_TEST_DATA: Final = (
    ('fp_0', (0, 0), BLS12_G2_POINT_ZERO_FP),
    (
        'fp_p_minus_1',
        (BLS12_P - 1, BLS12_P - 1),
        (
            (
                0x9BF1B857D8C15F317F649ACCFA7023EF21CFC03059936B83B487DB476FF9D2FE64C6147140A5F0A436B875F51FFDF07,
                0xBB10E09BDF236CB2951BD7BCC044E1B9A6BB5FD4B2019DCC20FFDE851D52D4F0D1A32382AF9D7DA2C5BA27E0F1C69E6,
            ),
            (
                0xDD416A927AB1C15490AB753C973FD377387B12EFCBE6BED2BF768B9DC95A0CA04D1A8F0F30DBC078A2350A1F823CFD3,
                0x171565CE4FCD047B35EA6BCEE4EF6FDBFEC8CC73B7ACDB3A1EC97A776E13ACDFEFFC21ED6648E3F0EEC53DDB6C20FB61,
            ),
        ),
    ),
)


@pytest.mark.parametrize(
    'id,first,result', VERIFYBLS12FP2TOG2_TEST_DATA, ids=[id for id, *_ in VERIFYBLS12FP2TOG2_TEST_DATA]
)
def test_verify_bls12fp2tog2(
    definition_dir: Path, id: str, first: tuple[int, int], result: tuple[tuple[int, int], tuple[int, int]]
) -> None:
    # Given
    pgm = f'BLS12MapFp2ToG2({first[0]}, {first[1]})'
    expected = f'<k>\n  ( {result[0][0]} x {result[0][1]} , {result[1][0]} x {result[1][1]} ) ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual


VERIFYBLS12G1INSUBGROUP_TEST_DATA: Final = (
    ('inf', BLS12_G1_INFTY, True),
    ('generator', BLS12_G1, True),
    ('point', BLS12_P1, True),
    ('point_not_in_subgroup', BLS12_P1_NOT_IN_SUBGROUP, False),
)


@pytest.mark.parametrize(
    'id,first,result', VERIFYBLS12G1INSUBGROUP_TEST_DATA, ids=[id for id, *_ in VERIFYBLS12G1INSUBGROUP_TEST_DATA]
)
def test_verify_bls12g1insubgroup(definition_dir: Path, id: str, first: tuple[int, int], result: bool) -> None:
    # Given
    pgm = f'BLS12G1InSubgroup(({first[0]}, {first[1]}))'
    result_str = 'true' if result else 'false'
    expected = f'<k>\n  {result_str} ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual


VERIFYBLS12G1ONCURVE_TEST_DATA: Final = (
    ('inf', BLS12_G1_INFTY, True),
    ('generator', BLS12_G1, True),
    ('point', BLS12_P1, True),
    ('point_not_in_subgroup', BLS12_P1_NOT_IN_SUBGROUP, True),
)


@pytest.mark.parametrize(
    'id,first,result', VERIFYBLS12G1ONCURVE_TEST_DATA, ids=[id for id, *_ in VERIFYBLS12G1ONCURVE_TEST_DATA]
)
def test_verify_bls12g1oncurve(definition_dir: Path, id: str, first: tuple[int, int], result: bool) -> None:
    # Given
    pgm = f'BLS12G1OnCurve(({first[0]}, {first[1]}))'
    result_str = 'true' if result else 'false'
    expected = f'<k>\n  {result_str} ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual


VERIFYBLS12G2INSUBGROUP_TEST_DATA: Final = (
    ('inf', BLS12_G2_INFTY, True),
    ('generator', BLS12_G2, True),
    ('point', BLS12_P2, True),
    ('point_not_in_subgroup', BLS12_P2_NOT_IN_SUBGROUP, False),
)


@pytest.mark.parametrize(
    'id,first,result', VERIFYBLS12G2INSUBGROUP_TEST_DATA, ids=[id for id, *_ in VERIFYBLS12G2INSUBGROUP_TEST_DATA]
)
def test_verify_bls12g2insubgroup(
    definition_dir: Path, id: str, first: tuple[tuple[int, int], tuple[int, int]], result: bool
) -> None:
    # Given
    pgm = f'BLS12G2InSubgroup(({first[0][0]} x {first[0][1]} , {first[1][0]} x {first[1][1]}))'
    result_str = 'true' if result else 'false'
    expected = f'<k>\n  {result_str} ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual


VERIFYBLS12G2ONCURVE_TEST_DATA: Final = (
    ('inf', BLS12_G2_INFTY, True),
    ('generator', BLS12_G2, True),
    ('point', BLS12_P2, True),
    ('point_not_in_subgroup', BLS12_P2_NOT_IN_SUBGROUP, True),
)


@pytest.mark.parametrize(
    'id,first,result', VERIFYBLS12G2ONCURVE_TEST_DATA, ids=[id for id, *_ in VERIFYBLS12G2ONCURVE_TEST_DATA]
)
def test_verify_bls12g2oncurve(
    definition_dir: Path, id: str, first: tuple[tuple[int, int], tuple[int, int]], result: bool
) -> None:
    # Given
    pgm = f'BLS12G2OnCurve(({first[0][0]} x {first[0][1]} , {first[1][0]} x {first[1][1]}))'
    result_str = 'true' if result else 'false'
    expected = f'<k>\n  {result_str} ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual




# https://github.com/ethereum/execution-spec-tests/blob/b48d1dc81233af6e4d6c7c84e60e8eaa4067a288/tests/prague/eip2537_bls_12_381_precompiles/test_bls12_g1msm.py
VERIFYBLS12G1MSM_TEST_DATA: Final = (
    ('g1_plus_inf', [BLS12_G1, BLS12_G1_INFTY], [1, 1], BLS12_G1),
    ('all_zero_scalars', [BLS12_G1, BLS12_G1_INFTY], [0, 0], BLS12_G1_INFTY),
    ('sum_to_identity_opposite', [BLS12_G1, bls12_neg_g1(BLS12_G1)], [1, 1], BLS12_G1_INFTY),
    ('scalars_sum_to_q', [BLS12_G1, BLS12_G1], [BLS12_Q - 1, 1], BLS12_G1_INFTY),
    ('combined_basic_cases', [BLS12_G1, BLS12_G1, BLS12_G1_INFTY], [1, 0, 5], BLS12_G1),
    ('identity_with_large_scalar', [BLS12_G1, BLS12_G1_INFTY], [1, 500], BLS12_G1),
    ('multiple_points_zero_scalar', [BLS12_G1, BLS12_P1, bls12_neg_g1(BLS12_G1)], [0, 0, 0], BLS12_G1_INFTY),
    ('max_discount', [BLS12_P1] * 3, [BLS12_Q] * 3, BLS12_G1_INFTY),
)


@pytest.mark.parametrize(
    'id, first,second,result', VERIFYBLS12G1MSM_TEST_DATA, ids=[id for id, *_ in VERIFYBLS12G1MSM_TEST_DATA]
)
def test_verify_bls12G1Msm(
    definition_dir: Path,
    id: str,
    first: list[tuple[int, int]],
    second: list[int],
    result: tuple[int, int],
) -> None:
    # Given
    point_list = [f'ListItem(({x[0]} , {x[1]}))' for x in first]
    scalar_list = [f'ListItem({x})' for x in second]
    point_str = ' '.join(point_list)
    scalar_str = ' '.join(scalar_list)
    pgm = f'BLS12G1Msm( {scalar_str} , {point_str} )'

    result_str = f'( {result[0]} , {result[1]} )'
    expected = f'<k>\n  {result_str} ~> .K\n</k>'

    # When
    actual = run(definition_dir, pgm)

    # Then
    assert expected == actual
