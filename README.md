
Just a Simple media Player (JSPlayer)
=====================================================================
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

##Development Environment##

* **Ubuntu** 12.04 (precise) 32-bit    
* Kernel **Linux** 3.2.0-55-generic-pae  
* Intel® Core™ i5-2430M CPU @ 2.40GHz × **4**  
* Compiler : **gcc** (4.6.3) (Ubuntu/Linaro 4.6.3-1ubuntu5)  
* Debuggers : **valkyrie** (2.0.0) and **kdbg** (2.5.0)

---------------------------------------------------------------------
##Development Libraries##
  
* **ffmpeg(2.0.1)** (git 2013-09-14-8728360)

* **SDL(2.0.0)** (stable release)

* **AntTweakBar(1.16)** (stable release)

* **glibc(2.15)** (Ubuntu EGLIBC 2.15-0ubuntu10.4)
  
---------------------------------------------------------------------  

This code is written in **C**.  
A rich GUI can be created for this media player if a nice GUI library like **CEGUI** or **Qt** is used. But they are written in C++. That doesn't limit their use in our C code, but I still chose not to use them for now.  
Unfortunately, **GTK+** doesn't work so well with **SDL**. So, though it is a very attractive library to use in this project, it wasn't used. The libraries that are being used are all written in C. 

###Please be patient while installing the libraries as sometimes you may have to install other dependencies for the library to be installed.###
  
---------------------------------------------------------------------
###Installing ffmpeg dev libraries

STEP 1 : Follow these instructions :
[Compile FFmpeg on Ubuntu, Debian, or Mint](http://trac.ffmpeg.org/wiki/UbuntuCompilationGuide)

STEP 2 :
Copy all the `.pc` files from `ffmpeg_build/lib/pkgconfig`
and paste them into `/usr/local/lib/pkgconfig`.

Note : The above procedure builds a static build of ffmpeg libraries on your machine.
ffmpeg libraries are now ready to use!
---------------------------------------------------------------------
###Installing SDL2 dev libraries
Simple DirectMedia Layer is a cross-platform development library designed to provide low level access to audio, keyboard, mouse, joystick, and graphics hardware via OpenGL and Direct3D. It is used by video playback software, emulators, and many popular games.

STEP 1 :  
`sudo apt-get install build-essential xorg-dev libudev-dev libts-dev libgl1-mesa-dev libglu1-mesa-dev libasound2-dev libpulse-dev libopenal-dev libogg-dev libvorbis-dev libaudiofile-dev libpng12-dev libfreetype6-dev libusb-dev libdbus-1-dev zlib1g-dev libdirectfb-dev`

Download the [SDL2-2.0.0.tar.gz](http://www.libsdl.org/tmp/download-2.0.php)

STEP 2 :  
`tar -xvzf SDL2-2.0.0.tar.gz` 
 
STEP 3 :  
`./configure`
  
STEP 4 :  
`make`

STEP 5 :  
`sudo make install`

STEP 7 :  
`sudo ldconfig`

Note : A dynamic build of SDL libraries is built on your machine.
SDL libraries are now ready to use!
---------------------------------------------------------------------
###Installing AntTweakBar library
AntTweakBar is a small and easy-to-use C/C++ library that allows programmers to quickly add a light and intuitive graphical user interface into graphic applications based on OpenGL (compatibility and core profiles), DirectX 9, DirectX 10 or DirectX 11 to interactively tweak parameters on-screen. AntTweakBar can be easily integrated with SDL.

Follow these instructions :
[Download, Installation and Use](http://anttweakbar.sourceforge.net/doc/tools:anttweakbar:download)

This library requires some other dependencies to work properly. Please install them accordingly.

STEP 1 :  
`sudo apt-get install libX11-devel`

STEP 2 :  
`sudo apt-get install libgl1-mesa-dev`


---------------------------------------------------------------------

Never heard of ffmpeg ?
------------------------
>
* Facebook, the largest social network, handles videos from users with ffmpeg tool.

>* Google Chrome, popular web browser, uses FFmpeg libraries for HTML5 audio and video support.

>* YouTube, the biggest video sharing website, converts uploaded videos with ffmpeg.

>*  Many of VLC's codecs are provided by the libavcodec library from the FFmpeg project.

We will working with **ffmpeg libraries** and not **ffmpeg tool**.


Thanks to [dranger](http://dranger.com/ffmpeg/) for providing a well written tutorial, which has been very inspirational.

Some of the information in the following notes is <s>stolen</s> liberally borrowed from many sources and modified accordingly.

[**ffmpeg**](https://trac.ffmpeg.org/wiki/UbuntuCompilationGuide) is a wonderful library for creating video applications or even general purpose utilities. ffmpeg takes care of all the hard work of video processing by doing all the decoding, encoding, muxing and demuxing for you. This can make media applications much simpler to write. It's simple, written in C, fast, and can decode almost any codec you'll find in use today, as well as encode several other formats.

The only prolem is that documentation for the ffmpeg libraries is very scarce and not upto the mark. Here are some of the sources which can be used as an alternative for documentation.
   
* [FFmpeg documentation - Paris Hackday Code](http://code.soundsoftware.ac.uk/projects/pmhd/embedded/index.html)

* [FFmpeg documentation - Doxygen](http://ffmpeg.org/doxygen/trunk/)

* [Demuxing](http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html)
* [Using libavformat and libavcodec - Martin Böhme](http://www.inb.uni-luebeck.de/~boehme/using_libavcodec.html)

There is a sample program that comes with ffmpeg called [ffplay](https://github.com/FFmpeg/FFmpeg/blob/master/ffplay.c). It is a simple C program that implements a complete video player using ffmpeg. JSPlayer is also a simple C program that implements a video player (though not complete as of now). This player is not meant to compete with feature rich and advanced video players which have been developed over years such as ffplay, VLC player etcetera. This will be helpful mainly to understand the working of ffmpeg libraries and how to make many libraries (ffmpeg, SDL etc) work in harmony with each other.

In making the player, we will be using SDL to output the audio and video of the media file. SDL is an excellent cross-platform multimedia library that's used in MPEG playback software, emulators, and many video games. You will need to download and install the SDL development libraries for your system in order to compile the program.

This tutorial is meant for people with a decent programming background. At the very least you should know C and have some idea about concepts like queues, mutexes, and so on. You should know some basics about multimedia; things like waveforms and such, but in case you don't know much about multimedia, you can always find some quality material with the help of google. 

* [How ADC works? - Hardware Secrets](http://www.hardwaresecrets.com/article/317)
