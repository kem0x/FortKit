# FortKit

Fortnite SDK and Type Mappings dumper.

- Usage: 
  - Compile on x64 Release and inject into Fortnite without AC running. Make sure you inject after the game loads everything, like after the login screen.
  - A folder in the executing directory named "DUMP" will be created with the contents of the SDK.


- Future plans:
  - A much needed complete rewrite 
  - Dump each context in a file instead of each class in a file.
  - Walk on classes's native functions before dumping the class itself and extract the offsets it uses (usually it's rcx+offset).
  - Generate DLL project file that is useable directly without any reversing skill requirements.
  - JSON Type Mappings 
