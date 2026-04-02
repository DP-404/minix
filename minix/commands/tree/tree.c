#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int walk_tree(const char *path, const char *name, int depth);
static int print_entry(const char *path, const char *name, int depth);
static void print_indent(int depth);
static const char *display_name(const char *path);
static void usage(void);

int
main(int argc, char *argv[])
{
	const char *path;

	if (argc > 2)
		usage();

	path = (argc == 2) ? argv[1] : ".";

	return walk_tree(path, display_name(path), 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}

static int
walk_tree(const char *path, const char *name, int depth)
{
	struct stat st;
	DIR *dirp;
	struct dirent *dp;
	int error;

	if (lstat(path, &st) != 0) {
		perror(path);
		return 1;
	}

	if (print_entry(path, name, depth) != 0)
		return 1;

	if (!S_ISDIR(st.st_mode) || S_ISLNK(st.st_mode))
		return 0;

	dirp = opendir(path);
	if (dirp == NULL) {
		perror(path);
		return 1;
	}

	error = 0;

	while ((dp = readdir(dirp)) != NULL) {
		char child_path[PATH_MAX];
		int n;

		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
			continue;

		n = snprintf(child_path, sizeof(child_path), "%s/%s", path, dp->d_name);
		if (n < 0 || (size_t)n >= sizeof(child_path)) {
			fprintf(stderr, "%s/%s: path too long\n", path, dp->d_name);
			error = 1;
			continue;
		}

		if (walk_tree(child_path, dp->d_name, depth + 1) != 0)
			error = 1;
	}

	if (closedir(dirp) != 0) {
		perror(path);
		error = 1;
	}

	return error;
}

static int
print_entry(const char *path, const char *name, int depth)
{
	struct stat st;

	if (lstat(path, &st) != 0) {
		perror(path);
		return 1;
	}

	print_indent(depth);
	fputs(name, stdout);

	if (S_ISLNK(st.st_mode))
		fputs(" @", stdout);
	else if (S_ISDIR(st.st_mode))
		fputs("/", stdout);

	putchar('\n');

	return 0;
}

static void
print_indent(int depth)
{
	while (depth-- > 0)
		fputs("    ", stdout);
}

static const char *
display_name(const char *path)
{
	const char *slash;

	if (strcmp(path, ".") == 0)
		return path;

	slash = strrchr(path, '/');
	if (slash == NULL)
		return path;

	if (slash[1] != '\0')
		return slash + 1;

	while (slash > path && *slash == '/')
		slash--;
	while (slash > path && slash[-1] != '/')
		slash--;

	return slash;
}

static void
usage(void)
{
	fprintf(stderr, "usage: tree [path]\n");
	exit(EXIT_FAILURE);
}
