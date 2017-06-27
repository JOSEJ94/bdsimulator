 #include <stdio.h>
struct cat_block
{  
    char owner[10]; //tabla
    char name[10];  //nombre
	char type[10]; //tipo
	int n_fields; //cantidad de campos
	int last; //ultimo campo ingresado...
} cat,cam;

void main()
{
	printf("Mostrando informacion del archivo catalogo.dat\n");
	FILE* streamer = fopen("catalogo.dat", "r");
	printf("------------------------------------------------------\n");
	printf("%-10s  %-10s  %-10s  %-10s  %-10s\n","Owner","Name","Type","N_Fields","Last");
	printf("======================================================\n");
    while (fread(&cat, sizeof(cat), 1, streamer)) 
       	printf("%-10s| %-10s| %-10s| %-10d| %-10d\n", cat.owner, cat.name, cat.type,cat.n_fields,cat.last);
    fclose(streamer);
	printf("------------------------------------------------------\n");
			
}