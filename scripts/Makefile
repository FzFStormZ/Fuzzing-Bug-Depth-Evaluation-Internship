CC=g++
CXXFLAGS= -Wall -Werror -Wno-unknown-pragmas -D__PIN__=1 -DPIN_CRT=1 -fno-stack-protector -fno-exceptions -funwind-tables -fasynchronous-unwind-tables -fno-rtti -DTARGET_IA32E -DHOST_IA32E -fPIC -DTARGET_LINUX -fabi-version=2 -faligned-new -O3 -fomit-frame-pointer -fno-strict-aliasing -I../pin/source/include/pin -I../pin/source/include/pin/gen -isystem ../pin/extras/stlport/include -isystem ../pin/extras/libstdc++/include -isystem ../pin/extras/crt/include -isystem ../pin/extras/crt/include/arch-x86_64 -isystem ../pin/extras/crt/include/kernel/uapi -isystem ../pin/extras/crt/include/kernel/uapi/asm-x86 -I../pin/extras/components/include -I../pin/extras/xed-intel64/include/xed -I../pin/source/tools/Utils -I../pin/source/tools/InstLib
LDFLAGS= -shared -fabi-version=2 -Wl,--hash-style=sysv ../pin/intel64/runtime/pincrt/crtbeginS.o -Wl,-Bsymbolic -Wl,--version-script=../pin/source/include/pin/pintool.ver -L../pin/intel64/runtime/pincrt -L../pin/intel64/lib -L../pin/intel64/lib-ext -L../pin/extras/xed-intel64/lib -lpin -lxed ../pin/intel64/runtime/pincrt/crtbeginS.o -lpin3dwarf -ldl-dynamic -nostdlib -lstlport-dynamic -lm-dynamic -lc-dynamic -lunwind-dynamic
OBJDIR=build


"$(OBJDIR)/imagecountbranches.so": "$(OBJDIR)/imagecountbranches.o"
	$(CC) -shared -Wl,--hash-style=sysv ../pin/intel64/runtime/pincrt/crtbeginS.o -Wl,-Bsymbolic -Wl,--version-script=../pin/source/include/pin/pintool.ver -fabi-version=2 -o $@ $< -L../pin/intel64/runtime/pincrt -L../pin/intel64/lib -L../pin/intel64/lib-ext -L../pin/extras/xed-intel64/lib -lpin -lxed ../pin/intel64/runtime/pincrt/crtendS.o -lpin3dwarf  -ldl-dynamic -nostdlib -lstlport-dynamic -lm-dynamic -lc-dynamic -lunwind-dynamic


"$(OBJDIR)/imagecountbranches.o": imagecountbranches.cpp
	mkdir $(OBJDIR)
	$(CC) $(CXXFLAGS) -c $< -o $@
	

.PHONY: clean
clean:
	rm -rf $(OBJDIR)