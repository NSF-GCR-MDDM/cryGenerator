# cryGenerator
Code to create ROOT files of CRY events for use with external MC generators

# Instructions:
1. Download CRY from: https://nuclear.llnl.gov/simulation/
2. Compile CRY by copying the Makefile within the cryMakefile folder into the src/ folder of cry
3. Within the src folder, do a "make clean"
4. Within the main Cry folder do a "make". This should compile CRY using the same flags as ROOT was compiled with
5. Add the following to your startup script: export CRYPATH=/path/to/cry_v1.7/src
6. Run "make" from the cryGenerator main folder.
7. ./cryGenerator
