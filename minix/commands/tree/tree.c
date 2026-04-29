#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

int comp(const void *a, const void *b) {
    return strcmp(*(char **)a, *(char **)b);
}

void walk(const char *path, int level, int *indent) {
    // check if dir can be opened, in case of permission errors or whatever
    DIR *dir = opendir(path);
    if (!dir) {
        printf(" Error: %s\n", strerror(errno));
        return;
    }

    // read dir
    struct dirent *entry;
    char **names = NULL;
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (
            strcmp(entry -> d_name, ".") == 0
            || strcmp(entry -> d_name, "..") == 0
        )
            continue;

        names = realloc(names, sizeof(char *) * (count+1));
        names[count++] = strdup(entry -> d_name);
    }
    closedir(dir);

    if (count > 0)
        qsort(names, count, sizeof(char *), comp);

    // actually print the tree, may be a separate func but it works so don't touch
    for (int i = 0; i < count; i++) {
        int is_last_entry = (i == count - 1);

        for (int j = 0; j < level; j++) {
            printf("%s    ", indent[j] ? " " : "|");
        }

        printf("%s-- %s\n", is_last_entry ? "\\--" : "+--", names[i]);

        char full_path[2048];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, names[i]);

        struct stat st;
        if (
            lstat(full_path, &st) == 0
            && S_ISDIR(st.st_mode)
        ) {
            indent[level] = is_last_entry;
            walk(full_path, level + 1, indent); // recursion is best used here
        }

        free(names[i]);
    }
    free(names);
}

int main(int argc, char *argv[]) {
    const char *start_path = (argc > 1) ? argv[1] : ".";
    int indent[128] = {0}; // let's hope this doesn't go too deep

    printf("%s\n", start_path);
    walk(start_path, 0, indent);

    return 0;
}
