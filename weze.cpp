//---------------------------
//  weze.cpp - przetwarzanie
//---------------------------

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
  listaZwierz[ileZwierz].oiz_defid = id;
  listaZwierz[ileZwierz].oiz_common.oic_x = x;
  listaZwierz[ileZwierz].oiz_common.oic_y = y;
  listaZwierz[ileZwierz].oiz_common.oic_po= 0; // nieprzetworzony

  // dok³adne info o obiekcie
  listaZwierz[ileZwierz].oiz_zapas = 8;
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
//! Ustaw losowo liczby 1-4
void UstalRandom4(short* num4)
{
  num4[0] = 1;
  num4[1] = 2;
  num4[2] = 3;
  num4[3] = 4;
} // UstalRandom4

//---------------------------------------------------------------------------
//! Pobierz poziom wzrostu roœliny o zadanych wspó³rzêdnych
short StanRosliny(short x, short y)
{
  short poz;

  if (x < 0
    ||y < 0
    ||x >= X_SIZE
    ||y >= Y_SIZE)
    return 0; // poza zakresem - tam nic nie ma!

  poz = mapa[x][y].pm_roslina.pm1_poz;
  if (poz <= 0)
    return 0; // tam nie ma roœliny!
  return listaRoslin[poz].oir_poziom;
} // StanRosliny

//---------------------------------------------------------------------------
//! Uniwersalna procedura sprawdzania czy roœlina jadalna
short RoslinaJadalna(short poziom)
{
  if (poziom==9)
    return 1; // tylko maksymalne s¹ jadalne

  return 0;
} // RoslinaJadalna

//---------------------------------------------------------------------------
//! Wybierz z s¹siedztwa miejsce do zjedzenia, wynik=1 oznacza ¿e wybrano
short WybierzFood(short x1, short y1, short* destX, short* destY)
{
  short a, stan;
  short kolej[4];

  UstalRandom4(kolej);

  // najpierw punkt bie¿¹cego pobytu
  stan = StanRosliny(x1, y1);
  if (RoslinaJadalna(stan))
    {
    *destX = x1;
    *destY = y1;
    return 1;
    }

  // w losowej kolejnoœci - punkty s¹siednie
  for (a=0; a<4; a++)
    {
    switch (kolej[a])
      {
      case 1: // na górze
        *destX = x1;
        *destY = y1-1;
        break;

      case 2: // z prawej
        *destX = x1+1;
        *destY = y1;
        break;

      case 3: // na dole
        *destX = x1;
        *destY = y1+1;
        break;

      case 4: // z lewej
        *destX = x1-1;
        *destY = y1;
        break;
      }
    stan = StanRosliny(*destX, *destY);
    if (RoslinaJadalna(stan))
      return 1;
    }
    
  // zostañ tam gdzie jesteœ
  *destX = x1;
  *destY = y1;
  return 0; // nie ma nic jadalnego w okolicy
} // WybierzFood

//---------------------------------------------------------------------------
//! Przetwarzanie obiektow o 1 jednostkê czasu
void PrzetworzMape(void)
{
  short x,y, poz;
  short mapX, mapY;
  short destX, destY;
  short zjedz=0;
  short poz2;
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
    if (listaZwierz[poz].oiz_defid > 0) // pomijaj martwe
      {
      mapX = listaZwierz[poz].oiz_common.oic_x;
      mapY = listaZwierz[poz].oiz_common.oic_y;

      //=== Ustal miejsce do którego ma siê przemieœciæ
      zjedz = WybierzFood(mapX, mapY, &destX, &destY);
      if (destX != mapX
        ||destY != mapY) // tylko gdy zmienia miejsce
        {
        // przepisz dane ze starego miejsca do nowego
        mapa[destX][destY].pm_zwierz.pm1_typ = mapa[mapX][mapY].pm_zwierz.pm1_typ;
        mapa[destX][destY].pm_zwierz.pm1_poz = mapa[mapX][mapY].pm_zwierz.pm1_poz;

        // wyma¿ w starym miejscu na mapie
        mapa[mapX][mapY].pm_zwierz.pm1_typ = 0;
        mapa[mapX][mapY].pm_zwierz.pm1_poz = 0;

        // w tabeli zwierz zmieñ wspó³rzêdna na mapie
        listaZwierz[poz].oiz_common.oic_x = destX;
        listaZwierz[poz].oiz_common.oic_y = destY;
        }
      //=== zjedz trawê w miejscu postoju
      if (zjedz)
        {
        poz2 = mapa[destX][destY].pm_roslina.pm1_poz;
        if (poz2 > 0) // jest trawa w tabeli
          listaRoslin[poz2].oir_poziom = 0;
        listaZwierz[poz].oiz_zapas += 2;          
        }
      //=== zmniejsz zapas
      if (listaZwierz[poz].oiz_zapas > 0)
        listaZwierz[poz].oiz_zapas--;

      //=== Niestety, jest martwy - usuñ go
      if (listaZwierz[poz].oiz_zapas == 0)
        {
        // 1.usuñ dane z mapy
        mapa[mapX][mapY].pm_zwierz.pm1_typ = 0;
        mapa[mapX][mapY].pm_zwierz.pm1_poz = 0;

        // 2.oznakuj w tabeli jako martwy
        listaZwierz[poz].oiz_defid = 0;
        }
      }

  // usuñ martwe z listy
  for (poz=1; poz<=ileZwierz; poz++)
    if (listaZwierz[poz].oiz_defid == 0) // do usuniêcia
      {
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
    znak[0] = 'A' + listaZwierz[poz].oiz_zapas -1;

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
} // main

// eof: weze.cpp

