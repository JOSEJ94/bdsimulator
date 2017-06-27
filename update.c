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

struct user_block
{
	int next;
	int row; //= 1
	char element[15]; //juan
	char table[10]; // =t1
	char field[10]; // = nombre
} user,aux;

void ellapsedTime(clock_t start);

/*
UPDATE table_name SET column1=value1 column2=value2 WHERE some_column=some_value;
*/

int main(int argc, char* argv[])
{
	clock_t start = clock(); //Iniciar timer...
	int hasWhere=0;
	int posWhere=0;
	//Inicio Parser
	if(argc<4) //4 es la menor cantidad requerida de argumentos para poder realizar la petición Update t1 set c1=1
	{
		printf("ERROR: SINTAXIS INCORRECTA\n");
		printf("Debe ser: update [tabla] set [campo]=[valor]\n");
		printf("O tambien: update [tabla] set [campo]=[valor] where [campo]=[valor] \n");
		ellapsedTime(start);
		return false;
	}
	if(checkTableName(argv[1])) //Si nombre de tabla esta disponible ->error porque tabla no existe
	{
		printf("Error: Tabla %s no existe\n",argv[1]);
		ellapsedTime(start);
		return false;
	}
	int i=1;
	//validar que haya set en la sentencia
	if(!cadenasIguales(argv[2],"set")){
		printf("Sintaxis Incorrecta. SET no encontrado\n");
		ellapsedTime(start);
		return false;	
	}
	if(cadenasIguales(argv[argc-1],"where"))// where sin condicion igual a error
	{
		printf("Error: No hay condicional despues de where\n",argv[i]);
		ellapsedTime(start);
		return false;	
	}
	//comprobar que los campos existan en el catalogo
	for(i=3;i<argc;i++)
	{
		if(cadenasIguales(argv[i],"where")){
			posWhere=i;
			hasWhere++;
		}else{
			if(!checkExpression(argv[i]))// campo=valor 
			{
				printf("Error: Campo o valor erroneo\n",argv[i]);
				ellapsedTime(start);
				return false;	
			}
		}
	}
	if(hasWhere>1)
	{
		printf("Error: Cantidad de 'where' encontrados: %d \n",hasWhere);
		ellapsedTime(start);
		return false;		
	}
	//Fin parser
	if(hasWhere==0)
	{
		for(i=3;i<argc;i++)
		{
			char* expression =strdup(argv[i]);
			char* name = strtok(expression,"=");
			char* value = strtok(NULL,"=");
			updateAll(name,value);		
		}	
	}
	else
	{
		for(i=3;i<posWhere;i++)
		{
			char* expression =strdup(argv[i]);
			char* campo = strtok(expression,"=");
			char* valor = strtok(NULL,"=");
			char* conditional =strdup(argv[posWhere+1]);
			char* fieldcond = strtok(conditional,"=");
			char* valuecond = strtok(NULL,"=");
			update(campo,valor,fieldcond,valuecond);
		}
	}
	printf("Fin\n",argv[i]);
	ellapsedTime(start);
	return true;
}

int updateAll(char* campo,char* element) 
{
	FILE* streamer = fopen("usuario.dat", "r+");
	int pos=cat.last;
	while (pos!=-1)
	{
		fseek(streamer,pos*sizeof(user),SEEK_SET);
		fread(&user, sizeof(user), 1, streamer);
		if(cadenasIguales(user.field,campo)&&cadenasIguales(user.table,cat.name))
		{
			strcpy(user.element,element);
			fseek(streamer,pos*sizeof(user),SEEK_SET);
			fwrite(&user, sizeof(user), 1, streamer);
		}
		pos=user.next;
	}
    fclose(streamer);
	return true;	
}

int update(char* campo,char* element, char* cam_cond, char* val_cond) 
{
	FILE* streamer = fopen("usuario.dat", "r+");
	int pos=cat.last;
	struct user_block** tupla = (struct user_block**)calloc(cat.n_fields,sizeof(struct user_block));
	struct user_block* anterior =(struct user_block*)malloc(sizeof(struct user_block));
	anterior=NULL;
	int updatable,i,ult_pos;
	while (pos!=-1)
	{
		//printf("POS: %d\n",pos);
		//printf("Leyendo una nueva tupla\n");
		updatable=0;
		if(anterior!=NULL)
			ult_pos=tupla[cat.n_fields-2]->next;
		for(i=0;i<cat.n_fields;i++)
		{
			struct user_block* elemento =(struct user_block*)malloc(sizeof(struct user_block));
			fseek(streamer,pos*sizeof(user),SEEK_SET);
			fread(elemento, sizeof(user), 1, streamer);
			tupla[i]=elemento;
			pos=elemento->next;
			//printf("Pos: %d Cargando tupla[%d] con valor %s correspondiente a fila %d NEXT: %d\n",ult_pos,i,tupla[i]->element,tupla[i]->row,tupla[i]->next);
			if(cadenasIguales(elemento->field,cam_cond)&&cadenasIguales(elemento->element,val_cond)) //Analiza la condicion
				updatable=1;
		}
		if(updatable==1)
		{
			//printf("Tupla de arriba es actualizable\n");
			if(anterior==NULL)
			{
				ult_pos=cat.last;
				for(i=0;i<cat.n_fields;i++)
				{
					if(cadenasIguales(tupla[i]->field,campo)){
						strcpy(tupla[i]->element,element);
						fseek(streamer,ult_pos*sizeof(user),SEEK_SET);
						fwrite(tupla[i], sizeof(user), 1, streamer);
					}
					ult_pos=tupla[i]->next;
				}
			}
			else
			{
				ult_pos=anterior->next;
				for(i=0;i<cat.n_fields;i++)
				{
					if(cadenasIguales(tupla[i]->field,campo)){
						strcpy(tupla[i]->element,element);
						fseek(streamer,ult_pos*sizeof(user),SEEK_SET);
						fwrite(tupla[i], sizeof(user), 1, streamer);
					}
					ult_pos=tupla[i]->next;
				}
			}
		}
		else
		{
			anterior=tupla[cat.n_fields-1];	
		}
	}
    fclose(streamer);
	return true;	
}

