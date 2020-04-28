#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
asmlinkage void sys_print_message(char *mesg){
	printk("%s\n",mesg);
}
