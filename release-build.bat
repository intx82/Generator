c:\Keil_v5\UV4\uv4 -b Gen_47-release.uvprojx -t "Generator" -o "release-build.log"
bootloader-tools\hex2bin.exe Objects\gen-release.hex
bootloader-tools\ba-fw-builder.exe -ib Objects\gen-release.bin -t dev_gen -o dev_gen
bootloader-tools\srec_cat.exe ba.hex -Intel Objects\gen-release.bin -Binary -offset 0x08001800 -o dev_gen_release.hex -Intel
pause