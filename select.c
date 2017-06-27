#include <stdio.h>
#include <stdlib.h>
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
	int last; //ultimo campo ingresado...
} cat,cam,aux;

struct user_block
{
	int next;
	int row; 
	char element[15]; 
	char table[10]; 
	char field[10]; 
} user;

//para generar la lista enlazada
typedef struct Nodo {
 struct user_block* data;
 struct Nodo* next;
} Nodo;

struct Nodo* first=NULL;

void ellapsedTime(clock_t start);
struct Nodo* add_to_list(struct user_block* data);

int main(int argc, char* argv[]) //Ejemplos: select c1 c2 from t1   ||  select c1 from t1;
{
	clock_t start = clock(); //Iniciar timer...
	int pos_from=0;
	//Inicio Parser
	if(argc<4) //4 es la menor cantidad requerida de argumentos para poder realizar la petición
	{
		printf("Sintaxis Incorrecta. Debe ser: select [campo] from [tabla]\n");
		ellapsedTime(start);
		return false;
	}
	if(checkTableName(argv[argc-1])) //Si nombre de tabla esta disponible ->error porque tabla no existe
	{
		printf("Error: Tabla %s no existe\n",argv[argc-1]);
		ellapsedTime(start);
		return false;
	}
	int i=1;
	//validar que haya from en la sentencia
	int hasFrom=0;
	for(i=1;i<argc;i++)
		if(cadenasIguales(argv[i],"from"))
			hasFrom++;
	if(hasFrom!=1){
		printf("Sintaxis Incorrecta. Debe ser: select [campo] from [tabla]\n");
		ellapsedTime(start);
		return false;	
	}
	//comprobar que los campos existan en el catalogo
	i=1;
	while(!cadenasIguales(argv[i],"from")){
		if(checkFieldName(argv[i]))
		{
			printf("Error: Campo <%s> no existe\n",argv[i]);
			ellapsedTime(start);
			return false;	
		}
		i++;
	}
	pos_from=i;
	//Fin parser
	//INICIO creación lista
	FILE* streamer = fopen("usuario.dat", "r");
	int siguiente = cat.last; //empieza desde el final :v
	while(siguiente!=-1){
		int k;
		struct user_block** vector = (struct user_block**)calloc(cat.n_fields,sizeof(struct user_block)); //un vector dinamico para cada tupla... Así se pueden ordenar independientemente de la solicitud...
		for(k=0;k<cat.n_fields;k++) //ejecutar de tupla en tupla...es decir, varios bloque de usuario juntos
		{
		struct user_block* temp =(struct user_block*)malloc(sizeof(struct user_block));
		fseek(streamer,siguiente*sizeof(user),SEEK_SET);
		fread(temp, sizeof(user), 1, streamer);
		for(i=1;i<pos_from;i++) //trabaja el sector de argumentos donde se encuentran los nombres de las tablas.
			if(cadenasIguales(temp->field,argv[i])) //esto filtra los datos leidos del archivo para que solo agregue a la lista los que corresponden a los campos pedidos
				vector[i-1]=temp;
		siguiente=temp->next;
		}
		for(k=pos_from-2;k>=0;k--)//invertido porque add_to_list los ingresa desde el inicio, entonces sale en orden...
			add_to_list(vector[k]);
	}	
	fclose(streamer);
	//imprimir la lista INICIO
	for(i=0;i<pos_from-1;i++) //formato de tabla
		printf("-------------");
	printf("\n");
	for(i=1;i<pos_from;i++)//formato de tabla
		printf(" %-10s |",argv[i]);
	printf("\n");
	for(i=0;i<pos_from-1;i++)//formato de tabla
		printf("=============");
	printf("\n");
	struct Nodo* current = first;
	int j=0;
    while(current != NULL){ //recorre la lista imprimiendo los nodos...
		struct user_block* ptr = current->data;
		printf(" %-10s |",ptr->element);
        current = current->next;
		j++;
		if(j==pos_from-1){
			printf("\n");
			j=0;
		}
    }
	for(i=0;i<pos_from-1;i++)
		printf("-------------");
	printf("\n");
	//imprimir la lista FIN
	ellapsedTime(start);
	return true;		
}

//agrega a la lista...
struct Nodo* add_to_list(struct user_block* data)
{
    struct Nodo* ptr = (struct Nodo*)malloc(sizeof(struct Nodo));
    ptr->data = data;
    ptr->next = first;
    first = ptr;
    return ptr;
}

//Comprueba en el catalogo si el nombre dado de una tabla esta disponible, es decir, si aun no ha sido usado...
int checkTableName(char* name){ 
	FILE* streamer = fopen("catalogo.dat", "r");
    while (fread(&cat, sizeof(cat), 1, streamer))
        if (cadenasIguales(cat.name, name)&&cadenasIguales(cat.owner,"null")) {
			fclose(streamer);
            return false;
        }
    fclose(streamer);
	return true;
}

//Comprueba en el catalogo si el nombre dado para un campo esta disponible, es decir, si no existe en el catalogo...
int checkFieldName(char* name){ //1=No existe el nombre || 0= Ya existe el nombre //cambiado cat por cam.
	FILE* streamer = fopen("catalogo.dat", "r");
    while (fread(&cam, sizeof(cam), 1, streamer))
        if (cadenasIguales(cam.name, name)&&cadenasIguales(cam.owner,cat.name)) {
			fclose(streamer);
            //printf("%s Campo Valido\n",cam.name);
            return false;
        }
    fclose(streamer);
	return true;
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

