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
} cat,cam;

struct user_block{
	int next; //Siguiente bloque fseek para ser leido;
	int row; //= fila en la que se encuentra este bloque
	char element[15]; //Dato que almacena esta estructura
	char table[10]; //Tabla a la que pertenece este bloque
	char field[10]; //Campo al que pertenece este bloque
} user,aux;

int checkTableName(char* name);
int checkFieldName(char* name);
int cadenasIguales(char* cad0, char* cad1);
int checkExpression(char* exp);
int delete(char* cam_cond, char* val_cond);
int updateLastTable();

//DELETE table_name WHERE column1='Diego' ; No es necesario colocar todos los nombres de los campos de la tabla.
int main(int argc, char* argv[]){
	int hasWhere = 0, i;
	if(argc < 4){
		printf("Sintaxis Incorrecta. \n Debe ser: delete [nombre_tabla] [campo] WHERE [campo = elemento_existente] \n");
		return false;
	}
	if(checkTableName(argv[1])){ //Si nombre de tabla esta disponible ->error porque tabla no existe.
		printf("Error: Tabla %s no existe\n",argv[1]);
		return false;
	}
	for(i=2; i<argc; i++){ //contar cantidad de where's. Si hay mas de uno debe dar error.
		if(cadenasIguales(argv[i], "where")){
			hasWhere++;
		}	
	}
	//Lo ideal es trabajar con una sola columna para borrar esa fila.
	if(cadenasIguales(argv[argc-1],"where")){// where sin condicion igual a error
		printf("Error: No hay condicional despues de where o hay varias condicionales\n");
		return false;	
	}
	if(hasWhere!=1){ //Muestra un error si hay mas de un where en la expresion
		printf("Error: Cantidad de where encontrados: %d \n",hasWhere);
		return false;	
	}
	//Probaremos solo con 1 campo para encontrar la fila a borrar.
	if(!checkExpression(argv[3])){
		printf("Error: El campo digitado no existe en la tabla.");
		return false;
	}
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
int checkFieldName(char* name){ //1=No existe el nombre || 0= Ya existe el nombre
	FILE* streamer = fopen("catalogo.dat", "r");
    while (fread(&cam, sizeof(cam), 1, streamer))
        if (cadenasIguales(cam.name, name)&&!cadenasIguales(cam.owner,"null")) {
			fclose(streamer);
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

//Separa la expresion y luego hace el delete...
int checkExpression(char* exp){//true = valido || false = no valido
	char* expression =strdup(exp);
	char* name = strtok(expression,"=");
	char* value = strtok(NULL,"=");
	if(checkFieldName(name))
		return false;
	delete(name, value);
	return true;		
}

//Actualiza el last de la tabla en caso de que elemento borrado sea el ultimo que se ingreso a usuarios.dat
int updateLastTable()
{
	FILE* streamer = fopen("catalogo.dat", "r+");
	int pos=0;
	while (fread(&cam, sizeof(cam), 1, streamer))
	{
		if(cadenasIguales(cam.name, cat.name)&&cadenasIguales(cat.owner,"null"))//busca la tabla que debe ser actualizada
		{//y la sobreescribe
		fseek(streamer,pos*sizeof(cat),SEEK_SET);
			fwrite(&cat, sizeof(cat), 1, streamer);
		    fclose(streamer);
			return true;
		}
		pos++;
	}
    fclose(streamer);
	return true;
}

//Borra un registro completo donde encuentre que campo_condicional contiene valor_condicional
int delete(char* cam_cond, char* val_cond) 
{
	FILE* streamer = fopen("usuario.dat", "r+");
	int pos=cat.last;
	struct user_block** tupla = (struct user_block**)calloc(cat.n_fields,sizeof(struct user_block));
	struct user_block* anterior =(struct user_block*)malloc(sizeof(struct user_block));
	anterior=NULL;//guarda el bloque anterior para poder accceder al next de el y cambiarlo al next del ultimo de la tupla
	int deletable,i,ult_pos = 0, cont = 0;
	while (pos!=-1)//mientras no sea el fin de esta tabla
	{
		//printf("POS: %d\n",pos);
		//printf("Leyendo una nueva tupla\n");
		deletable=0;//Si pasa a 1(True) indica que la tupla actual debe ser eliminada
		if(anterior!=NULL)
			ult_pos=tupla[cat.n_fields-2]->next;
		for(i=0;i<cat.n_fields;i++)//Recorra la tupla y carguela con los user_block correspondientes
		{
			struct user_block* elemento =(struct user_block*)malloc(sizeof(struct user_block));
			fseek(streamer,pos*sizeof(user),SEEK_SET);
			fread(elemento, sizeof(user), 1, streamer);
			tupla[i]=elemento;
			pos=elemento->next;
			//printf("Pos: %d Cargando tupla[%d] con valor %s correspondiente a fila %d NEXT: %d\n",ult_pos,i,tupla[i]->element,tupla[i]->row,tupla[i]->next);
			if(cadenasIguales(elemento->field,cam_cond)&&cadenasIguales(elemento->element,val_cond)) //Analiza la condicion
				deletable=1;
		}
		if(deletable==1)//Si es borrable entonces evalue si es la tupla inmediata desde el ultimo de la tabla o en medio de varias tuplas
		{
			//printf("Tupla de arriba es borrable\n");
			cont++;
			if(anterior==NULL)//Si es la primera tupla que se leyo (Desde el ultimo indicado en la tabla que esta en catalogo)
			{
				cat.last=pos;
				//printf("Cambiando valor de cat.last en tabla\n");
				updateLastTable();
			}
			else//Si es una tupla que se encuentra entre varias tuplas o es la ultima lectura del archivo...
			{
				anterior->next=tupla[cat.n_fields-1]->next;
				//printf("Cambiando next del bloque anterior(el que tiene %s en fila %d con next %d)\n", anterior->element,anterior->row,anterior->next);
				//guardar bloque actualizado
				fseek(streamer,ult_pos*sizeof(user),SEEK_SET);
				fwrite(anterior, sizeof(user), 1, streamer);
			}
		}
		else
		{
			anterior=tupla[cat.n_fields-1];	//cambia el anterior como el ultimo elemento de la tupla actual
		}									//Esto prepara el anterior de la siguiente tupla...
	}
    fclose(streamer);
	printf("Cantidad de registros borrados: %d ", cont);
	return true;	
}


