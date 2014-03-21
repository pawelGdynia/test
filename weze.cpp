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

static short xSize = 60;
static short ySize = 60;

static PUNKT_MAPY* mapa;
static DEF_GRUNT defGrunt[] = {
  {0,0}, // 0, ziemia rolna
  {1,1}, // 1, ocean
  {0,1}  // 2, pustynia
  };
#define ILE_DEFGRUNT (sizeof(defGrunt)/sizeof(defGrunt[0]))

static DEF_ROSLINA defRoslina[] = {
  {9, 2},
  };
#define ILE_DEFROSLINA (sizeof(defRoslina)/sizeof(defRoslina[0]))

static DEF_ZWIERZ defZwierz[] = {
  {10, 1, 0, 3}, // 0: roœlino¿erca
  {10, 1, 1, 3}, // 1: drapie¿nik
  };
#define ILE_DEFZWIERZ (sizeof(defZwierz)/sizeof(defZwierz[0]))

/* funkcje do zrobienia:
- wype³nij mapê danymi z pliku txt
*/

static long ileGen=0; // ile przebiegów
static long martwe=0; // ile odesz³o
static OBIEKTINFO_GRUNT*   listaGrunt;
static OBIEKTINFO_ROSLINA* listaRoslin;
static OBIEKTINFO_ZWIERZ*  listaZwierz;
static short ileGrunt =0;
static short ileRoslin=0;
static short ileZwierz=0;

//---------------------------------------------------------------------------
//! Zwraca adres punktu na mapie, niezale¿nie od wewnêtrznej organizacji
PUNKT_MAPY* PtrPunktMapy(short x, short y)
{
  return &(mapa[(x*ySize)+y]);
} // PtrPunktMapy

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
  martwe = 0;
} // PusteTabele

//---------------------------------------------------------------------------
//! Dopisz na mapie dane gruntu
void DodajGrunt(short x, short y, short id)
{
  PUNKT_MAPY* ptr;
  ptr = PtrPunktMapy(x,y);
  if (ileGrunt >= xSize * ySize)
    {
    printf("E:DodajGrunt()");
    return; // nie dodawaj - nie ma miejsca
    }
  // info w samej mapie
  ptr->pm_grunt.pm1_tab = TAB_GRUNT;
  ptr->pm_grunt.pm1_poz = ileGrunt;

  // info w obiekcie (najbli¿sze wolne miejsce)
  listaGrunt[ileGrunt].g_common.x = x; // ogólne
  listaGrunt[ileGrunt].g_common.y = y;
  listaGrunt[ileGrunt].g_common.po= 0; // nieprzetworzony
  // dok³adne info o obiekcie
  listaGrunt[ileGrunt].g_def = id;

  ileGrunt++;
} // DodajGrunt

//---------------------------------------------------------------------------
//! Dopisz na mapie dane roœliny
void DodajRosline(short x, short y, short id)
{
  PUNKT_MAPY* ptr;
  if (ileRoslin >= xSize * ySize)
    {
    printf("E:DodajRosline()");
    return; // nie dodawaj - nie ma miejsca
    }
  ptr = PtrPunktMapy(x,y);
  // info w samej mapie
  ptr->pm_roslina.pm1_tab = TAB_ROSLINA;
  ptr->pm_roslina.pm1_poz = ileRoslin;

  // info w obiekcie
  listaRoslin[ileRoslin].r_common.x = x;
  listaRoslin[ileRoslin].r_common.y = y;
  listaRoslin[ileRoslin].r_common.po= 0; // nieprzetworzony
  // dok³adne info o obiekcie
  listaRoslin[ileRoslin].r_def = id;
  // poziom wzrostu - ustaw maksymalny
  listaRoslin[ileRoslin].r_poziom = defRoslina[id].dr_czasWzrostu;

  ileRoslin++;
} // DodajRosline

