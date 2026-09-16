### here's my TODO and current working progress:
#### observing:
1. good enough error propagation
   remove all "print error → continue → return 0"

2. strict camera lifecycle model
   init → open → configure → map → queue → stream
   and deterministic cleanup
#### implementing:
3. state machine logic for application in main.c based on app.c functions 
   +config path in argv

4. lightweight C++ daemon for converting .raw images to .png
   (lightweight sounds ridiculous considering I use OpenCV now, but I'll come up with something)

5. IPC between camera daemon and converter daemon
   the camera will tell the converter the name of the new file (path)

6. A utility for creating a custom configuration for a camera
   