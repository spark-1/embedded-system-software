# Embedded System Software

## First <br>
package install for compile(optional) <br>
``` $sudo apt-get install -y build-essential linux-headers-$(uname -r) ``` <br>
check present kernel version <br>
``` $uname -r ``` <br>

## Application Command <br>
application code compile <br>
``` $gcc -o (file_name) (file_name).c ``` <br>
application excute <br>
``` $./(file_name) ``` <br>

## Module Command <br>
module compile <br>
``` $make ``` <br>
module insertion <br>
``` $sudo insmod (file_name).ko ``` <br>
make device file <br>
``` $sudo sh mknod.sh ``` <br>
module release <br>
``` $sudo rmmod (file_name) ``` <br>
remove device file <br>
``` $sudo rm /dev/(file_name) ``` <br>
display kernel buffer message <br>
``` $dmesg ``` <br>
``` $dmesg | tail -(number)``` <br>
remove compile created files <br>
``` $make clean ``` <br>

## System Environment <br>
*Ubuntu 18.04.3 LTS* <br>
*linux-image-4.15.0-99-generic* <br>
*64 bit* <br>
*Intel® Core™ i5-7200U CPU @ 2.50GHz × 2* <br>
