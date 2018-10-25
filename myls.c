#include<stdio.h>
#include<dirent.h>
#include<string.h>
#include<sys/stat.h>
#include<time.h>
#include<pwd.h>
#include<grp.h>
#include<sys/types.h>
#include<stdlib.h>

const int SIZE = 80;

struct fileInfo {
    char fileName[SIZE];
};

// Function to compare the two filenames to sort them alphabetically
int compare(const void *s1, const void *s2) {
    struct fileInfo *S1 = (struct fileInfo *)s1;
    struct fileInfo *S2 = (struct fileInfo *)s2;
    return strcmp(S1->fileName, S2->fileName);
}

// Function to get the length of size of file
int get_length(size_t size) {
    if (size < 10) {
        return 1;
    } else {
        return 1 + get_length((size / 10));
    }
}

// Function to print the file permissions based on the attributes of the file
void print_file_permissions(struct stat attr) {
    printf( (S_ISDIR(attr.st_mode)) ? "d" : "-");
    printf( (attr.st_mode & S_IRUSR) ? "r" : "-");
    printf( (attr.st_mode & S_IWUSR) ? "w" : "-");
    printf( (attr.st_mode & S_IXUSR) ? "x" : "-");
    printf( (attr.st_mode & S_IRGRP) ? "r" : "-");
    printf( (attr.st_mode & S_IWGRP) ? "w" : "-");
    printf( (attr.st_mode & S_IXGRP) ? "x" : "-");
    printf( (attr.st_mode & S_IROTH) ? "r" : "-");
    printf( (attr.st_mode & S_IWOTH) ? "w" : "-");
    printf( (attr.st_mode & S_IXOTH) ? "x" : "-");
}

int main (int argc, char **argv) { 
    struct dirent *dp;
    struct stat attr;
    struct tm *timeInfo;
    struct group *grp;
    struct passwd *pwd;
    struct fileInfo info[SIZE];
    char buffer[80];
    DIR *dirp;
    int index = 0;
    int length = 0;
    
    // Open the current directory
    dirp = opendir(".");
    
    if (dirp == NULL) {
        perror ("Error");
    }

    // Read all the directory contents
    while ((dp = readdir(dirp)) != NULL) {
        
        // Copy the first charcter to check for '.' and '..'  
        char firstChar[SIZE] = {dp->d_name[0]};
        if ((strcmp(firstChar, ".") !=0) && (strcmp(dp->d_name, "..") != 0)) {
            
            stat(dp->d_name, &attr);

            // Get the length of size of file to format the output
            int count = get_length(attr.st_size);
            if (count > length) {
                length = count;
            }
            strcpy(info[index].fileName, dp->d_name);
            
            index++;
        }
    }
    
    closedir(dirp);
    
    // Sort the files in alphabetical order
    qsort(info, index, sizeof(struct fileInfo), compare);
    
    for (int i = 0; i < index; i++) {

        // Get each file stats
        stat(info[i].fileName, &attr);

        /* Convert the last modified time into localtime and format the output time */ 
        timeInfo = localtime(&attr.st_mtime);
        strftime(buffer, 80, "%b %d %H:%M", timeInfo);
        
        // Get the user id to print the owner name
        pwd = getpwuid(attr.st_uid);

        // Get the group user id to print the group user name
        grp = getgrgid(attr.st_gid);

        // Print the file permissions and other stats of file
        print_file_permissions(attr);
        printf (" %-1ld %-1s %-1s %*ld %s %-1s\n", attr.st_nlink, pwd->pw_name, grp->gr_name, length, attr.st_size, buffer, info[i].fileName);
    }

    return 0;
}