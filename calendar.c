/* $RCSfile: calendar.c,v $ -- programa para ejecutar programas en fechas determinadas.
 *	Incluir este programa en el fichero autoexec.bat para que se ejecute
 *	al encender el ordenador.
 * Autor: Luis Colorado.
 * $Log: calendar.c,v $
 * Revision 1.1  1995/10/02 19:07:28  luis
 * Initial revision
 *
 * Revision 4.3  1995/04/02  16:28:05  luis
 * *** empty log message ***
 *
 * Revision 4.2  1995/04/02  16:24:56  luis
 * Inclusion de los campos estandar de RCS
 *
 * Revision 4.1  1995/04/02  16:18:19  luis
 * Correccion de un error referente al modo de calcular
 * la forma de actualizar las fechas.  Antes se sumaba
 * 86400s a la fecha en curso.  Esto tenia el problema
 * de fallar en los cambios de hora debido a que cuando
 * se adelanta una hora el dia no llega a 86400s y por tanto,
 * el sumar 86400s a un instante de tiempo puede dar lugar a
 * dos cambios de fecha en vez de uno.  Ahora se usa como
 * variable temporal una que mantiene la hora con la fecha en
 * formato local, en la que no intervienen los cambios de hora
 * debido a que los cambios se producen en la parte de la fecha
 * de la estructura tm.
 *
 * Revision: 3.2 (94/06/08)
 * Revision: 2.3 (29.11.93).
 * Revision: 2.2 (4.10.93).
 * Revision: 2.1 (3.10.93).
 * Revision: 2.0 (2.10.93).
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define IMPRIMIR_FECHAS 1
#define DEBUG(X)
#define RETURN(X) { result = (X); goto final; }

int  dias_mes [2] [12] = {
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

int tipo_anio (int y)
{
	int result;

	DEBUG (
		fprintf (stderr,
			"===>tipo_anio (%d)", y);
	)
	if (y % 400 == 0) RETURN (1);
	if (y % 100 == 0) RETURN (0);
	RETURN ((y % 4) == 0);
final:
	DEBUG (
		fprintf (stderr,
			"...%d\n", result);
	)
	return result;
}

int avanzar_tm (struct tm *f, int n)
{
	int ta; /* TIPO DE AÑO */

	DEBUG (
		fprintf (stderr, "===>avanzar_tm ({\n");
		fprintf (stderr, "    tm_wday->%d,\n", f->tm_wday);
		fprintf (stderr, "    tm_yday->%d,\n", f->tm_yday);
		fprintf (stderr, "    tm_mday->%d,\n", f->tm_mday);
		fprintf (stderr, "    tm_mon->%d,\n", f->tm_mon+1);
		fprintf (stderr, "    tm_year->%d}, %d)\n", f->tm_year+1900,
			n);
	)
	/* ACTUALIZAMOS EL DIA DE LA SEMANA */
	f->tm_wday += n;
	f->tm_wday %= 7;

	/* ...EL DIA DEL MES (Y EL DIA DEL AÑO) */
	f->tm_mday += n;
	f->tm_yday += n;
	ta = tipo_anio (f->tm_year + 1900);
	while (f->tm_mday > dias_mes [ta][f->tm_mon])
	{
		f->tm_mday -= dias_mes [ta][f->tm_mon];

		/* ...EL MES DEL AÑO */
		f->tm_mon++;
		if (f->tm_mon >= 12)
		{
			f->tm_mon = 0;
			/* ...EL AÑO */
			f->tm_yday -= 365 - ta;
			f->tm_year++;
			ta = tipo_anio (f->tm_year + 1900);
		}
	}
	DEBUG (
		fprintf (stderr, "<===avanzar_tm ({\n");
		fprintf (stderr, "    tm_wday->%d,\n", f->tm_wday);
		fprintf (stderr, "    tm_yday->%d,\n", f->tm_yday);
		fprintf (stderr, "    tm_mday->%d,\n", f->tm_mday);
		fprintf (stderr, "    tm_mon->%d,\n", f->tm_mon+1);
		fprintf (stderr, "    tm_year->%d}, %d)\n", f->tm_year+1900,
			n);
	)
}

