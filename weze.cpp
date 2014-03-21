//---------------------------
//  weze.cpp - przetwarzanie
//---------------------------

#pragma hdrstop
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <windows.h>

#include "weze.h"




//   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//!  @name weze_przetwarzanie
//!  Alogorytmy przetwarzania obiektów i mapy
//@{ @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static PUNKT_MAPY mapa[X_SIZE][Y_SIZE];

static DEF_GRUNT defGrunt[] = {
  {0,0}, // 0, ziemia rolna
  {1,1}, // 1, ocean
  {0,1}  // 2, pustynia
  };
#define ILE_DEFGRUNT (sizeof(defGrunt)/sizeof(defGrunt[0]))

static DEF_ROSLINA defRoslina[] = {
  {9},
  };
#define ILE_DEFROSLINA (sizeof(defRoslina)/sizeof(defRoslina[0]))

static DEF_ZWIERZ defZwierz[] = {
  {10, 5, 0, 3},
  };
#define ILE_DEFZWIERZ (sizeof(defZwierz)/sizeof(defZwierz[0]))

/* funkcje do zrobienia:
- wype³nij mapê danymi z pliku txt
*/

static long ileGen=0; // ile przebiegów
static long martwe=0; // ile odesz³o
static OBIEKTINFO_GRUNT   listaGrunt [X_SIZE*Y_SIZE+1];
static OBIEKTINFO_ROSLINA listaRoslin[X_SIZE*Y_SIZE+1];
static OBIEKTINFO_ZWIERZ  listaZwierz[X_SIZE*Y_SIZE+1];
static short ileGrunt =0;
static short ileRoslin=0;
static short ileZwierz=0;
//---------------------------------------------------------------------------
//! Przygotuj 3 tabele na opisy obiektów
void PusteTabele(void)
{
  ileGrunt  = 0;
  ileRoslin = 0;
  ileZwierz = 0;
  memset(listaGrunt,  0, sizeof(listaGrunt));
  memset(listaRoslin, 0, sizeof(listaRoslin));
  memset(listaZwierz, 0, sizeof(listaZwierz));

  memset(mapa, 0, sizeof(mapa));
  martwe = 0;
} // PusteTabele

//---------------------------------------------------------------------------
//! Dopisz na mapie dane gruntu
void DodajGrunt(short x, short y, short id)
{
  // info w samej mapie
  mapa[x][y].pm_grunt.pm1_tab = TAB_GRUNT;
  mapa[x][y].pm_grunt.pm1_poz = ileGrunt;

  // info w obiekcie (najbli¿sze wolne miejsce)
  listaGrunt[ileGrunt].oig_common.oic_x = x; // ogólne
  listaGrunt[ileGrunt].oig_common.oic_y = y;
  listaGrunt[ileGrunt].oig_common.oic_po= 0; // nieprzetworzony
  // dok³adne info o obiekcie
  listaGrunt[ileGrunt].oig_defpoz = id;

  ileGrunt++;
} // DodajGrunt

//---------------------------------------------------------------------------
//! Dopisz na mapie dane roœliny
void DodajRosline(short x, short y, short id)
{
  // info w samej mapie
  mapa[x][y].pm_roslina.pm1_tab = TAB_ROSLINA;
  mapa[x][y].pm_roslina.pm1_poz = ileRoslin;

  // info w obiekcie
  listaRoslin[ileRoslin].oir_common.oic_x = x;
  listaRoslin[ileRoslin].oir_common.oic_y = y;
  listaRoslin[ileRoslin].oir_common.oic_po= 0; // nieprzetworzony
  // dok³adne info o obiekcie
  listaRoslin[ileRoslin].oir_defpoz = id;
  // poziom wzrostu - ustaw maksymalny
  listaRoslin[ileRoslin].oir_poziom = defRoslina[id].defr_czasWzrostu;

  ileRoslin++;
} // DodajRosline

