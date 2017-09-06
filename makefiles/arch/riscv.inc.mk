# Target architecture for the build. 
export TARGET_ARCH ?= riscv64-unknown-elf

# define build specific options
CFLAGS_CPU   = -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -DCOREIF_NG=1
CFLAGS_LINK  = -nostartfiles
CFLAGS_DBG  ?= -g3
CFLAGS_OPT  ?= -Og

export LINKFLAGS += -L$(RIOTCPU)/$(CPU)/ldscripts
export LINKER_SCRIPT ?= $(CPU_MODEL).ld
export LINKFLAGS += -T$(LINKER_SCRIPT)

# export compiler flags
export CFLAGS += $(CFLAGS_CPU) $(CFLAGS_LINK) $(CFLAGS_DBG) $(CFLAGS_OPT)
# export assmebly flags
export ASFLAGS += $(CFLAGS_CPU) $(CFLAGS_DBG)
# export linker flags
export LINKFLAGS += $(CFLAGS_CPU) $(CFLAGS_LINK) $(CFLAGS_DBG) $(CFLAGS_OPT) -Wl,--gc-sections -static -lgcc
