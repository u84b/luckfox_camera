### here's my TODO and current working progress:
#### observing:
1. strict camera lifecycle model
   init → open → configure → map → queue → stream
   and deterministic cleanup
#### implementing:
2. state machine logic for application in main.c based on app.c functions 
   +config path in argv

3. **lightweight C++ daemon for converting .raw images to .png
   (lightweight sounds ridiculous considering I use OpenCV now, but I'll come up with something)**

4. **IPC between camera daemon and converter daemon
   the camera will tell the converter the name of the new file (path)**

5. A utility for creating a custom configuration for a camera


### NOTE:
### bold text is current tasks