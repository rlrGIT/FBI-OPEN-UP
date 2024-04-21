#/usr/bin/zsh

# RUN IN THE LINUX SOURCE DIRECTORY

# use default config as a base
cp /boot/config-"$(uname -r)" .config

# see if this compiles -- if there is no space, the DEBUG stuff can go,
# it's not directly related to symbols, but it'll be tough to debug
# Enable with: -d DEBUG_INFO_NONE -e DEBUG_KERNEL -e DEBUG_INFO
./scripts/config --file .config -e KALLSYMS -e KALLSYMS_ALL

# disable keys and signing - we don't need verification, this will cause
# build failure if not disabled.
./scripts/config --file .config --disable MODULE_SIG
./scripts/config --file .config --set-str SYSTEM_TRUSTED_KEYS ''

# Build modules and headers (required)
make -j$(nproc) LOCALVERSION=-exposed-symbols KDEB_PKG_VERSION=$(make kernelversion)-exposed
make headers_install
sudo make modules_install

# Actual build
sudo make install

sudo update-grub
sudo dpkg --list | ack "linux-image*"
