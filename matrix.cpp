#include <SDL2/SDL.h>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>



// para hacer la ruta necesitamos esto y poder retroceder
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
    return (int)10 * sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

// Heurística Chebyshev
int heuristicaChebyshev(int x1, int y1, int x2, int y2)
{
    return 10 * fmax(abs(x1 - x2), abs(y1 - y2));
}

int heuristicaOctile(int x1, int y1, int x2, int y2) {
    int dx = abs(x1 - x2);
    int dy = abs(y1 - y2);
    return 10 * (dx + dy) + (14 - 2 * 10) * fmin(dx, dy);
}
int heuristicaDijkstra(int x1, int y1, int x2, int y2) {
    return 0;
}
int heuristicaWeightedManhattan(int x1, int y1, int x2, int y2) {
    return 15 * (abs(x1 - x2) + abs(y1 - y2)); // Peso 1.5x
}
int heuristicaSuperWeightedManhattan(int x1, int y1, int x2, int y2) {
    return 20 * (abs(x1 - x2) + abs(y1 - y2)); // Peso 1.5x
}


// funcion de comparar para usar en quick sort por que no voy a programar un quick sort ahorita
int compararNodos(const void *a, const void *b)
{
    Nodo *nodoA = *(Nodo **)a;
    Nodo *nodoB = *(Nodo **)b;
    return (nodoA->costo + nodoA->heuristica) - (nodoB->costo + nodoB->heuristica);
}


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

// Reconstruye el camino desde el nodo objetivo por el padre

void reconstruirCamino(Nodo *nodo, int ***ruta, int *tamanoRuta)
{
    if (nodo == NULL)
        return;

    // Llamada recursiva para subir por los nodos padres
    reconstruirCamino(nodo->padre, ruta, tamanoRuta);

    // Reservamos espacio para la nueva coordenada
    *ruta = (int **)realloc(*ruta, (*tamanoRuta + 1) * sizeof(int *));
    if (*ruta == NULL)
    {
        // Manejo de error
        fprintf(stderr, "Error al reasignar memoria.\n");
        exit(1);
    }
    (*ruta)[*tamanoRuta] = (int *)malloc(2 * sizeof(int));
    if ((*ruta)[*tamanoRuta] == NULL)
    {
        // Manejo de error
        fprintf(stderr, "Error al reservar memoria para la coordenada.\n");
        exit(1);
    }

    // Guardamos la coordenada del nodo actual
    (*ruta)[*tamanoRuta][0] = nodo->x;
    (*ruta)[*tamanoRuta][1] = nodo->y;

    // Incrementamos el tamaño de la ruta
    (*tamanoRuta)++;
}


void aStarDiag(int ***ruta, int *tamanoruta,
               int inicio[2], int objetivo[2],
               int **mapa, int filas, int columnas,
               int (*heuristica)(int, int, int, int))
{
    Nodo *abiertos[filas * columnas];
    Nodo *cerrados[filas * columnas];
    int tamAbiertos = 0, tamCerrados = 0;

    Nodo *inicioNodo = crearNodo(
        inicio[0], inicio[1],
        0, heuristica(inicio[0], inicio[1], objetivo[0], objetivo[1]),
        NULL);
    abiertos[tamAbiertos++] = inicioNodo;

    while (tamAbiertos > 0)
    {
        qsort(abiertos, tamAbiertos, sizeof(Nodo *), compararNodos);
        Nodo *nodoActual = abiertos[0];

        for (int i = 0; i < tamAbiertos - 1; i++)
        {
            abiertos[i] = abiertos[i + 1];
        }
        tamAbiertos--;

        if (nodoActual->x == objetivo[0] && nodoActual->y == objetivo[1])
        {

            printf("Camino encontrado:\n");
            reconstruirCamino(nodoActual, ruta, tamanoruta);
            printf("\n");
            return;
        }

        cerrados[tamCerrados++] = nodoActual;

        int movimientos[8][2] = {
            {-1, 0}, {1, 0}, {0, -1}, {0, 1}, // Cardinales
            {-1, -1},
            {-1, 1},
            {1, -1},
            {1, 1} // se añadieron diagonales y por algun motivo no los puedo ordenar or vscode shift alt f
        };

        for (int i = 0; i < 8; i++)
        {
            int xVecino = nodoActual->x + movimientos[i][0];
            int yVecino = nodoActual->y + movimientos[i][1];

            if (xVecino >= 0 && xVecino < filas && yVecino >= 0 && yVecino < columnas &&
                mapa[xVecino][yVecino] == 0 &&
                !estaEnLista(cerrados, tamCerrados, xVecino, yVecino))
            {
                int nuevoCosto = nodoActual->costo + (i < 4 ? 10 : 14);
                int h = heuristica(xVecino, yVecino, objetivo[0], objetivo[1]);

                Nodo *vecinoNodo = crearNodo(xVecino, yVecino, nuevoCosto, h, nodoActual);

                if (!estaEnLista(abiertos, tamAbiertos, xVecino, yVecino))
                {
                    abiertos[tamAbiertos++] = vecinoNodo;
                }
            }
        }
    }

    printf("No se encontró camino\n");
}

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;
const int TILE_SIZE = 6;

