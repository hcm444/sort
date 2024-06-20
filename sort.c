#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define ARRAY_SIZE 100
#define SAMPLE_RATE 44100
#define AMPLITUDE 28000
#define DURATION 0.1  // Sound duration in seconds

typedef void (*VisualizationCallback)(SDL_Renderer *renderer, int array[], int size, int idx1, int idx2);

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void drawArray(SDL_Renderer *renderer, int array[], int size, int highlight1, int highlight2) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    int barWidth = SCREEN_WIDTH / size;
    for (int i = 0; i < size; ++i) {
        int barHeight = (array[i] * SCREEN_HEIGHT) / size;
        SDL_Rect bar = {i * barWidth, SCREEN_HEIGHT - barHeight, barWidth, barHeight};
        if (i == highlight1 || i == highlight2) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }
        SDL_RenderFillRect(renderer, &bar);
    }

    SDL_RenderPresent(renderer);
}

void generateSineWave(int length, Uint8 **buffer, Uint32 *lengthBytes) {
    double frequency = 200 + (length * 5);  // Base frequency of 200 Hz, increasing with length
    int numSamples = (int)(SAMPLE_RATE * DURATION);
    *lengthBytes = numSamples * sizeof(Sint16);
    *buffer = (Uint8 *)malloc(*lengthBytes);
    Sint16 *samples = (Sint16 *)(*buffer);

    for (int i = 0; i < numSamples; ++i) {
        double time = (double)i / SAMPLE_RATE;
        samples[i] = (Sint16)(AMPLITUDE * sin(2.0 * M_PI * frequency * time));
    }
}

void bubbleSort(int array[], int size, VisualizationCallback callback, SDL_Renderer *renderer) {
    for (int i = 0; i < size - 1; ++i) {
        for (int j = 0; j < size - i - 1; ++j) {
            if (array[j] > array[j + 1]) {
                swap(&array[j], &array[j + 1]);
                callback(renderer, array, size, j, j + 1);

                Uint8 *buffer;
                Uint32 length;
                generateSineWave(array[j], &buffer, &length);
                Mix_Chunk *chunk = Mix_QuickLoad_RAW(buffer, length);
                Mix_PlayChannel(-1, chunk, 0);

                SDL_Delay(50);  // Adjust delay for visualization speed

                Mix_FreeChunk(chunk);
                free(buffer);
            }
        }
    }
}

void merge(int array[], int left, int mid, int right, VisualizationCallback callback, SDL_Renderer *renderer) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    int *L = (int *)malloc(n1 * sizeof(int));
    int *R = (int *)malloc(n2 * sizeof(int));

    for (int i = 0; i < n1; ++i) L[i] = array[left + i];
    for (int i = 0; i < n2; ++i) R[i] = array[mid + 1 + i];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            array[k] = L[i];
            i++;
        } else {
            array[k] = R[j];
            j++;
        }
        callback(renderer, array, ARRAY_SIZE, k, -1);

        Uint8 *buffer;
        Uint32 length;
        generateSineWave(array[k], &buffer, &length);
        Mix_Chunk *chunk = Mix_QuickLoad_RAW(buffer, length);
        Mix_PlayChannel(-1, chunk, 0);

        SDL_Delay(50);  // Adjust delay for visualization speed

        Mix_FreeChunk(chunk);
        free(buffer);
        k++;
    }

    while (i < n1) {
        array[k] = L[i];
        callback(renderer, array, ARRAY_SIZE, k, -1);

        Uint8 *buffer;
        Uint32 length;
        generateSineWave(array[k], &buffer, &length);
        Mix_Chunk *chunk = Mix_QuickLoad_RAW(buffer, length);
        Mix_PlayChannel(-1, chunk, 0);

        SDL_Delay(50);  // Adjust delay for visualization speed

        Mix_FreeChunk(chunk);
        free(buffer);
        i++;
        k++;
    }

    while (j < n2) {
        array[k] = R[j];
        callback(renderer, array, ARRAY_SIZE, k, -1);

        Uint8 *buffer;
        Uint32 length;
        generateSineWave(array[k], &buffer, &length);
        Mix_Chunk *chunk = Mix_QuickLoad_RAW(buffer, length);
        Mix_PlayChannel(-1, chunk, 0);

        SDL_Delay(50);  // Adjust delay for visualization speed

        Mix_FreeChunk(chunk);
        free(buffer);
        j++;
        k++;
    }

    free(L);
    free(R);
}

void mergeSort(int array[], int left, int right, VisualizationCallback callback, SDL_Renderer *renderer) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        mergeSort(array, left, mid, callback, renderer);
        mergeSort(array, mid + 1, right, callback, renderer);

        merge(array, left, mid, right, callback, renderer);
    }
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    if (Mix_OpenAudio(SAMPLE_RATE, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Sorting Algorithm Visualization",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int array[ARRAY_SIZE];
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        array[i] = rand() % ARRAY_SIZE;
    }

    printf("Choose sorting algorithm: 1 for Bubble Sort, 2 for Merge Sort: ");
    int choice;
    scanf("%d", &choice);

    drawArray(renderer, array, ARRAY_SIZE, -1, -1);

    if (choice == 1) {
        bubbleSort(array, ARRAY_SIZE, drawArray, renderer);
    } else if (choice == 2) {
        mergeSort(array, 0, ARRAY_SIZE - 1, drawArray, renderer);
    } else {
        printf("Invalid choice!\n");
    }

    SDL_Event e;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }
    }

    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
