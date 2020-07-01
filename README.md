# Linux Kernel Development

Linux provides a powerful and expansive API for applications, but sometimes that’s not enough. Interacting with a piece of hardware or conducting operations that require accessed to privileged information in the system require a kernel module.

<br/>

```bash
# My Environment Info.

$ cat /etc/redhat-release
CentOS Linux release 8.1.1911 (Core)

$ cat /proc/version
Linux version 4.18.0-147.8.1.el8_1.x86_64 (mockbuild@kbuilder.bsys.centos.org) (gcc version 8.3.1 20190507 (Red Hat 8.3.1-4) (GCC)) #1 SMP Thu Apr 9 13:49:54 UTC 2020
```

<br/>

Traditional application development paradigms can be largely discarded. Other than loading and unloading of your module, you’ll be writing code that responds to system events rather than operates in a sequential pattern. With kernel development, you’re writing APIs, not applications themselves.

You also have no access to the standard library. While the kernel provides some functions like printk (which serves as a replacement for printf) and kmalloc (which operates in a similar fashion to malloc), you are largely left to your own devices. Additionally, when your module unloads, you are responsible for completely cleaning up after yourself. There is no garbage collection.

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

### lkm_example

```bash
$ sudo yum install elfutils-libelf-devel
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