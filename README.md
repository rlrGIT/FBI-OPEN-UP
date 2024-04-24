# FBI-OPEN-UP
A basic Linux rootkit targeting aarch64.

## Build
Build/clean with:
```shell
make [clean]
```
Then load it into a [test kernel](#Building-a-test-kernel) with:
```shell
sudo insmod fbi.ko
```
Track logging with:
```shell
sudo dmesg -n 7        # set log filter to allow KERN_NOTICE
sudo dmesg | tail -100 # see logs
```
Remove module with:
```shell
sudo rmmod fbi
```
Look for module with:
```shell
sudo lsmod | ack fbi
```
### Use a VM snapshot
I highly recommend taking a snapshot/backup of your VM before you modify and run modules, so that if you produce any kind of
deadlock, segfault, or bad situation, you can just restore your machine to the way it was moments before disaster.
You may not be able to `sudo rmmod` your way out of a bad enough module failure, and you may have to restart your
computer. Which, it might not be able to do, if you didn't program in fault handling.

## Testing
### Building a test kernel
Clone the [linux repository](https://github.com/torvalds/linux). I recommend just cloning the whole repo if you have time,
but `--depth=1` should be fine.

Find your current Linux kernel version with `uname -r`. You can then fetch the git tags and select 
the exact or closest version that matches `uname -r`. Before you build, install the following dependencies:
```shell
sudo apt install bc binutils bison dwarves flex gcc git make openssl pahole perl-base libssl-dev libelf-dev
```
Next, set some options for compiling your kernel.

### Kernel compiling
Currently, testing requires a custom compiled Linux kernel with several features enabled/disabled.
In order to apply these changes, run `make defconfig` (only import another .config file if you know
what you're doing) and then change this default file with `make menuconfig`. Not using menu config
may cause your changes to be discarded at compile time if your formatting is wrong or you include
an invalid option. What's worse, if you modify one option, and you compile without realizing that
option depended on other options (which `make menuconfig` would handle automatically for you) 
you will brick your build :)

Enable kprobes, and all kernel symbols:
```
KALLSYMS=y
KALLSYMS_ALL=y
KPROBES=y
```
Disable module signing, and set system trusted keys to empty:
```
MODULE_SIG=y
SYSTEM_TRUSTED_KEYS=''
```
If you need a trimmer kernel image because `/boot` is too small, disable debugging symbols and info:
```
DEBUG_INFO=n
```
This may not be enough, and you may have to delete other kernels with apt, dpkg, or rm -rf :)

If you have an encrypted file system, you will need to enable the encryption file system as a module:
```
DM_CRYPT=m
```
Because this is a module you need to make sure you run `make module_install` at some point, or `make all`.
If this fails, you may have to enable or set default encryption algorithms. This can be done by 
googling how to do that. Or ask ChatGippity. But look in the Cryptographic API tab in `make menuconfig`.

After you've completed the above, you can compile/insall the image, headers, and modules with:
```
make -j$(nproc) LOCALVERSION=-i-used-my-mouse-to-copy-this-to-clipboard
```
```
make headers_install
sudo make modules_install
sudo make install
```
Verify you have actually succeeded by calling:
```
sudo update-grub
```
and see if your kernel is listed. You should be able to reboot and choose your new kernel with grub.

### It didn't work
Collect what error messages you can, and look at the reference configs in the [kconfigs](https://github.com/rlrGIT/FBI-OPEN-UP/tree/main/kconfigs)
directory. There might be a setting that is required that wasn't mentioned explicitly, esp if you
are using a different distro.

## Current Abilites
Currently, the only supported feature is dumping the address of the `kallsyms_lookup_name` function,
which enables us to look for hijackable kernel symbols. We do not need direct exposure to the symbol
through a header file. Instead we can use build in debugger kprobes to dump the address. At a minimum, 
this code requires module signing disabled, and at least `KALLSYMS=y`, and `KPROBES=y` to
dump `kallsyms_lookup_name`. Without `KALLSYMS_ALL`, symbol exposure is limited, which means return
oriented programming or address searching may be required to hijack syscalls.

----------

### Root Kit Resources:

http://www.ouah.org/LKM_HACKING.html#I.1

https://docs.kernel.org/trace/kprobes.html

https://xcellerator.github.io/posts/linux_rootkits_11/

https://github.com/f0rb1dd3n/Reptile

https://github.com/milabs/stamina

### Kernel Memory Resources aarch64:

https://people.kernel.org/linusw/

https://people.kernel.org/linusw/how-the-arm32-kernel-starts

https://krinkinmu.github.io/2024/01/14/aarch64-virtual-memory.html

https://stackoverflow.com/questions/45216054/arm64-linux-memory-write-protection-wont-disable

### Building the Kernel

https://docs.kernel.org/kbuild/kconfig.html
