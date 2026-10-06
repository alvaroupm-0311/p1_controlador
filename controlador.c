/*********************************************************
 *
 * ALUMNOS QUE HAN REALIZADO ESTA PRÁCTICA:
 *
 * GRUPO: so-XXX-YY
 *
 * ALUMNO 1
 *   Nombre:
 *   Correo:
 *
 * ALUMNO 2
 *   Nombre:
 *   Correo:
 *
 *********************************************************/
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sysexits.h>
#include <time.h>

//Constante de ejecucion
#define HIJOS_MAX 4

static void uso(void);
static void convertir(const char* fich_video, const char* dir_resultados);
static void esperar_hijos(int *hijos_activos, int *exitos, int *fallos);

int main(int argc, char** argv)
{
	const char* dir_resultados;
	if (argc < 3) {
		uso();
		exit(EX_USAGE);
	}
	dir_resultados = argv[1];
	int hijos_activos = 0;
	int exitos = 0;
	int fallos = 0;

	time_t inicio = time(NULL);

	for (int i = 2; i < argc; i++){
		while(hijos_activos>=HIJOS_MAX){
			esperar_hijos(&hijos_activos, &exitos, &fallos);
		} 
		pid_t PID_HIJO = fork();
		if(PID_HIJO == 0){

			printf("Procesando video '%d' \n", i);
			convertir(argv[i], dir_resultados);

		}else if (PID_HIJO <0){
			perror("Hubo un fallo\n");
			
		}else{

			hijos_activos ++;
		} 
		
	}

	while(hijos_activos>0){
		esperar_hijos(&hijos_activos, &exitos, &fallos);
	} 

	time_t fin = time(NULL);

	printf("\tRESUMEN FINAL\n");
	printf("\t\tNumero de exitos %d", exitos);
	printf("\t\tNumero de fallos %d", fallos);
	printf("\t\tTiempo de ejecucion %0.f", difftime(fin, inicio));

	exit(EX_OK);
}

static void uso(void)
{
	fprintf(stderr, "\nUso: paralelo dir_resultados fich_video...\n");
	fprintf(stderr, "\nEjemplo: paralelo fotogramas orig/*.mp4\n\n");
}

static void convertir(const char* fich_video, const char* dir_resultados)
{
	const char* nombre_base;
	char nombre_sin_ext[MAXPATHLEN];
	char nombre_destino[MAXPATHLEN];
	char orden[MAXPATHLEN*3];
	char* punto;

	nombre_base = strrchr(fich_video, '/');
	if (nombre_base == NULL)
		nombre_base = fich_video;
	else
		nombre_base++;

	/* Copiamos el nombre base y le quitamos la extensión, si la tiene,
	 * para sustituirla por ".jpg", que es el formato del fotograma
	 * extraído. */
	snprintf(nombre_sin_ext, sizeof(nombre_sin_ext), "%s", nombre_base);
	punto = strrchr(nombre_sin_ext, '.');
	if (punto != NULL)
		*punto = '\0';

	snprintf(nombre_destino, sizeof(nombre_destino), "%s/%s.jpg", dir_resultados, nombre_sin_ext);

	/* -y: sobrescribe el fichero destino si ya existe.
	 * -i: fichero de entrada (el vídeo).
	 * -vframes 1: extrae un único fotograma.
	 * -q:v 2: calidad alta para el JPEG resultante. */
	snprintf(orden, sizeof(orden), "ffmpeg -y -i '%s' -vframes 1 -q:v 2 '%s'", fich_video, nombre_destino);
	//fprintf(stderr, "AVISO: La versión baśica del programa usa system() para lanzar procesos nuevos. Los estudiantes deben cambiarla por fork-exec-wait\n");
	//system(orden);
    execlp("ffmpeg", "ffmpeg", "-y", "-i", fich_video, "-vframes", "1", "-q:v", "2", nombre_destino, NULL);
	perror("execlp");
	exit(EXIT_FAILURE);
}
static void esperar_hijos(int *hijos_activos, int *exitos, int *fallos){
	int estado;
	pid_t pid = wait(&estado);
	if(pid<0){
		perror("wait");
		return;			
	}

	(*hijos_activos)--;

	if(WIFEXITED(estado) && WEXITSTATUS(estado) == 0)
		(*exitos)++;
	else
		(*fallos)++;
			
}