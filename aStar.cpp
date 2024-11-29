#include <SDL2/SDL.h>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>

// Definimos la estructura Nodo
typedef struct Nodo
{
    int x, y;
    int costo;
    int heuristica;
    struct Nodo *padre;
} Nodo;

// Crea un nuevo nodo
Nodo *crearNodo(int x, int y, int costo, int heuristica, Nodo *padre)
{
    Nodo *nodo = (Nodo *)malloc(sizeof(Nodo));
    nodo->x = x;
    nodo->y = y;
    nodo->costo = costo;
    nodo->heuristica = heuristica;
    nodo->padre = padre;
    return nodo;
}

// Heurística Manhattan
int heuristicaManhattan(int x1, int y1, int x2, int y2)
{
    return 10 * (abs(x1 - x2) + abs(y1 - y2));
}

// Heurística Euclidiana
int heuristicaEuclidiana(int x1, int y1, int x2, int y2)
{
    return (int)(10 * sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2)));
}

// Heurística Chebyshev
int heuristicaChebyshev(int x1, int y1, int x2, int y2)
{
    return 10 * fmax(abs(x1 - x2), abs(y1 - y2));
}

// Compara nodos (para ordenar por costo + heurística)
int compararNodos(const void *a, const void *b)
{
    Nodo *nodoA = *(Nodo **)a;
    Nodo *nodoB = *(Nodo **)b;
    return (nodoA->costo + nodoA->heuristica) - (nodoB->costo + nodoB->heuristica);
}

// Verifica si un nodo está en una lista
bool estaEnLista(Nodo **lista, int tam, int x, int y)
{
    for (int i = 0; i < tam; i++)
    {
        if (lista[i]->x == x && lista[i]->y == y)
        {
            return true;
        }
    }
    return false;
}

// Reconstruye el camino desde el nodo objetivo
int** reconstruirCamino(Nodo *nodo, int *tamanoRuta)
{
    Nodo *nodoActual = nodo;
    int **ruta = NULL;
    *tamanoRuta = 0;

    // Primero contamos cuántos nodos hay en el camino
    while (nodoActual != NULL)
    {
        (*tamanoRuta)++;
        nodoActual = nodoActual->padre;
    }

    // Reservamos memoria para la ruta
    ruta = (int **)malloc(*tamanoRuta * sizeof(int *));
    nodoActual = nodo;
    int index = *tamanoRuta - 1;

    // Ahora reconstruimos la ruta
    while (nodoActual != NULL)
    {
        ruta[index] = (int *)malloc(2 * sizeof(int));
        ruta[index][0] = nodoActual->x;
        ruta[index][1] = nodoActual->y;
        nodoActual = nodoActual->padre;
        index--;
    }

    return ruta;
}

// Función A* que regresa la ruta como un arreglo de coordenadas
int** aStar(
    int inicio[2], int objetivo[2], 
    int **mapa, int filas, int columnas, 
    int (*heuristica)(int, int, int, int), int *tamanoRuta)
{
    Nodo *abiertos[filas * columnas];
    Nodo *cerrados[filas * columnas];
    int tamAbiertos = 0, tamCerrados = 0;

    Nodo *inicioNodo = crearNodo(
        inicio[0], inicio[1], 
        0, heuristica(inicio[0], inicio[1], objetivo[0], objetivo[1]), 
        NULL
    );
    abiertos[tamAbiertos++] = inicioNodo;

    while (tamAbiertos > 0) {
        qsort(abiertos, tamAbiertos, sizeof(Nodo *), compararNodos);
        Nodo *nodoActual = abiertos[0];

        for (int i = 0; i < tamAbiertos - 1; i++) {
            abiertos[i] = abiertos[i + 1];
        }
        tamAbiertos--;

        if (nodoActual->x == objetivo[0] && nodoActual->y == objetivo[1]) {
            // Reconstruimos la ruta y la retornamos
            int **ruta = reconstruirCamino(nodoActual, tamanoRuta);

            // Liberar memoria de nodos
            for (int i = 0; i < tamAbiertos + tamCerrados; i++) {
                free(abiertos[i]);
                free(cerrados[i]);
            }

            return ruta;
        }

        cerrados[tamCerrados++] = nodoActual;

        int movimientos[8][2] = {
            {-1, 0}, {1, 0}, {0, -1}, {0, 1},   // Cardinales
            {-1, -1}, {-1, 1}, {1, -1}, {1, 1}  // Diagonales
        };

        for (int i = 0; i < 8; i++) {
            int xVecino = nodoActual->x + movimientos[i][0];
            int yVecino = nodoActual->y + movimientos[i][1];

            if (xVecino >= 0 && xVecino < filas && yVecino >= 0 && yVecino < columnas &&
                mapa[xVecino][yVecino] == 0 &&
                !estaEnLista(cerrados, tamCerrados, xVecino, yVecino)) {
                int nuevoCosto = nodoActual->costo + (i < 4 ? 10 : 14);
                int h = heuristica(xVecino, yVecino, objetivo[0], objetivo[1]);

                Nodo *vecinoNodo = crearNodo(xVecino, yVecino, nuevoCosto, h, nodoActual);

                if (!estaEnLista(abiertos, tamAbiertos, xVecino, yVecino)) {
                    abiertos[tamAbiertos++] = vecinoNodo;
                }
            }
        }
    }

    printf("No se encontró camino\n");
    return NULL;
}

