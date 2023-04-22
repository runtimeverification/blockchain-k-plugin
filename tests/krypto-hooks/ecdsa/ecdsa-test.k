requires "krypto.md"

module ECDSA-TEST
    imports INT
    imports BYTES
    imports STRING
    imports BOOL
    imports KRYPTO

    syntax String ::= Hex2Raw ( String ) [function]
 // -----------------------------------------------
    rule Hex2Raw ( S ) => #unparseByteStack( #parseByteStack ( S ) )

    syntax Int ::= #addrFromPrivateKey ( String ) [function, klabel(addrFromPrivateKey)]
 // ------------------------------------------------------------------------------------
    rule [addrFromPrivateKey]: #addrFromPrivateKey ( KEY ) => #addr( #parseHexWord( Keccak256 ( ECDSAPubKey( KEY ) ) ) )

    syntax Int ::= #parseHexWord ( String ) [function]
 // --------------------------------------------------
    rule #parseHexWord("")   => 0
    rule #parseHexWord("0x") => 0
    rule #parseHexWord(S)    => String2Base(replaceAll(S, "0x", ""), 16) requires (S =/=String "") andBool (S =/=String "0x")

    syntax Bytes ::= #parseByteStack   ( String ) [function, memo]
                   | #parseHexBytes    ( String ) [function]
                   | #parseHexBytesAux ( String ) [function]
 // --------------------------------------------------------
    rule #parseByteStack(S) => #parseHexBytes(replaceAll(S, "0x", ""))

    rule #parseHexBytes(S)  => #parseHexBytesAux(#alignHexString(S))
    rule #parseHexBytesAux("") => .Bytes
    rule #parseHexBytesAux(S)  => Int2Bytes(lengthString(S) /Int 2, String2Base(S, 16), BE)
      requires lengthString(S) >=Int 2

    syntax String ::= #alignHexString ( String ) [function, total]
 // --------------------------------------------------------------
    rule #alignHexString(S) => S             requires         lengthString(S) modInt 2 ==Int 0
    rule #alignHexString(S) => "0" +String S requires notBool lengthString(S) modInt 2 ==Int 0

    syntax String ::= #unparseByteStack ( Bytes ) [function, klabel(unparseByteStack), symbol]
 // ------------------------------------------------------------------------------------------
    rule #unparseByteStack(WS) => Bytes2String(WS)

    syntax Int ::= #addr ( Int ) [function]
 // ---------------------------------------
    rule #addr(W) => W %Word pow160

    syntax Int ::= Int "%Word" Int [function, total]
 // ------------------------------------------------
    rule  _ %Word W1 => 0            requires W1  ==Int 0
    rule W0 %Word W1 => W0 modInt W1 requires W1 =/=Int 0

    syntax Int ::= "pow160" [alias] /* 2 ^Int 160 */
 // ------------------------------------------------
    rule pow160 => 1461501637330902918203684832716283019655932542976

    configuration
       <k> $PGM:Pgm </k>

    syntax Pgm ::= Bytes | String | Int
 // -----------------------------------

endmodule
