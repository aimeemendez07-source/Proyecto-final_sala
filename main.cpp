//Proyecto final - Sala de cine
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <raylib.h>
using namespace std;

enum Estado{LIBRE, RESERVADO, VENDIDO};

struct Asiento{
    int columna, id;
    char fila;
    Estado estado;
    char *cliente;
};

int filas=0;
int columnas=0;
Asiento** sala=NULL;
int ultimoId = 0;

//Configuracion de la ventana
const int ANCHO=900;
const int ALTO=650;
const int TAM_ASIENTO=25;
int OFFSET_X=0;
const int OFFSET_Y=120;

//Variables 
int pantalla=0;
char mensaje[100]="";
int filaSel=-1, colSel=-1;
char nombreInput[50]="";
bool escribiendo=false;
int letraIndex=0;
char infoReserva[200] = "";
bool cancelado=false;

//Prototipos 
void cargarDimensiones();
void inicializarSala();
void reservarAsiento(int fila, int columna, const char* nombre);
void venderAsiento(int fila, int columna, const char* nombre);
void cancelarReserva(int fila, int columna, const char* nombre);
void dibujarMenu();
void dibujarSala();
void guardarEstado();
void cargarEstado();
int buscarIdPorNombre(const char* nombre);
void limpiar();

int main(){
    cargarDimensiones();
    inicializarSala();
    cargarEstado();
    
    // Calcular offset después de conocer las columnas
    OFFSET_X = (ANCHO - (columnas * TAM_ASIENTO)) / 2;
    
    InitWindow(ANCHO, ALTO, "Sistema de Reservaciones de Cine");
    SetWindowPosition(GetScreenWidth()/2 - ANCHO/2, GetScreenHeight()/2 - ALTO/2);
    
    while(!WindowShouldClose()){
        if(pantalla==0){
            if(IsKeyPressed(KEY_ONE)) pantalla=1;
            if(IsKeyPressed(KEY_TWO)){ 
                pantalla=2; 
                strcpy(mensaje, "Haga clic en un asiento LIBRE");
            }
            if(IsKeyPressed(KEY_THREE)){ 
                pantalla=3; 
                strcpy(mensaje, "Haga clic en el asiento que desea cancelar");
            }
            if(IsKeyPressed(KEY_FOUR)){
                pantalla=4;
                strcpy(mensaje, "Haga clic en un asiento a comprar");
            }
            if(IsKeyPressed(KEY_FIVE)){ 
                guardarEstado(); 
                strcpy(mensaje, "¡Guardado!");
            }
            if(IsKeyPressed(KEY_SIX)) break;
        }
        
        //Seleccion de asientos para reservar o cancelar
        if((pantalla==2 || pantalla==3 || pantalla==4) && !escribiendo){
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                Vector2 mouse=GetMousePosition();
                for(int i=0; i<filas; i++){
                    for(int j=0; j<columnas; j++){
                        Rectangle r={ OFFSET_X + j * TAM_ASIENTO, OFFSET_Y + i * TAM_ASIENTO, TAM_ASIENTO - 3, TAM_ASIENTO - 3 };
                        if(CheckCollisionPointRec(mouse, r)){
                            filaSel=i; 
                            colSel=j;

                            if(pantalla==2 && sala[i][j].estado==LIBRE){
                                escribiendo=true;
                                letraIndex=0;
                                nombreInput[0]='\0';
                            }else if(pantalla==3 && sala[i][j].estado==RESERVADO){
                                escribiendo=true;
                                cancelado=true;
                                letraIndex=0;
                                nombreInput[0]='\0';
                            }else if(pantalla==4){
                                if(sala[i][j].estado == LIBRE || sala[i][j].estado == RESERVADO){
                                    escribiendo=true;
                                    letraIndex=0;
                                    nombreInput[0]='\0';
                                }else{
                                    strcpy(mensaje, "Asiento no disponible");
                                }
                            }else{
                                strcpy(mensaje, "Asiento no disponible");
                            }
                        }
                    }
                }
            }
        }
        
        //Ingresar nombre
        if(escribiendo){
            int key=GetCharPressed();
            while(key>0 && letraIndex<49){
                nombreInput[letraIndex++]=(char)key;
                nombreInput[letraIndex]='\0';
                key=GetCharPressed();
            }
            if(IsKeyPressed(KEY_BACKSPACE) && letraIndex>0){
                nombreInput[--letraIndex]='\0';
            }
            if(IsKeyPressed(KEY_ENTER) && strlen(nombreInput)>0){
                if(pantalla==2){
                    reservarAsiento(filaSel, colSel, nombreInput);
                    strcpy(mensaje, infoReserva);
                }else if(pantalla==3 && cancelado){
                    cancelarReserva(filaSel, colSel, nombreInput);
                    cancelado=false;
                }else if(pantalla==4){
                    venderAsiento(filaSel, colSel, nombreInput);
                }
                guardarEstado();
                escribiendo=false;
                pantalla=0;
            }
            if(IsKeyPressed(KEY_ESCAPE)){
                escribiendo=false;
                pantalla=0;
            }
        }
        
        //Dibujado
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        int tituloAncho=MeasureText("SISTEMA DE RESERVACIONES", 30);
        DrawText("SISTEMA DE RESERVACIONES", (ANCHO - tituloAncho)/2, 15, 30, DARKBLUE);
        
        if(escribiendo){
            DrawRectangle(ANCHO/2 - 200, ALTO/2 - 60, 400, 120, LIGHTGRAY);
            DrawRectangleLines(ANCHO/2 - 200, ALTO/2 - 60, 400, 120, DARKGRAY);
            DrawText("Ingrese su nombre:", ANCHO/2 - 180, ALTO/2 - 40, 20, BLACK);
            DrawRectangle(ANCHO/2 - 180, ALTO/2 - 10, 360, 30, WHITE);
            DrawRectangleLines(ANCHO/2 - 180, ALTO/2 - 10, 360, 30, BLACK);
            DrawText(nombreInput, ANCHO/2 - 170, ALTO/2 - 5, 20, BLACK);
            DrawText("ENTER = Confirmar, ESC = Cancelar", ANCHO/2 - 200, ALTO/2 + 35, 15, GRAY);
        }else if(pantalla==0){
            dibujarMenu();
            if(strlen(mensaje)>0){
                DrawText(mensaje, ANCHO/2 - MeasureText(mensaje, 20)/2, ALTO - 40, 20, GREEN);
            }
        }else{
            dibujarSala();
            DrawText(mensaje, ANCHO/2 - MeasureText(mensaje, 20)/2, ALTO - 30, 20, pantalla==2? BLUE : RED);
            
            //Boton volver
            Rectangle btnVolver={ ANCHO - 80, ALTO - 40, 70, 30 };
            DrawRectangleRec(btnVolver, LIGHTGRAY);
            DrawRectangleLinesEx(btnVolver, 2, DARKGRAY);
            DrawText("Volver", ANCHO - 70, ALTO - 35, 16, BLACK);
            
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                Vector2 mouse=GetMousePosition();
                if(CheckCollisionPointRec(mouse, btnVolver)){
                    pantalla=0;
                }
            }
        }
        EndDrawing();
    }
    CloseWindow();
    limpiar();
    return 0;
}

