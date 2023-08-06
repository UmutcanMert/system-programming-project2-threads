/**
  UMUTCAN MERT 21120205006

**/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; /* mutex icin */

int hunter_no=1;/* kacinci hunter oldugunu belirtmek icin*/

typedef enum { BEAR, BIRD, PANDA} AnimalType;

typedef enum { ALIVE, DEAD } AnimalStatus;

typedef struct {
    int x;
    int y;
} Location;

typedef enum { FEEDING, NESTING, WINTERING } SiteType;

typedef struct {
    /** animal can be DEAD or ALIVE*/
    AnimalStatus status;
    /** animal type, bear, bird, panda*/
    AnimalType type;
    /** its location in 2D site grid*/
    Location location;
} Animal;

/*example usage*/
Animal bird, bear, panda;

/** type of Hunter*/
typedef struct {
    /** points indicate the number of animals, a hunter killed*/
    int points;
    /** its location in the site grid*/
    Location location;
} Hunter;

/** type of a site (a cell in the grid)*/
typedef struct {
    /** array of pointers to the hunters located at this site*/
    Hunter **hunters;
    /** the number of hunters at this site*/
    int nhunters;
    /** array of pointers to the animals located at this site*/
    Animal **animals;
    /** the number of animals at this site*/
    int nanimals;
    /** the type of site*/
    SiteType type;
} Site;

/** 2D site grid*/
typedef struct {
    /** number of rows, length at the x-coordinate*/
    int xlength;
    /** number of columns, length at the y-coordinate*/
    int ylength;
    /** the 2d site array*/
    Site **sites;
} Grid;

/* initial grid, empty*/
Grid grid = {0, 0, NULL};

Grid initgrid(int xlength, int ylength) {
    grid.xlength = xlength;
    grid.ylength = ylength;

    grid.sites = (Site **)malloc(sizeof(Site *) * xlength);
    for (int i = 0; i < xlength; i++) {
        grid.sites[i] = (Site *)malloc(sizeof(Site) * ylength);
        for (int j = 0; j < ylength; j++) {
            grid.sites[i][j].animals = NULL;
            grid.sites[i][j].hunters = NULL;
            grid.sites[i][j].nhunters = 0;
            grid.sites[i][j].nanimals = 0;
            double r = rand() / (double)RAND_MAX;
            SiteType st;
            if (r < 0.33)
                st = WINTERING;
            else if (r < 0.66)
                st = FEEDING;
            else
                st = NESTING;
            grid.sites[i][j].type = st;


        }
    }
    return grid;
}

void deletegrid() {
    for (int i = 0; i < grid.xlength; i++) {
        free(grid.sites[i]);
    }

    free(grid.sites);

    grid.sites = NULL;
    grid.xlength = -1;
    grid.ylength = -1;
}

void printgrid() {
    for (int i = 0; i < grid.xlength; i++) {
        for (int j = 0; j < grid.ylength; j++) {
            Site *site = &grid.sites[i][j];
            int count[3] = {0}; /* do not forget to initialize*/
            for (int a = 0; a < site->nanimals; a++) {
                Animal *animal = site->animals[a];
                count[animal->type]++;
            }

            printf("|%d-{%d, %d, %d}{%d}|", site->type, count[0], count[1],
                   count[2], site->nhunters);

        }
        printf("\n");
    }
}

