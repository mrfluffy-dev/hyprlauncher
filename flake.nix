{
  description = "A Nix-flake-based C/C++ development environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    systems.url = "github:nix-systems/default-linux";

    hyprtoolkit = {
      url = "github:hyprwm/hyprtoolkit";
      inputs.nixpkgs.follows = "nixpkgs";
    };

    aquamarine = {
      url = "github:hyprwm/aquamarine";
      inputs = {
        nixpkgs.follows = "nixpkgs";
        systems.follows = "systems";
      };
    };

    hyprgraphics = {
      url = "github:hyprwm/hyprgraphics";
      inputs = {
        nixpkgs.follows = "nixpkgs";
        systems.follows = "systems";
      };
    };

    hyprutils = {
      url = "github:hyprwm/hyprutils";
      inputs = {
        nixpkgs.follows = "nixpkgs";
        systems.follows = "systems";
      };
    };

    hyprlang = {
      url = "github:hyprwm/hyprlang";
      inputs = {
        hyprutils.follows = "hyprutils";
        nixpkgs.follows = "nixpkgs";
        systems.follows = "systems";
      };
    };

    # ✨ new: hyprwire
    hyprwire = {
      url = "github:hyprwm/hyprwire";
      inputs = {
        nixpkgs.follows = "nixpkgs";
        systems.follows = "systems";
      };
    };
  };

  outputs = { self, ... }@inputs:
  let
    supportedSystems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
    forEachSupportedSystem = f:
      inputs.nixpkgs.lib.genAttrs supportedSystems (system:
        f { pkgs = import inputs.nixpkgs { inherit system; }; });
  in {
    ###### devShells (unchanged) ################################################
    devShells = forEachSupportedSystem ({ pkgs }:
    let
      linuxOnly = with pkgs; pkgs.lib.optionals pkgs.stdenv.isLinux [ pixman libdrm ];
    in {
      default = pkgs.mkShell {
        packages =
          with pkgs; [
            # toolchain / tooling
            clang-tools
            cmake
            pkg-config
            codespell
            conan
            cppcheck
            doxygen
            gtest
            lcov
            vcpkg
            vcpkg-tool
            cairo

            # libs CMake is asking for (provide .pc files)
            icu
            libqalculate
            libxkbcommon

            # hypr* deps as flake inputs (export their dev outputs)
            inputs.hyprutils.packages.${pkgs.system}.default
            inputs.hyprwire.packages.${pkgs.system}.default
            inputs.hyprtoolkit.packages.${pkgs.system}.default
            inputs.hyprlang.packages.${pkgs.system}.default
            inputs.aquamarine.packages.${pkgs.system}.default
            inputs.hyprgraphics.packages.${pkgs.system}.default
          ]
          ++ linuxOnly
          ++ (pkgs.lib.optionals (!pkgs.stdenv.isDarwin) [ gdb ]);
      };
    });

    ###### packages: build hyprlauncher ########################################
    packages = forEachSupportedSystem ({ pkgs }:
    let
      lib = pkgs.lib;
      stdenv = pkgs.stdenv;
      linuxOnly = lib.optionals stdenv.isLinux [ pkgs.pixman pkgs.libdrm ];
    in {
      default = stdenv.mkDerivation {
        pname = "hyprlauncher";
        version = "unstable-${self.shortRev or "dirty"}";
        src = ./.;

        nativeBuildInputs = [
          pkgs.cmake
          pkgs.pkg-config
          pkgs.ninja
        ];

        buildInputs =
          [
            pkgs.cairo
            pkgs.icu
            pkgs.libqalculate
            pkgs.libxkbcommon

            # hypr* deps from flake inputs
            inputs.hyprutils.packages.${pkgs.system}.default
            inputs.hyprwire.packages.${pkgs.system}.default
            inputs.hyprtoolkit.packages.${pkgs.system}.default
            inputs.hyprlang.packages.${pkgs.system}.default
            inputs.aquamarine.packages.${pkgs.system}.default
            inputs.hyprgraphics.packages.${pkgs.system}.default
          ] ++ linuxOnly;

        # Use out-of-source build; your CMake puts the binary at project root,
        # but in Nix we'll grab it from the build dir.
        cmakeFlags = [ "-DCMAKE_BUILD_TYPE=Release" ];
        enableParallelBuilding = true;

        # The default cmake phases: cmakeConfigurePhase → cmakeBuildPhase
        # Install the produced binary named 'hyprlauncher'
        installPhase = ''
          runHook preInstall
          install -Dm755 hyprlauncher "$out/bin/hyprlauncher"
          runHook postInstall
        '';

        meta = with lib; {
          description = "HyprLauncher built via Nix";
          homepage = "https://github.com/hyprwm";
          license = licenses.bsd3; # change if needed
          mainProgram = "hyprlauncher";
          platforms = supportedSystems;
        };
      };
    });

    ###### apps: nix run ########################################################
    apps = forEachSupportedSystem ({ pkgs }: {
      default = {
        type = "app";
        program = "${self.packages.${pkgs.system}.default}/bin/hyprlauncher";
      };
    });
  };
}
