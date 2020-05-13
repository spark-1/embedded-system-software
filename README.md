# Embedded System Software

## First <br>
package install for compile(optional) <br>
``` $sudo apt-get install -y build-essential linux-headers-$(uname -r) ``` <br>
check present kernel version <br>
``` $uname -r ``` <br>
## Module Command <br>
Module Compile <br>
``` $make ``` <br>
Module Insertion <br>
``` $sudo insmod (file_name).ko ``` <br>
Module Release <br>
``` $sudo rmmod (file_name) ``` <br>
display kernel buffer message <br>
``` $dmesg ``` <br>
remove compile created files <br>
``` $make clean ``` <br>

*Ubuntu 18.04.3 LTS* <br>
*linux-image-4.15.0-99-generic* <br>
*64 bit* <br>
*Intel® Core™ i5-7200U CPU @ 2.50GHz × 2* <br>
