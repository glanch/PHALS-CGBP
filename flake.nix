{
  inputs = {
    nixpkgs = {
      url = "github:nixos/nixpkgs/nixos-unstable";
    };
    flake-utils = {
      url = "github:numtide/flake-utils";
    };
  };
  outputs = { nixpkgs, flake-utils, nur, ... }: flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs {
        inherit system;
      };
      scipoptsuite = (with pkgs; stdenv.mkDerivation {
        pname = "scip";
        version = "8.0.3";
        src = fetchTarball {
          url = "https://scipopt.org/download/release/scipoptsuite-8.0.3.tgz";
          sha256 = "1yf8nzixl8bhpvg656lg9183sv25y0xgqi3v50101qg1iwh3mh2i";
        };
        nativeBuildInputs = [
          clang
          cmake
          zlib
          readline
          gmp
          pkgconfig
          boost
          tbb_2021_8
          ipopt
        ];
        buildPhase = "make -j 4";
        installPhase = ''
          make install 
        '';
      }

      );


    in
    rec {
      defaultApp = flake-utils.lib.mkApp {
        drv = defaultPackage;
      };
      defaultPackage = scipoptsuite;
      devShell = pkgs.mkShell {
        buildInputs = with pkgs; [
          scipoptsuite
          gnumake
          pkg-config
          gdb
          cmake
          clang-tools_15
          valgrind
        ];

      };
    }
  );
}
