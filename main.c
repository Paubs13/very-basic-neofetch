#include <stdio.h>
#include <string.h>
#define DFL "\033[0m" //Default Colour
#define GRN "\e[0;32m" // Green
#define MAG "\e[0;35m" // Magenta
#define BCYN "\e[1;36m" // Bold Cyan

typedef struct{
    char hostname[50];
    char name[100];
    char kernel_version[150];
    char cpu[100];
    struct{
        struct{
            char RAW[100];
            int kB;
            int mB;
            int gB;
        }total;
        struct{
            char RAW[100];
            int kB;
            int mB;
            int gB;
        }usage;
    }memory;
    struct{
        char RAW[100];
        int seconds;
        int minutes;
        int hours;
    }uptime;
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

void get_everything(char *filename, char *data, int size){
    FILE *pointer = fopen(filename, "r");
    if (pointer == NULL) {
        printf("Error Opening File \n");
        return; //same as the first function, probably should've separated this
    }
    fgets(data, size, pointer);

    if (fclose(pointer) == EOF){
        printf("Error Closing File");
        return;
    }
    return;
}


int main() {
    char name_needle[12] = "PRETTY_NAME";
    char kernel_needle[14] = "Linux version";
    char cpu_needle[11] = "model name";
    char total_memory_needle[9] = "MemTotal";
    char memory_usage_needle[7] = "Active";

    // HOSTNAME BLOCK - - - - - - - - - - - - - - - - - - -
    get_everything("/etc/hostname", os.hostname, sizeof(os.hostname));
    printf(BCYN "Hostname: " MAG "%s", os.hostname);

    // OS VERSION BLOCK - - - - - - - - - - - - - - - - - -
    get_info("/etc/os-release", name_needle, os.name, sizeof(os.name));
    char *os_name_cleanup = (strstr(os.name, "=")) + 2;
    char *os_char_toRemove = strrchr(os.name, '"');
    *os_char_toRemove = '\0';
    printf(BCYN "Operating System: " MAG "%s \n", os_name_cleanup);

    // KERNEL VERSION BLOCK - - - - - - - - - - - - - - - -
    get_info("/proc/version", kernel_needle, os.kernel_version, sizeof(os.kernel_version));
    char *kernel_toRemovefrom = (strstr(os.kernel_version, "(")) - 1; //Best way i found to cut out the random data that follows the kernel release
    *kernel_toRemovefrom = '\0';
    char *kernel_cleanup = (strstr(os.kernel_version, "n ")) + 2; //best way i found to cut out "Linux Version"
    printf(BCYN "Kernel Version: " MAG "%s \n", kernel_cleanup);

    // CPU MODEL BLOCK - - - - - - - - - - - - - - - - - -
    get_info("/proc/cpuinfo", cpu_needle, os.cpu, sizeof(os.cpu));
    char *cpu_cleanup = (strstr(os.cpu, ":")) + 2;
    printf(BCYN "CPU model: " MAG "%s", cpu_cleanup);

    // MEMORY BLOCK - - - - - - - - - - - - - - - - - - - -
    // Total Memory
    get_info("/proc/meminfo", total_memory_needle, os.memory.total.RAW, sizeof(os.memory.total.RAW));
    sscanf(os.memory.total.RAW,"MemTotal:        %d", &os.memory.total.kB); // We filter out inncesary stuff to convert it into an integer
    convert_to_readable(&os.memory.total.kB, &os.memory.total.mB, &os.memory.total.gB, 1024); //It's store in Kb's so for better readability for the user, they're converted into bigger units

    // Memory Usage
    get_info("/proc/meminfo", memory_usage_needle, os.memory.usage.RAW, sizeof(os.memory.usage.RAW)); //It's the same process as the total memory sub-block
    sscanf(os.memory.usage.RAW, "Active:          %d", &os.memory.usage.kB);
    convert_to_readable(&os.memory.usage.kB, &os.memory.usage.mB, &os.memory.usage.gB, 1024);
    printf(BCYN "Memory Usage: " MAG "%dgB %dmB %dkB of " GRN"%dgB %dmB %dkB total\n", os.memory.usage.gB, os.memory.usage.mB, os.memory.usage.kB, os.memory.total.gB, os.memory.total.mB, os.memory.total.kB); //now we display everything

    //UPTIME BLOCK - - - - - - - - - - - - - -
    get_everything("/proc/uptime", os.uptime.RAW, sizeof(os.uptime.RAW)); //I use get_everything when the file it's only storing the value and we don't need to iterate on it
    sscanf(os.uptime.RAW, "%d", &os.uptime.seconds); // Same as the memory block, but to filter out a second value that's not relevant for this program, the important it's the first value
    convert_to_readable(&os.uptime.seconds, &os.uptime.minutes, &os.uptime.hours, 60);//Like with memory, convert seconds into bigger units to be able to quickly read it
    printf(BCYN "Uptime: " MAG "%d h %d m %d s \n", os.uptime.hours, os.uptime.minutes, os.uptime.seconds);

    printf(DFL);
    return 0;
}
