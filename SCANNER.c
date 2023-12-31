
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 200 //lon line
#define MAX_TOKEN_LENGTH 20 //lon token
#define MAX_ENTRIES 100 //lon entries
#define MAX_STRING_LENGTH 100 //long string

typedef struct { //contenido de cadena content
    int ent;
    char content[MAX_TOKEN_LENGTH];
    } Symbol;

int Int[MAX_ENTRIES]; // Array para enteros
double Real[MAX_ENTRIES]; // Array para reales
Symbol IDs[MAX_ENTRIES]; //Array para identificadores
Symbol Cadena[MAX_ENTRIES]; //Array para cadenas
int idsIndex = 0;
int intIndex = 0;
int realIndex = 0;
int cadenaIndex = 0;
int bolMultilinea = 0; //boleano para multilinea
int bolID = 0; //boleano para multilinea
int errorFatal = 0; //fatal

//Simbolos especiales
const char specSymbols[] = "+-*/<>=;:,()[].";

//Palabras reservadas n indices
char *palarbasClave[] = {"program", "real", "repeat", "procedure", "string", "until",
                         "function", "array", "for", "begin", "of", "to", "if", "do",
                         "end", "var", "then", "readLn", "integer", "else", "writeLn"};
int palabrasTokens[] = {24, 25, 26, 27, 28, 29,
                        30, 31, 32, 33, 34, 35, 36, 37,
                        38, 39, 40, 41, 42, 43, 44};
int indexClave = 21;

// Simbolos de -Σ
const char ignorarSimbolos[] = "$@#&";

void analizaNumero(char *text, int *index, int lon) { 
    char temp[MAX_TOKEN_LENGTH]; //Almacena en lo que reconoce
    int acum = 0, state = 1;
    char c = text[*index];
    temp[acum++] = c; //Agregamos el digito del scaner a temporal
    (*index)++;
    
    if (c == '.') { //se encontro punto
        state = 2;
    } while(*index < lon) {
        c = text[*index];
        switch(state) {
            case 1: 
                if (isdigit(c)) { // Recibiendo numeros y procesandolos
                    temp[acum++] = c;
                    (*index)++;
                } else if (c == '.') { //Se agrega el punto y se avanza
                    temp[acum++] = c;
                    (*index)++;
                    state = 2;
                } else { //Delimitador
                    state = 3; 
                }
                break;
            case 2:
                if (isdigit(c)) { //Recibiendo números
                    temp[acum++] = c;
                    (*index)++;
                } else {
                    state = 3; //Delimitador
                }
                break;
        } if (state == 3) {
            temp[acum] = '\0'; //se agrega nulo para validar
            if (strchr(temp, '.')) { //Se asigna a los reales
                Real[realIndex++] = atof(temp);
                printf("<47,%d>\n", realIndex);
            } else { //se asigna a los enteros
                Int[intIndex++] = atoi(temp);
                printf("<46,%d>\n", intIndex);
            } return;
        }
    }
}

//Analiza ID y palabras clave
void analizaIDC(char *text, int *index, int lon) {
    char compara[MAX_TOKEN_LENGTH]; //Almacena para comparar
    int c = 0;

    //Valida estructura ID
    if(isalpha(text[*index])|| (text[*index]== '_' && *index + 1 < lon && isalnum(text[*index +1]))) {
        bolID = 1; 
        compara[c++] = text[*index];
        (*index)++;
    } else {
        errorFatal = 1;
    }

    while (*index < lon && (isalpha(text[*index]) || isdigit(text[*index]) || text[*index] == '_')) {
        compara[c++] = text[*index]; //Mientras lea A-Z 0..9 o _ incrementa
        (*index)++;
        } compara[c] = '\0'; //se agrega nulo para validar

    if (bolID){
    for (int d = 0; d < indexClave; d++) { //Analiza en array de palabras si alguna coincide
        if (strcasecmp(compara, palarbasClave[d]) == 0) { //strcasecmp analiza palabras clave case sensitive
            printf("<%d>\n", palabrasTokens[d]); //Token palabra clave
            return;
        }
    }
    //No es palabra clave, lo asigna a los IDs
    IDs[idsIndex].ent = idsIndex + 1;
    strcpy(IDs[idsIndex].content, compara);
    printf("<45,%d>\n", idsIndex + 1);  //incrementa en posición de IDs
    idsIndex++;
    }
}

void comentarioLinea(char* text, int* index, int lon) {
    (*index)++;

    while (*index < lon && text[*index] != '\n') {
        if (text[*index] == '{') {
            errorFatal = 1;
            return;
        } else if (text[*index] == '}') {
            printf("Comentario (se desecha)\n");
            (*index)++;
            return;
        } (*index)++;
    }
    errorFatal =1;
}

void comentarioMultilinea(char *text, int *index, int lon) {
        // int inicioComentario = *index; //Posición

    if (!bolMultilinea) { //Si esta bandera no es 0 seguimos analizando lineas
        bolMultilinea = 1;
        (*index) +=2;
    }

    while (*index < lon - 1 && bolMultilinea) { //quitamos 1 para dejar espacio para *)
        if (text[*index] == '*' && text[*index + 1] == ')') {
            bolMultilinea = 0; 
            *index += 2; //avanzamos *)
            printf("Comentario (se desecha)\n");
        } else if (text[*index] == '(' && text[*index +1] == '*') {
            errorFatal =1;
            return;
        } else {
            (*index)++;
        }
    } 
}