//---------------------------------------------------------------------------
//! Dopisz na mapie dane zwierza
void DodajZwierz(short x, short y, short id)
{
  // info w samej mapie
  mapa[x][y].pm_zwierz.pm1_tab = TAB_ZWIERZ;
  mapa[x][y].pm_zwierz.pm1_poz = ileZwierz;

  // info w obiekcie
  listaZwierz[ileZwierz].oiz_common.oic_x = x;
  listaZwierz[ileZwierz].oiz_common.oic_y = y;
  listaZwierz[ileZwierz].oiz_common.oic_po= 0; // nieprzetworzony
  // dok³adne info o obiekcie
  listaZwierz[ileZwierz].oiz_defpoz = id;
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
      // grunty rolne - wszêdzie uprawne (typ 0)
      DodajGrunt(x, y, 0);

      // roœliny
      if (x%2 && y%2) // tylko kilka rz¹dków roœliny zwyk³ej (typ 0)
        DodajRosline(x, y, 0);
      }
  DodajZwierz(1, 1, 0);
  DodajZwierz(2, 2, 0);
  DodajZwierz(3, 3, 0);
} // ZapelnijMape

short losowe[24][4] = { // wszystkie mo¿liwe kolejnoœci dla 4 elementów
  {1,2,3,4}, {1,2,4,3}, {1,3,2,4}, {1,3,4,2}, {1,4,2,3}, {1,4,3,2},
  {2,1,3,4}, {2,1,4,3}, {2,3,1,4}, {2,3,4,1}, {2,4,1,3}, {2,4,3,1},
  {3,1,2,4}, {3,1,4,2}, {3,2,1,4}, {3,2,4,1}, {3,4,1,2}, {3,4,2,1},
  {4,1,2,3}, {4,1,3,2}, {4,2,1,3}, {4,2,3,1}, {4,3,1,2}, {4,3,2,1}
  };
//---------------------------------------------------------------------------
//! Ustaw losowo liczby 1-4
void UstalRandom4(short* num4)
{
  short a;
  a = random(24); // wybierz - która kolejnoœæ z 24 zostanie u¿yta
  num4[0] = losowe[a][0];
  num4[1] = losowe[a][1];
  num4[2] = losowe[a][2];
  num4[3] = losowe[a][3];
} // UstalRandom4

//---------------------------------------------------------------------------
//! SprawdŸ czy zadane wspó³rzêdne mieszcz¹ siê na mapie
short CzyPunktZakres(short x, short y)
{
  if (x < 0
    ||y < 0
    ||x >= X_SIZE
    ||y >= Y_SIZE)
    return 0; // poza zakresem - tam nic nie ma!
  return 1; // ok
} // CzyPunktZakres

//---------------------------------------------------------------------------
//! Sprawdza czy dla podanych wspó³rzêdnych wystêpuje obiekt - roœlina
short CzyMapaRoslina(short x, short y)
{
  if (mapa[x][y].pm_roslina.pm1_tab == 0)
    return 0; // nie ma tam roœliny

  return 1; // jest
} // CzyMapaRoslina

//---------------------------------------------------------------------------
//! Któr¹ pozycjê w tabeli zajmuje roœlina?
short GetPozRoslina(short x, short y)
{
  return mapa[x][y].pm_roslina.pm1_poz;
} // GetPozRoslina

//---------------------------------------------------------------------------
//! Sprawdza czy dla podanych wspó³rzêdnych wystêpuje obiekt - roœlina
short CzyMapaZwierz(short x, short y)
{
  if (mapa[x][y].pm_zwierz.pm1_tab == 0)
    return 0; // nie ma tam nikogo

  return 1; // jest
} // CzyMapaZwierz

//---------------------------------------------------------------------------
//! Któr¹ pozycjê w tabeli zajmuje zwierz?
short GetPozZwierz(short x, short y)
{
  return mapa[x][y].pm_zwierz.pm1_poz;
} // GetPozZwierz

//---------------------------------------------------------------------------
//! Uniwersalna procedura sprawdzania czy roœlina jadalna
short RoslinaJadalna(short x, short y)
{
  short poz, ktoraDef;
  short poziomAkt, maxPoziomDef;

  if (CzyMapaRoslina(x,y)==0)
    return 0; // nie ma tam roœliny!
  poz = GetPozRoslina(x,y); // która roœlina z listy
  poziomAkt = listaRoslin[poz].oir_poziom;
  ktoraDef  = listaRoslin[poz].oir_defpoz;
  maxPoziomDef = defRoslina[ktoraDef].defr_czasWzrostu; // poziom maksymalny z defincji

  if (poziomAkt == maxPoziomDef)
    return 1; // tylko maksymalne s¹ jadalne

  return 0;
} // RoslinaJadalna

