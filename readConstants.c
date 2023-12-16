#include "local.h"

int readThresholds(int thresholds[]) {
    FILE* file = fopen("thresholds.txt", "r"); 

    if (file == NULL) {
        printf("ERROR OPENING THE thresholds file\n");
        return 0;
    }

    char thresholdName[60];
    int value;

    // read data from the file into the array
    int i = 0;
    while (fscanf(file, "%s %d", thresholdName, &value) == 2) {
        thresholds[i] = value;

        // check if the array is full
        if (++i >= 12) {
            break;
        }
    }
    fclose(file);
    return i;
}
int getRandom(int min, int max){
  return (int) (min + (rand() % (max - min)));
}

// int main() {
//     int thresholds[10];
//     int count;
//     count = readThresholds(thresholds);
//     if (count == 0) {
//         printf("Error reading thresholds\n");
//         return -1;
//     }
//     printf("Thresholds read: %d\n", count);
//     for (int i = 0; i < count; i++) {
//         printf("Threshold %d: %d\n", i + 1, thresholds[i]);
//     }
//     return 0;
// }