// Definir el tamaño de la ventana y otras configuraciones para SDL
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;
const int TILE_SIZE = 6;

void drawMatrix(SDL_Renderer* renderer, int rows, int cols, int **mapa, int **ruta, int tamanoRuta) {
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            SDL_Rect tile = {
                col * TILE_SIZE, 
                row * TILE_SIZE, 
                TILE_SIZE, 
                TILE_SIZE
            };

            // Determinar color según el estado de la celda
            if (mapa[row][col] == 1) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Bloqueado (negro)
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Libre (verde)
            }

            SDL_RenderFillRect(renderer, &tile);
        }
    }

    // Dibujar la ruta
    for (int i = 0; i < tamanoRuta; ++i) {
        int x = ruta[i][0];
        int y = ruta[i][1];
        SDL_Rect tile = {
            x * TILE_SIZE, 
            y * TILE_SIZE, 
            TILE_SIZE, 
            TILE_SIZE
        };
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Ruta (blanco)
        SDL_RenderFillRect(renderer, &tile);
    }
}

int main(int argc, char* argv[]) {
    srand(static_cast<unsigned int>(time(0))); // Semilla para colores aleatorios

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Error al inicializar SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "A* Pathfinding",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        SDL_Log("Error al crear ventana: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Error al crear renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Crear el mapa (0 es libre, 1 es bloqueado)
    int filas = 100;
    int columnas = 100;
    int **mapa = (int **)malloc(filas * sizeof(int *));
    for (int i = 0; i < filas; i++) {
        mapa[i] = (int *)malloc(columnas * sizeof(int));
        for (int j = 0; j < columnas; j++) {
            mapa[i][j] = rand() % 5 == 0 ? 1 : 0; // Coloca un 20% de obstáculos
        }
    }

    // Puntos de inicio y objetivo
    int inicio[2] = {0, 0};
    int objetivo[2] = {99, 99};

    // Llamar a A* con la heurística de Manhattan
    int tamanoRuta = 0;
    int **ruta = aStar(inicio, objetivo, mapa, filas, columnas, heuristicaManhattan, &tamanoRuta);

    // Limpiar la memoria del mapa
    for (int i = 0; i < filas; i++) {
        free(mapa[i]);
    }
    free(mapa);

    if (ruta != NULL) {
        // Dibujar la matriz y la ruta
        bool quit = false;
        SDL_Event event;

        while (!quit) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    quit = true;
                }
            }

            SDL_RenderClear(renderer);
            drawMatrix(renderer, filas, columnas, mapa, ruta, tamanoRuta);
            SDL_RenderPresent(renderer);
        }

        // Liberar la memoria de la ruta
        for (int i = 0; i < tamanoRuta; i++) {
            free(ruta[i]);
        }
        free(ruta);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