//---------------------------------------------------------------------------
//! Wybierz z s¹siedztwa miejsce do zjedzenia, wynik=1 oznacza ¿e wybrano
short WybierzFood(short x1, short y1, short* destX, short* destY)
{
  short a;
  short kolej[4];
  short lastX=x1;
  short lastY=y1;

  UstalRandom4(kolej);

  // najpierw punkt bie¿¹cego pobytu
  if (RoslinaJadalna(x1, y1))
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
    if (CzyPunktZakres(*destX, *destY))
      {
      if (CzyMapaZwierz(*destX, *destY)==0)
        {
        lastX = *destX;
        lastY = *destY;

        if (RoslinaJadalna(*destX, *destY))
          return 1; // wybierz ten punkt
        }
      }
    }

  // przyjmij inny punkt w otoczeniu - wolny
  *destX = lastX;
  *destY = lastY;
  return 0; // nie ma nic jadalnego w okolicy
} // WybierzFood

//---------------------------------------------------------------------------
//! Wybierz z s¹siedztwa miejsce do umieszczenia noworodka (wynik=1 to zgoda)
short WybierzDlaNowego(short x1, short y1, short* destX, short* destY)
{
  short kolej[4];
  short a;

  UstalRandom4(kolej);
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
    if (CzyPunktZakres(*destX, *destY))
      if (CzyMapaZwierz(*destX, *destY)==0) // miejsce jest wolne
        return 1; // wybierz ten punkt
    }

  return 0; // nie mam miejsca
} // WybierzDlaNowego

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
/*  for (y=0; y<Y_SIZE; y++)
    for (x=0; x<X_SIZE; x++)
      {
      if (CzyMapaRoslina(x,y)) // tu jest roœlina
        {
        poz = GetPozRoslina(x,y);
        if (listaRoslin[poz].oir_poziom < 9)
          listaRoslin[poz].oir_poziom++; // wzrost +1
        }
      }*/

  for (poz=0; poz<ileRoslin; poz++)
    {
    if (listaRoslin[poz].oir_poziom < 9)
      listaRoslin[poz].oir_poziom++; // wzrost +1
    }
  // zwierzêta
  for (poz=0; poz<ileZwierz; poz++)
    if (listaZwierz[poz].oiz_defpoz >= 0) // pomijaj martwe (czyli ujemne)
      {
      mapX = listaZwierz[poz].oiz_common.oic_x;
      mapY = listaZwierz[poz].oiz_common.oic_y;

      //=== Ustal miejsce do którego ma siê przemieœciæ
      zjedz = WybierzFood(mapX, mapY, &destX, &destY);
      if (destX != mapX
        ||destY != mapY) // tylko gdy zmienia miejsce
        {
        // przepisz dane ze starego miejsca do nowego
        mapa[destX][destY].pm_zwierz.pm1_tab = mapa[mapX][mapY].pm_zwierz.pm1_tab;
        mapa[destX][destY].pm_zwierz.pm1_poz = mapa[mapX][mapY].pm_zwierz.pm1_poz;

        // wyma¿ w starym miejscu na mapie
        mapa[mapX][mapY].pm_zwierz.pm1_tab = 0;
        mapa[mapX][mapY].pm_zwierz.pm1_poz = 0;

        // w tabeli zwierz zmieñ wspó³rzêdna na mapie
        listaZwierz[poz].oiz_common.oic_x = destX;
        listaZwierz[poz].oiz_common.oic_y = destY;
        }
      //=== zjedz trawê w miejscu postoju
      if (zjedz)
        {
        if (RoslinaJadalna(destX, destY))
          {
          poz2 = GetPozRoslina(destX, destY);
          if (CzyMapaRoslina(destX, destY)) // jest trawa w tabeli
            {
            listaRoslin[poz2].oir_poziom = 0;
            if (listaZwierz[poz].oiz_zapas < 20)
              listaZwierz[poz].oiz_zapas += 3;
            }
          }
        }

      //=== zmniejsz zapas
      if (listaZwierz[poz].oiz_zapas > 0)
        listaZwierz[poz].oiz_zapas--;

      //=== Niestety, jest martwy - usuñ go z mapy
      if (listaZwierz[poz].oiz_zapas <= 0)
        {
        // 1.usuñ dane z mapy
        mapa[destX][destY].pm_zwierz.pm1_tab = 0;
        mapa[destX][destY].pm_zwierz.pm1_poz = 0;

        // 2.oznakuj w tabeli jako martwy
        listaZwierz[poz].oiz_defpoz = -1;
        }
      // rozmna¿anie
      if (listaZwierz[poz].oiz_zapas >=20) // s¹ nadwy¿ki do wydania
        {
        short ok;
        short destX2, destY2;

        ok = WybierzDlaNowego(destX, destY, &destX2, &destY2);
        if (ok)
          {
          listaZwierz[poz].oiz_zapas -= 10; // zmniejsz poziom zapasu matce
          DodajZwierz(destX2, destY2, 1);
          }
        }
      }

  // usuñ martwe z listy
  for (poz=ileZwierz-1; poz>=0; poz--)
    if (listaZwierz[poz].oiz_defpoz == -1) // do usuniêcia
      {
      if (poz != (ileZwierz-1)) // to nie jest ostatni
        {
        memmove(listaZwierz+poz, listaZwierz+poz+1, sizeof(listaZwierz[0])*(ileZwierz-poz-1));

        // przenumeruj obiekty na mapie (o -1)
        short a;
        for (a=0; a<ileZwierz-poz-1; a++)
          {
          x = listaZwierz[poz+a].oiz_common.oic_x;
          y = listaZwierz[poz+a].oiz_common.oic_y;
          mapa[x][y].pm_zwierz.pm1_poz--; // przenumeruj o 1 w dó³
          }
        }
      ileZwierz--;
      martwe++;
      }
} // PrzetworzMape

