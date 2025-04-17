{
  description = "Blockchain K plugin";

  inputs = {
    k-framework.url = "github:runtimeverification/k/v7.1.240";
    nixpkgs.follows = "k-framework/nixpkgs";
    flake-utils.follows = "k-framework/flake-utils";
    rv-utils.follows = "k-framework/rv-utils";
    poetry2nix.follows = "k-framework/poetry2nix";

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
    ckzg4844 = {
      url = "github:ethereum/c-kzg-4844/d1168fc791b910286c7308f7dd99fa566567e942";
      flake = false;
    };
    blst = {
      url = "github:supranational/blst/e99f7db0db413e2efefcfd077a4e335766f39c27";
      flake = false;
    };
  };
  outputs = { self, nixpkgs, k-framework, poetry2nix, flake-utils, rv-utils
    , cpp-httplib, cryptopp, libff, ate-pairing, xbyak
    , ckzg4844, blst, ... }@inputs:
    let
      buildInputs = pkgs:
        with pkgs; [
          autoconf
          automake
          boost.dev
          cmake
          clang
          gmp
          gmp.dev
          mpfr
          mpfr.dev
          libtool
          openssl.dev
          pkg-config
          secp256k1
          k
        ];

      overlay = final: prev:
        let poetry2nix = inputs.poetry2nix.lib.mkPoetry2Nix { pkgs = prev; };
        in {
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
              mkdir -p $out/deps/c-kzg-4844
              cp -rv ${cpp-httplib}/* $out/deps/cpp-httplib/
              cp -rv ${cryptopp}/* $out/deps/cryptopp/
              cp -rv ${libff}/* $out/deps/libff/
              cp -rv ${ckzg4844}/* $out/deps/c-kzg-4844/
              chmod -R u+w $out
              cp -rv ${ate-pairing}/* $out/deps/libff/depends/ate-pairing/
              cp -rv ${xbyak}/* $out/deps/libff/depends/xbyak/
              cp -rv ${blst}/* $out/deps/c-kzg-4844/blst/
            '';
          };

          blockchain-k-plugin = prev.stdenv.mkDerivation {
            name = "blockchain-k-plugin-${self.rev or "dirty"}";
            src = final.blockchain-k-plugin-src;
            buildInputs = buildInputs prev;
            dontUseCmakeConfigure = true;
            patchPhase = ''
              substituteInPlace Makefile \
                --replace-fail '-DNDEBUG -g2 -O3' '-DNDEBUG -g2 -O2'
            '';
            buildPhase = with prev; ''
              ${
                lib.strings.optionalString (stdenv.isAarch64 && stdenv.isDarwin)
                "APPLE_SILICON=true"
              } make -j$NIX_BUILD_CORES \
                  GMP_PREFIX=${gmp.dev} \
                  MPFR_PREFIX=${mpfr.dev} \
                  BOOST_PREFIX=${boost.dev}
            '';
            installPhase = ''
              mkdir -p $out/krypto/lib
              cp build/krypto/lib/krypto.a $out/krypto/lib

              mkdir -p $out/krypto/src/plugin
              cp plugin/krypto.md $out/krypto/src/plugin
            '';
          };

          py-krypto-tests = poetry2nix.mkPoetryApplication {
            python = prev.python310;
            projectDir = ./krypto;

            buildInputs =
              [ prev.k.openssl.procps.secp256k1 final.blockchain-k-plugin ];

            overrides = poetry2nix.overrides.withDefaults
              (finalPython: prevPython: {
                kframework = prev.pyk-python310;
                flake8-type-checking =
                  prevPython.flake8-type-checking.overridePythonAttrs (old: {
                    propagatedBuildInputs = (old.propagatedBuildInputs or [ ])
                      ++ [ finalPython.poetry ];
                  });
              });

            checkPhase = ''
              runHook preCheck

              export PATH="${prev.k.openssl.procps.secp256k1}/bin:$PATH"
              export K_PLUGIN_ROOT="${final.blockchain-k-plugin}"

              pytest src/tests                  \
                --maxfail=1                     \
                --verbose                       \
                --durations=0                   \
                --numprocesses=$NIX_BUILD_CORES \
                --dist=worksteal

              runHook postCheck
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
          overlays = [ k-framework.overlay k-framework.overlays.pyk overlay ];
        };
      in {
        defaultPackage = pkgs.blockchain-k-plugin;
        packages = { inherit (pkgs) py-krypto-tests blockchain-k-plugin; };
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
