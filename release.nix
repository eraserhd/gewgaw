{ nixpkgs ? (import ./nixpkgs.nix), ... }:
let
  pkgs = import nixpkgs {
    config = {};
    overlays = [
      (import ./overlay.nix)
    ];
  };
  gewgaw = pkgs.callPackage ./derivation.nix {};
in {
  test = pkgs.runCommandNoCC "gewgaw-test" {} ''
    mkdir -p $out
    : ${pkgs.gewgaw}
  '';
}