void analizaCadena(char *text, int *index, int lon) {
    char cadena[MAX_STRING_LENGTH]; //Almacena 
    int a = 0;
    (*index)++; 
    while (*index < lon && text[*index] != '\'') { //Analiza que no sea fin de linea o '
        cadena[a++] = text[*index];
        (*index)++;
    } if (*index < lon && text[*index] == '\'') { //Analiza que no sea fin de linea y exista '
        cadena[a] = '\0'; //nulo para validar

        //Lo manda a tabla de cadenas
        Cadena[cadenaIndex].ent = cadenaIndex + 1;
        strcpy(Cadena[cadenaIndex].content, cadena);
        cadenaIndex++;
        (*index)++; // Salta el caracter de cierre
    } else {
        errorFatal = 1;
    }
}

void analizaSimbolos(char *text, int *index, int lon) {
    char c = text[*index];
    char nextChar = (*index + 1 < lon) ? text[*index + 1] : '\0'; //función para analizar siguiente caracter, para tokens compuestos

    switch (c) {
        case ':':
            if (nextChar == '=') {
                printf("<11>\n");
                (*index)++;  // avanza dos
            } else {
                printf("<49>\n");
            } break;

        case '+':
            printf("<1>\n");
            break;

        case '-':
            printf("<2>\n");
            break;

        case '*':
            printf("<3>\n");
            break;

        case '/':
            printf("<4>\n");
            break;

        case '<':
            if (nextChar == '=') {
                printf("<6>\n");
                (*index)++;
            } else if (nextChar == '>') {
                printf("<10>\n");
                (*index)++;
            } else {
                printf("<5>\n");
            } break;

        case '=':
            printf("<9>\n");
            break;

        case ';':
            printf("<12>\n");
            break;

        case ',':
            printf("<13>\n");
            break;

        case '(':
            printf("<16>\n");
            break;

        case ')':
            printf("<17>\n");
            break;

        case '[':
            printf("<18>\n"); 
            break;

        case ']':
            printf("<19>\n");
            break;

        case '>':
            if (nextChar == '=') {
                printf("<8>\n");
                (*index)++;
            } else {
                printf("<7>\n");
            } break;

        case '.':
            printf("<15>");
            break;
    } (*index)++;
}

int simbolosIgnorar(char c) { // Simbolos de -Σ
    if (strchr(ignorarSimbolos, c)) {
        errorFatal =1;
    }
    return 0;
}

void scanner(char *text) { //Q0
    int lon = strlen(text); //guarda longitud de linea
    int i = 0; //puntero

    if (bolMultilinea) {
        comentarioMultilinea(text, &i, lon);
    }

    while(i < lon) {
        char c = text[i];

        if (isdigit(c)) { //analiza Real o Entero
            analizaNumero(text, &i, lon);
        } else if (isalpha(c) || c == '_') { //analiza ID o Clave
            analizaIDC(text, &i, lon);
        } else if (c == '{') { //analiza Comentario 1 linea -> {
            comentarioLinea(text, &i, lon);
        } else if (c == '(' && i+1 < lon && text[i+1] == '*') { //analiza comentario Multilinea (*
            i++; 
            comentarioMultilinea(text, &i, lon);
        } else if (c == '\'') { //analiza cadenas
            analizaCadena(text, &i, lon);
        } else if (strchr(specSymbols, c)) { // analiza Simbolos especiales
            analizaSimbolos(text, &i, lon);
        } else if (simbolosIgnorar(c)) { //simbolos -Σ
            i++;
        } else if (c == ' ' || c == '\n' || c == '\r') { 
            i++;
        } else { // Caracteres no declarados
            errorFatal =1;
            i++; 
        } 
    }
}

void tablaIDs() {
    printf("\n\nTabla de identificadores\n");
    for (int i = 0; i < idsIndex; i++) {
        printf("<%d,%s>\n", IDs[i].ent, IDs[i].content);
    }
}

void tablaEnteros(){
    printf("\nTabla de Enteros:\n");
    for (int i = 0; i < intIndex; i++){
        printf("%d,%d\n", i + 1, Int[i]);
    }
}

void tablaReal(){
    printf("\nTabla de Reales:\n");
    for (int i = 0; i < realIndex; i++){
        printf("%d,%f\n", i + 1, Real[i]);
    }
}

void tablaCadenas() {
    printf("\nTabla de cadenas:\n");
    for (int i = 0; i < cadenaIndex; i++) {
        printf("<%d,%s>\n", Cadena[i].ent, Cadena[i].content);
    }
}

int main() {
    FILE *sourceCode;
    char text[MAX_LINE_LENGTH]; //Almacena linea
    
    sourceCode = fopen("example2.txt", "r"); //archivo con source code
    
    if(sourceCode == NULL) {
        printf("Error al abrir el archivo de entrada.\n");
        return 1;
    }

    while (fgets(text, MAX_LINE_LENGTH, sourceCode)) {
        scanner(text);
        if (errorFatal) {
            printf("------> Error fatal <------\n");
            fclose(sourceCode);
            return 1;
        }
    } if (bolMultilinea) {
        errorFatal = 1;
        printf("\n------> Error fatal <------\n");
        fclose(sourceCode);
        return 1;
    }
    
    fclose(sourceCode);
    tablaIDs();
    tablaReal();
    tablaEnteros();
    tablaCadenas();
    return 0;
}