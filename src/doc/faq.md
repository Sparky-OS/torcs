\page faq FAQ

\anchor topofthepage

\section faq_section_9 0. About this Document

\anchor c0_1
\subsection faq_c0_1 0.1 License and copyright

Copyright (c) 2004-2026 [Bernhard Wymann](mailto:berni4you@gmx.ch).

Permission is granted to copy, distribute and/or modify this document
under the terms of the GNU Free Documentation License, Version 1.2
or any later version published by the Free Software Foundation;
with no Invariant Sections, no Front-Cover Texts, and no Back-Cover
Texts. A copy of the license is available [here](http://www.gnu.org/copyleft).

\anchor c0_2
\subsection faq_c0_2 0.2 Document version

2026-03-31.

\section faq_section_10 1. About TORCS in General

\anchor c1_1
\subsection faq_c1_1 1.1 What is TORCS?

[TORCS](http://torcs.org), **The Open Racing Car Simulator**,
is a car racing simulation, which allows you to drive in races against opponents simulated by
the computer. You can also develop your own computer-controlled driver (also called a robot) in
C or C++. TORCS is "Open Source" (GNU General Public License Version 2 or later).

\anchor c1_2
\subsection faq_c1_2 1.2 Who develops TORCS?

The TORCS project has been created by Eric Espié and Christophe Guionneau. Currently Bernhard Wymann,
and other contributors continue to develop the project.

\anchor c1_3
\subsection faq_c1_3 1.3 Which features does TORCS have?

There are many different cars and tracks, and around 100 opponents to race against.
You can steer with a joystick, gamepad or steering wheel, if it is supported by your platform.
It is also possible to drive with the mouse or the keyboard. Graphic features
lighting, smoke, skid marks and glowing brake disks. The simulation features a simple damage
model, collisions, tire and wheel properties
(springs, dampers, stiffness, ...), aerodynamics (ground effect, spoilers, ...) and much more.
The gameplay allows different types of races from the simple practice session up to the championship.
Enjoy racing against your friends in the split screen mode with up to four human players.

\anchor c1_4
\subsection faq_c1_4 1.4 What do you mean by a robot?

In TORCS a "robot" is a piece of program code that drives a car, so all the opponents you
choose are robots.

\anchor c1_5
\subsection faq_c1_5 1.5 Why do you not replace GLUT with SDL?

We will switch sooner or later because of graphics context handling, portable threading, networking etc.

\anchor c1_6
\subsection faq_c1_6 1.6 Are there official championships?

Currently there are none organized by the TORCS team, but if there is interest we could start doing them again.
There might be championships organized by conferences, universities, or companies, so keep an eye out.

\anchor c1_7
\subsection faq_c1_7 1.7 Should I start my own racing sim project?

Probably not. Why? Do a search on SourceForge and common search engines,
you will find between 15 and 40 open source car/driving/racing sim
projects, and most of them have been "promising" and
died. Instead of starting another dead project contribute to the
few existing successful ones.

Believe it or not, but your project will very likely be a dead
one as well. The reason is quite simple, people underestimate the
required effort and the change of their lives during the project lifespan, think
about it. Just that you get an idea, TORCS turns now 30 in 2027.

\section faq_section_11 2. Platforms and Requirements

\anchor c2_1
\subsection faq_c2_1 2.1 Which operating systems does TORCS run on?

Linux (x86, AMD64 and ppc are known to work), Windows and FreeBSD.

\anchor c2_2
\subsection faq_c2_2 2.2 What are the hardware requirements?

\anchor c2_2_1
\subsubsection faq_c2_2_1 2.2.1 Robot development

By today's standards, almost any modern system is sufficient.

Minimum: 400MHz CPU, 128MB RAM, OpenGL 1.2 compatible graphics card with 16 MB RAM.

Recommended: 600MHz CPU, 256MB RAM, OpenGL 1.3 compatible graphics card with 64 MB RAM.

\anchor c2_2_2
\subsubsection faq_c2_2_2 2.2.2 Driving yourself

By today's standards, almost any modern system is sufficient.

Minimum: 550MHz CPU, 128MB RAM, OpenGL 1.3 compatible graphics card with 32 MB RAM.

Recommended: 800MHz CPU, 256MB RAM, OpenGL 1.3 compatible graphics card with 64 MB RAM.

\anchor c2_3
\subsection faq_c2_3 2.3 What are the software requirements?

You need recent, working and properly configured OpenGL/DRI drivers. You need also several
libraries. To avoid problems you should prefer
[FreeGLUT](http://FreeGLUT.sourceforge.net) over GLUT. Make
sure that you use exactly plib version 1.8.5. For
AMD64 you need to set the "-fPIC" compiler switch when
compiling plib (export CFLAGS="-fPIC", export
CPPFLAGS="-fPIC", export CXXFLAGS="-fPIC"). You need as well OpenAL.

\section faq_section_12 3. Installation

\anchor c3_1
\subsection faq_c3_1 3.1 How do I install TORCS?

You can find installation instructions in the README file.

\section faq_section_13 4. Problems and Solutions

\anchor c4_1
\subsection faq_c4_1 4.1 General problem solving approach

You may encounter this kind of problems: system configuration
problems, build problems and runtime problems. If you face a
problem, follow these steps:

- Have a look into this FAQ, perhaps your problem is already known
- Ask for help in community channels or use available assistant tools.
- Clean up your old TORCS installation, including old environment settings
- Read the installation instructions carefully, double check your steps
- Make sure that you have installed the latest OpenGL/DRI and sound drivers
- Test your OpenGL setup with some other application
- Have a look into the mailing list, perhaps there is already a solution
- If you get stuck learn how to [report a problem](#c5_1) and contact the mailing list

\anchor c4_2
\subsection faq_c4_2 4.2 Build Problems

\anchor c4_2_1
\subsubsection faq_c4_2_1 4.2.1 ./configure reports "configure: error: Can't find GL/glut.h"

Download and install [FreeGLUT](http://FreeGLUT.sourceforge.net)
or install GLUT which is probably included in your distribution (you need also the header files,
which may be in a separate package like glut-devel). I suggest using FreeGLUT.

\anchor c4_2_2
\subsubsection faq_c4_2_2 4.2.2 ./configure reports "configure: error: Can't find plib/ssg.h" or "plib/ul.h".

TORCS requires [plib](http://plib.sourceforge.net) 1.8.5 and its
header files. Download and install plib.

\anchor c4_2_3
\subsubsection faq_c4_2_3 4.2.3 TORCS does not compile on Windows

To compile TORCS in Windows you need Visual Studio 2022 (for the other versions you have to adapt the project
files by yourself, but this is usually pretty easy and done quickly). Follow exactly the instructions in
the README file to build a release.

\anchor c4_2_4
\subsubsection faq_c4_2_4 4.2.4 TORCS does not compile with make: 'Make-config' is up to date

You have some environment variables set like $TORCS_BASE and $MAKE_DEFAULT (to follow the
robot tutorial), they are pointing now to the wrong directory. Unset or update the variables
suitable for the new directories.

\anchor c4_2_5
\subsubsection faq_c4_2_5 4.2.5 Compiling stops with "error: `GL_TEXTURE0_ARB' undeclared".

If the compiler stops with the above message you have probably OpenGL header files installed which do
not define the GL_TEXTURE0_ARB and other symbols. To fix it for TORCS you need to install
a gl.h which defines the missing symbols. You can get it e.g. from Mesa.

\anchor c4_2_6
\subsubsection faq_c4_2_6 4.2.6 Compiling fails with "The syntax of
the command is incorrect. Error executing `c:\\winnt\\system32\\cmd.exe`"

You have probably installed your sources in a path which contains blanks or other
special characters, e.g. on the desktop. Move your torcs-1.3.9 folder to a location with a
"clean" path, a drive is the best bet (e.g. `D:\\torcs-1.3.9`).

\anchor c4_3
\subsection faq_c4_3 4.3 Startup and Runtime Problems

In several situations TORCS will crash at startup. The causes are often not bugs of TORCS, but
a broken installation or buggy OpenGL/DRI drivers.

\anchor c4_3_1
\subsubsection faq_c4_3_1 4.3.1 Broken OpenGL/DRI drivers or '~' file

```text
/usr/local/bin/torcs: line 54: 31895 Segmentation fault
$LIBDIR/torcs-bin -l $LOCAL_CONF -L $LIBDIR -D $DATADIR $*
```

There are two known possibilities which cause such a crash:

- You have buggy OpenGL/DRI driver, you should update to a recent version
- You have edited a TORCS XML file (e.g. endrace.xml) and your editor left a copy with an appended
"~" (e.g. endrace.xml~). Remove those "~" files.

\anchor c4_3_2
\subsubsection faq_c4_3_2 4.3.2 Multiple GLU libraries

```text
linuxModLoad: ...  modules/graphic/ssggraph.so:
undefined symbol: gluBuild2DMipmaps
/usr/local/bin/torcs: line 50: 21215 Segmentation fault
./torcs -l $LOCAL_CONF $*
```

You have probably two GLU libraries installed on your system and TORCS has been linked to the
wrong one, which does not export the "gluBuild2DMipmaps" symbol. First find all your GLU
libraries on the system (cd /usr; find . -name "*GLU*"). Then check which one exports the
"gluBuild2DMipmaps" symbol (e.g. nm ./lib/libGLU.so | grep gluBuild2DMipmaps). Finally move
or remove the GLU libraries which do not define the symbol, such that the linker can't see it.
Now recompile and reinstall TORCS.

\anchor c4_3_3
\subsubsection faq_c4_3_3 4.3.3 Not removed old TORCS version

```text
/usr/lib/torcs/torcs-bin: relocation
error: /usr/lib/torcs/lib/libtgfclient.so: undefined symbol:
_Z15GfParmWriteFilePKcPvPc
```

If you see such a message you have probably not cleaned up your old TORCS installation.
Make sure that you have removed all old TORCS files, then recompile and reinstall TORCS.

\anchor c4_3_4
\subsubsection faq_c4_3_4 4.3.4 TORCS reports missing files and crashes

```text
data/fonts/b5.glf: No such file or directory
data/fonts/b5.glf: No such file or directory
data/fonts/b5.glf: No such file or directory
data/fonts/b5.glf: No such file or directory
data/fonts/b7.glf: No such file or directory
data/fonts/b7.glf: No such file or directory
data/fonts/b7.glf: No such file or directory
data/fonts/b7.glf: No such file or directory
data/fonts/digital.glf: No such file or directory
Can't open file data/img/splash-main.png
Can't open file data/img/splash-single-player.png
Can't open file data/img/splash-qrdrv.png
/usr/local/bin/torcs: line 54:6839 Floating point
exception$LIBDIR/torcs-bin -l $LOCAL_CONF -L $LIBDIR -D $DATADIR $*
```

You have build TORCS from sources and forgot to run make datainstall on Linux or setup_win32-data-from-CVS.bat on Windows.

\anchor c4_3_5
\subsubsection faq_c4_3_5 4.3.5 My screen resolution is not supported.

You have a screen resolution which is not available in the selection menu of TORCS (e.g.
1152x864). Exit TORCS and edit the file .torcs/config/screen.xml, change the x, y,
window width and height to your values.

\anchor c4_3_6
\subsubsection faq_c4_3_6 4.3.6 I have a problem with the fullscreen mode

If you switched TORCS to fullscreen you get a scrolling screen or the screen size is not fully
utilized. If you use the original GLUT library you should choose the resolution of your
screen in the display settings. To fix the problem you can also download and install
FreeGLUT, then recompile and reinstall plib and TORCS.

\anchor c4_4
\subsection faq_c4_4 4.4 Other Problems

\anchor c4_4_1
\subsubsection faq_c4_4_1 4.4.1 How do I capture a movie?

First you have to configure the video capturing in config/raceengine.xml. The captured frames
are stored in the output directory, so you need to have write permission and plenty of
space (~ 1GB for one minute, depends on framerate and resolution).

During the simulation you can hit "c" to start and stop the capturing. The frames are
stored in the output directory and named this way: torcs-ssss-ffffffff.png, ssss is
the sequence number and ffffffff the frame number in the sequence.

Now you need a tool to compose the frames to a movie. If you use mencoder (of the mplayer 0.92)
you have first to convert the images to jpg's with the png2jpg tool from
[src/misc/png2jpg](http://cvs.sourceforge.net/viewcvs.py/torcs/torcs/torcs/src/misc/?only_with_tag=r1-2-2).
Now
encode the video with mencoder -ovc lavc -lavcopts vcodec=mpeg4:vhq:vbitrate=1800 \*.jpg -mf on:fps=25.

\anchor c4_4_2
\subsubsection faq_c4_4_2 4.4.2 Can I develop a robot using Windows?

Yes, you can. Here is a rough guide:

- Download the TORCS source and manage to compile and run it.
- Download e.g. the bt robot package, it is a little enhanced version of
the robot developed in the tutorial, so there is good documentation available
to understand it.
- Copy the sources into a new directory, e.g. when you choose the name "myrobot"
create a directory /src/drivers/myrobot.
- Copy all the bt files into the myrobot directory.
- Now rename all "bt*" files into "myrobot*".
- Edit myrobot.def and also change all bt strings into myrobot.
- The same for myrobot.dsp, .xml, perhaps more, you get the idea.
- Change the "bt" module entry point in myrobot.cpp to "myrobot", also make
the strings for the robots names and description match with those in myrobot.xml.
- Add the myrobot project to TORCS, open the project with vc++ and try to compile it.
- If you want to deploy the files you have to do that manually, create a
directory in "runtime" where the other robots are and copy the required files
there (myrobot.dll, .xml, setup subdirectories etc.).
- Now TORCS should pick up your robot.

\anchor c4_4_3
\subsubsection faq_c4_4_3 4.4.3 Is my joystick/steering wheel/gamepad supported?

If your operating system supports your device it will also work within TORCS. To learn how
to setup your input device visit the TORCS site and read the "how to drive" section.

\anchor c4_4_4
\subsubsection faq_c4_4_4 4.4.4 How do I set up a multiplayer game?

TORCS supports split screen multiplayer games with up to four players. Setup:

- Set up the players in the "Configure Players" screen.
- If more than one player uses the keyboard, read [this](http://www.sjbaker.org/steve/omniv/keyboards_are_evil.html) .
- Now choose a race, select the players in the configuration screen.
- Start the race, stop it with "p" to pause.
- Now create the split screens with "]", or remove them with "[".
- If you want to change the view of a "mini screen" point with the mouse on it and set it up as usual (F1-F11, ">", "<", ...).
- When you have set up all views press "p" to end the pause and start the race.
- Enjoy.

\anchor c4_4_5
\subsubsection faq_c4_4_5 4.4.5 I cannot adjust the joystick/wheel centering

Before you start TORCS make sure that the joystick/wheel is correctly set up/calibrated for the system.
You can use the "jscal" and "jstest" utilities for that, you should find them in your Linux distribution.
If this works fine center your axis (e.g. if you have noncentering axis like sliders), then start TORCS
and set up your players and controls. It is important to center the axis before you go to the calibration.

\section faq_section_14 5. Problem reporting

\anchor c5_1
\subsection faq_c5_1 5.1 When do I report a problem?

You should report a problem after you have carefully read the available documentation and did
not find a solution for the problem.

\anchor c5_2
\subsection faq_c5_2 5.2 How and where do I report a problem?

First subscribe to the [TORCS-users mailing list](http://lists.sourceforge.net/lists/listinfo/torcs-users) (otherwise your messages need to be approved, so it can take long
till they become distributed). Send us as much information about the problem as you know. At
least provide the following:

- A good description of the problem
- The TORCS version, did you compile from source?
- The console output of TORCS
- Your operating system (uname -a)
- The output of "glxinfo -l" (Linux, FreeBSD)
- The /var/log/XFree86.0.log file (Linux, FreeBSD)

Please do not file a bug in the bug reporting section on SourceForge, it is not a support area
for users (if you know for sure that you have found a bug in TORCS, then it is ok to report it
in the bug tracking system).

\section faq_section_15 6. Researchers FAQ

\anchor c6_1
\subsection faq_c6_1 6.1 How do I explore the source code?

Use a powerful IDE or IDE/editor plugins for navigating the source code (e.g. ctags, virtual assist x, etc.),
AI assistants or integrations, and set breakpoints and explore the call stack. A good development environment can speed you up a lot, so invest some time
initially for being much faster afterwards.

\anchor c6_2
\subsection faq_c6_2 6.2 Where is the physical simulation code located in the source tree?

Navigate to src/modules/simu, then either simuv2 or simuv3 (simuv2 is the default simulation core for TORCS,
simuv3 is outdated and will be removed at some point, but might be interesting for you).

\anchor c6_3
\subsection faq_c6_3 6.3 How does the simulation work?

It integrates differential equations with Euler steps, timestep is 0.002 seconds (500Hz, simulation time), look
at `src/interfaces/raceman.h` (macro `RCM_MAX_DT_SIMU`). The raceengine runs as well with this rate, but the robots are just
called all 0.02 seconds (50Hz).

\anchor c6_4
\subsection faq_c6_4 6.4 Can I record/send telemetry data somewhere?

There is no such (maintained) functionality in TORCS currently, but you can easily add it. If you are interested in
simulation data you can add some code in the raceengine or in the simulation code, if you are interested in the actions
of your robot you can add code there.

\anchor c6_5
\subsection faq_c6_5 6.5 Where is the simulation timestep performed?

The simulation timestep is performed in src/libs/raceengineclient/raceengine.cpp, ReOneStep.

\anchor c6_6
\subsection faq_c6_6 6.6 How often is the robot code called?

The robots are called all 0.02s (50Hz), do not confuse this with the physics/raceengine simulation timestep which is 0.002s (500Hz).

\anchor c6_7
\subsection faq_c6_7 6.7 Is there a command line mode for AI training?

Yes, since version 1.3.3. This mode is useful for testing as well. All you need is a race configuration xml,
the easiest way to create one is using TORCS. As example navigate in TORCS to the race configuration, select "Challenge Race",
set up a grid without human players. Then exit TORCS and run from the command line (Linux and other POSIX-like systems):

```text
~/torcs_bin/bin/torcs -r ~/.torcs/config/raceman/dtmrace.xml
```

```text
wtorcs.exe -r config/raceman/dtmrace.xml
```

Any valid race configuration file can be consumed, so you can either set it up by TORCS, edit it manually or generate it somehow.
You can think of these XML files as command files for a state engine, have a look at src/libs/raceengineclient/racestate.cpp.

\anchor c6_8
\subsection faq_c6_8 6.8 How do I extend TORCS such that the AI can restart the race?

The solution presented here is based on a discussion started by Alessandro Ghidotti Piovan on the [torcs-devel mailing list](http://lists.sourceforge.net/lists/listinfo/torcs-devel).
To feed back commands to the race state engine (src/libs/raceengineclient/racestate.cpp) add a field in the tCarCtrl struct to allow the robot to command the race state engine:

```text
typedef struct {
    ....
    ....
    int        askRestart;    /** boolean; 1 = robot asked to restart the race */
} tCarCtrl;
```

```text
bool restartRequested = false;

for (i = 0; i < s->_ncars; i++) {
    if(s->cars[i]->ctrl.askRestart) {
        restartRequested = true;
            s->cars[i]->ctrl.askRestart = false;
        }
    }
   
    if(restartRequested){   
        ReRaceCleanup();
        ReInfo->_reState = RE_STATE_PRE_RACE;
        GfuiScreenActivate(ReInfo->_reGameScreen);
    }
```

```text
if(!car->ctrl.askRestart){
       if (isStuck(car))
        {
            // as your robot tutorial and/or set car->askRestart = true if you need.
        }
    }
```

\anchor c6_9
\subsection faq_c6_9 6.9 How do I cite TORCS?

If you use TORCS for your research, please cite it as follows:

B. Wymann, E. Espié, C. Guionneau, C. Dimitrakakis, R. Coulom, A. Sumner. TORCS: The Open Racing Car Simulator, v1.3.6, 2014.

```bibtex
@misc{TORCS,
author = "Bernhard Wymann, Eric Espi{\'e}, Christophe Guionneau, Christos Dimitrakakis, R{\'e}mi Coulom, Andrew Sumner",
howpublished = "http://www.torcs.org",
title = "{TORCS}, {T}he {O}pen {R}acing {C}ar {S}imulator",
year = "2014",
}
```

\section faq_section_16 7. Links

\anchor c7_1
\subsection faq_c7_1 7.1 TORCS

- The main TORCS site [www.torcs.org](http://www.torcs.org) .
- The [TORCS-users mailing list](http://lists.sourceforge.net/lists/listinfo/torcs-users) .

\anchor c7_2
\subsection faq_c7_2 7.2 Libraries and Drivers

- FreeGLUT [freeglut.sourceforge.net](http://freeglut.sourceforge.net) .
- Plib [plib.sourceforge.net](http://plib.sourceforge.net) .
- OpenAL [www.openal.org](http://www.openal.org) .