//---------------------------------------------------------------------------
//! Dopisz na mapie dane zwierza
void DodajZwierz(short x, short y, short id)
{
  PUNKT_MAPY* ptrMapa;
  OBIEKTINFO_ZWIERZ* ptrZ;
  DEF_ZWIERZ* ptrDef;      // definicja

  if (ileZwierz >= xSize * ySize)
    {
    printf("E:DodajZwierz()");
    return; // nie dodawaj - nie ma miejsca
    }
  ptrMapa = PtrPunktMapy(x,y);
  ptrZ = listaZwierz+ileZwierz;
  ptrDef = defZwierz + id;
  // info w samej mapie
  ptrMapa->pm_zwierz.pm1_tab = TAB_ZWIERZ;
  ptrMapa->pm_zwierz.pm1_poz = ileZwierz;

  // info w obiekcie
  ptrZ->z_common.x = x;
  ptrZ->z_common.y = y;
  ptrZ->z_common.po= 0; // nieprzetworzony
  // dok³adne info o obiekcie
  ptrZ->z_def = id;
  ptrZ->z_zapas = ptrDef->dz_maxZapas-2; // max -2

  ileZwierz++;
} // DodajZwierz

//---------------------------------------------------------------------------
//! Wype³nij struktury pierwsz¹ generacj¹ obiektów
void ZapelnijMape(void)
{
  short x,y;

  for (x=0; x<xSize; x++)
    for (y=0; y<ySize; y++)
      {
      // grunty rolne - wszêdzie uprawne (typ 0)
      DodajGrunt(x, y, 0);

      // roœliny
      //if (y%2) // tylko kilka rz¹dków roœliny zwyk³ej (typ 0)
        DodajRosline(x, y, 0);
      if (x==y)
        DodajZwierz(x, y, 0); // gatunek "0"
      }
 DodajZwierz(5, 1, 1); // gatunek "1"
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
//  a = 11;
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
    ||x >= xSize
    ||y >= ySize)
    return 0; // poza zakresem - tam nic nie ma!
  return 1; // ok
} // CzyPunktZakres

//---------------------------------------------------------------------------
//! Sprawdza czy dla podanych wspó³rzêdnych wystêpuje obiekt - roœlina
short CzyMapaRoslina(short x, short y)
{
  PUNKT_MAPY* ptr;
  ptr = PtrPunktMapy(x,y);

  if (ptr->pm_roslina.pm1_tab == 0)
    return 0; // nie ma tam roœliny

  return 1; // jest
} // CzyMapaRoslina

//---------------------------------------------------------------------------
//! Któr¹ pozycjê w tabeli zajmuje roœlina?
short GetPozRoslina(short x, short y)
{
  PUNKT_MAPY* ptr;
  ptr = PtrPunktMapy(x,y);

  return ptr->pm_roslina.pm1_poz;
} // GetPozRoslina

//---------------------------------------------------------------------------
//! Sprawdza czy dla podanych wspó³rzêdnych zwierz zwyk³y (1) czy drapie¿nik (2)
short CzyMapaZwierz(short x, short y)
{
  PUNKT_MAPY* ptr;
  DEF_ZWIERZ* ptrDef;      // definicja
  ptr = PtrPunktMapy(x,y);

  if (ptr->pm_zwierz.pm1_tab == 0)
    return 0; // nie ma tam nikogo
  ptrDef = defZwierz + listaZwierz[ptr->pm_zwierz.pm1_poz].z_def;
  if (ptrDef->dz_drapieznik > 0)
    return 2; // jest drapie¿nik

  return 1; // jest roœlino¿erca
} // CzyMapaZwierz

//---------------------------------------------------------------------------
//! Któr¹ pozycjê w tabeli zajmuje zwierz?
short GetPozZwierz(short x, short y)
{
  PUNKT_MAPY* ptr;
  ptr = PtrPunktMapy(x,y);

  return ptr->pm_zwierz.pm1_poz;
} // GetPozZwierz

//---------------------------------------------------------------------------
//! Uniwersalna procedura sprawdzania czy roœlina jadalna
short RoslinaJadalna(short x, short y)
{
  short poz, ktoraDef;

  OBIEKTINFO_ROSLINA* ptrRoslina;
  DEF_ROSLINA* ptrDef;

  if (CzyMapaRoslina(x,y)==0)
    return 0; // nie ma tam roœliny!

  poz = GetPozRoslina(x,y); // która roœlina z listy
  ptrRoslina = listaRoslin+poz;
  ptrDef    = defRoslina + ptrRoslina->r_def;

  //-----------
  if (ptrRoslina->r_poziom == ptrDef->dr_czasWzrostu)
    return 1; // tylko maksymalne s¹ jadalne

  return 0;
} // RoslinaJadalna