void printsite(Site *site) {
    int count[3] = {0}; /* do not forget to initialize*/
    for (int a = 0; a < site->nanimals; a++) {
        Animal *animal = site->animals[a];
        count[animal->type]++;
    }
    printf("|%d-{%d,%d,%d}{%d}|", site->type, count[0], count[1], count[2],
           site->nhunters);
}
/*malloc ile ayrilan site->animals en son free ediliyor */
void deletesite(){

    for(int i=0;i < grid.xlength; i++){
      for(int j=0; j < grid.ylength; j++){
          Site *site= &grid.sites[i][j];
          free(site->animals);

      }
   }
}
Animal nesting, wintering, feeding; /*site->type NESTING,FEEDING,WINTERING icin global animal tanimi*/
void *simulateanimal(void* args) {

    Animal* animal = (Animal*) args;
    Site *site = &grid.sites[animal->location.x][animal->location.y];

    /*Gelen animal DEAD ise kaynaklari serbest birakip fonksiyondan cikilir*/
    if(animal->status == DEAD){
        printf("animal dead\n");
        pthread_exit(NULL);
        return NULL;
    }

    /*Kritik nokta olarak dusunulen yer icin yapildi*/
    pthread_mutex_lock(&mutex);

    if(site->animals==NULL){
        site->animals = (Animal**)malloc(sizeof(Animal*)*(site->nanimals+1));
        site->animals[site->nanimals] = animal;
        site->nanimals++;
    }
    pthread_mutex_unlock(&mutex);

    /* Alttaki if kosullari site tipine gore islem yapmak icin yapildi */
    if(site->type== WINTERING){
        double r = rand() / (double)RAND_MAX; /* 0.5 ihtimal icin random sayi olusur */
        if(r <= 0.5){
            /* ilgili site alanindaki animal sayisi kadar doner ve tum animal olur */
            int countAnimal=0;
            for (int k = 0; k < site->nanimals; k++){
                site->animals[k]->status=DEAD;
                countAnimal++;
                printf("animal: %d- %d  WINTERING 0.5 ihtimalle oldu *************\n",animal->location.x,animal->location.y);
            }
            site->nanimals -= countAnimal; /* site deki animal sayisindan olen animal sayisi cikariliyor */
       }else{
           site->nanimals=0; /* site alanindaki animal baska komsu site alaninan gectiginden buradaki animal sayisi sifirlanir */
           int a=0;
           int b=0;
           a= rand() % 3 - 1; /* 0 , 1 ya da -1 uretir. Buna gore random komsu lokasyona gecer */
           b= rand() % 3 - 1;

           /* animal random komsu lokasyona gitmesi icin, eger random sayi 0 degilse koordinatlara degerler random degerler eklenir */
           if(animal->location.x > 0 && animal->location.x< grid.xlength-1){
              animal->location.x +=a;
           }
           if(animal->location.y > 0 && animal->location.y <grid.ylength-1){
              animal->location.y +=b;
           }
           if(animal->location.x ==0){
              if(a==1){
                animal->location.x += a;
              }
           }
           else if(animal->location.x == grid.xlength - 1){
              if(a == -1){
                animal->location.x +=a;
              }
           }
           if(animal->location.y ==0){
              if(b == 1){
                animal->location.y += b;
              }
           }
           else if(animal->location.y == grid.ylength -1){
              if( b == -1){
                animal->location.y += b;
              }
           }

           wintering.type = animal->type;
           wintering.status = ALIVE;
           wintering.location.x = animal->location.x;
           wintering.location.y = animal->location.y;

           pthread_t wintering_thread;
           int result=pthread_create(&wintering_thread, NULL, simulateanimal, &wintering);
           if(result != 0){
             perror("thread create:");
             exit(-1);
           }
           pthread_join(wintering_thread,NULL);
       }
    }
    if(site->type == FEEDING){
      double r = rand() / (double)RAND_MAX;
      if(r>0.8){
           site->nanimals=0;
           int a=0;
           int b=0;
           a= rand() % 3 - 1; /* 0 , 1 ya da -1 uretir. Buna gore random komsu lokasyona gecer */
           b= rand() % 3 - 1;

           /* animal random komsu lokasyona gitmesi icin, eger random sayi 0 degilse koordinatlara degerler random degerler eklenir */
           if(animal->location.x > 0 && animal->location.x< grid.xlength-1){
              animal->location.x +=a;
           }
           if(animal->location.y > 0 && animal->location.y <grid.ylength-1){
              animal->location.y +=b;
           }
           if(animal->location.x ==0){
              if(a==1){
                animal->location.x += a;
              }
           }
           else if(animal->location.x == grid.xlength - 1){
              if(a == -1){
                animal->location.x +=a;
              }
           }
           if(animal->location.y ==0){
              if(b == 1){
                animal->location.y += b;
              }
           }
           else if(animal->location.y == grid.ylength -1){
              if( b == -1){
                animal->location.y += b;
              }
           }

           feeding.type = animal->type;
           feeding.status = ALIVE;
           feeding.location.x = animal->location.x;
           feeding.location.y = animal->location.y;

           pthread_t feeding_thread;
           int result=pthread_create(&feeding_thread, NULL, simulateanimal, &feeding);
           if(result != 0){
             perror("thread create:");
             exit(-1);
           }
           pthread_join(feeding_thread,NULL);
      }
    }
    if(site->type == NESTING){

        int a=0;
        int b=0;
        a = rand() % grid.xlength;
        b=  rand() % grid.ylength;

        nesting.type= animal->type;
        nesting.status = ALIVE;
        nesting.location.x = a;
        nesting.location.y = b;

        pthread_t nesting_thread;
        int result=pthread_create(&nesting_thread, NULL, simulateanimal, &nesting);
        if(result != 0){
          perror("thread create:");
          exit(-1);
        }
        pthread_join(nesting_thread,NULL);
    }

    usleep(1000);/* gelen her thread 1 ms uyuma kosulu icin */
    pthread_exit(NULL);
    return NULL;
}

