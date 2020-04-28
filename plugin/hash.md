Cryptographic Hashes
--------------------

``` {.k .cryptography-hashes}
module HASH
    imports STRING-SYNTAX
    imports BYTES
```

For each hash function, we support two overloads -- `String -> String` and
`Bytes -> Bytes`:

The `Bytes -> Bytes` overload returns:

- 32-byte long bytestring for `Sha2_256`
- 32-byte long bytestring for `Sha3_256`
- 32-byte long bytestring for `Keccak256`
- 20-byte long bytestring for `RipEmd160`

``` {.k .cryptography-hashes}
    syntax Bytes  ::= Keccak256 ( Bytes )       [function, hook(HASH.keccak256raw)]
                    | "Sha2_256" "(" Bytes ")"  [function, hook(HASH.sha256raw)]
                    | "Sha3_256" "(" Bytes ")"  [function, hook(HASH.sha3_256raw)]
                    | RipEmd160 ( Bytes )       [function, hook(HASH.ripemd160raw)]
```

The `String -> String` overload returns:

- 64-character hex-encoded string for `Sha2_256`
- 64-character hex-encoded string for `Sha3_256`
- 64-character hex-encoded string for `Keccak256`
- 40-character hex-encoded string for `RipEmd160`

``` {.k .cryptography-hashes}
    syntax String ::= Keccak256 ( String )      [function, hook(HASH.keccak256)]
                    | "Sha2_256" "(" String ")" [function, hook(HASH.sha256)]
                    | "Sha3_256" "(" String ")" [function, hook(HASH.sha3_256)]
                    | RipEmd160 ( String )      [function, hook(HASH.ripemd160)]
```

This is a helper function and part of the BLAKE2 hash function.

``` {.k .cryptography-hashes}
    syntax String ::= Blake2Compress ( String ) [function, hook(HASH.blake2compress)]
```

``` {.k .cryptography-hashes}
endmodule
```
