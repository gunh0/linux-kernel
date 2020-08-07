# Linux Kernel Development

Linux provides a powerful and expansive API for applications, but sometimes that’s not enough. Interacting with a piece of hardware or conducting operations that require accessed to privileged information in the system require a kernel module.

<br/>

```bash
# My Environment Info.

$ cat /etc/redhat-release
CentOS Linux release 8.1.1911 (Core)

$ cat /proc/version
# 1st
Linux version 4.18.0-147.8.1.el8_1.x86_64 (mockbuild@kbuilder.bsys.centos.org) (gcc version 8.3.1 20190507 (Red Hat 8.3.1-4) (GCC)) #1 SMP Thu Apr 9 13:49:54 UTC 2020

# 2nd
Linux version 3.10.0-1127.13.1.el7.x86_64 (mockbuild@kbuilder.bsys.centos.org) (gcc version 4.8.5 20150623 (Red Hat 4.8.5-39) (GCC) ) #1 SMP Tue Jun 23 15:46:38 UTC 2020

# 3th
Linux version 3.10.0-1062.el7.x86_64 (mockbuild@kbuilder.bsys.centos.org) (gcc version 4.8.5 20150623 (Red Hat 4.8.5-36) (GCC) ) #1 SMP Wed Aug 7 18:08:02 UTC 2019

$ sudo timedatectl set-timezone Asia/Seoul
```

<br/>

Traditional application development paradigms can be largely discarded. Other than loading and unloading of your module, you’ll be writing code that responds to system events rather than operates in a sequential pattern. With kernel development, you’re writing APIs, not applications themselves.

You also have no access to the standard library. While the kernel provides some functions like printk (which serves as a replacement for printf) and kmalloc (which operates in a similar fashion to malloc), you are largely left to your own devices. Additionally, when your module unloads, you are responsible for completely cleaning up after yourself. There is no garbage collection.

<br/>

<br/>

### VI environment setting for development

```bash
vi /root/.vimrc
```

or

```bash
# The .vimrc file must always exist in the home directory, so go to the home directory to create the .vimrc.

cd ~
vim .vimrc
```

<br/>

`.vimrc`	 // my custom

```
syntax on
set hlsearch
set nu
set autoindent
set ts=4
set sts=4
set cindent
set laststatus=2
set shiftwidth=4
set showmatch
set smartcase
set smarttab
set smartindent
set ruler
set fileencodings=utf8,euc-kr
set nobackup
filetype on
set background=dark
colorscheme delek
set history=1000	"VI edit history up to 1,000"
set hlsearch	"Enable search keyword highlighting"
```

<br/>

<br/>

### Directories in the Root of the Kernel Source Tree 

| Directory     | Description                                   |
| ------------- | --------------------------------------------- |
| arch          | Architecture-specific source                  |
| block         | Block I/O layer                               |
| crypto        | Crypto API                                    |
| Documentation | Kernel source documentation                   |
| drivers       | Device drivers                                |
| firmware      | Device firmware needed to use certain drivers |
| fs            | The VFS and the individual filesystems        |
| include       | Kernel headers                                |
| init          | Kernel boot and initialization                |
| ipc           | Interprocess communication code               |
| kernel        | Core subsystems, such as the scheduler        |
| lib           | Helper routines                               |
| mm            | Memory management subsystem and the VM        |
| net           | Networking subsystem                          |
| samples       | Sample, demonstrative code                    |
| scripts       | Scripts used to build the kernel              |
| security      | Linux Security Module                         |
| sound         | Sound subsystem                               |
| usr           | Early user-space code (called initramfs)      |
| tools         | Tools helpful for developing Linux            |
| virt          | Virtualization infrastructure                 |

<br/>

<br/>

##### To be able to perform a successful kernel build, you will need to install the following packages:

- **yum groupinstall "Development Tools"**
- **yum install ncurses-devel**
- **yum install qt3-devel** (This is only necessary if you wish to use *make xconfig* instead of *make gconfig* or *make menuconfig*.)
- **yum install hmaccalc zlib-devel binutils-devel elfutils-libelf-devel**

<br/>

<br/>

### Basic command in Linux system

``` bash
$ lsmod
# lsmod is a command on Linux systems. It shows which loadable kernel modules are currently loaded.

$ lsmod | grep ABC
# To find out whether a specific module is loaded, filter the output with grep.

$ modinfo [-0] [-F field] [-k kernel] [modulename|filename...]
# modinfo command in Linux system is used to display the information about a Linux Kernel module.
```

<br/>

<br/>

## Done List

- lkm_example
- make_device_file

<br/>

<br/>

### lkm_example

```bash
$ sudo yum install elfutils-libelf-devel kernel-devel
# The elfutils-libelf-devel package contains the libraries to create applications for handling compiled objects. libelf allows you to access the internals of the ELF object file format, so you can see the different sections of an ELF file.

$ sudo make
# It should compile your module successfully.

$ sudo insmod lkm_example.ko
# If all goes well, you won’t see a thing. The printk function doesn’t output to the console but rather the kernel log.

$ sudo dmesg
# You should see the message line prefixed by a timestamp.

$ lsmod | grep "lkm_example"
# We can also check to see if the module is still loaded.

$ sudo rmmod lkm_example
# Remove the module

$ sudo make clean
# to delete the executable file and all the object files from the directory
```

<br/>

To test our module and see the output of the kernel log without having to run separate commands

```bash
$ make test
```

<br/>

<br/>

### make_device_file

Since applications are restricted from viewing the contents of kernel space memory, applications must use an API to communicate with them.

While there are technically multiple ways to accomplish this, the most common is to create a device file.

```bash
$ make test
# Run “make test”, you’ll see the output of the device’s major number. 
```

Now when you run “make test”, you’ll see the output of the device’s major number. In our example, this is automatically assigned by the kernel. However, you’ll need this value to create the device.

Take the value you obtain from “make test” and use it to create a device file so that we can communicate with our kernel module from user space.

```bash
$ sudo mknod /dev/lkm_example c "MAJOR NUMBER" 0

$ cat /dev/lkm_example
# greb content from the device.
```

When we’re done with the device, delete it and unload the module.

```bash
$ sudo rm /dev/lkm_example
$ sudo rmmod lkm_example
```

<br/>

<br/>