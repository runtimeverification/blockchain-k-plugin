Cryptographic Primitives
========================

Here we implement the various cryptographic primitives needed for various blockchain systems.

```k
module KRYPTO
    imports STRING-SYNTAX
    imports INT-SYNTAX
    imports LIST
```

Throughout,
-   `String` values are interpreted as hex-encoded data
-   `Bytes` are interpreted directly as raw byte strings, i.e. they are NOT hex-encoded

Hash Functions
--------------

For hooked hash functions, we provide both hex-encoded and raw versions of each function.

### Hooked Hash Functions with Hex Encoded Outputs

-   `Keccak256` takes a byte string and returns a 64-character hex-encoded string of the 32-byte keccak256 hash of the input.
-   `Sha256` takes a byte string and returns a 64-character hex-encoded string of the 32-byte SHA2-256 hash of the input.
-   `Sha512` takes a byte string and returns a 128-character hex-encoded string of the 64-byte SHA2-512 hash of the input.
-   `Sha3_256` takes a byte string and returns 64-character hex-encoded string of the 32-byte SHA3-256 hash of the input.
-   `RipEmd160` takes a byte string and returns a 40-character hex-encoded string of the 20-byte RIPEMD160 hash of the input.
-   `Blake2Compress` takes a byte string containing a packed encoding of the arguments to the Blake2b compression
    function F and returns a 128-character hex-encoded string of the 64-byte result from the compression function.

```k
    syntax String ::= Keccak256 ( Bytes )                         [function, hook(KRYPTO.keccak256)]
                    | Sha256 ( Bytes )                            [function, hook(KRYPTO.sha256)]
                    | Sha512 ( Bytes )                            [function, hook(KRYPTO.sha512)]
                    | "Sha512_256" "(" Bytes ")"                  [function, hook(KRYPTO.sha512_256)]
                    | "Sha3_256" "(" Bytes ")"                    [function, hook(KRYPTO.sha3)]
                    | RipEmd160 ( Bytes )                         [function, hook(KRYPTO.ripemd160)]
                    | Blake2Compress ( Bytes )                    [function, hook(KRYPTO.blake2compress)]
 // ---------------------------------------------------------------------------------------------------------
```

### Hooked Hash Functions with Raw Outputs

These functions compute the same hash function as those named above except that they return a raw byte string.

```k
    syntax Bytes ::= Keccak256raw ( Bytes )                       [function, hook(KRYPTO.keccak256raw)]
                    | Sha256raw ( Bytes )                         [function, hook(KRYPTO.sha256raw)]
                    | Sha512raw ( Bytes )                         [function, hook(KRYPTO.sha512raw)]
                    | "Sha512_256raw" "(" Bytes ")"               [function, hook(KRYPTO.sha512_256raw)]
                    | "Sha3_256raw" "(" Bytes ")"                 [function, hook(KRYPTO.sha3raw)]
                    | RipEmd160raw ( Bytes )                      [function, hook(KRYPTO.ripemd160raw)]
 // -------------------------------------------------------------------------------------------------------
```

### Other Hooked Hash Functions

These hooked hash functions are broken on default Ubuntu installations because of either a bug in `libcrypto++` package, or it isn't in that version of the library.
As such, we we hide them behind a separate tangle tag to avoid breaking projects that do not use them.

-   `Blake2b256` takes a byte string and returns a 64-character hex-encoded string of the 32-byte Blake2b256 hash of the input..
-   `Blake2b256raw` takes a byte string and returns the 32-byte Blake2b256 hash of the input.
-   `ED25519VerifyMessage` takes a public key, a message, and a signature and returns true if the message was signed by the private key.

```libcrypto-extra
    syntax String ::= Blake2b256 ( Bytes )                        [function, hook(KRYPTO.blake2b256)]
    syntax Bytes  ::= Blake2b256raw ( Bytes )                     [function, hook(KRYPTO.blake2b256raw)]
    syntax Bool   ::= ED25519VerifyMessage( Bytes, Bytes, Bytes ) [function, hook(KRYPTO.ed25519verify)]
 // --------------------------------------------------------------------------------------------------------
```

ECDSA Functions
---------------

For hooked ECDSA functions,

-   `ECDSARecover` takes a 32-character byte string of a message, v, r, s of the signed message and returns the 64-character public key used to sign the message.
-   `ECDSASign` takes a 32-character byte string of a message hash, a 32-character byte string of a private key, and returns the 65 byte hex-encoded signature in [r,s,v] form
-   `ECDSAPubKey` takes a 32-character byte string of a private key, and returns the 64 byte hex-encoded public key
    See [this StackOverflow post](https://ethereum.stackexchange.com/questions/15766/what-does-v-r-s-in-eth-gettransactionbyhash-mean) for some information about v, r, and s.

```k
    syntax Bytes  ::= ECDSARecover ( Bytes , Int , Bytes , Bytes ) [function, hook(KRYPTO.ecdsaRecover)]
    syntax String ::= ECDSASign ( Bytes, Bytes )                   [function, hook(KRYPTO.ecdsaSign)]
                    | ECDSAPubKey ( Bytes )                        [function, hook(KRYPTO.ecdsaPubKey)]
 // ---------------------------------------------------------------------------------------------------------
```

BN128 Curve Functions
---------------------

The BN128 elliptic curve is defined over 2-dimensional points over the fields of zero- and first-degree polynomials modulo a large prime. (x, y) is a point on G1, whereas (x1 x x2, y1 x y2) is a point on G2, in which x1 and y1 are zero-degree coefficients and x2 and y2 are first-degree coefficients. In each case, (0, 0) is used to represent the point at infinity.

-   `BN128Add` adds two points in G1 together,
-   `BN128Mul` multiplies a point in G1 by a scalar.
-   `BN128AtePairing` accepts a list of points in G1 and a list of points in G2 and returns whether the sum of the product of the discrete logarithm of the G1 points multiplied by the discrete logarithm of the G2 points is equal to zero.
-   `isValidPoint` takes a point in either G1 or G2 and validates that it actually falls on the respective elliptic curve.

```k
    syntax G1Point ::= "(" Int "," Int ")" [prefer]
    syntax G2Point ::= "(" Int "x" Int "," Int "x" Int ")"
    syntax G1Point ::= BN128Add(G1Point, G1Point) [function, hook(KRYPTO.bn128add)]
                     | BN128Mul(G1Point, Int)     [function, hook(KRYPTO.bn128mul)]
 // -------------------------------------------------------------------------------

    syntax Bool ::= BN128AtePairing(List, List) [function, hook(KRYPTO.bn128ate)]
 // -----------------------------------------------------------------------------

    syntax Bool ::= isValidPoint(G1Point) [function, hook(KRYPTO.bn128valid)]
                  | isValidPoint(G2Point) [function, klabel(isValidG2Point), hook(KRYPTO.bn128g2valid)]
 // ---------------------------------------------------------------------------------------------------
endmodule
```