void drawMatrix(SDL_Renderer *renderer, int rows, int cols, int **mapa, int **ruta, int tamano)
{
    for (int row = 0; row < rows; ++row)
    {
        SDL_Rect tile;
        for (int col = 0; col < cols; ++col)
        {
            tile = {
                col * TILE_SIZE,
                row * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE};

            // Generar colores aleatorios
            if (mapa[col][row] == 0)
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
            // SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, 255);
            SDL_RenderFillRect(renderer, &tile);
        }

        for (int i = 0; i < tamano; i++)
        {
            tile = {
                ruta[i][0] * TILE_SIZE,
                ruta[i][1] * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE};
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

            // SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, 255);
            SDL_RenderFillRect(renderer, &tile);
        }
    }
}

//---------------------------------
void imprimirRuta(int **ruta)
{
    printf("Ruta encontrada:\n");
    int i = 0;
    while (ruta[i])
    { // Iteramos hasta encontrar un puntero nulo

        printf("(%d, %d)\n", ruta[i][0], ruta[i][1]);
        free(ruta[i]);
        i++;
    }
}

// Función para liberar la memoria de la ruta
void liberarRuta(int **ruta)
{
    if (!ruta)
        return;

    int i = 0;
    while (ruta[i])
    { // Liberamos cada fila de la ruta
        free(ruta[i]);
        i++;
    }
    free(ruta); // Finalmente liberamos la memoria del arreglo de punteros
}

int main(int argc, char *argv[])
{
    int filas = 100, columnas = 100;

    // Crear el mapa
    int **mapa = (int **)malloc(filas * sizeof(int *));
    for (int i = 0; i < filas; i++)
    {
        mapa[i] = (int *)malloc(columnas * sizeof(int));
    }

    // Inicializar el mapa con ceros
    for (int i = 0; i < filas; i++)
    {
        for (int j = 0; j < columnas; j++)
        {
            mapa[i][j] = 0;
        }
    }

    // Dibujar un muro horizontal que bloquea parte del camino
    for (int j = 0; j < 35; j++)
    {
        mapa[15][j] = 1;
    }

    // Dibujar un muro horizontal que divide el mapa parcialmente
    for (int i = 1; i < 50; i++)
    {
        mapa[i][20] = 1;
    }

    // Dibujar un muro diagonal hacia abajo
    for (int i = 40, j = 40; i < 60 && j < 60; i++, j++)
    {
        mapa[i][j] = 1;
    }

    // Dibujar un zigzag en el mapa
    for (int i = 70; i < 80; i++)
    {
        if (i % 2 == 0)
        {
            for (int j = 70; j < 75; j++)
            {
                mapa[i][j] = 1;
            }
        }
        else
        {
            for (int j = 75; j < 80; j++)
            {
                mapa[i][j] = 1;
            }
        }
    }

    int inicio[2] = {0, 0};
    int objetivo[2] = {90, 90};

    int **ruta = NULL;

    // Ejecutar el algoritmo A* con heurística de Chebyshev
    printf("Usando heurística de Chebyshev:\n");
    int tamanoruta = 0;
    aStarDiag(&ruta, &tamanoruta, inicio, objetivo, mapa, filas, columnas,heuristicaOctile);

    srand(static_cast<unsigned int>(time(0))); // Semilla para colores aleatorios

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_Log("Error al inicializar SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Matriz de Cuadrados",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN);

    if (!window)
    {
        SDL_Log("Error al crear ventana: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        SDL_Log("Error al crear renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    const int rows = WINDOW_HEIGHT / TILE_SIZE;
    const int cols = WINDOW_WIDTH / TILE_SIZE;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }

        // Dibujar matriz
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fondo negro
        SDL_RenderClear(renderer);
        drawMatrix(renderer, rows, cols, mapa, ruta, tamanoruta);
        SDL_RenderPresent(renderer);

        SDL_Delay(100); // Actualizar cada 100 ms
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    // Liberar la memoria del mapa
    for (int i = 0; i < filas; i++)
    {
        free(mapa[i]);
    }
    free(mapa);

    printf("se culmino la liberacion de memoria del mapa");

    // Imprimir la ruta encontrada
    if (ruta != NULL)
    {
        printf("Ruta encontrada:\n");
        int i = 0;
        for (int i = 0; i < tamanoruta; i++)
        {
            printf("(%d, %d)\n", ruta[i][0], ruta[i][1]);
        }

        printf("se va acceder al loop dentro de el cual se va a a liberar una ruta\n");
        for (int i = 0; i < tamanoruta; i++)
        {
            printf("se va a liberar una ruta\n");
            free(ruta[i]);
            printf("se libero una memoria\n");
        }
        printf("se va a liberar la memoria principal de la ruta\n");
        free(ruta);
        printf("se culmino la liberacion de memoria de la ruta\n");
    }
    else
    {
        printf("No se encontró una ruta.\n");
    }

    return 0;
}