//---------------------------------------------------------------------------
//! Wybierz z s¹siedztwa miejsce do zjedzenia, wynik=1 oznacza ¿e wybrano
short WybierzFood(short x1, short y1, short* destX, short* destY, DEF_ZWIERZ* def)
{
  short a;
  short kolej[4];
  short lastX=x1;
  short lastY=y1;

  UstalRandom4(kolej);
  // w losowej kolejnoœci - sprawdŸ punkty s¹siednie
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
    if (CzyPunktZakres(*destX, *destY)) // punkt nie wyszed³ poza mapê
      {
      if (def->dz_drapieznik)
        {
        if (CzyMapaZwierz(*destX, *destY)==1) // znalaz³ roœlino¿ercê
          return 2; // zjedz go!
        }
      else // drapie¿nik nie jada roœlin
        {
        if (def->dz_roslinozerca
          &&CzyMapaZwierz(*destX, *destY)==0)
          {
          lastX = *destX;
          lastY = *destY;
          if (RoslinaJadalna(*destX, *destY))
            return 1; // wybierz ten punkt
          }
        }
      }
    }
  // SprawdŸ czy w punkcie bie¿¹cym jest roœlina
  if (def->dz_roslinozerca
    &&RoslinaJadalna(x1, y1))
    {
    *destX = x1;
    *destY = y1;
    return 1;
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
// Przetwórz roœlinê z listy
void ProcessRoslina(short poz)
{
  OBIEKTINFO_ROSLINA* ptrRoslina;
  DEF_ROSLINA* ptrDef;

  ptrRoslina = listaRoslin+poz;
  ptrDef     = defRoslina + ptrRoslina->r_def;

  if (ptrRoslina->r_poziom < ptrDef->dr_czasWzrostu)
    ptrRoslina->r_poziom++; // wzrost +1
} // ProcessRoslina

//---------------------------------------------------------------------------
// Przetwórz zwierzê z listy
void ProcessZwierz(short poz)
{
  short mapSrcX, mapSrcY;
  short mapDestX, mapDestY;
  short zjedz=0;
  short poz2;
  PUNKT_MAPY* ptrSrc=NULL;
  PUNKT_MAPY* ptrDst=NULL;

  OBIEKTINFO_ZWIERZ* ptrZ1; // zwierz przetwarzany
  DEF_ZWIERZ* defZ1;        // definicja

  OBIEKTINFO_ZWIERZ* ptrZ2; // zwierz zjadany
  DEF_ZWIERZ* defZ2;        // definicja
  DEF_ROSLINA* defR;        // definicja

  ptrZ1 = listaZwierz+poz;
  if (ptrZ1->z_def < 0) // pomijaj martwe (czyli ujemne)
    return;

  defZ1 = defZwierz + (ptrZ1->z_def);
  mapSrcX = ptrZ1->z_common.x;
  mapSrcY = ptrZ1->z_common.y;

  //=== Ustal miejsce do którego ma siê przemieœciæ (oraz czy coœ zje)
  zjedz  = WybierzFood(mapSrcX, mapSrcY, &mapDestX, &mapDestY, defZ1); // zwraca 1 lub 2
  ptrSrc = PtrPunktMapy(mapSrcX, mapSrcY);
  ptrDst = PtrPunktMapy(mapDestX, mapDestY);

  //=== najpierw zjedz, potem siê przemieœæ
  if (zjedz)
    {
    if (zjedz==2) // zjedz roœlino¿ercê
      {
      poz2 = GetPozZwierz(mapDestX, mapDestY);
      ptrZ2 = listaZwierz+poz2;
      if (ptrZ1->z_zapas < defZ1->dz_maxZapas)
        {
        // tylko gdy nie jest pe³ny
        defZ2 = defZwierz + ptrZ2->z_def;
        ptrZ1->z_zapas += defZ2->dz_kalorie;
        ptrZ2->z_zapas = 0; // wyzeruj zjedzonego
        ptrZ2->z_def= -1;
        }
      }
    else if (RoslinaJadalna(mapDestX, mapDestY))
      {
      poz2 = GetPozRoslina(mapDestX, mapDestY);
      if (CzyMapaRoslina(mapDestX, mapDestY)) // jest trawa w tabeli
        {
        defR = defRoslina + listaRoslin[poz2].r_def;
        listaRoslin[poz2].r_poziom = 0;
        if (ptrZ1->z_zapas < defZ1->dz_maxZapas)
          {
          // tylko gdy nie jest pe³ny
          ptrZ1->z_zapas += defR->dr_kalorie;
          }
        }
      }
    }

  if (mapDestX != mapSrcX
    ||mapDestY != mapSrcY) // tylko gdy zmienia miejsce
    {
    // przepisz dane ze starego miejsca do nowego
    ptrDst->pm_zwierz.pm1_tab = ptrSrc->pm_zwierz.pm1_tab;
    ptrDst->pm_zwierz.pm1_poz = ptrSrc->pm_zwierz.pm1_poz;

    // wyma¿ w starym miejscu na mapie
    ptrSrc->pm_zwierz.pm1_tab = 0;
    ptrSrc->pm_zwierz.pm1_poz = 0;

    // w tabeli zwierz zmieñ wspó³rzêdna na mapie
    ptrZ1->z_common.x = mapDestX;
    ptrZ1->z_common.y = mapDestY;
    }

  //=== zmniejsz zapas
  if (listaZwierz[poz].z_zapas > 0)
    listaZwierz[poz].z_zapas--;

  //=== Niestety, jest martwy - usuñ go z mapy
  if (listaZwierz[poz].z_zapas <= 0)
    {
    // 1.usuñ dane z mapy
    ptrDst->pm_zwierz.pm1_tab = 0;
    ptrDst->pm_zwierz.pm1_poz = 0;

    // 2.oznakuj w tabeli jako martwy
    listaZwierz[poz].z_def = -1;
    }
  // rozmna¿anie
  if (ptrZ1->z_zapas >= defZ1->dz_maxZapas) // s¹ nadwy¿ki do wydania
    {
    short ok;
    short mapNewX, mapNewY;

    ok = WybierzDlaNowego(mapDestX, mapDestY, &mapNewX, &mapNewY);
    if (ok)
      {
      ptrZ1->z_zapas -= 5; // zmniejsz poziom zapasu matce
      DodajZwierz(mapNewX, mapNewY, ptrZ1->z_def);
      }
    }
} // ProcessZwierz

//---------------------------------------------------------------------------
//! Usuñ zw³oki z listy
void UsunMartwe(void)
{
  short x,y, poz;
  PUNKT_MAPY* ptrSrc=NULL;

  for (poz=ileZwierz-1; poz>=0; poz--)
    if (listaZwierz[poz].z_def == -1) // do usuniêcia
      {
      if (poz != (ileZwierz-1)) // to nie jest ostatni
        {
        memmove(listaZwierz+poz, listaZwierz+poz+1, sizeof(listaZwierz[0])*(ileZwierz-poz-1));

        // przenumeruj obiekty na mapie (o -1)
        short a;
        for (a=0; a<ileZwierz-poz-1; a++)
          {
          x = listaZwierz[poz+a].z_common.x;
          y = listaZwierz[poz+a].z_common.y;
          ptrSrc = PtrPunktMapy(x, y);
          ptrSrc->pm_zwierz.pm1_poz--; // przenumeruj o 1 w dó³
          }
        }
      ileZwierz--;
      martwe++;
      }

} // UsunMartwe

//---------------------------------------------------------------------------
//! Przetwarzanie obiektow o 1 jednostkê czasu
void PrzetworzMape(void)
{
  short poz, max;

  ileGen++;
  // przegl¹danie kolejnych obiektów jest szybsze ni¿ przegl¹danie wg mapy
  // nie tracimy czasu na puste komórki

  max = ileRoslin; // ustal iloœæ przed - nowo dodane nie bêd¹ uwzglêdnione
  for (poz=0; poz<max; poz++)
    ProcessRoslina(poz);

  // zwierzêta
  max = ileZwierz; // ustal iloœæ przed - nowo dodane nie bêd¹ uwzglêdnione
  for (poz=0; poz<max; poz++)
    ProcessZwierz(poz);

  // usuñ martwe z listy
  UsunMartwe();

} // PrzetworzMape

//@} weze_przetwarzanie




//   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//!  @name weze_output
//!  Drukowanie aktualnego stanu mapy
//@{ @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static HANDLE hStdOut;

// wyœwietlanie zajmuje du¿o wiêcej czasu ni¿ generowananie, wiêc mo¿na
// pokazywaæ co któr¹œ klatkê - wg zmiennej czestoPokaz
static short czestoPokaz=1; //!< Co któr¹ klatkê pokazywaæ?
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
  char  znak[2]= "_";
  short poz;
  PUNKT_MAPY* ptr;
  ptr = PtrPunktMapy(x,y);

  // grunt
  poz = ptr->pm_grunt.pm1_poz;
  if (ptr->pm_grunt.pm1_tab != 0)
    {
    SetTextColor(FOREGROUND_RED | FOREGROUND_GREEN);
    strcpy(znak, ".");
    }

  // roœliny
  if (CzyMapaRoslina(x,y)) // jest trawa w tabeli
    {
    poz = GetPozRoslina(x,y);
    znak[0] = '0' + listaRoslin[poz].r_poziom;
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
    znak[0] = 'A' + listaZwierz[poz].z_zapas -1;
    if (listaZwierz[poz].z_def == 0)
      SetTextColor(FOREGROUND_BLUE);
    else // 2
      SetTextColor(FOREGROUND_RED);
    }
  PutZnak(znak);
} // DrukujZnakMapy

