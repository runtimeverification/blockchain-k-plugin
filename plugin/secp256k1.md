```k
module CRYPTOGRAPHY-SECP256K1-ELLIPTIC-CURVE
    imports STRING-SYNTAX
    imports INT-SYNTAX
    imports LIST
```

-   `ECDSARecover` takes a 32-character byte string of a message, v, r, s of the signed message and returns the 64-character public key used to sign the message.
-   `ECDSASign` takes a 32-character byte string of a message hash, a 32-character byte string of a private key, and returns the 65 byte hex-encoded signature in [r,s,v] form
-   `ECDSAPubKey` takes a 32-character byte string of a private key, and returns the 64 byte hex-encoded public key
    See [this StackOverflow post](https://ethereum.stackexchange.com/questions/15766/what-does-v-r-s-in-eth-gettransactionbyhash-mean) for some information about v, r, and s.

```k
    syntax String ::= ECDSARecover ( String , Int , String , String ) [function, hook(SECP256K1.ecdsaRecover)]
                    | ECDSASign ( String, String )                    [function, hook(SECP256K1.ecdsaSign)]
                    | ECDSAPubKey ( String )                          [function, hook(SECP256K1.ecdsaPubKey)]
endmodule
```
