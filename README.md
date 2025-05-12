# cryGenerator
Code to create ROOT files of CRY events for use with external MC generators

# Instructions:
1. Download CRY from: https://nuclear.llnl.gov/simulation/
2. Compile CRY by copying the Makefile within this project's CRY/ folder into the src/ folder of cry
3. Run "make" from the main cry folder. This should compile CRY using the same flags as ROOT was compiled with
4. Add the following to your startup script: export CRYPATH=/path/to/cry_v1.7/src
5. Run "make" from the cryGenerator main folder.