void cargarDimensiones(){
    filas=15;
    columnas=20; 
    ifstream archivoDimension("dimensiones.txt");
    if(archivoDimension.is_open()){
        archivoDimension >> filas >> columnas;
        archivoDimension.close();
        cout << "Dimensiones cargadas: " << filas << " filas, " << columnas << " columnas" << endl;
    }else{
        cerr << "No se pudo abrir el archivo de dimensiones, usando valores por defecto" << endl;
    }
    
    ofstream archivoCrear("dimensiones.txt");
    if(archivoCrear.is_open()){
        archivoCrear << filas << " " << columnas << endl;
        archivoCrear.close();
        cout << "Dimensiones guardadas: " << filas << " filas, " << columnas << " columnas" << endl;
    }else{
        cerr << "No se pudo crear el archivo" << endl;
    }
}

void inicializarSala(){
    sala=(Asiento**)malloc(filas*sizeof(Asiento*));
    for(int i=0; i<filas; i++){
        sala[i]=(Asiento*)malloc(columnas*sizeof(Asiento));
        for(int j=0; j<columnas; j++){
            sala[i][j].estado=LIBRE;
            sala[i][j].fila='A'+i;
            sala[i][j].columna=j+1;
            sala[i][j].id=0;
            //Mem dinamica para el nombre
            sala[i][j].cliente=(char*)malloc(1);
            sala[i][j].cliente[0] = '\0';
        }
    }
}

int buscarIdPorNombre(const char* nombre){
    for(int i=0; i<filas; i++){
        for(int j=0; j<columnas; j++){
            if(sala[i][j].estado==RESERVADO && strcmp(sala[i][j].cliente, nombre)==0){
                return sala[i][j].id;
            }
        }
    }
    return -1;
}