//---------------------------------------------------------------------------
//! Wyrzuæ stan mapy na standard output
void DrukujMape(void)
{
  short x, y;
  short a, wazA, wazB;
  wazA = 0;
  wazB = 0;
  for (a=0; a<ileZwierz;a++)
    {
    OBIEKTINFO_ZWIERZ* ptrZ1 = listaZwierz+a;
    if (ptrZ1->z_def == 0)
      wazA++;
    else
      wazB++;
    }
  COORD coord = {0,0};
  SetConsoleCursorPosition(hStdOut, coord);
  printf("MAPA %ux%u: %lu (+%u)      \n", xSize, ySize, ileGen, czestoPokaz);
  printf("weze: %u %u   \n", wazA, wazB);
  printf("dead: %lu   \n", martwe);
  for (y=0; y<ySize; y++)
    {
    for (x=0; x<xSize; x++)
      DrukujZnakMapy(x,y);
    printf("\n"); // koniec linii
    }
  printf("\n");
  SetTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
  printf("Wcisnij SPACJE, 1,2,3,4,5 q(koniec)");
  printf("\n");  
} // DrukujMape

/*
Parametry wywo³ania:
1) xSize
2) ySize
3) czestoPokaz
*/

//---------------------------------------------------------------------------
//! G³ówne wejœcie do programu
int main(int argc, char* argv[])
{
  short a, znak = ' ';
  if (argc >= 3)
    {
    a = atoi(argv[1]);
    if (a>3)
      xSize = a;
    a = atoi(argv[2]);
    if (a >3)
      ySize = a;
    }
  if (argc==4)
    {
    a = atoi(argv[3]);
    if (a>1)
      czestoPokaz = a;
    }
  mapa = new PUNKT_MAPY[xSize * ySize];
  memset(mapa, 0, sizeof(PUNKT_MAPY)*xSize*ySize);
  listaGrunt  = new OBIEKTINFO_GRUNT[xSize * ySize +1];
  listaRoslin = new OBIEKTINFO_ROSLINA[xSize * ySize +1];
  listaZwierz = new OBIEKTINFO_ZWIERZ[xSize * ySize +1];

  clrscr();
  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  SetTextColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
  ileGen = 0;
  PusteTabele();
  ZapelnijMape();
  DrukujMape();
  znak = getch(); // praca krokowa

  _next:
  PrzetworzMape();
  if ((ileGen % czestoPokaz)==0)
    {
    DrukujMape();
    znak = getch(); // praca krokowa
    }
  SetTextColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
  if (znak == '1')
    czestoPokaz = 1;
  if (znak == '2')
    czestoPokaz = 10;
  if (znak == '3')
    czestoPokaz = 100;
  if (znak == '4')
    czestoPokaz = 1000;
  if (znak == '5')
    czestoPokaz = 10000;

  if (znak == 'q')
    {
    delete[] mapa;
    delete[] listaGrunt;
    delete[] listaRoslin;
    delete[] listaZwierz;
    return 0;
    }
  goto _next;

// return 0;
} // main

//@} weze_output

// eof: weze.cpp

