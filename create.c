#include <stdio.h>
#include <string.h>
#include <time.h>
#define true 1
#define false 0

struct cat_block
{ 
    char owner[10]; //tabla
    char name[10];  //nombre
	char type[10]; //tipo
	int n_fields; //cantidad de campos
	int first; //ultimo campo ingresado...
} cat,cam;

int checkTableName(char* name);
int checkFieldName(char* name);
int checkField(char* sentence); 
void createField(char* sentence);
void createTable(char* name, int quantity);
int checkReserved(char* sentence);
int cadenasIguales(char* cad0, char* cad1);
void ellapsedTime(clock_t start);

int main(int argc, char* argv[]) //create t1 c1:int c2:text c3:char
{
	clock_t start = clock();
	switch(argc)
	{
		case 1:
		{
			printf("Sintaxis Básica: create [nombre_tabla + nombre_columna + : + tipo_dato]\n");
			ellapsedTime(start);
			return true;	
		}
		case 2:{
			printf("Error! Es necesario crear columnas para la tabla. \n Sintaxis Básica: create [nombre_tabla + nombre_columna + ':' + tipo_dato]\n");
			ellapsedTime(start);
		return false;
		}
		default:
		{
			if(checkTableName(argv[1])){
				int i;
				for(i=2;i<argc;i++)
					if(!checkField(argv[i]))
						return false;
				createTable(argv[1],argc-2);
				for(i=2;i<argc;i++)
					createField(argv[i]);
				printf("Tabla %s creada exitosamente\n", cat.name);
				ellapsedTime(start);
				return true;	
			}
		}		
	}
	printf("Error: Escriba [create] para más información\n");
	ellapsedTime(start);
	return false;	
}

//Comprueba que el nombre de la tabla no exista en el catalogo
int checkTableName(char* name){ //1=No existe el nombre || 0= Ya existe el nombre
	FILE* streamer = fopen("catalogo.dat", "r");
	if(!streamer)
		streamer=fopen("catalogo.dat", "w+");
    while (fread(&cat, sizeof(cat), 1, streamer))
        if (cadenasIguales(cat.name, name)&&cadenasIguales(cat.owner,"null")) {
			fclose(streamer);
            printf("%s ya existe\n",cat.name);
            return false;
        }
    fclose(streamer);
	strcpy(cat.name,name);
	return true;
}

//Comprueba que el nombre del campo no exista en el catalogo
int checkFieldName(char* name){ //1=No existe el nombre || 0= Ya existe el nombre
	FILE* streamer = fopen("catalogo.dat", "r");
    while (fread(&cat, sizeof(cat), 1, streamer))
        if (cadenasIguales(cat.name, name)&&!cadenasIguales(cat.owner,"null")) {
			fclose(streamer);
            printf("%s ya existe\n",cat.name);
            return false;
        }
    fclose(streamer);
	return true;
}

//Comprueba un campo (su nombre y su tipo) para saber si es valido
int checkField(char* sentence)
{
	int i;
	int control = false;
	char* name; 
	char* type;
	char* text =strdup(sentence);
	for(i=0;i<strlen(sentence);i++)
		if(sentence[i]==':') 
			if(control==false) //si es el primer :
				control= true;
			else //Si no es el primer : de la oracion...
				return false;
	name = strtok(text,":");
	type = strtok(NULL,":");
	if(checkFieldName(name)&&checkReserved(type))
		return true;
	return false;	
}			

//Crea un atributo(Campo) dado...
void createField(char* sentence)
{
	char* text =strdup(sentence);
	char* name = strtok(text,":");
	char* type= strtok(NULL,":");
	strcpy(cam.owner,cat.name);
	strcpy(cam.type,type);
	strcpy(cam.name,name);
	cat.n_fields=0;
	cat.first =0;
	FILE* streamer = fopen("catalogo.dat", "a");
	fwrite(&cam, sizeof(cam), 1, streamer);
	fclose(streamer);	
}

//Crea una tabla con un nombre dado
void createTable(char* name,int quantity)
{
	strcpy(cat.name, name);
	strcpy(cat.owner, "null");
	strcpy(cat.type, "tabla");
	cat.n_fields=quantity;
	cat.first =-1;
	FILE* streamer = fopen("catalogo.dat", "a");
	fwrite(&cat, sizeof(cat), 1, streamer);
	fclose(streamer);	
}

//revisa que el tipo de dato de un campo SEA una palabra reservada	
int checkReserved(char* sentence)
{
	char* reserved[] = {"int","float","text"};
	int i;
	for(i=0;i<3;i++)
		if(cadenasIguales(reserved[i],sentence))
			return true;
	return false;
}	

//compara 2 cadenas de texto para ver si son iguales
int cadenasIguales(char* cad0, char* cad1) {
    if(strcmp(cad0,cad1)!=0)
		return false;
	return true;
}

//muestra el tiempo que ha pasado desde que comenzó el programa (para evaluar eficiencia)
void ellapsedTime(clock_t start){ 
	clock_t diff = clock() - start;
	int msec = diff * 1000 / CLOCKS_PER_SEC;
	printf("Procesado en %d.%d segundos\n",msec/1000,msec%1000);
}