void reservarAsiento(int fila, int columna, const char* nombre){
    int idExistente;

    if(fila<0 || fila>=filas || columna<0 || columna>=columnas){
        strcpy(mensaje, "Posicion invalida");
        return;
    }
    if(sala[fila][columna].estado!=LIBRE){
        strcpy(mensaje, "Asiento no disponible");
        return;
    }
    
    // Liberar memoria existente
    free(sala[fila][columna].cliente);
    // Asignar nueva memoria dinámica para el nombre
    sala[fila][columna].cliente=(char*)malloc((strlen(nombre)+1)*sizeof(char));
    strcpy(sala[fila][columna].cliente, nombre);

    // Buscar si el nombre ya tiene reserva activa
    idExistente=buscarIdPorNombre(nombre);
    if(idExistente!=-1){
        //El cliente ya existe, usar el mismo ID
        sala[fila][columna].id=idExistente;
        snprintf(infoReserva, sizeof(infoReserva), "¡Bienvenido de vuelta! Asiento %c%d | ID existente: %d | Cliente: %s", 'A'+fila, columna+1, idExistente, nombre);
    } else {
        // Cliente nuevo, generar nuevo ID
        ultimoId++;
        sala[fila][columna].id=ultimoId;
        snprintf(infoReserva, sizeof(infoReserva), "¡Nuevo cliente! Asiento %c%d | ID asignado: %d | Cliente: %s", 'A'+fila, columna+1, ultimoId, nombre);
    }
    sala[fila][columna].estado=RESERVADO;

    snprintf(infoReserva, sizeof(infoReserva), "Reserva exitosa! Asiento %c%d | ID: %d | Cliente: %s", 'A'+fila, columna+1, sala[fila][columna].id, sala[fila][columna].cliente);
}

void venderAsiento(int fila, int columna, const char* nombre){
    if(fila<0 || fila>=filas || columna<0 || columna>=columnas){
        strcpy(mensaje, "Posicion invalida");
        return;
    }
    
    if(sala[fila][columna].estado==VENDIDO){
        strcpy(mensaje, "Asiento ya vendido");
        return;
    }
    
    if(sala[fila][columna].estado==LIBRE){
        free(sala[fila][columna].cliente);
        sala[fila][columna].cliente=(char*)malloc((strlen(nombre)+1)*sizeof(char));
        strcpy(sala[fila][columna].cliente, nombre);
        ultimoId++;
        sala[fila][columna].id=ultimoId;
        sala[fila][columna].estado=VENDIDO;
        snprintf(infoReserva, sizeof(infoReserva), "Venta exitosa! Asiento %c%d | ID: %d | Cliente: %s", 'A'+fila, columna+1, ultimoId, nombre);
        strcpy(mensaje, infoReserva);
        return;
    }
    
    if(sala[fila][columna].estado==RESERVADO){
        if(strcmp(sala[fila][columna].cliente, nombre)==0){
            sala[fila][columna].estado=VENDIDO;
            snprintf(infoReserva, sizeof(infoReserva), "Reserva convertida a venta! Asiento %c%d | ID: %d | Cliente: %s", 'A'+fila, columna+1, sala[fila][columna].id, nombre);
            strcpy(mensaje, infoReserva);
        }else{
            strcpy(mensaje, "El nombre no coincide con la reserva");
        }
        return;
    }
}

void cancelarReserva(int fila, int columna, const char* nombre){
    if(fila<0 || fila>=filas || columna<0 || columna>=columnas){
        strcpy(mensaje, "Posicion invalida");
        return;
    }
    if(sala[fila][columna].estado!=RESERVADO){
        strcpy(mensaje, "No hay reserva en esa posicion");
        return;
    }
    if(strcmp(sala[fila][columna].cliente, nombre)!=0){
        strcpy(mensaje, "El nombre no coincide con la reserva");
        return;
    }
    //Liberar memoria del nombre
    free(sala[fila][columna].cliente);
    //Reiniciar como libre con memoria vacía
    sala[fila][columna].cliente=(char*)malloc(1);
    sala[fila][columna].cliente[0]='\0';
    sala[fila][columna].estado=LIBRE;
    sala[fila][columna].id=0;
    
    strcpy(mensaje, "Reserva cancelada");
}

void guardarEstado(){
    int longitud;
    FILE *archivo=fopen("cine.bin","wb");
    if(archivo!=NULL){
        fwrite(&filas, sizeof(int), 1, archivo);
        fwrite(&columnas, sizeof(int), 1, archivo);
        fwrite(&ultimoId, sizeof(int), 1, archivo);

        for(int i=0; i<filas; i++){
            for(int j=0; j<columnas; j++){
                fwrite(&sala[i][j].columna, sizeof(int), 1, archivo);
                fwrite(&sala[i][j].id, sizeof(int), 1, archivo);
                fwrite(&sala[i][j].fila, sizeof(char), 1, archivo);
                fwrite(&sala[i][j].estado, sizeof(int), 1, archivo);
                
                longitud=strlen(sala[i][j].cliente)+1;
                fwrite(&longitud, sizeof(int), 1, archivo);
                fwrite(sala[i][j].cliente, sizeof(char), longitud, archivo);
            }
        }
        fclose(archivo);
    }
}

