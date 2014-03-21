//----------------
//  program g³ówny
//----------------

#pragma hdrstop
#include <stdio.h>
#include <string.h>
#include <conio.h>

#include "weze.h"

static PUNKT_MAPY mapa[X_SIZE][Y_SIZE];

static DEF_GRUNT defGrunt[] = {
  {1,0,0}, // ziemia rolna
  {2,1,1}, // ocean
  {3,0,1}  // pustynia
  };
#define ILE_DEFGRUNT (sizeof(defGrunt)/sizeof(defGrunt[0]))

static DEF_ROSLINA defRoslina[] = {
  {1, 9},
  };
#define ILE_DEFROSLINA (sizeof(defRoslina)/sizeof(defRoslina[0]))

static DEF_ZWIERZ defZwierz[] = {
  {1, 10, 5, 0, 3},
  };
#define ILE_DEFZWIERZ (sizeof(defZwierz)/sizeof(defZwierz[0]))

/* funkcje do zrobienia:
- wype³nij mapê danymi z pliku txt
- zrzut mapy na standard output
  (ten sam format co input)
- przetwarzanie + pêtla N-razy
*/

static short ileGen=0; // ile przebiegów
static OBIEKTINFO_GRUNT   listaGrunt [X_SIZE*Y_SIZE+1];
static OBIEKTINFO_ROSLINA listaRoslin[X_SIZE*Y_SIZE+1];
static OBIEKTINFO_ZWIERZ  listaZwierz[X_SIZE*Y_SIZE+1];
static short ileGrunt =1; // zacznij od elementu 1, pomiñ zerowe miejsce w tabeli
static short ileRoslin=1;
static short ileZwierz=1;
//---------------------------------------------------------------------------
//! Przygotuj 3 tabele na opisy obiektów
void PusteTabele(void)
{
  ileGrunt  = 1;
  ileRoslin = 1;
  ileZwierz = 1;
  memset(listaGrunt,  0, sizeof(listaGrunt));
  memset(listaRoslin, 0, sizeof(listaRoslin));
  memset(listaZwierz, 0, sizeof(listaZwierz));

  memset(mapa, 0, sizeof(mapa));
} // PusteTabele

//---------------------------------------------------------------------------
//! Dopisz na mapie dane gruntu
void DodajGrunt(short x, short y, short id)
{
  // info w samej mapie
  mapa[x][y].pm_grunt.pm1_typ = TYPOB_GRUNT;
  mapa[x][y].pm_grunt.pm1_poz = ileGrunt;

  // info "systemowe"
  listaGrunt[ileGrunt].oig_common.oic_x = x;
  listaGrunt[ileGrunt].oig_common.oic_y = y;
  listaGrunt[ileGrunt].oig_common.oic_po= 0; // nieprzetworzony

  // dok³adne info o obiekcie
  listaGrunt[ileGrunt].oig_defid = id;
  ileGrunt++;
} // DodajGrunt

//---------------------------------------------------------------------------
//! Dopisz na mapie dane roœliny
void DodajRosline(short x, short y, short id)
{
  // info w samej mapie
  mapa[x][y].pm_roslina.pm1_typ = TYPOB_ROSLINA;
  mapa[x][y].pm_roslina.pm1_poz = ileRoslin;

  // info "systemowe"
  listaRoslin[ileRoslin].oir_common.oic_x = x;
  listaRoslin[ileRoslin].oir_common.oic_y = y;
  listaRoslin[ileRoslin].oir_common.oic_po= 0; // nieprzetworzony

  // dok³adne info o obiekcie
  listaRoslin[ileRoslin].oir_poziom = 5;
  ileRoslin++;
} // DodajRosline

//---------------------------------------------------------------------------
//! Dopisz na mapie dane zwierza
void DodajZwierz(short x, short y, short id)
{
  // info w samej mapie
  mapa[x][y].pm_zwierz.pm1_typ = TYPOB_ZWIERZ;
  mapa[x][y].pm_zwierz.pm1_poz = ileZwierz;

  // info "systemowe"
  listaZwierz[ileZwierz].oiz_common.oic_x = x;
  listaZwierz[ileZwierz].oiz_common.oic_y = y;
  listaZwierz[ileZwierz].oiz_common.oic_po= 0; // nieprzetworzony

  // dok³adne info o obiekcie
  listaZwierz[ileZwierz].oiz_zapas = 5;
  ileZwierz++;
} // DodajZwierz

//---------------------------------------------------------------------------
//! Wype³nij struktury pierwsz¹ generacj¹ obiektów
void ZapelnijMape(void)
{
  short x,y;

  for (x=0; x<X_SIZE; x++)
    for (y=0; y<Y_SIZE; y++)
      {
      // grunty rolne - wszêdzie uprawne
      DodajGrunt(x, y, 1);

      // roœliny
      if (y<4) // tylko kilka rz¹dków
        DodajRosline(x, y, 1);
      }
  DodajZwierz(3, 3, 1); // tylko 1 sztuka
} // ZapelnijMape

//---------------------------------------------------------------------------
//! Przetwarzanie obiektow o 1 jednostkê czasu
void PrzetworzMape(void)
{
  short x,y, poz;
  ileGen++;

  // roœliny
  for (y=0; y<Y_SIZE; y++)
    for (x=0; x<X_SIZE; x++)
      {
      poz = mapa[x][y].pm_roslina.pm1_poz;
      if (listaRoslin[poz].oir_poziom < 9)
        listaRoslin[poz].oir_poziom++; // wzrost +1
      }

  // zwierzêta
  for (poz=1; poz<=ileZwierz; poz++)
    {
    if (listaZwierz[poz].oiz_zapas > 0)
      listaZwierz[poz].oiz_zapas--; // zmniejsz zapas
    }
} // PrzetworzMape

//---------------------------------------------------------------------------
//! Zamieñ punkt mapy na literkê do wydruku
void DrukujZnakMapy(short x, short y)
{
  char znak[2]= "_";
  short poz;

  // grunt
  poz = mapa[x][y].pm_grunt.pm1_poz;
  if (poz > 0)
    strcpy(znak, ".");

  // roœliny
  poz = mapa[x][y].pm_roslina.pm1_poz;
  if (poz > 0)
    znak[0] = '0' + listaRoslin[poz].oir_poziom;

  // zwierzeta
  poz = mapa[x][y].pm_zwierz.pm1_poz;
  if (poz > 0)
    znak[0] = 'A' + listaZwierz[poz].oiz_zapas;

  printf(znak);
} // DrukujZnakMapy

//---------------------------------------------------------------------------
//! Wyrzuæ stan mapy na standard output
void DrukujMape(void)
{
  short x, y;
  printf("====== MAPA nr: %u ===\n", ileGen);
  for (y=0; y<Y_SIZE; y++)
    {
    for (x=0; x<X_SIZE; x++)
      DrukujZnakMapy(x,y);
    printf("\n"); // koniec linii
    }
  printf("\n");    
} // DrukujMape

//---------------------------------------------------------------------------
//! G³ówne wejœcie do programu
#pragma argsused
int main(int argc, char* argv[])
{
  short a;

  printf("WEZE 1.0");
  printf("\n");

  ileGen = 0;
  PusteTabele();
  ZapelnijMape();
  for (a=0; a<10; a++)
    {
    PrzetworzMape();
    DrukujMape();
    }
  return 0;
}

// eof