cmp_tm (struct tm *t1, struct tm *t2)
{
	int result;

	DEBUG (
		fprintf (stderr, "===>cmt_tm (%d/%d/%d[%d], %d/%d/%d[%d])",
			t1->tm_mday, t1->tm_mon+1,
			t1->tm_year+1900, t1->tm_wday,
			t2->tm_mday, t2->tm_mon+1,
			t2->tm_year+1900, t2->tm_wday);
	)

	if (t1->tm_year != t2->tm_year)
		RETURN (t1->tm_year - t2->tm_year);
	if (t1->tm_yday != t2->tm_yday)
		RETURN (t1->tm_yday - t2->tm_yday);
	RETURN (0);
final:
	DEBUG (
		fprintf (stderr, "...%d\n", result);
	)
	return result;
}

int comprobar (char *c, int n)
/* comprueba si un determinado numero n esta en una lista indicada en c */
{
	char *p_term;
	int x1, x2;
	static char *sep = ",";
	int result;

	DEBUG(
		fprintf (stderr,
			"==>comprobar (\"%s\", %d)", c, n);
	)

	if (strcmp (c, "*") == 0) RETURN (1);

	for (	p_term = strtok (c, sep);
		p_term;
		p_term = strtok (NULL, sep)
	) {
		switch (sscanf (p_term, "%u-%u", &x1, &x2)) {
		case 1:
			if (n == x1) RETURN (1);
			break;
		case 2:
			if ((x1 <= n) && (n <= x2)) RETURN (1);
			break;
		default:
			fprintf (stderr,
				"calendar: Error de formato\n");
			RETURN (0);
		}
	}
	RETURN (0);
final:
	DEBUG (
		fprintf (stderr, "===> %d\n", result);
	)
	return result;
}

void procesa_macros (char *s, struct tm *t)
{
	static char auxiliar [1000];
	char *p, *q;
	static char *tab_dsem [] = {
		"domingo", "lunes", "martes", "miercoles",
		"jueves", "viernes", "sabado"
	};
	static char *tab_mes [] = {
		"enero", "febrero", "marzo", "abril", "mayo", "junio",
		"julio", "agosto", "septiembre", "octubre", "noviembre",
		"diciembre",
	};
	DEBUG (
		fprintf (stderr, "===>procesa_macros (\"%s\", ...)", s);
	)
	p = s;
	q = auxiliar;
	while (*p) {
		if (*p != '$') {
			*q++ = *p++;
			continue;
		}
		p++;
		switch (*p) {
		case '$':
			*q++ = *p++;
			continue;
		case '\0':
			*q++ = '$';
			continue;
		case 'a':
			q += sprintf (q, "%02d", t->tm_year % 100);
			p++;
			continue;
		case 'A':
			q += sprintf (q, "%4d", t->tm_year + 1900);
			p++;
			continue;
		case 'm':
			q += sprintf (q, "%d", t->tm_mon + 1);
			p++;
			continue;
		case 'M':
			q += sprintf (q, "%s", tab_mes [t->tm_mon]);
			p++;
			continue;
		case 'd':
			q += sprintf (q, "%02d", t->tm_mday);
			p++;
			continue;
		case 'D':
			q += sprintf (q, "%d", t->tm_mday);
			p++;
			continue;
		case 's':
			q += sprintf (q, "%d", t->tm_wday);
			p++;
			continue;
		case 'S':
			q += sprintf (q, "%s", tab_dsem [t->tm_wday]);
			p++;
			continue;
		default:
			*q++ = '$';
			*q++ = *p++;
			continue;
		}
	}
	*q = '\0';
	strcpy (s, auxiliar);
	DEBUG (
		fprintf (stderr, "...s <=== \"%s\"\n", s);
	)
}

