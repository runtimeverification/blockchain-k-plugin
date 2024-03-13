{
  description = "Blockchain K plugin";

  inputs = {
    nixpkgs.url = "nixpkgs/nixos-21.11";
    flake-utils.url = "github:numtide/flake-utils";
    cpp-httplib = {
      url =
        "github:yhirose/cpp-httplib/72ce293fed9f9335e92c95ab7d085feed18c0ee8";
      flake = false;
    };
    cryptopp = {
      url = "github:weidai11/cryptopp/69bf6b53052b59ccb57ce068ce741988ae087317";
      flake = false;
    };
    libff = {
      url = "github:scipr-lab/libff/5835b8c59d4029249645cf551f417608c48f2770";
      flake = false;
    };
    ate-pairing = {
      url =
        "github:herumi/ate-pairing/e69890125746cdaf25b5b51227d96678f76479fe";
      flake = false;
    };
    xbyak = {
      url = "github:herumi/xbyak/f0a8f7faa27121f28186c2a7f4222a9fc66c283d";
      flake = false;
    };
  };
  outputs = { self, nixpkgs, flake-utils, cpp-httplib, cryptopp, libff
    , secp256k1, ate-pairing, xbyak }:
    let
      buildInputs = pkgs:
        with pkgs; [
          autoconf
          automake
          cmake
          clang
          gmp
          libtool
          openssl.dev
          pkg-config
          procps
          secp256k1
        ];

      overlay = final: prev: {
        blockchain-k-plugin-src = prev.stdenv.mkDerivation {
          name = "blockchain-k-plugin-${self.rev or "dirty"}-src";

          src = prev.lib.cleanSource (prev.nix-gitignore.gitignoreSourcePure [
            ./.gitignore
            "result*"
            "*.nix"
            "deps/*"
          ] ./.);
          dontBuild = true;
          installPhase = ''
            mkdir $out
            cp -rv $src/* $out
            chmod -R u+w $out
            mkdir -p $out/deps/cpp-httplib
            mkdir -p $out/deps/cryptopp
            mkdir -p $out/deps/libff
            mkdir -p $out/deps/secp256k1
            cp -rv ${cpp-httplib}/* $out/deps/cpp-httplib/
            cp -rv ${cryptopp}/* $out/deps/cryptopp/
            cp -rv ${libff}/* $out/deps/libff/
            chmod -R u+w $out
            cp -rv ${ate-pairing}/* $out/deps/libff/depends/ate-pairing/
            cp -rv ${xbyak}/* $out/deps/libff/depends/xbyak/
            cp -rv ${secp256k1}/* $out/deps/secp256k1/
          '';
        };

        blockchain-k-plugin = prev.stdenv.mkDerivation {
          name = "blockchain-k-plugin-${self.rev or "dirty"}";
          src = final.blockchain-k-plugin-src;
          buildInputs = buildInputs prev;
          dontUseCmakeConfigure = true;
          buildPhase = with prev; ''
            ${
              lib.strings.optionalString (stdenv.isAarch64 && stdenv.isDarwin)
              "APPLE_SILICON=true"
            } make libcryptopp libff libsecp256k1 
          '';
          installPhase = ''
            mkdir -p $out/include
            cp -rv $src/plugin-c $out/include/
            cp -rv $src/plugin $out/include/
            mkdir -p $out/lib
            cp -rv build/* $out/lib/
          '';
        };
      };
    in flake-utils.lib.eachSystem [
      "x86_64-linux"
      "x86_64-darwin"
      "aarch64-linux"
      "aarch64-darwin"
    ] (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ overlay ];
        };
      in {
        defaultPackage = pkgs.blockchain-k-plugin;
        devShell = pkgs.mkShell {
          buildInputs = buildInputs pkgs;
          shellHook = ''
            ${pkgs.lib.strings.optionalString
            (pkgs.stdenv.isAarch64 && pkgs.stdenv.isDarwin)
            "export APPLE_SILICON=true"}
          '';
        };
      }) // {
        inherit overlay;
      };
}
