# Embedded System Software

## First <br>
package install for compile(optional) <br>
```bash $sudo apt-get install -y build-essential linux-headers-$(uname -r) ``` <br>
check present kernel version <br>
```bash $uname -r ``` <br>
## Module Command <br>
Module Compile <br>
```bash $make ``` <br>
Module Insertion <br>
```bash $sudo insmod (file_name).ko ``` <br>
Module Release <br>
```bash $sudo rmmod (file_name) ``` <br>
display kernel buffer message <br>
```bash $dmesg ``` <br>
remove compile created files <br>
```bash $make clean ``` <br>

*Ubuntu 18.04.3 LTS* <br>
*linux-image-4.15.0-99-generic* <br>
*64 bit* <br>
*Intel® Core™ i5-7200U CPU @ 2.50GHz × 2* <br>
