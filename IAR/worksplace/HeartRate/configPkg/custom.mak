## THIS IS A GENERATED FILE -- DO NOT EDIT
.configuro: .libraries,rm3 linker.cmd package/cfg/heartBeat_prm3.orm3

# To simplify configuro usage in makefiles:
#     o create a generic linker command file name 
#     o set modification times of compiler.opt* files to be greater than
#       or equal to the generated config header
#
linker.cmd: package/cfg/heartBeat_prm3.xdl
	$(SED) 's"^\"\(package/cfg/heartBeat_prm3cfg.cmd\)\"$""\"C:/work/source/ALL_HeartRate_CC1310/software/IAR/worksplace/HeartRate/configPkg/\1\""' package/cfg/heartBeat_prm3.xdl > $@
	-$(SETDATE) -r:max package/cfg/heartBeat_prm3.h compiler.opt compiler.opt.defs
