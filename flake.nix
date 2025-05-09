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
      libraries = with pkgs; [
            SDL2
            SDL
            SDL2_image
            SDL2_ttf
            SDL_mixer
            freetype
            glib
            harfbuzz
            lerc
            libsysprof-capture
            libtiff
            ncurses
            pcre2
            xorg.libX11
          ];
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [
          autoconf
          automake
          nushell
          cmake
          gdb
          pkg-config
        ] ++ libraries;
        LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath libraries;
        shellHook = ''
          exec nu
        '';
      };
    };
}
