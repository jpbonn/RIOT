#!/bin/sh

# default path to OpenOCD configuration file
OPENOCD_CONFIG=${RIOTBOARD}/${BOARD}/dist/sifive-freedom-e300-hifive1.cfg

# ELF file to flash
ELFFILE='C:\\Projects\\RIOT\\examples\\hello-world\\bin\\hifive1\\hello-world.elf'

echo "### Flashing Target ###"
echo "${ELFFILE}"
echo

openocd -f ${OPENOCD_CONFIG} \
        -c "reset halt" \
        -c "flash write_image erase ${ELFFILE}" \
        -c "verify_image ${ELFFILE}"


