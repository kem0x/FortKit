# FortKit

Fortnite SDK dumper.

- Usage: 
  - Compile on x64 (preferly release for speed's sake) and inject in fortnite process with no AC.
  - It will output `Mappings.usmap` and `DUMP` folder in fortnite's win64 folder.


- Future plans:
  - Dump each context in a file instead of each class in a file.
  - Walk on classes's native functions before dumping the class itself and extract the offsets it uses (usually it's rcx+offset).
  - Generate DLL project file that is useable directly without any reversing skill requirements.
