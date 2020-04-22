{ stdenv, darwin, fetchFromGitHub, ... }:

stdenv.mkDerivation rec {
  pname = "decals";
  version = "0.1.0";

  src = ./.;

  buildInputs = with darwin.apple_sdk.frameworks; [
    Cocoa
    CoreFoundation
    darwin.libobjc
  ];

  makeFlags = [ "prefix=${placeholder "out"}" ];

  meta = with stdenv.lib; {
    description = "TODO: fill me in";
    homepage = https://github.com/eraserhd/decals;
    license = licenses.publicDomain;
    platforms = platforms.all;
    maintainers = [ maintainers.eraserhd ];
  };
}
