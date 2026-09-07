1. config_json.c / config_json.h
   JSON → camera_config

2. good enough error propagation
   remove all "print error → continue → return 0"

3. strict camera lifecycle model
   init → open → configure → map → queue → stream
   and deterministic cleanup
