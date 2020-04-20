{ nixpkgs ? (import ./nixpkgs.nix), ... }:
let
  pkgs = import nixpkgs {
    config = {};
    overlays = [
      (import ./overlay.nix)
    ];
  };
  decals = pkgs.callPackage ./derivation.nix {};
in {
  test = pkgs.runCommandNoCC "decals-test" {} ''
    mkdir -p $out
    : ${pkgs.decals}
  '';
}