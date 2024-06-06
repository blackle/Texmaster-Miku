# Texmaster 2009: Hatsune Miku Edition

This is an LD_PRELOAD library for Texmaster 2009 that sets the background image to a random picture of Hatsune Miku. Instead of replacing the data/bg_xx.bmp files on load, it hooks onto SDL_UpperBlit and replaces the background with a random one every time it changes.

The Hatsune Miku images are fetched from safebooru using the `miku_fetcher.py` script. Those files are downscaled to 320x240 and go into a big blob file. The blob file is linked into the library as rodata and randomly indexed as needed. A constructor function calls madvise to hint to the kernel that its section will be randomly accessed (this is overkill, but an interesting usecase.)

![Screenshot of title screen](/screenshot1.png?raw=true)
![Screenshot of gameplay](/screenshot1.png?raw=true)