void cargarEstado(){
    int longitud, filasA, columnasA;
    FILE *archivo=fopen("cine.bin","rb");
    if(archivo!=NULL){
        fread(&filasA, sizeof(int), 1, archivo);
        fread(&columnasA, sizeof(int), 1, archivo);
        fread(&ultimoId, sizeof(int), 1, archivo);
        
        if(filasA==filas && columnasA==columnas){
            for(int i=0; i<filas; i++){
                for(int j=0; j<columnas; j++){
                    fread(&sala[i][j].columna, sizeof(int), 1, archivo);
                    fread(&sala[i][j].id, sizeof(int), 1, archivo);
                    fread(&sala[i][j].fila, sizeof(char), 1, archivo);
                    fread(&sala[i][j].estado, sizeof(int), 1, archivo);
                    
                    fread(&longitud, sizeof(int), 1, archivo);
                    free(sala[i][j].cliente);
                    sala[i][j].cliente=(char*)malloc(longitud*sizeof(char));
                    fread(sala[i][j].cliente, sizeof(char), longitud, archivo);
                }
            }
        }
        fclose(archivo);
    }
}

void dibujarMenu(){
    const char* opciones[]={"1 - Ver sala", "2 - Reservar", "3 - Cancelar", "4 - Comprar", "5 - Guardar", "6 - Salir"};
    for(int i=0; i<6; i++){
        Rectangle btn={ ANCHO/2 - 120, 100 + i * 50, 240, 42 };
        DrawRectangleRec(btn, LIGHTGRAY);
        DrawRectangleLinesEx(btn, 2, DARKGRAY);
        DrawText(opciones[i], ANCHO/2 - MeasureText(opciones[i], 20)/2, 100 + i * 50 + 13, 20, DARKBLUE);
    }
}

void dibujarSala(){
    //Pantalla
    int pantallaX=OFFSET_X;
    int pantallaY=OFFSET_Y-35;
    int pantallaAncho=columnas*TAM_ASIENTO;
    DrawRectangle(pantallaX, pantallaY, pantallaAncho, 22, DARKGRAY);
    DrawText("PANTALLA", pantallaX + pantallaAncho/2 - 30, pantallaY + 4, 12, WHITE);
    
    //Numeros de columnas
    for(int j=0; j<columnas; j++){
        DrawText(TextFormat("%d", j+1), OFFSET_X + j * TAM_ASIENTO + TAM_ASIENTO/3, OFFSET_Y - 12, 10, BLACK);
    }
    
    //Asientos
    for(int i=0; i<filas; i++){
        DrawText(TextFormat("%c", 'A'+i), OFFSET_X - 18, OFFSET_Y + i * TAM_ASIENTO + TAM_ASIENTO/3, 13, BLACK);
        
        for(int j=0; j<columnas; j++){
            Color color;
            const char* texto="";
            
            switch(sala[i][j].estado){
                case LIBRE: color=GREEN; texto=" "; break;
                case RESERVADO: color=YELLOW; texto="R"; break;
                case VENDIDO: color=RED; texto="X"; break;
            }
            
            DrawRectangle(OFFSET_X + j * TAM_ASIENTO, OFFSET_Y + i * TAM_ASIENTO, TAM_ASIENTO - 3, TAM_ASIENTO - 3, color);
            DrawRectangleLines(OFFSET_X + j * TAM_ASIENTO, OFFSET_Y + i * TAM_ASIENTO, TAM_ASIENTO - 3, TAM_ASIENTO - 3, BLACK);
            DrawText(texto, OFFSET_X + j * TAM_ASIENTO + TAM_ASIENTO/3, OFFSET_Y + i * TAM_ASIENTO + TAM_ASIENTO/7, 12, BLACK);
        }
    }
    
    // Leyenda
    DrawRectangle(10, ALTO - 85, 130, 75, LIGHTGRAY);
    DrawRectangleLines(10, ALTO - 85, 130, 75, DARKGRAY);
    DrawText("Leyenda:", 15, ALTO - 80, 12, BLACK);
    DrawRectangle(15, ALTO - 62, 12, 12, GREEN);
    DrawText("Libre", 32, ALTO - 62, 11, BLACK);
    DrawRectangle(15, ALTO - 45, 12, 12, YELLOW);
    DrawText("Reservado", 32, ALTO - 45, 11, BLACK);
    DrawRectangle(15, ALTO - 28, 12, 12, RED);
    DrawText("Vendido", 32, ALTO - 28, 11, BLACK);
}

void limpiar(){
    if(sala != NULL){
        for(int i=0; i<filas; i++){
            if(sala[i]){
                for(int j=0; j<columnas; j++){
                    free(sala[i][j].cliente);  // Liberar cada nombre dinámico
                }
                free(sala[i]);
            }
        }
        free(sala);
        sala = NULL;
    }
}