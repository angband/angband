{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
      gcu_libraries = with pkgs; [
        ncurses
      ];
      sdl_sound_libraries = with pkgs; [
        SDL
        SDL_mixer
      ];
      sdl2_libaries = with pkgs; [
        SDL2
        SDL2_image
        SDL2_ttf
        libtiff
        harfbuzz
        glib
        lerc
        libsysprof-capture
        pcre2
        freetype
      ];
      sdl_libraries = with pkgs; [
        SDL
        SDL_image
        SDL_ttf
      ];

      x11_libraries = with pkgs; [
        xorg.libX11
      ];

      all_libraries =
        gcu_libraries ++ sdl_sound_libraries ++ sdl2_libaries ++ sdl_libraries ++ x11_libraries;
      build_dependencies = with pkgs; [
        autoconf
        automake
        cmake
        pkg-config
      ];
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        packages =
          with pkgs;
          [
            nushell
            gdb
          ]
          ++ all_libraries
          ++ build_dependencies;
        LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath all_libraries;
        shellHook = ''
          exec nu
        '';
      };
      packages.${system} = {
        angband-sdl = pkgs.stdenv.mkDerivation {
          name = "angband";
          src = self;
          nativeBuildInputs = build_dependencies;
          buildInputs = sdl_libraries ++ sdl_sound_libraries;
          configurePhase = ''
            cmake -B build -DSUPPORT_GCU_FRONTEND=OFF -DSUPPORT_SDL2_FRONTEND=OFF -DSUPPORT_SDL_FRONTEND=ON -DSUPPORT_X11_FRONTEND=OFF -DSUPPORT_SDL_SOUND=ON -DCMAKE_BUILD_TYPE=Release
          '';
          buildPhase = ''
            cmake --build build
          '';
          installPhase = ''
            mkdir -p $out/bin
            cp build/Angband $out/bin/angband
          '';
        };
        angband-gcu = pkgs.stdenv.mkDerivation {
          name = "angband";
          src = self;
          nativeBuildInputs = build_dependencies;
          buildInputs = gcu_libraries ++ sdl_sound_libraries;
          configurePhase = ''
            cmake -B build -DSUPPORT_GCU_FRONTEND=ON -DSUPPORT_SDL2_FRONTEND=OFF -DSUPPORT_SDL_FRONTEND=OFF -DSUPPORT_X11_FRONTEND=OFF -DSUPPORT_SDL_SOUND=ON -DCMAKE_BUILD_TYPE=Release
          '';
          buildPhase = ''
            cmake --build build
          '';
          installPhase = ''
            mkdir -p $out/bin
            cp build/Angband $out/bin/angband
          '';
        };
        angband-x11 = pkgs.stdenv.mkDerivation {
          name = "angband";
          src = self;
          nativeBuildInputs = build_dependencies;
          buildInputs = x11_libraries ++ sdl_sound_libraries;
          configurePhase = ''
            cmake -B build -DSUPPORT_GCU_FRONTEND=OFF -DSUPPORT_SDL2_FRONTEND=OFF -DSUPPORT_SDL_FRONTEND=OFF -DSUPPORT_X11_FRONTEND=ON -DSUPPORT_SDL_SOUND=ON -DCMAKE_BUILD_TYPE=Release
          '';
          buildPhase = ''
            cmake --build build
          '';
          installPhase = ''
            mkdir -p $out/bin
            cp build/Angband $out/bin/angband
          '';
        };
        angband-sdl2 = pkgs.stdenv.mkDerivation {
          name = "angband";
          src = self;
          nativeBuildInputs = build_dependencies;
          buildInputs = sdl2_libaries;
          configurePhase = ''
            cmake -B build -DSUPPORT_GCU_FRONTEND=OFF -DSUPPORT_SDL2_FRONTEND=ON -DSUPPORT_SDL_FRONTEND=OFF -DSUPPORT_X11_FRONTEND=OFF -DSUPPORT_SDL_SOUND=OFF -DCMAKE_BUILD_TYPE=Release
          '';
          buildPhase = ''
            cmake --build build
          '';
          installPhase = ''
            mkdir -p $out/bin
            cp build/Angband $out/bin/angband
          '';
        };
        angband = pkgs.stdenv.mkDerivation {
          name = "angband";
          src = self;
          nativeBuildInputs = build_dependencies;
          buildInputs = all_libraries;
          configurePhase = ''
            cmake -B build -DSUPPORT_GCU_FRONTEND=ON -DSUPPORT_SDL2_FRONTEND=OFF -DSUPPORT_SDL_FRONTEND=ON -DSUPPORT_X11_FRONTEND=ON -DSUPPORT_SDL_SOUND=ON -DCMAKE_BUILD_TYPE=Release
          '';
          buildPhase = ''
            cmake --build build
          '';
          installPhase = ''
            mkdir -p $out/bin
            cp build/Angband $out/bin/angband
          '';
        };
      };
    };
}
