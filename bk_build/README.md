# nabu Android 17 kernel build

The default entry point is `nabu_defconfig`. The scripts do not flash, reboot,
change swap, or touch a connected device.

```sh
JOBS=4 ./bk_build/build.sh
```

`build.sh` fixes AOSP Clang 20 r547379, LLVM binutils, and pahole v1.25. It
builds the kernel and bkk-control, then creates and validates separate packages from
`bk_build/anykernel`. `KERNEL_DIR`, `OUT_DIR`, `DEFCONFIG`, `CLANG_DIR`,
`GCC64_DIR`, `GCC32_DIR`, and `PAHOLE` can be overridden. `JOB` is accepted as
an alias for `JOBS`; `JOBS` takes precedence. The local defaults use the
uwuAOSP Clang and the nabu GCC prebuilts. `nabu-perf_defconfig` is not a
supported default because it does not select `CONFIG_MACH_XIAOMI_NABU`.

Package names include the kernel release suffix:
`bk-Kernel_nabu-A17-Hyper-Pan-HHMMSS.zip`. The ZIP contains only
files consumed by the kernel installer. `build-module.sh` can also independently compile
the ARM64 helpers and creates `bkk-control-1.0-HHMMSS.zip` in the same
`out/packages/` directory. Module metadata stays under `module-artifacts/`;
kernel metadata stays under `artifacts/`. Flash the kernel ZIP from recovery,
then install the module ZIP through KernelSU. Flashing a new kernel does not
replace an existing bkk-control module.

GitHub Actions uses the same script with pinned toolchains. The
`构建并发布 nabu 内核` workflow is started manually and builds the kernel and
module in one run. It uploads two workflow artifacts and publishes both ZIPs
and their SHA256 files in one GitHub Release. CI sets `DISABLE_LTO_CACHE=1`
because the ThinLTO cache exceeds the hosted runner disk budget.

The packaged `dtb` follows the HyperOS vendor_boot order: `sm8150.dtb`,
`sm8150p.dtb`, `sm8150p-v2.dtb`, and `sm8150-v2.dtb`. The package targets
`nabu` and handles boot and vendor_boot separately. Restore the boot image
matching the installed system before installing from a flashed PBRP image;
the installer rejects a boot image carrying `twrpfastboot=1`.

The package embeds the fixed PBRP 4.0 recovery ramdisk from
`PBRP-nabu-4.0-20241222-2341-UNOFFICIAL.zip`. It installs that ramdisk only to
the active boot slot. It removes a hard-coded `androidboot.force_normal_boot`
value and leaves normal/recovery selection to the nabu bootloader, matching the
stock HyperOS boot header. The source ZIP and ramdisk hashes are recorded in
`bk_build/recovery/README.md` and in every build's `build-info.txt`.

The bkk-control module runs `bk-reburnout.sh` from its service script. It applies
the nabu cpuset layout and swappiness 180. Foreground and top-app tasks retain
CPUs 0-7; only their main rendering workers and the two busiest threads use
CPUs 4-7. SystemUI transition workers and the launcher rendering, gesture, and
widget-composition threads retain their dedicated big-core affinity. It enables `Re.burnout-mode`
only after a sustained CPU/GPU load. The mode raises CPU, GPU, UFS, DDR, LLCC,
and GPU-bus performance requests, exits on sustained low load or 80 C, and
restores every saved sysfs value. Create
`/data/adb/bk-kernel/Re.burnout-mode.disabled` to disable the dynamic mode;
write `1`, `0`, or `auto` to `Re.burnout-mode.force` for validation.

The bkk-control module also configures a 1 GiB zram backing loop from Android's
`/data/per_boot` area. HyperOS can initialize zram before that encrypted path
and a free loop node are ready, so the kernel permits only the first missing
backing device to be attached later without resetting active swap. Direct I/O
and a 512 MiB initial writeback budget limit flash wear. After that budget is
used, a screen-off device can add at most 256 MiB per uptime day when the 1 GiB
backing file has space. Incompressible pages are written back when the display
turns off; normal idle pages are written back after one minute.