void *simulatehunter(void *args) {

    Hunter* hunter = (Hunter*) args;

    Site *site = &grid.sites[hunter->location.x][hunter->location.y];

    printf("hunter baslangic konumu: %d-%d\n",hunter->location.x,hunter->location.y);
    int a=0; /*random x icin */
    int b=0; /* random y icin */
    int i=0; /* dongu icin */
    while(i<1000){ /*her hunter 1000 kez hareket edecek sekilde tasarladim, sonrasında thread sonlandirdim */
        i++;

        /* random komsu icin random sayi */
        a= rand() % 3 - 1; /* 0 , 1 ya da -1 uretir. Buna gore random komsu lokasyona gecer */
        b= rand() % 3 - 1;

        /* hunter random komsu lokasyona gitmesi icin, eger random sayi 0 degilse koordinatlara degerler random degerler eklenir */
        if(hunter->location.x > 0 && hunter->location.x< grid.xlength-1){
           hunter->location.x +=a;
        }
        if(hunter->location.y > 0 && hunter->location.y <grid.ylength-1){
           hunter->location.y +=b;
        }
        if(hunter->location.x ==0){
           if(a==1){
             hunter->location.x += a;
           }
        }
        else if(hunter->location.x == grid.xlength - 1){
           if(a == -1){
              hunter->location.x +=a;
           }
        }
        if(hunter->location.y ==0){
           if(b == 1){
             hunter->location.y += b;
           }
        }
        else if(hunter->location.y == grid.ylength -1){
           if( b == -1){
             hunter->location.y += b;
           }
        }

        site = &grid.sites[hunter->location.x][hunter->location.y];

        /* eger site alaninda animal varsa girer ve oldurup puan kazanir */
        if(site->nanimals > 0) {
           hunter->points += site->nanimals; /* site'daki hayvan sayisi kadar puan kazan */
           site->nanimals = 0; /* site'daki hayvanlar olduruldu */
           printf("hunter %d point: %d \n",hunter_no,hunter->points);
        }

    }

    hunter_no++;/*hunter numarasi artar*/
    site->nhunters++;/*hunter sayisi artiriliyor. printgrid fonksiyonunda nhunters direk yazildiginden burada direk artirma islemi yaptim*/

    usleep(1000); /* gelen her thread 1 ms uyuma kosulu icin */
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if(argc < 2){
      printf("argc is less 2 argument\n");
      return -1;
    }
    /*argv degeri aliniyor*/
    int numberOfArc = atoi(argv[1]); /* argv den gelen arguman icin int tipine donusturme */

    initgrid(5,5);

    /*Animal tanimlaniyor. Baslangicta 3 adet static olarak tanimlaniyor(example usagedeki) ve thread olusturuluyor.*/
    bear.location.x = rand() % grid.xlength;
    bear.location.y = rand() % grid.ylength;
    bear.status = ALIVE;
    bear.type = BEAR;

    bird.location.x = rand() % grid.xlength;
    bird.location.y = rand() % grid.ylength;
    bird.status = ALIVE;
    bird.type = BIRD;

    panda.location.x = rand() % grid.xlength;
    panda.location.y = rand() % grid.ylength;
    panda.status = ALIVE;
    panda.type = PANDA;

    /*Animal threadler olustu*/
    pthread_t threadBear, threadBird, threadPanda;
    int result1=pthread_create(&threadBear, NULL, simulateanimal,&bear);
    if(result1 != 0){
       perror("thread create:");
       exit(-1);
    }
    int result2=pthread_create(&threadBird, NULL, simulateanimal, &bird);
    if(result2 != 0){
       perror("thread create:");
       exit(-1);
    }
    int result3=pthread_create(&threadPanda, NULL, simulateanimal,&panda);
    if(result3 != 0){
       perror("thread create:");
       exit(-1);
    }
    /*Hunter icin thread ve tanimlama*/
    pthread_t threadHunter[numberOfArc];
    Hunter* huntersArc = (Hunter*)malloc(sizeof(Hunter)*numberOfArc);
    for (int i = 0; i < numberOfArc; i++) {
        huntersArc[i].location.x = rand() % grid.xlength;
        huntersArc[i].location.y = rand() % grid.xlength;
        huntersArc[i].points = 0;

        int result=pthread_create(&threadHunter[i], NULL, simulatehunter, &huntersArc[i]);
        if(result != 0){
          perror("thread create:");
          exit(-1);
        }
    }

    /*Animal ve Hunter joinleri*/
    pthread_join(threadBear, NULL);
    pthread_join(threadBird, NULL);
    pthread_join(threadPanda, NULL);

    for (int i = 0; i < numberOfArc; i++) {
        pthread_join(threadHunter[i], NULL);
    }
    free(huntersArc);

    printgrid();
    deletesite(); /* malloc icin */
    deletegrid();

    return 0;
}