void main (int argc, char **argv)
{
	char *p_l_ano, *p_l_mes, *p_l_dmes, *p_l_dsem, *p_com;
	time_t hora;
	int lim = 0;
	struct tm t1, t2;
	char n_fich [100];
	static char linea [1000];
	static char *lin [10000];
	int n;
	FILE *f;
	static char *sep1 = " \t\n";
	static char *sep2 = "\n";
	char *p;

	p = getenv ("HOME");
	sprintf (n_fich, "%s/.calendar.", p ? p : ".");
	argc--; argv++;

	/* fichero con los datos de la ultima ejecucion */
	strcpy (strrchr (n_fich, '.'), ".dat");
	while (argc)
	{
		if (strcmp (argv [0], "-a") == 0)
		{
			f = fopen (n_fich, "wt");
			if (f) {
				fprintf (f, "%ld\n", time (NULL));
				fclose (f);
			}
			exit (0);
		} else if (argv [0][0] == '+')
		{
			sscanf (argv [0], "%d", &lim);
		}
		argc--; argv++;
	}
	/* ABRIMOS EL FICHERO CON LOS DATOS DE LA ULTIMA EJECUCION */
	f = fopen (n_fich, "r");
	hora = 0L;
	if (f) {
		fscanf (f, "%ld", &hora);
		fclose (f);
	}

	/* COPIAMOS LA FECHA DE LA ULTIMA EJECUCION EN LA ESTRUCTURA
	 * t1. */
	memcpy (&t1, localtime (&hora), sizeof t1);

	/* OBTENEMOS LA FECHA LIMITE DEL CALCULO */
	time (&hora);
	memcpy (&t2, localtime (&hora), sizeof t2);
	avanzar_tm (&t2, lim);

	/* SI t2 ES ANTERIOR A LA FECHA ACTUAL, LO AVANZAMOS
	 * UN DIA PARA NO VOLVER A EJECUTAR LOS DATOS CORRESPONDIENTES
	 * A ESE DIA. */ 
 	if (cmp_tm (&t1, localtime (&hora)) < 0)
 		avanzar_tm (&t1, 1);

	/* leemos el fichero de configuracion */
	strcpy (strrchr (n_fich, '.'), ".cnf");
	f = fopen (n_fich, "rt");
	if (!f) {
		sprintf (linea, "calendar: %s", n_fich);
		perror (linea);
		exit (1);
	}
	n = 0;
	while (fgets (linea, sizeof linea, f)) {
		int l;
		char *p;

		p = strtok (linea, sep2);
		if (!p) continue;
		while (isspace (*p)) p++;
		if (p [0] == '#') continue;
		l = strlen (p);
		lin [n] = malloc (l + 1);
		strcpy (lin [n], p);
		DEBUG (
			fprintf (stderr, "%s-->%s\n", n_fich, p);
		)
		n++;
	}
	fclose (f);


#ifdef IMPRIMIR_FECHAS
	fprintf (stderr,
		"Desde Fecha:      %d/%d/%d %d:%d:%d\n",
		t1.tm_mday, t1.tm_mon + 1, t1.tm_year + 1900,
		t1.tm_hour, t1.tm_min, t1.tm_sec);
	fprintf (stderr,
		"Hasta Fecha:      %d/%d/%d %d:%d:%d\n",
		t2.tm_mday, t2.tm_mon + 1, t2.tm_year + 1900,
		t2.tm_hour, t2.tm_min, t2.tm_sec);
#endif

	while (cmp_tm (&t1, &t2) <= 0) {
		int i;
		for (i = 0; i < n; i++) {
			strcpy (linea, lin [i]);
			p_l_dmes = strtok (linea, sep1);
			if (!p_l_dmes) continue;
			p_l_mes = strtok (NULL, sep1);
			if (!p_l_mes) continue;
			p_l_ano = strtok (NULL, sep1);
			if (!p_l_ano) continue;
			p_l_dsem = strtok (NULL, sep1);
			if (!p_l_dsem) continue;
			p_com = strtok (NULL, sep2);
			if (!p_com) continue;

			if (	comprobar (p_l_dmes, t1.tm_mday)
				&& comprobar (p_l_mes, t1.tm_mon + 1)
				&& comprobar (p_l_ano, t1.tm_year)
				&& comprobar (p_l_dsem, t1.tm_wday)
			) {
				procesa_macros (p_com, &t1);
				system (p_com);
			}
		}
		avanzar_tm (&t1, 1);
	}

	/* actualizamos la hora de la ultima ejecucion */
	strcpy (strrchr (n_fich, '.'), ".dat");
	f = fopen (n_fich, "w");
	if (f) {
		fprintf (f, "%ld\n", hora);
		fclose (f);
	}
}

/* fin de calendar.c */
