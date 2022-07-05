# GNU-Linux-AudioPlayback
An user app to play headerless audio files (.raw) on GNU-Linux based OS.

This code is meant to play stereo 44100 Hz 16bit digital audio signal. The code might not run properly using audio files with different parameters/resolutions.

This code uses the ALSA build resources. If not installed, they can be installed by entering the following command line on terminal: 
"sudo apt install libasound2-dev"

When compiling, asoundlib resource link must be called. "-lasound".

I made this code just for fun. Don't expect professional performance from it.

Author: Rafael Sabe
