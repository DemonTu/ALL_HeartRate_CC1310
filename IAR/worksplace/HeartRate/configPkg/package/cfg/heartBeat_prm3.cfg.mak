# invoke SourceDir generated makefile for heartBeat.prm3
heartBeat.prm3: .libraries,heartBeat.prm3
.libraries,heartBeat.prm3: package/cfg/heartBeat_prm3.xdl
	$(MAKE) -f C:\work\source\ALL_HeartRate_CC1310\software\IAR\worksplace\HeartRate/src/makefile.libs

clean::
	$(MAKE) -f C:\work\source\ALL_HeartRate_CC1310\software\IAR\worksplace\HeartRate/src/makefile.libs clean

