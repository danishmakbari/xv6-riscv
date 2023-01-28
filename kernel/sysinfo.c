#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64 sys_sysinfo(void)
{
	struct sysinfo ksi, *usi;
	struct proc *p;

	p = myproc();
	argaddr(0, (uint64 *) &usi);

	ksi.freemem = kavail();
	ksi.nproc = procnum();

	if (copyout(p->pagetable, (uint64) usi, (char *) &ksi, sizeof(ksi)) < 0) {
		return -1;
	}

	return 0;
}

