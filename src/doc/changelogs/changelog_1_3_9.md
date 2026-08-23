\page changelog_1_3_9 Changelog TORCS 1.3.9

## Changes since 1.3.8

- Updateded .gitignore for vscode in Linux (Bernhard).
- Add description of source code setup with vscode in Linux (Bernhard).
- Removed legacy module shutdown code from linuxspec.cpp, due to problem discovered by
  Botond István Horváth (Botond, Bernhard).
- Added changelogs to doxygen (Bernhard).
- VS 2022 compatibility, see TORCS_VS2022.sln, project files and dependencies. Beware, this will
  break the VS 2008 build, because of the updated libpng headers (Bernhard).
- Replaced GLUT with FreeGLUT in Windows (Bernhard).
- Cleanup VS 2022 build, diabled warnings 4244 and 4305 for now (Bernhard).
- Removed VS 6.0 projects and libraries (Bernhard).
- Added change to FreeGLUT to behave the same like GLUT on the F10 key. FreeGLUT does respect
  that Windows wants to open the menubar menu if TORCS runs in a window, so TORCS seems to hang
  when the button is not pressed again. See also https://github.com/freeglut/freeglut/issues/73
  (Bernhard).
- Updated ogg and vorbis libraries (Bernhard).
- Added manifest to wtorcs.exe with "Per Monitor High DPI Aware", this should fix full screen
  scaling issues with multi monitor systems and other than 100% dpi settings (Bernhard).
- Removed VS 2008 projects and libraries (Bernhard).
- Changed naming of precompiled libraries for Windows build, in preparation for having 64 bit
  libraries in a different folder, but with the same names (Bernhard).
- Renamed TORCS_VS2022.sln to TORCS.sln (because of removal of other VS builds) (Bernhard).
- Added Directory.Build.props to define $(TorcsRuntimeDir) depending on the build type,
  either runtime (Release) or runtimed (Debug), adopted post build steps (Bernhard).
- Replaced copy with xcopy in post build steps to get more verbose output (Bernhard).
- Prepared 64 bit libraries and dll's, except ALut and OpenAL, coming next (Bernhard).
- Updated OpenAL with open-soft 1.24.3, including libraries and headers for 32 bit,
  replaced ALut with FreeAlut (Bernhard).
- Built and added 64 bit version of FreeALut (Bernhard).
- Added initial 64 bit builds for Windows, select x64 as target in VS 2022. The build goes then
  as with Win32 into runtime for Release, and into runtimed for Debug (Bernhard).
- Complete rework of 155-DTM (Philipp Körber, Bernhard).
- Fix for track normal calculations in pathological cases with broken geometry, specific for
  track ole-dirt (Bernhard).
- Updated version number to 1.3.9-test1 (Bernhard).
- Added googletest 1.17.0 for future unit tests (Bernhard).
- Added proof of concept for normal and parameterized tests, see robottools_test
  project (Berhard).
- Litte refactoring of RtTrackSideNormalG (Bernhard).
- Added unittests for simuv2/susp.cpp (Bernhard).
- Added AGENTS.md and build skill for open code (Bernhard).
- Added unittests for simuv2/brake.cpp (Bernhard).
- Added unittests for simuv2/axle.cpp and simuv2/differential.cpp (Bernhard).
- Brake, added scaling of hardcoded constants with simulation timestep (Bernhard).
- Got rid of some more warnings on the x64 build (Bernhard).
- Added unittests for simuv2/engine.cpp (Bernhard).
- Added skill for test writing regarding floating point checks (Bernhard).
- Added unittests for simuv2/transmission.cpp (Bernhard).
- Added unittests for simuv2/aero.cpp (Bernhard).
- Little refactoring of simuv2/differential.cpp, collected duplictaed code into
  new applyBrakeToSpinVel function (Bernhard).
- Added unittests for simuv2/steer.cpp (Bernhard).
- Added FAQ to doxygen documentation (Bernhard).
- Wrote and added new track manual (Bernhard).
- Added new configuration to build 64 bit windows installer, it is able to detect
  and clean up old 32 bit installations (Bernhard).
- Updated version number to 1.3.9 (Bernhard).
