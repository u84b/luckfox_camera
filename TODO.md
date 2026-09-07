#### here's my TODO and current working progress:
1. config.c / config.h
   JSON → camera configuration
   but I won't create another structure like "config_json"
   it sounds useless for me now and it's more efficient to set values in camera structure from json directly

2. app.c abstraction addition (VERY IMPORTANT)

3. good enough error propagation
   remove all "print error → continue → return 0"

4. strict camera lifecycle model
   init → open → configure → map → queue → stream
   and deterministic cleanup

5. state machine logic for application in main.c based on app.c functions 
   +config path in argv

5. lightweight C++ daemon for converting .raw images to .png

6. IPC between camera daemon and converter daemon
   the camera will tell the converter the name of the new file (path)
   