int updateLastTable()
{
	FILE* streamer = fopen("catalogo.dat", "r+");
	int pos=0;
	while (fread(&cam, sizeof(cam), 1, streamer))
	{
		if(cadenasIguales(cam.name, cat.name)&&cadenasIguales(cat.owner,"null"))
		{
		fseek(streamer,pos*sizeof(cat),SEEK_SET);
			fwrite(&cat, sizeof(cat), 1, streamer);
			//printf("Cambiando valor de cat.last a %d\n",cat.last);
		    fclose(streamer);
			return true;
		}
		pos++;
	}
    fclose(streamer);
	return true;
}

//Comprueba en el catalogo si el nombre dado de una tabla esta disponible, es decir, si aun no ha sido usado...
int checkTableName(char* name){ 
	FILE* streamer = fopen("catalogo.dat", "r");
	if(!streamer)
		streamer = fopen("catalogo.dat","w+");
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
        if (cadenasIguales(cam.name, name)&&!cadenasIguales(cam.owner,"null")) {
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

int checkExpression(char* exp){//true = valido || false = no valido
	char* expression =strdup(exp);
	char* name = strtok(expression,"=");
	char* value = strtok(NULL,"=");
	if(checkFieldName(name))
		return false;
	if(cadenasIguales(cam.type,"int"))
	{
		if(!validateInteger(value))
			return false;
	}
	else
		if(cadenasIguales(cam.type,"float"))
		{
			if(!validateFloat(value))
				return false;
		}
	return true;		
}

int validateInteger(char* number) {
	int i = 0, esNumero = true, j;
	j = strlen(number); //Guardamos la longitud de la cadena de numeros ingresada.

	while (i < j) { //Recorremos la cadena de numeros
		if (!isdigit(number[i])) //Se comprueba si cada digito corresponde a un numero
			return false;
		i++;
		
	}
	return true;
}

int validateFloat(char* decimal) {
	int i = 0, esDecimal = true, j, puntos = 0;
	j = strlen(decimal);

	while (i < j && esDecimal == true) {
		if (isdigit(decimal[i])) {
			i++;
		}
		if (decimal[i] == '.') {
			i++;
			puntos++;
		}
		if (puntos > 1)
			esDecimal = false;
		if(isalpha(decimal[i]))
			return false;
	}
	return esDecimal;
}

/*
int update(char* campo,char* element, char* cam_cond, char* val_cond) 
{
	FILE* streamer = fopen("usuario.dat", "r+");
	int pos=cat.last;
	struct user_block** tupla = (struct user_block**)calloc(cat.n_fields,sizeof(struct user_block));
	struct user_block* anterior =(struct user_block*)malloc(sizeof(struct user_block));
	anterior=NULL;
	int updatable,i,ult_pos;
	while (pos!=-1)
	{
		//printf("POS: %d\n",pos);
		//printf("Leyendo una nueva tupla\n");
		updatable=0;
		if(anterior!=NULL)
			ult_pos=tupla[cat.n_fields-2]->next;
		for(i=0;i<cat.n_fields;i++)
		{
			struct user_block* elemento =(struct user_block*)malloc(sizeof(struct user_block));
			fseek(streamer,pos*sizeof(user),SEEK_SET);
			fread(elemento, sizeof(user), 1, streamer);
			tupla[i]=elemento;
			pos=elemento->next;
			//printf("Pos: %d Cargando tupla[%d] con valor %s correspondiente a fila %d NEXT: %d\n",ult_pos,i,tupla[i]->element,tupla[i]->row,tupla[i]->next);
			if(cadenasIguales(elemento->field,cam_cond)&&cadenasIguales(elemento->element,val_cond)) //Analiza la condicion
				updatable=1;
		}
		if(updatable==1)
		{
			//printf("Tupla de arriba es actualizable\n");
			if(anterior==NULL)
			{
				ult_pos=cat.last;
				for(i=0;i<cat.n_fields;i++)
				{
					if(cadenasIguales(tupla[i]->field,campo)){
						strcpy(tupla[i]->element,element);
						fseek(streamer,ult_pos*sizeof(user),SEEK_SET);
						fwrite(tupla[i], sizeof(user), 1, streamer);
					}
					ult_pos=tupla[i]->next;
				}
			}
			else
			{
				ult_pos=anterior->next;
				for(i=0;i<cat.n_fields;i++)
				{
					if(cadenasIguales(tupla[i]->field,campo)){
						strcpy(tupla[i]->element,element);
						fseek(streamer,ult_pos*sizeof(user),SEEK_SET);
						fwrite(tupla[i], sizeof(user), 1, streamer);
					}
					ult_pos=tupla[i]->next;
				}
			}
		}
		else
		{
			anterior=tupla[cat.n_fields-1];	
		}
		//if(anterior!=NULL)
		//	printf("Anterior: Fila %s Valor %s Next: %d\n", anterior->row,anterior->element,anterior->next);
	}
    fclose(streamer);
	return true;	
}
*/

