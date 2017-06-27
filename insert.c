#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
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
	int row; //= 1
	char element[15]; //juan
	char table[10]; // =t1
	char field[10]; // = nombre
} user;

struct cat_block** campos; //Arreglo Dinamico Global.

void createUserBlock2(char* element,char* field, char* table, struct user_block** tupla);
int validateInteger(char* number);
int validateString(char* word);
int validateFloat(char* decimal);
int n_fields(char* table);
void loadTable();
void createUserBlock(struct user_block* bloque);
void ellapsedTime(clock_t start);

int main(int argc, char* argv[]){
	clock_t start = clock(); //Iniciar timer...
		if(argc==1)
		{
			printf("Sintaxis Basica: insert [tabla] [campo] values [valor]\nPuede ingresar la cantidad de campos que permite la tabla\n");
			ellapsedTime(start);
			return true;
		}
		if(argc>4)
		{
			//INICIO PARSER
			if(!checkTableName(argv[1]))
			{
				int i;
				int hasValues=0;
				for(i=0;i<argc;i++)
					if(cadenasIguales(argv[i],"values"))
						hasValues++;
				if(hasValues!=1){
					printf("Error: Cantidad de 'values' encontrados: %d. Revisar sintaxis'\n",hasValues);
					ellapsedTime(start);
					return false;	
				}
				i=2;
				while(!cadenasIguales(argv[i],"values"))
					i++;	
				//Revisamos la sentencia de campos y elementos escritos en consola y valida que todo se encuentre correcto antes de guardar.
				int posValue=i+1;	//Esta variable la usamos para validar la sentencia de los fields.
				int posValueFinal=i+1;	//Esta variable es para guardar los elementos en el archivo.
				for(i=0;i+2<posValue-1;i++){
					if(!checkExpression(argv[2+i], argv[posValue+i])){
						ellapsedTime(start);
						return false;
					}	
				}
				//FIN PARSER	
				loadTable();
				//Rellenar los espacios de una fila con NULL antes de insertar los elementos.				
				    struct user_block** tupla2 = (struct user_block**)calloc(cat.n_fields,sizeof(struct user_block)); //un vector dinamico para una tupla... Así se pueden ordenar antes de guardar en disco...
					for(i=0;i<cat.n_fields;i++){ //bloques de usuario en null temporales..
						struct user_block* temp =(struct user_block*)malloc(sizeof(struct user_block));
						createUserBlock2("NULL",campos[i]->name,cat.name,tupla2);
				}
				//empezar a guardar...
				for(i=0;i+2<posValueFinal-1;i++)//bloques de usuario del usuario
					createUserBlock2(argv[posValueFinal+i],argv[2+i],argv[1],tupla2);
				for(i=0; i<cat.n_fields; i++){//grabar en archivo
					createUserBlock(tupla2[i]);
					updateLastTable();			
				}
				printf("Insercion realizada con exito\n");
				ellapsedTime(start);
				return true;
			}	
			else{
			printf("Error: Tabla <%s> no existe\n",argv[1]);
			ellapsedTime(start);			
			return false;	
			}
		}
		else
		{
		printf("Error: No hay suficientes argumentos. Escriba 'insert' para mas informacion\n");
		ellapsedTime(start);
		return false;
		}
}

void createUserBlock(struct user_block* bloque)	{
	bloque->row=calculateRow(bloque);
	bloque->next=cat.last;
	FILE* streamer = fopen("usuario.dat", "a");
	fwrite(bloque, sizeof(user), 1, streamer);
	fclose(streamer);	
	cat.last=calcPos();	
}

void createUserBlock2(char* element,char* field, char* table, struct user_block** tupla)	{	
	int i = 0;
	for(i=0; i<cat.n_fields; i++){
		if(cadenasIguales(campos[i]->name, field)){
			struct user_block* temp =(struct user_block*)malloc(sizeof(struct user_block));
			strcpy(temp->element, element);
			strcpy(temp->field, field);
			strcpy(temp->table, table);
			tupla[i] = temp;
		}
	}	
}

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

int calculateRow(struct user_block* bloque)
{
	FILE* streamer = fopen("catalogo.dat", "r+");
	int pos=0;
	int num=0;
	while (fread(&aux, sizeof(aux), 1, streamer)){
        if (cadenasIguales(aux.name, bloque->field)&&cadenasIguales(bloque->table,aux.owner)) {
			num = aux.n_fields;
			aux.n_fields++;
            fseek(streamer,pos*sizeof(aux),SEEK_SET);
			fwrite(&aux, sizeof(aux), 1, streamer);
		    fclose(streamer);
			return num;
        }
		pos++;
	}
	fclose(streamer);
	return 0;
}

int updateLastTable() //Funcionando
{
	FILE* streamer = fopen("catalogo.dat", "r+");
	int pos=0;
	while (fread(&cam, sizeof(cam), 1, streamer))
	{
		//printf("CAM Owner: %s, Name: %s, Last: %d\n",cam.owner,cam.name,cam.last);
		//printf("CAT Owner: %s, Name: %s, Last: %d\n",cat.owner,cat.name,cat.last);
		if(cadenasIguales(cam.name, cat.name)&&cadenasIguales(cat.owner,"null"))
		{
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

//tamaño archivo/tamaño de cada registro= n -> fseek(n-1) -> pos donde se guardo ese registro...
int calcPos()
{
	FILE* stream = fopen("usuario.dat", "a");
	fseek(stream,0,SEEK_END);
	int pos = ftell(stream);
	pos/=sizeof(user);
	//printf("%d\n",pos);
	fclose(stream);
	return pos-1;
}

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

//Validacion para Numeros enteros.
int validateInteger(char* number){
	int i = 0, esNumero = true, j;
	j = strlen(number); //Guardamos la longitud de la cadena de numeros ingresada.
	
	while( i < j && esNumero == true){ //Recorremos la cadena de numeros
		if(isdigit(number[i]) == true){ //Se comprueba si cada digito corresponde a un numero
			i++;
		}
		else
			esNumero = false;	//Si se encuentra algo diferente, se sale del ciclo.
	}
	return esNumero;
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
		if (puntos > 1){
			esDecimal = false;
		}
		 if (isalpha(decimal[i])){
			esDecimal = false;
		}
	}
	return esDecimal;
}

int checkExpression(char* field, char* element){ //true = valido || false = no valido
	if(checkFieldName(field)){
		printf("%s no es un campo reconocido\n", field);
		return false;
	}
	if(cadenasIguales(cam.type,"int")){
		if(!validateInteger(element)){
			printf("Valor no corresponde con el tipo de campo\n");
			return false;
			}
		}
	else
		if(cadenasIguales(cam.type,"float")){
			if(!validateFloat(element))
			{
				printf("Valor no corresponde con el tipo de campo\n");
				return false;
			}
		}
		else
			return true;
}

void loadTable(){
	int i=0, cont=0;
	
	FILE* streamer = fopen("catalogo.dat", "r");
	campos = (struct cat_block**)calloc(cat.n_fields,sizeof(struct cat_block)); 
	
	while (i < cat.n_fields){
		struct cat_block* temp =(struct cat_block*)malloc(sizeof(struct cat_block));
		fread(temp, sizeof(cat), 1, streamer);
		if(cadenasIguales(temp->owner, cat.name)){
			campos[i] = temp;
			i++;
		}
	}
	fclose(streamer);
}

void ellapsedTime(clock_t start){ 
	clock_t diff = clock() - start;
	int msec = diff * 1000 / CLOCKS_PER_SEC;
	printf("Procesado en %d.%d segundos\n",msec/1000,msec%1000);
}
