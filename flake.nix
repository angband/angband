{
  description = "flake to build angband with different modes";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
      gcu_libraries = [ pkgs.ncurses ] ++ sdl2_sound_libraries;
      sdl_sound_libraries = with pkgs; [
        SDL
        SDL_mixer
      ];
      sdl2_sound_libraries = with pkgs; [
        SDL2
        SDL2_mixer
      ];
      sdl2_libaries =
        with pkgs;
        [
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
        ]
        ++ sdl2_sound_libraries;
      sdl_libraries =
        with pkgs;
        [
          SDL
          SDL_image
          SDL_ttf
        ]
        ++ sdl_sound_libraries;

      x11_libraries =
        with pkgs;
        [
          xorg.libX11
        ]
        ++ sdl2_sound_libraries;

      all_libraries = gcu_libraries ++ sdl2_libaries ++ x11_libraries;
      build_dependencies = with pkgs; [
        autoconf
        automake
        pkg-config
      ];

      derive_template = {
        name = "angband";
        src = self;
        nativeBuildInputs = build_dependencies ++ [ pkgs.autoreconfHook ];
        installFlags = [ "bindir=$(out)/bin" ];
      };
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
        angband-sdl = pkgs.stdenv.mkDerivation (
          derive_template
          // {
            buildInputs = sdl_libraries;
            configureFlags = [
              "--disable-curses"
              "--disable-x11"
              "--disable-sdl2"
              "--disable-sdl2-mixer"
              "--enable-sdl"
              "--enable-sdl-mixer"
              "--disable-sdl2-mixer"
            ];
          }
        );
        angband-gcu = pkgs.stdenv.mkDerivation (
          derive_template
          // {
            buildInputs = gcu_libraries;
            configureFlags = [
              "--enable-curses"
              "--disable-x11"
              "--disable-sdl2"
              "--enable-sdl2-mixer"
              "--disable-sdl"
              "--disable-sdl-mixer"
            ];
          }
        );
        angband-x11 = pkgs.stdenv.mkDerivation (
          derive_template
          // {
            buildInputs = x11_libraries;
            configureFlags = [
              "--disable-curses"
              "--enable-x11"
              "--disable-sdl2"
              "--enable-sdl2-mixer"
              "--disable-sdl"
              "--disable-sdl-mixer"
            ];
          }
        );
        angband-sdl2 = pkgs.stdenv.mkDerivation (
          derive_template
          // {
            buildInputs = sdl2_libaries;
            configureFlags = [
              "--disable-curses"
              "--disable-x11"
              "--enable-sdl2"
              "--enable-sdl2-mixer"
              "--disable-sdl"
              "--disable-sdl-mixer"
            ];
          }
        );
        angband = pkgs.stdenv.mkDerivation (
          derive_template
          // {
            buildInputs = all_libraries;
            configureFlags = [
              "--enable-curses"
              "--enable-x11"
              "--enable-sdl2"
              "--enable-sdl2-mixer"
              "--disable-sdl"
              "--disable-sdl-mixer"
            ];
          }
        );
      };
    };
}
