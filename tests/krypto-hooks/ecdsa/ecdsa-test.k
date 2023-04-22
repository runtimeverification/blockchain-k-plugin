requires "krypto.md"

module ECDSA-TEST
    imports INT
    imports BYTES
    imports STRING
    imports BOOL
    imports KRYPTO

    syntax HexString ::= r"0x[0-9a-fA-F]*" [token]

    syntax String ::= HexToken2String( HexString ) [function, hook(STRING.token2string)]
                    | Hex2Raw( HexString )         [function]
                    | Hex2RawAux( String )         [function]
 // ------------------------------------------------------------------------------------
    rule Hex2Raw( HS ) => Hex2RawAux(replaceAll(HexToken2String(HS), "0x", ""))
    rule Hex2RawAux( "" ) => ""
    rule Hex2RawAux( S ) => Bytes2String( Int2Bytes( lengthString( S ) /Int 2, String2Base( S, 16 ), BE ) ) [owise]

    syntax Int ::= #addrFromPrivateKey ( String ) [function, klabel(addrFromPrivateKey)]
 // ------------------------------------------------------------------------------------
    rule [addrFromPrivateKey]: #addrFromPrivateKey ( KEY ) => #addr( #parseHexWord( Keccak256 ( Hex2Raw( ECDSAPubKey( Hex2Raw( KEY ) ) ) ) ) )

    syntax Int ::= #parseHexWord ( String ) [function]
 // --------------------------------------------------
    rule #parseHexWord("")   => 0
    rule #parseHexWord("0x") => 0
    rule #parseHexWord(S)    => String2Base(replaceAll(S, "0x", ""), 16) requires (S =/=String "") andBool (S =/=String "0x")

    syntax String ::= #unparseByteStack ( Bytes ) [function, klabel(unparseByteStack), symbol]
 // ------------------------------------------------------------------------------------------
    rule #unparseByteStack(WS) => Bytes2String(WS)

    configuration
       <k> $PGM:Pgm </k>

    syntax Pgm ::= Bytes | String | Int
 // -----------------------------------

endmodule
