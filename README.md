# Embedded System Software

## First <br>
package install for compile(optional) <br>
``` $sudo apt-get install -y build-essential linux-headers-$(uname -r) ``` <br>

check present kernel version <br>
``` $uname -r ``` <br>

## Application Command <br>
application code compile <br>
``` $gcc -o <file_name> <file_name>.c ``` <br>

application excute <br>
``` $./<file_name> ``` <br>

## Module Command And Usable Command<br>
module compile <br>
``` $make ``` <br>

module insertion <br>
``` $sudo insmod <module_name>.ko ``` <br>

make device file <br>
``` $sudo sh mknod.sh ``` <br>

module release <br>
``` $sudo rmmod <module_name> ``` <br>

remove device file <br>
``` $sudo rm /dev/<file_name> ``` <br>

remove proc file <br>
``` $sudo rm /proc/<file_name> ``` <br>

display kernel buffer message <br>
``` $dmesg ``` <br>
``` $dmesg | tail -<number>``` <br>

remove compile created files <br>
``` $make clean ``` <br>

transfer module files to raspberry pi <br>
``` $scp <module_name>.ko pi@<raspberry IP address>:~/ ``` <br>

access local to raspberry pi <br>
``` $ssh pi@<raspberry IP address> ``` <br>

## Kernel Shark Command <br>
install kernel shark <br>
``` $sudo apt-get install kernelshark ``` <br>

record process excute pattern using kernel shark <br>
``` $trace-cmd record -e sched ./(file_name) ``` <br>

show kernel shark gui<br>
``` $kernelshark ``` <br>

## Build Cross Compile Environment In Local <br>
install cross compile tool <br>
``` $sudo apt install gcc-arm-linux-gnueabi ``` <br>
``` $sudo apt install ncurses-dev ``` <br>

decompression kernel source(firstable need kernel source!) <br>
``` $linux-rpi-4.19.97-update.tar.gz ``` <br>
``` $tar xvf linux-rpi-4.19.97-update.tar.gz -C ~ ``` <br>

ready to cross compile <br>
``` $cd ~/linux-rpi ``` <br>
``` $make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- menuconfig ``` <br>
``` Load > .config > Ok > Exit > Yes ``` <br>

if ready to cross compile cause error <br>
``` $sudo apt-get install bison ``` <br>
``` $sudo apt-get install flex ``` <br>
``` $sudo apt install libssl-dev ``` <br>

compile kernel and module <br>
``` $make j2 ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- modules_prepare ``` <br>

## Raspberry Pi Environment <br>
*Raspberry Pi 3 B+* <br>
*Raspbian 2020-02-13* <br>
*linux-rpi-4.19.97* <br>
*1GB LPDDR2* <br>
*1.2GHz Quad-Core ARM Cortex-A53* <br>
*Built-in WiFi & Bluetooth* <br>
*40 pins* <br>

## Local System Environment <br>
*Ubuntu 18.04.3 LTS* <br>
*linux-image-4.15.0-99-generic* <br>
*64 bit* <br>
*Intel® Core™ i5-7200U CPU @ 2.50GHz × 2* <br>
