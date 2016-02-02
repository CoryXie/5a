
int
myaccess(char *f)
{
	return access(f, R_OK);
}

void*
mysbrk(ulong size)
{
	return sbrk(size);
}

int
mycreat(char *n, int p)
{

	return creat(n, p);
}

int
mywait(int *s)
{
	return wait(s);
}

int
mydup(int f1, int f2)
{
	return dup2(f1,f2);
}

int
mypipe(int *fd)
{
	return pipe(fd);
}

int
systemtype(int sys)
{
	return sys & Plan9;
}

int
pathchar(void)
{
	return '/';
}

char*
mygetwd(char *path, int len)
{
	return getcwd(path, len);
}

int
myexec(char *path, char *argv[])
{
	return execv(path, argv);
}

int
myfork(void)
{
	return fork();
}