//@} weze_przetwarzanie




//   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//!  @name weze_output
//!  Drukowanie aktualnego stanu mapy
//@{ @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static HANDLE hStdOut;
//---------------------------------------------------------------------------
//! Ustaw kolor tekstu - kolor zdefiniowany w windows.h
void SetTextColor(short kolor)
{
  SetConsoleTextAttribute(hStdOut, kolor);
} // SetTextColor

//---------------------------------------------------------------------------
//! Drukuj tekst wg bie¿¹cego atrybutu
void PutZnak(char* znak)
{
  unsigned long count;
  WriteConsole(hStdOut, znak, 1, &count, NULL);
} // PutZnak

//---------------------------------------------------------------------------
//! Zamieñ punkt mapy na literkê do wydruku
void DrukujZnakMapy(short x, short y)
{
  char znak[2]= "_";
  short poz;

  // grunt
  poz = mapa[x][y].pm_grunt.pm1_poz;
  if (mapa[x][y].pm_grunt.pm1_tab != 0)
    {
    SetTextColor(FOREGROUND_BLUE);
    strcpy(znak, ".");
    }

  // roœliny
  if (CzyMapaRoslina(x,y)) // jest trawa w tabeli
    {
    poz = GetPozRoslina(x,y);
    znak[0] = '0' + listaRoslin[poz].oir_poziom;
    if (RoslinaJadalna(x,y))
      {
      SetTextColor(FOREGROUND_RED | FOREGROUND_GREEN /*| FOREGROUND_INTENSITY*/);
      znak[0] = 'o';
      }
    else
      SetTextColor(FOREGROUND_GREEN);
    }

  // zwierzeta
  if (CzyMapaZwierz(x,y)) // jest zwierz w tabeli
    {
    poz = GetPozZwierz(x,y);
    znak[0] = 'A' + listaZwierz[poz].oiz_zapas -1;
    //if (listaZwierz[poz].oiz_zapas < 10)
    //  SetTextColor(FOREGROUND_BLUE);
    //else
      SetTextColor(FOREGROUND_RED);
    }
  PutZnak(znak);
} // DrukujZnakMapy

//---------------------------------------------------------------------------
//! Wyrzuæ stan mapy na standard output
void DrukujMape(void)
{
  short x, y;

  COORD coord = {0,0};
  SetConsoleCursorPosition(hStdOut, coord);
  printf("MAPA %ux%u: %lu\n", X_SIZE, Y_SIZE, ileGen);
  printf("weze: %u\n", ileZwierz);
  printf("dead: %lu\n", martwe);
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
int main(int /*argc*/, char* /*argv[]*/)
{
  short a, znak = ' ';

  clrscr();
  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  SetTextColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
  ileGen = 0;
  PusteTabele();
  ZapelnijMape();
  _next:
  PrzetworzMape();
  if ((ileGen%10000)==0)
    DrukujMape();
//  if (ileGen > 7000)
//    znak = getch(); // praca krokowa
  SetTextColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
  if (znak == 'q')
    return 0;
  goto _next;

// return 0;
} // main

//@} weze_output

// eof: weze.cpp

