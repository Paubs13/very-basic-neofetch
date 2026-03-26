#include <stdio.h>
#include <string.h>

typedef struct{
    char name[100];
    char kernel_version[150];
    char cpu[100];
    struct{
        char usage[100];
        int usage_kilobytes;
        int usage_megabytes;
        int usage_gigabytes;
    }memory;
} os_data; os_data os;

void get_info(char *filename, char *needle, char *data, int size){
    FILE *pointer = fopen(filename, "r");
    if (pointer == NULL) {
        printf("Error Opening File \n");
        return; //This is, if for some reason the file doesn't open, it exits gracefully
    }

    while(fgets(data, size, pointer) != NULL){
        if(strstr(data, needle) != NULL){
            break;
            //To avoid overwriting, exits the loop on first match with "needle"
        }
    }
    if (fclose(pointer) == EOF){
        printf("Error Closing File");
        return;
    }
    return;
}

//I realised that the conversions between kB/mB/gB followed the same patterns as s/n/h so i made it as a function, yay
void convert_to_readable(int *small_unit, int *mid_unit, int *large_unit, int maximum_size){
    if(*small_unit >= maximum_size){
        *mid_unit = *small_unit / maximum_size;
        *small_unit = *small_unit - (*mid_unit * maximum_size);
        if(*mid_unit >= maximum_size){
            *large_unit = *mid_unit / maximum_size;
            *mid_unit = *mid_unit - (*large_unit * maximum_size);
        }
    }
}

void get_uptime(char *filename){
    char data[100];
    int hours = 0;
    int minutes = 0;
    int seconds = 0;

    FILE *pointer = fopen(filename, "r");
    if (pointer == NULL) {
        printf("Error Opening File \n");
        return; //same as the first function, probably should've separated this
    }
    fgets(data, sizeof(data), pointer);

    //convertion to readable metrics incoming
    sscanf(data, "%d", &seconds);
    convert_to_readable(&seconds, &minutes, &hours, 60);
    printf("Uptime: %d h %d m %d s \n", hours, minutes, seconds);

    if (fclose(pointer) == EOF){
        printf("Error Closing File");
        return;
    }
    return;
}


int main() {
    os.memory.usage_gigabytes = 0;
    char name_needle[12] = "PRETTY_NAME";
    char kernel_needle[14] = "Linux version";
    char cpu_needle[11] = "model name";
    char memory_needle[7] = "Active";

    //OS VERSION BLOCK - - - - - - - - - - - - - -
    get_info("/etc/os-release", name_needle, os.name, sizeof(os.name));
    char *os_name_cleanup = (strstr(os.name, "=")) + 2;
    char *os_char_toRemove = strrchr(os.name, '"');
    *os_char_toRemove = '\0';
    printf("Operating System: %s \n", os_name_cleanup);

    //KERNEL VERSION BLOCK - - - - - - - - - - -
    get_info("/proc/version", kernel_needle, os.kernel_version, sizeof(os.kernel_version));
    char *kernel_toRemovefrom = (strstr(os.kernel_version, "(")) - 1; //Best way i found to cut out the random data that follows the kernel release
    *kernel_toRemovefrom = '\0';
    char *kernel_cleanup = (strstr(os.kernel_version, "n ")) + 2; //best way i found to cut out "Linux Version"
    printf("Kernel Version: %s \n", kernel_cleanup);

    //CPU MODEL BLOCK - - - - - - - - - - - - -
    get_info("/proc/cpuinfo", cpu_needle, os.cpu, sizeof(os.cpu));
    char *cpu_cleanup = (strstr(os.cpu, ":")) + 2;
    printf("CPU model: %s", cpu_cleanup);

    //MEMORY USAGE BLOCK - - - - - - - - - - -
    get_info("/proc/meminfo", memory_needle, os.memory.usage, sizeof(os.memory.usage));
    sscanf(os.memory.usage, "Active:          %d", &os.memory.usage_kilobytes); //this extracts the kilobytes value
    convert_to_readable(&os.memory.usage_kilobytes, &os.memory.usage_megabytes, &os.memory.usage_gigabytes, 1024); //no using google to figure out your ram usage
    printf("Memory Usage: %d GB %d MB %d KB \n", os.memory.usage_gigabytes, os.memory.usage_megabytes, os.memory.usage_kilobytes);

    get_uptime("/proc/uptime"); //wow only one line for this one
    return 0;
}
