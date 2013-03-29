/*
  multiraw.c
  Copyright 2013 by Waitman Gobble <ns@waitman.net>
  see LICENSE for copying information
*/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>

/* command line parms to send */
char cmd[1024] = {0};

/* Max number of threads to launch */
#define MAX_THREADS 4
/* override with switch */
int num_threads = MAX_THREADS;
/* verbosity flag */
int verbose = 0;
/* original dcraw flag */
int original = 0;

typedef struct {
        char *f;
        long start;
        long end;
} parm;

/* call runner in a thread */
void *runner(void *arg) {

	parm	*p=(parm *)arg;
	long	ln = 0;
	FILE	*file = fopen(p->f, "r");

	if (file != NULL) {
		char	line[1024] = {0};
		while (fgets(line, sizeof line, file) != NULL) {
                        if (ln > p->end) break;
			if (ln >= p->start) {
				char out[1024]={0};
				if (original)
				  sprintf(out, "dcraw\t%s\t%s",cmd,line);
				else
				  sprintf(out, "dcraw-m\t%s\t%s",cmd,line);
				if (verbose) 
					printf("%li\t%s",ln,out);
				system((char *)out);
			}
			++ln;
		}
		fclose(file);
	}

        return (NULL);
}


void usage (void) {
	printf("\nusage: multiraw [-enatvoh] /path/to/file\n\n");
	printf("\
-e\tCommand Line Options to pass to dcraw-m, example: -e '-e -R' \n\
-n\tNumber of Threads\n\
-a\tAutomatically determine number of threads\n\
-t\tReport Exec Time (secs)\n\
-v\tVerbose Messages\n\
-o\tUse original dcraw\n\
-h\tHelp\n\
\n\n");

}

int main(int argc, char **argv){
	
	int 		c;
        time_t 		exec_begin, exec_end;
        int		time_spent = 0;
	pthread_t       *threads;           /* ptr to thread proc  */
	parm            *p;                 /* pass data to thread */
	int		auto_threads = 0;

	/* get system parameters */

        int mib[2], maxproc, cpus;
        size_t len;

        mib[0] = CTL_KERN;
        mib[1] = KERN_MAXPROC;                  /* Process limits */
        len = sizeof(maxproc);
        sysctl(mib, 2, &maxproc, &len, NULL, 0);

        mib[0] = CTL_HW;
        mib[1] = HW_NCPU;                       /* number of CPUs */
        sysctl(mib, 2, &cpus, &len, NULL, 0);

	/* get command line parameters */
	
        while ((c = getopt (argc, argv, "vhtan:e:")) != -1) {
                switch (c) {
                        case 'e':    /* pass command line args */
				strncpy(cmd,optarg,sizeof(cmd));
				break;
                        case 'n':    /* number of threads */
                                num_threads = atoi(optarg);
                                break;
			case 'a':    /* automatically calc threads */
				auto_threads = 1;
				break;
                        case 't':    /* report execution time */
                                /* get start time */
                                exec_begin = time(NULL);
                                /* flag to report at end of exec */
                                time_spent=1;
                                break;
			case 'v':    /* verbosity */
				verbose = 1;
				break;
			case 'o':    /* original dcraw */
				original = 1;
				break;
			case 'h':    
				usage();
				exit(0);
				break;
			default:
				break;
		}
	}

	argc -= optind;
	argv += optind;

	/* no file specified */

	if (argc<1) {
		usage();
		exit(1);
	}

	/* count the lines in the file */

	long ln = 0;
	FILE *fp = fopen(argv[0],"r");
	if (fp != NULL) {
		char line[1024] = {0};
		while (fgets(line, sizeof line, fp) != NULL)
      	 		++ln;

		fclose(fp);
	} else {
		printf("FATAL: Could not open %s for reading.\n",argv[0]);
		exit(1);
	}

        if (verbose)
                printf("Max Processes: %i\nCPUS: %i\n",maxproc,cpus);

	/* auto calculate number of threads */

	if (auto_threads) {
		char* pos = strstr(cmd, "-e");
		if (pos) {
			num_threads = cpus * 8;
		} else {
			num_threads = cpus;
		}
		if (verbose) 
			printf("Auto Calibrate Threads: %i\n",num_threads);
	}

	/* more threads than files */

	if (num_threads>ln) {
		num_threads=ln;
		if (verbose)
			printf("Cutting threads: %i\n",num_threads);
	}

	/* are we sane? 
	   figure out if requested number of threads is over the top */

	if (num_threads > maxproc) {
		printf("FATAL: %i threads exceeds allowable processes (%i)\n",
					num_threads,maxproc);
		exit(1);
	}

	if (verbose) {
		if (original)
		 printf("Processing: %s... with dcraw %s\n",argv[0],cmd);
		else
		 printf("Processing: %s... with dcraw-m %s\n",argv[0],cmd);

		printf("%li lines in file, %i threads\n",ln,num_threads);
	}

	/* calculate number of files per thread */

	long pt = ln / num_threads;

	if (pt<2) {
		if (ln>num_threads) {
			pt=2;
			num_threads=ln/pt;
			if (verbose) 
				printf("Auto-adjust threads: %i\n",num_threads);
		}
	}

	long i;
	int thr=1;

        /* init thread pointers */
        threads = (pthread_t *)malloc(num_threads*sizeof(*threads));

        /* init data struct */
        p = (parm *)malloc(sizeof(parm)*num_threads);

	/* launch the threads */

	for (i=0;i<ln-(pt*2);i+=pt) {
		if (verbose)
			printf("Setting up thread: %i ... %li to %li\n",
					thr,i,(i+pt-1));
		p[thr].f=argv[0];
		p[thr].start=i;
		p[thr].end=(i+pt-1);
        	pthread_create(&threads[thr],NULL,runner,(void *)(p+thr));
		++thr;
		if (thr==num_threads) break;
	}

	/* launch the final thread */

	if (verbose) 
		printf("Setting up thread: %i ... %li to %li\n",thr,i,ln);

	p[thr].f=argv[0];
	p[thr].start=i;
	p[thr].end=ln;
	pthread_create(&threads[thr],NULL,runner,(void *)(p+thr));

        /* wait for all threads to finish */

        for (i=1;i<=num_threads;i++) {
                pthread_join(threads[i],NULL);
        }
        free(p);

	/* report perceived execution time */

        if (time_spent>0) {
                exec_end = time(NULL);
                time_spent = (int)exec_end - (int)exec_begin;
                printf ("Exec: %d seconds\n",time_spent);
        }
        return (0);
}

