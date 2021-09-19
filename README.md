# FortKit

Fortnite SDK dumper, currently broken.

- Usage: 
  - Make a folder inside win64 dir called `DUMP` 
  - Compile on x64 and inject in fortnite process with no AC (after the login screen)


- Future plans:
  - Dump each context in a file instead of each class in a file.
  - Walk on classes's native functions before dumping the class itself and extract the offsets it uses (usually it's rcx+offset).
  - Generate DLL project file that is useable directly without any reversing skill requirements.
