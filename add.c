// add.c
 
#include <linux/module.h>
#include <linux/init.h>
#include "add.h"
 
int add(int a, int b)
{
    return a + b;
}
EXPORT_SYMBOL(add);