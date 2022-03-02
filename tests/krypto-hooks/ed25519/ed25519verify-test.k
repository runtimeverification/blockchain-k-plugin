requires "krypto.md"

module ED25519VERIFY-TEST
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

    syntax TSTSpecifier  ::= "TST" ":" Int
    syntax SKSpecifier   ::= "SK"  ":" HexString
    syntax PKSpecifier   ::= "PK"  ":" HexString
    syntax MSGSpecifier  ::= "MSG" ":" HexString
    syntax SIGSpecifier  ::= "SIG" ":" HexString

    syntax Test ::= TSTSpecifier SKSpecifier PKSpecifier MSGSpecifier SIGSpecifier
    syntax Tests ::= List{Test,""}

    configuration
       <k> $PGM:Tests ~> #finalizeOutput </k>
       <testNum>   0  </testNum>
       <secretKey> 0x </secretKey>
       <publicKey> 0x </publicKey>
       <message>   0x </message>
       <signature> 0x </signature>

    syntax KItem ::= "#finalizeOutput"
 // ----------------------------------
    rule <k> #finalizeOutput => . ... </k>
         <testNum>   _ => 0  </testNum>
         <secretKey> _ => 0x </secretKey>
         <publicKey> _ => 0x </publicKey>
         <message>   _ => 0x </message>
         <signature> _ => 0x </signature>

    rule <k> T:Test TS:Tests => T ~> TS ... </k>

    rule <k> TST : TESTNUM
             SK  : SECRETKEY
             PK  : PUBLICKEY
             MSG : MESSAGE
             SIG : SIGNATURE
          => #runTest
          ...
         </k>
         <testNum>   _ => TESTNUM   </testNum>
         <secretKey> _ => SECRETKEY </secretKey>
         <publicKey> _ => PUBLICKEY </publicKey>
         <message>   _ => MESSAGE   </message>
         <signature> _ => SIGNATURE </signature>

    rule <k> .Tests => . ... </k>

    syntax KItem ::= "#runTest"
 // ---------------------------
    rule <k> #runTest => ED25519VerifyMessage( Hex2Raw( PUBLICKEY ), Hex2Raw( MESSAGE ), Hex2Raw( SIGNATURE ) ) ... </k>
         <publicKey> PUBLICKEY </publicKey>
         <message>   MESSAGE   </message>
         <signature> SIGNATURE </signature>

    rule <k> true => . ... </k>

endmodule
